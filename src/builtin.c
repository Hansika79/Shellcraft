#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "shellcraft.h"

int is_builtin(const char *s) {
    if (!s) return 0;
    return !strcmp(s,"cd") || !strcmp(s,"exit") || !strcmp(s,"export") ||
           !strcmp(s,"unset") || !strcmp(s,"history") || !strcmp(s,"jobs") ||
           !strcmp(s,"fg") || !strcmp(s,"bg") || !strcmp(s,"help");
}

int run_builtin(Command *c, int *should_exit) {
    char **a = c->argv;
    if (!a || !a[0]) return 0;
    if (!strcmp(a[0],"exit")) { *should_exit = 1; return 1; }
    if (!strcmp(a[0],"cd")) {
        const char *p = a[1] ? a[1] : getenv("HOME");
        if (!p) fprintf(stderr,"cd: HOME is not set\n");
        else if (chdir(p) != 0) perror("cd");
        return 1;
    }
    if (!strcmp(a[0],"export")) {
        if (!a[1]) { fprintf(stderr,"Usage: export NAME=VALUE\n"); return 1; }
        char *eq = strchr(a[1], '=');
        if (!eq || eq == a[1]) { fprintf(stderr,"Usage: export NAME=VALUE\n"); return 1; }
        size_t n = (size_t)(eq - a[1]);
        char *name = malloc(n + 1);
        if (!name) { perror("malloc"); return 1; }
        memcpy(name, a[1], n); name[n] = '\0';
        if (setenv(name, eq + 1, 1) != 0) perror("export");
        free(name); return 1;
    }
    if (!strcmp(a[0],"unset")) {
        if (!a[1]) fprintf(stderr,"Usage: unset NAME\n");
        else if (unsetenv(a[1]) != 0) perror("unset");
        return 1;
    }
    if (!strcmp(a[0],"history")) { history_print(); return 1; }
    if (!strcmp(a[0],"jobs")) { jobs_print(); return 1; }
    if (!strcmp(a[0],"fg")) {
        int id = a[1] ? atoi(a[1]) : 0;
        if (!job_foreground(id)) fprintf(stderr,"fg: job not found\n");
        return 1;
    }
    if (!strcmp(a[0],"bg")) {
        int id = a[1] ? atoi(a[1]) : 0;
        if (!job_background(id)) fprintf(stderr,"bg: job not found\n");
        return 1;
    }
    if (!strcmp(a[0],"help")) {
        puts("Shellcraft built-ins: cd exit export unset history jobs fg bg help");
        puts("Supports pipelines, <, >, >>, 2>&1, environment expansion, and &.");
        return 1;
    }
    return 0;
}
