#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT HUP INT TERM
printf 'b\na\na\n' | "$root/bin/sort" -u > "$tmp/sorted"
printf 'a\nb\n' > "$tmp/expected"
cmp "$tmp/expected" "$tmp/sorted"
printf 'x\nx\ny\n' | "$root/bin/uniq" -c > "$tmp/counts"
printf '      2 x\n      1 y\n' > "$tmp/expected"
cmp "$tmp/expected" "$tmp/counts"
mkdir "$tmp/dir"
"$root/bin/mkdir" -p "$tmp/dir/a/b"
"$root/bin/touch" "$tmp/dir/a/b/file"
"$root/bin/chmod" u+x "$tmp/dir/a/b/file"
test -x "$tmp/dir/a/b/file"
"$root/bin/link" -s file "$tmp/dir/a/b/link"
test "$("$root/bin/readlink" "$tmp/dir/a/b/link")" = file
printf 'hello\n' | "$root/bin/tee" "$tmp/tee" > "$tmp/out"
cmp "$tmp/out" "$tmp/tee"
test "$("$root/bin/basename" /one/two.txt .txt)" = two
test "$("$root/bin/dirname" /one/two)" = /one
printf 'abc' | "$root/bin/tail" -c 2 > "$tmp/tail"
test "$(cat "$tmp/tail")" = bc
