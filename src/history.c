#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shellcraft.h"

#define HISTORY_LIMIT 100
static char *items[HISTORY_LIMIT];
static size_t used = 0;

void history_add(const char *line) {
    if (!line || !*line) return;
    if (used == HISTORY_LIMIT) {
        free(items[0]);
        memmove(items, items + 1, (HISTORY_LIMIT - 1) * sizeof(items[0]));
        used--;
    }
    items[used] = malloc(strlen(line) + 1);
    if (items[used]) { strcpy(items[used], line); used++; }
}
void history_print(void) {
    for (size_t i = 0; i < used; i++) printf("%zu  %s\n", i + 1, items[i]);
}
