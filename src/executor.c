#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "shellcraft.h"

extern int register_job(pid_t pgid, pid_t *pids, int count, const char *command);

static int redirect_command(Command *c, int in_fd, int out_fd) {
    if (in_fd != STDIN_FILENO && dup2(in_fd, STDIN_FILENO) < 0) { perror("dup2"); return -1; }
    if (out_fd != STDOUT_FILENO && dup2(out_fd, STDOUT_FILENO) < 0) { perror("dup2"); return -1; }
    if (c->input_file) {
        int fd = open(c->input_file, O_RDONLY);
        if (fd < 0) { perror(c->input_file); return -1; }
        if (dup2(fd, STDIN_FILENO) < 0) { perror("dup2"); close(fd); return -1; }
        close(fd);
    }
    if (c->output_file) {
        int flags = O_WRONLY | O_CREAT | (c->append_output ? O_APPEND : O_TRUNC);
        int fd = open(c->output_file, flags, 0666);
        if (fd < 0) { perror(c->output_file); return -1; }
        if (dup2(fd, STDOUT_FILENO) < 0) { perror("dup2"); close(fd); return -1; }
        close(fd);
    }
    if (c->stderr_to_stdout && dup2(STDOUT_FILENO, STDERR_FILENO) < 0) { perror("dup2"); return -1; }
    return 0;
}

int execute_pipeline(Pipeline *p, const char *line) {
    if (!p || p->count <= 0) return 0;
    if (p->count == 1 && is_builtin(p->commands[0].argv[0]) && !p->background) {
        int ignored_exit = 0;
        /* Parent-run builtins are handled in main; a defensive fallback. */
        return run_builtin(&p->commands[0], &ignored_exit);
    }

    int prev_read = -1;
    pid_t *pids = calloc((size_t)p->count, sizeof(*pids));
    if (!pids) { perror("calloc"); return -1; }
    pid_t pgid = 0;
    int spawned = 0;

    for (int i = 0; i < p->count; i++) {
        int pipefd[2] = {-1, -1};
        if (i < p->count - 1 && pipe(pipefd) < 0) { perror("pipe"); goto fail; }
        pid_t pid = fork();
        if (pid < 0) { perror("fork"); if (pipefd[0] >= 0) close(pipefd[0]); if (pipefd[1] >= 0) close(pipefd[1]); goto fail; }
        if (pid == 0) {
            signal(SIGINT, SIG_DFL); signal(SIGQUIT, SIG_DFL); signal(SIGTSTP, SIG_DFL);
            if (pgid == 0) setpgid(0, 0); else setpgid(0, pgid);
            int out = pipefd[1] >= 0 ? pipefd[1] : STDOUT_FILENO;
            if (redirect_command(&p->commands[i], prev_read >= 0 ? prev_read : STDIN_FILENO, out) < 0) _exit(1);
            if (prev_read >= 0) close(prev_read);
            if (pipefd[0] >= 0) close(pipefd[0]);
            if (pipefd[1] >= 0) close(pipefd[1]);
            for (int a = 0; a < p->commands[i].argc; a++) {
                char *expanded = expand_word(p->commands[i].argv[a]);
                if (!expanded) { perror("expand"); _exit(1); }
                free(p->commands[i].argv[a]); p->commands[i].argv[a] = expanded;
            }
            execvp(p->commands[i].argv[0], p->commands[i].argv);
            perror(p->commands[i].argv[0]);
            _exit(127);
        }
        if (pgid == 0) pgid = pid;
        setpgid(pid, pgid);
        pids[spawned++] = pid;
        if (prev_read >= 0) close(prev_read);
        if (pipefd[1] >= 0) close(pipefd[1]);
        prev_read = pipefd[0];
    }
    if (prev_read >= 0) close(prev_read);
    if (p->background) {
        register_job(pgid, pids, spawned, line);
        return 0;
    }
    int last_status = 0;
    for (int i = 0; i < spawned; i++) {
        int status;
        if (waitpid(pids[i], &status, 0) >= 0 && i == spawned - 1) {
            if (WIFEXITED(status)) last_status = WEXITSTATUS(status);
            else if (WIFSIGNALED(status)) last_status = 128 + WTERMSIG(status);
        }
    }
    free(pids);
    return last_status;
fail:
    if (prev_read >= 0) close(prev_read);
    for (int i = 0; i < spawned; i++) waitpid(pids[i], NULL, 0);
    free(pids);
    return -1;
}
