#ifndef SHELLCRAFT_H
#define SHELLCRAFT_H

#include <stddef.h>
#include <sys/types.h>

typedef enum {
    TOK_WORD, TOK_PIPE, TOK_IN, TOK_OUT, TOK_APPEND, TOK_ERR_TO_OUT,
    TOK_BACKGROUND
} TokenType;

typedef struct {
    TokenType type;
    char *text;
} Token;

typedef struct {
    char **argv;
    int argc;
    char *input_file;
    char *output_file;
    int append_output;
    int stderr_to_stdout;
} Command;

typedef struct {
    Command *commands;
    int count;
    int background;
} Pipeline;

typedef struct {
    pid_t pgid;
    pid_t *pids;
    int count;
    int remaining;
    int job_id;
    char *command;
    int running;
    int stopped;
} Job;

int tokenize(const char *line, Token **out, size_t *count);
void free_tokens(Token *tokens, size_t count);
int parse_tokens(Token *tokens, size_t count, Pipeline *out);
void free_pipeline(Pipeline *pipeline);
char *expand_word(const char *word);
int is_builtin(const char *name);
int run_builtin(Command *cmd, int *should_exit);
int execute_pipeline(Pipeline *pipeline, const char *line);
void history_add(const char *line);
void history_print(void);
void jobs_print(void);
void jobs_reap(void);
int job_foreground(int job_id);
int job_background(int job_id);
void jobs_cleanup(void);

#endif
