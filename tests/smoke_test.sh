#!/bin/sh
set -eu
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
printf 'echo hello\nexit\n' | ./shellcraft > "$tmp/out"
grep -q 'hello' "$tmp/out"
printf 'echo one > %s/file\necho two >> %s/file\ncat < %s/file\nexit\n' "$tmp" "$tmp" "$tmp" | ./shellcraft > "$tmp/redir"
grep -q 'one' "$tmp/redir"
grep -q 'two' "$tmp/redir"
printf 'printf abc | wc -c\nexit\n' | ./shellcraft > "$tmp/pipe"
grep -q '3' "$tmp/pipe"
printf 'export SC_TEST=works\nprintenv SC_TEST\nexit\n' | ./shellcraft > "$tmp/env"
grep -q 'works' "$tmp/env"
echo "Smoke tests passed."
