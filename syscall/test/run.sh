#!/bin/bash

. ../test/lib.sh

make || exit 1

objdump -d main.o | grep 'mov\s*\(0x[0-9a-f]*\)\?('
SLOG=$(mktemp)

testouts test strace ./main 2>$SLOG

e=$?
grep '^---' $SLOG
rm $SLOG

exit $e
