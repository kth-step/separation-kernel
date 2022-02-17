#/bin/bash

TMP=$(mktemp)

$CC -Iinc -S offsets.c -o $TMP
grep "#define" $TMP | sed -e "s/^\s*//" > inc/offsets.h

rm $TMP
