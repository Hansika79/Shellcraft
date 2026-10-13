#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "shellcraft.h"

static char *dup_range(const char *s, size_t n) {
    char *p = malloc(n + 1);
    if (!p) return NULL;
    memcpy(p, s, n);
    p[n] = '\0';
    return p;
}

static int push(Token **arr, size_t *n, TokenType type, const char *s, size_t len) {
    Token *tmp = realloc(*arr, (*n + 1) * sizeof(**arr));
    if (!tmp) return -1;
    *arr = tmp;
    (*arr)[*n].type = type;
    (*arr)[*n].text = dup_range(s, len);
    if (!(*arr)[*n].text) return -1;
    (*n)++;
    return 0;
}

int tokenize(const char *line, Token **out, size_t *count) {
    Token *tokens = NULL;
    size_t n = 0, i = 0, len = strlen(line);
    char *word = malloc(len + 1);
    size_t w = 0;
    int quote = 0, escaped = 0;
    if (!word) return -1;

    while (i < len) {
        char c = line[i];
        if (escaped) { word[w++] = c; escaped = 0; i++; continue; }
        if (c == '\\' && quote != '\'') { escaped = 1; i++; continue; }
        if (quote) {
            if (c == quote) quote = 0;
            else word[w++] = c;
            i++;
            continue;
        }
        if (c == '\'' || c == '"') { quote = c; i++; continue; }
        if (isspace((unsigned char)c)) {
            if (w) { if (push(&tokens, &n, TOK_WORD, word, w)) goto fail; w = 0; }
            i++; continue;
        }
        if (c == '2' && i + 3 < len && strncmp(line + i, "2>&1", 4) == 0) {
            if (w) { if (push(&tokens, &n, TOK_WORD, word, w)) goto fail; w = 0; }
            if (push(&tokens, &n, TOK_ERR_TO_OUT, "2>&1", 4)) goto fail;
            i += 4; continue;
        }
        if (c == '|' || c == '<' || c == '>' || c == '&') {
            if (w) { if (push(&tokens, &n, TOK_WORD, word, w)) goto fail; w = 0; }
            if (c == '>' && i + 1 < len && line[i+1] == '>') {
                if (push(&tokens, &n, TOK_APPEND, ">>", 2)) goto fail;
                i += 2; continue;
            }
            TokenType t = c == '|' ? TOK_PIPE : c == '<' ? TOK_IN :
                          c == '>' ? TOK_OUT : TOK_BACKGROUND;
            if (push(&tokens, &n, t, line + i, 1)) goto fail;
            i++; continue;
        }
        word[w++] = c;
        i++;
    }
    if (escaped) word[w++] = '\\';
    if (quote) goto fail;
    if (w && push(&tokens, &n, TOK_WORD, word, w)) goto fail;
    free(word);
    *out = tokens; *count = n;
    return 0;
fail:
    free(word);
    free_tokens(tokens, n);
    return -1;
}

void free_tokens(Token *tokens, size_t count) {
    for (size_t i = 0; i < count; i++) free(tokens[i].text);
    free(tokens);
}
