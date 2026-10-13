#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shellcraft.h"

char *expand_word(const char *word) {
    if (!word) return NULL;
    size_t cap = strlen(word) + 64, n = 0;
    char *out = malloc(cap);
    if (!out) return NULL;
    for (size_t i = 0; word[i];) {
        const char *value = NULL;
        char name[256];
        size_t used = 0;
        if (word[i] == '~' && i == 0 && (word[i+1] == '/' || word[i+1] == '\0')) {
            value = getenv("HOME");
            i++;
        } else if (word[i] == '$') {
            size_t j = i + 1;
            if (word[j] == '?') {
                snprintf(name, sizeof(name), "%d", 0);
                value = name;
                i = j + 1;
            } else {
                size_t k = 0;
                if (word[j] == '{') {
                    j++;
                    while (word[j] && word[j] != '}' && k < sizeof(name)-1) name[k++] = word[j++];
                    if (word[j] == '}') j++;
                } else {
                    while ((isalnum((unsigned char)word[j]) || word[j] == '_') && k < sizeof(name)-1)
                        name[k++] = word[j++];
                }
                if (k == 0) { i++; if (n + 1 >= cap) goto grow; out[n++] = '$'; continue; }
                name[k] = '\0';
                value = getenv(name);
                i = j;
            }
        } else {
            if (n + 2 >= cap) goto grow;
            out[n++] = word[i++];
            continue;
        }
        if (value) {
            used = strlen(value);
            while (n + used + 1 >= cap) goto grow;
            memcpy(out + n, value, used);
            n += used;
        }
        continue;
grow:
        cap *= 2;
        char *tmp = realloc(out, cap);
        if (!tmp) { free(out); return NULL; }
        out = tmp;
    }
    out[n] = '\0';
    return out;
}
