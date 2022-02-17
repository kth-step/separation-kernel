#/bin/bash

TMP=$(mktemp)

$CC -Iinclude -S offsets.c -o $TMP
grep "#define" $TMP | sed -e "s/^\s*//" > offsets.h

rm $TMP
