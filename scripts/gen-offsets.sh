#/bin/sh

[ -z "$1" ] && exit 1
[ -z "$2" ] && exit 2
[ -z "$(command -v $CC)" ] && exit 3

TMP=$(mktemp /tmp/XXXXXX.s)
$CC -Iinc -S $1 -o $TMP
grep "#define" $TMP | sed -e "s/^\s*//" > $2
rm $TMP
