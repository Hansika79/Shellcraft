# Shellcraft Design

## Architecture

1. **REPL (`main.c`)** reads a command line and displays the current working directory.
2. **Lexer (`lexer.c`)** converts text into words and operators.
3. **Parser (`parser.c`)** groups words into commands and commands into a pipeline.
4. **Expansion (`expand.c`)** expands environment variables and `~`.
5. **Built-ins (`builtin.c`)** handles commands that must affect the shell process itself.
6. **Executor (`executor.c`)** creates pipes, forks child processes, redirects file descriptors, and executes external programs.
7. **History (`history.c`)** stores the latest 100 entered commands in memory.
8. **Jobs (`jobs.c`)** tracks basic background process groups.

## Execution flow

```text
Input -> Lexer -> Parser -> Expansion -> Built-in or Executor
                                      Executor -> fork/pipe/dup2/execvp/waitpid
```

## Main system calls

- `fork()` creates child processes.
- `execvp()` launches external programs using `PATH`.
- `waitpid()` waits for child processes and reaps background processes.
- `pipe()` connects the output of one process to the input of another.
- `dup2()` connects pipes and files to standard input/output/error.
- `open()` opens redirected files.
- `chdir()` changes the shell's working directory.
- `setenv()` and `unsetenv()` modify the shell environment.

## Validation

Use `make test` for automated smoke tests. Also manually test `cd`, pipelines, redirection, variables, background execution, and Ctrl+C behavior.
