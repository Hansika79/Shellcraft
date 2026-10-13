#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "shellcraft.h"

#define MAX_JOBS 64
static Job jobs[MAX_JOBS];
static int next_id = 1;

static Job *find_job(int id) {
    for (int i = 0; i < MAX_JOBS; i++)
        if (jobs[i].running && jobs[i].job_id == id) return &jobs[i];
    return NULL;
}
static void clear_job(Job *j) {
    free(j->pids); free(j->command); memset(j, 0, sizeof(*j));
}
int register_job(pid_t pgid, pid_t *pids, int count, const char *command) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (!jobs[i].running) {
            Job *j = &jobs[i];
            j->pgid = pgid; j->pids = pids; j->count = count;
            j->remaining = count; j->job_id = next_id++;
            j->command = strdup(command ? command : "");
            j->running = 1;
            printf("[%d] %d\n", j->job_id, (int)pgid);
            return j->job_id;
        }
    }
    free(pids);
    fprintf(stderr, "shellcraft: job table full\n");
    return -1;
}
void jobs_reap(void) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
        for (int i = 0; i < MAX_JOBS; i++) {
            Job *j = &jobs[i];
            if (!j->running) continue;
            for (int k = 0; k < j->count; k++) {
                if (j->pids[k] == pid) {
                    if (WIFSTOPPED(status)) j->stopped = 1;
                    else if (WIFCONTINUED(status)) j->stopped = 0;
                    else if (WIFEXITED(status) || WIFSIGNALED(status)) {
                        j->pids[k] = -1;
                        if (j->remaining > 0) j->remaining--;
                    }
                    if (j->remaining == 0) {
                        fprintf(stderr, "[%d] Done %s\n", j->job_id, j->command);
                        clear_job(j);
                    }
                    break;
                }
            }
        }
    }
}
void jobs_print(void) {
    jobs_reap();
    for (int i = 0; i < MAX_JOBS; i++)
        if (jobs[i].running)
            printf("[%d] %s %s\n", jobs[i].job_id,
                   jobs[i].stopped ? "Stopped" : "Running", jobs[i].command);
}
int job_foreground(int id) {
    Job *j = id ? find_job(id) : NULL;
    if (!j) {
        for (int i = MAX_JOBS - 1; i >= 0; i--) if (jobs[i].running) { j = &jobs[i]; break; }
    }
    if (!j) return 0;
    j->stopped = 0;
    kill(-j->pgid, SIGCONT);
    int status;
    while (waitpid(-j->pgid, &status, WUNTRACED) > 0) {
        if (WIFSTOPPED(status)) { j->stopped = 1; return 1; }
        if (errno == ECHILD) break;
    }
    clear_job(j);
    return 1;
}
int job_background(int id) {
    Job *j = id ? find_job(id) : NULL;
    if (!j) {
        for (int i = MAX_JOBS - 1; i >= 0; i--) if (jobs[i].running && jobs[i].stopped) { j = &jobs[i]; break; }
    }
    if (!j) return 0;
    if (kill(-j->pgid, SIGCONT) != 0) return 0;
    j->stopped = 0;
    printf("[%d] %s &\n", j->job_id, j->command);
    return 1;
}
void jobs_cleanup(void) {
    for (int i = 0; i < MAX_JOBS; i++) if (jobs[i].running) clear_job(&jobs[i]);
}
