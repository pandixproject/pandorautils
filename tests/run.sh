#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT HUP INT TERM
printf 'b\na\na\n' | "$root/bin/psort" -u > "$tmp/sorted"
printf 'a\nb\n' > "$tmp/expected"
cmp "$tmp/expected" "$tmp/sorted"
printf 'x\nx\ny\n' | "$root/bin/puniq" -c > "$tmp/counts"
printf '      2 x\n      1 y\n' > "$tmp/expected"
cmp "$tmp/expected" "$tmp/counts"
mkdir "$tmp/dir"
"$root/bin/pmkdir" -p "$tmp/dir/a/b"
"$root/bin/ptouch" "$tmp/dir/a/b/file"
"$root/bin/pchmod" u+x "$tmp/dir/a/b/file"
test -x "$tmp/dir/a/b/file"
"$root/bin/plink" -s file "$tmp/dir/a/b/link"
test "$("$root/bin/preadlink" "$tmp/dir/a/b/link")" = file
printf 'hello\n' | "$root/bin/ptee" "$tmp/tee" > "$tmp/out"
cmp "$tmp/out" "$tmp/tee"
test "$("$root/bin/pbasename" /one/two.txt .txt)" = two
test "$("$root/bin/pdirname" /one/two)" = /one
printf 'abc' | "$root/bin/ptail" -c 2 > "$tmp/tail"
test "$(cat "$tmp/tail")" = bc
