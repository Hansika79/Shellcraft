CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -D_POSIX_C_SOURCE=200809L -Iinclude
TARGET = shellcraft
SOURCES = src/main.c src/lexer.c src/parser.c src/expand.c src/builtin.c src/history.c src/jobs.c src/executor.c

.PHONY: all clean test
all: $(TARGET)

$(TARGET): $(SOURCES) include/shellcraft.h
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

test: $(TARGET)
	sh tests/smoke_test.sh

clean:
	rm -f $(TARGET)
