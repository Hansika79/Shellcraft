#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "shellcraft.h"

#define INPUT_SIZE 4096


static void prompt(void) {
    printf("shellcraft$ ");
    fflush(stdout);
}

int main(void) {
    char line[INPUT_SIZE];
    int should_exit = 0;
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    while (!should_exit) {
        jobs_reap();
        prompt();
        if (!fgets(line, sizeof(line), stdin)) {
            if (feof(stdin)) break;
            if (errno == EINTR) { clearerr(stdin); putchar('\n'); continue; }
            perror("fgets"); break;
        }
        line[strcspn(line, "\n")] = '\0';
        if (!*line) continue;
        history_add(line);

        Token *tokens = NULL;
        size_t count = 0;
        if (tokenize(line, &tokens, &count) != 0) {
            fprintf(stderr, "shellcraft: invalid quoting or out of memory\n");
            continue;
        }
        if (count == 0) { free_tokens(tokens, count); continue; }
        Pipeline pipeline;
        if (parse_tokens(tokens, count, &pipeline) != 0) {
            fprintf(stderr, "shellcraft: syntax error\n");
            free_tokens(tokens, count);
            continue;
        }
        /* Expand arguments before built-ins so export/cd accept variables. */
        for (int c = 0; c < pipeline.count; c++) {
            for (int a = 0; a < pipeline.commands[c].argc; a++) {
                char *expanded = expand_word(pipeline.commands[c].argv[a]);
                if (expanded) { free(pipeline.commands[c].argv[a]); pipeline.commands[c].argv[a] = expanded; }
            }
            if (pipeline.commands[c].input_file) {
                char *x = expand_word(pipeline.commands[c].input_file);
                if (x) { free(pipeline.commands[c].input_file); pipeline.commands[c].input_file = x; }
            }
            if (pipeline.commands[c].output_file) {
                char *x = expand_word(pipeline.commands[c].output_file);
                if (x) { free(pipeline.commands[c].output_file); pipeline.commands[c].output_file = x; }
            }
        }
        if (pipeline.count == 1 && !pipeline.background &&
            is_builtin(pipeline.commands[0].argv[0])) {
            run_builtin(&pipeline.commands[0], &should_exit);
        } else {
            execute_pipeline(&pipeline, line);
        }
        free_pipeline(&pipeline);
        free_tokens(tokens, count);
    }
    jobs_reap();
    jobs_cleanup();
    putchar('\n');
    return 0;
}
