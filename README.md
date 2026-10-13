# Shellcraft

Shellcraft is a small Unix-style command shell written in C for an operating-systems project.

## Features

- Interactive prompt showing the current working directory
- External command execution using `fork()` and `execvp()`
- Pipelines using `|`
- Input redirection `<`
- Output redirection `>` and append redirection `>>`
- Standard error redirection `2>&1`
- Built-ins: `cd`, `exit`, `export`, `unset`, `history`, `jobs`, `fg`, `bg`, `help`
- Environment-variable expansion: `$NAME` and `${NAME}`
- Home-directory expansion: `~`
- Basic background jobs with `&`
- Bounded in-memory command history
- Makefile and smoke tests

## Build

On Ubuntu or WSL:

```sh
sudo apt update
sudo apt install build-essential
make
```

## Run

```sh
./shellcraft
```

Example commands:

```sh
pwd
ls -la | wc -l
export USER_TAG=student
echo $USER_TAG
echo first > demo.txt
echo second >> demo.txt
cat < demo.txt
sleep 10 &
jobs
history
exit
```

## Test

```sh
make test
```

## Limitations

This is an educational shell, not a full POSIX shell. It does not implement command substitution, globbing, here-documents, shell scripting grammar, persistent history, terminal foreground process-group handoff, or all edge cases of job control. `fg` and `bg` provide basic process-group continuation and should be tested in an interactive terminal. Quote handling is intentionally minimal. Variable expansion occurs after tokenization, so advanced quoting semantics differ from Bash.
