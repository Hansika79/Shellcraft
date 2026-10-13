#include <stdlib.h>
#include <string.h>
#include "shellcraft.h"

static char *copy_string(const char *s) {
    size_t n = strlen(s);
    char *p = malloc(n + 1);
    if (p) memcpy(p, s, n + 1);
    return p;
}

static void free_command(Command *c) {
    for (int i = 0; i < c->argc; i++) free(c->argv[i]);
    free(c->argv);
    free(c->input_file);
    free(c->output_file);
}

void free_pipeline(Pipeline *p) {
    if (!p) return;
    for (int i = 0; i < p->count; i++) free_command(&p->commands[i]);
    free(p->commands);
    memset(p, 0, sizeof(*p));
}

static int add_command(Pipeline *p) {
    Command *tmp = realloc(p->commands, (size_t)(p->count + 1) * sizeof(*tmp));
    if (!tmp) return -1;
    p->commands = tmp;
    memset(&p->commands[p->count], 0, sizeof(*tmp));
    p->count++;
    return 0;
}

static int add_arg(Command *c, const char *s) {
    char **tmp = realloc(c->argv, (size_t)(c->argc + 2) * sizeof(*tmp));
    if (!tmp) return -1;
    c->argv = tmp;
    c->argv[c->argc] = copy_string(s);
    if (!c->argv[c->argc]) return -1;
    c->argc++;
    c->argv[c->argc] = NULL;
    return 0;
}

int parse_tokens(Token *tokens, size_t count, Pipeline *out) {
    memset(out, 0, sizeof(*out));
    if (count == 0) return 0;
    if (add_command(out)) goto fail;

    for (size_t i = 0; i < count; i++) {
        Token *t = &tokens[i];
        Command *c = &out->commands[out->count - 1];
        if (t->type == TOK_WORD) {
            if (add_arg(c, t->text)) goto fail;
        } else if (t->type == TOK_PIPE) {
            if (c->argc == 0 || add_command(out)) goto fail;
        } else if (t->type == TOK_IN || t->type == TOK_OUT || t->type == TOK_APPEND) {
            if (i + 1 >= count || tokens[i+1].type != TOK_WORD) goto fail;
            char **target = t->type == TOK_IN ? &c->input_file : &c->output_file;
            free(*target);
            *target = copy_string(tokens[++i].text);
            if (!*target) goto fail;
            if (t->type == TOK_APPEND) c->append_output = 1;
            else if (t->type == TOK_OUT) c->append_output = 0;
        } else if (t->type == TOK_ERR_TO_OUT) {
            c->stderr_to_stdout = 1;
        } else if (t->type == TOK_BACKGROUND) {
            if (i != count - 1) goto fail;
            out->background = 1;
        }
    }
    if (out->count == 0 || out->commands[out->count - 1].argc == 0) goto fail;
    return 0;
fail:
    free_pipeline(out);
    return -1;
}
