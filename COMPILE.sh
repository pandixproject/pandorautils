#!/bin/bash
mkdir -p bin
for f in src/*.c; do
    name=$(basename "$f" .c)
    gcc -O2 -Wall -o "bin/$name" "$f"
done
echo "Done, binaries are avaidable in bin/"
