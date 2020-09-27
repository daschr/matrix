#!/bin/bash

[ -e "./bin2c" ] || cc -O3  -Wall -pedantic -o bin2c bin2c.c || exit 1

out=src/files.c
files=(misc/cert.pem misc/html/index.html misc/html/redirect.html)

echo "" > "$out"
for f in "${files[@]}"; do
	echo "embed \"$f\""
	./bin2c "$f" >> "$out"
done 
