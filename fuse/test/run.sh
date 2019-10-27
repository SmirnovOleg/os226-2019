#!/bin/bash

. ../test/lib.sh

make

mkdir -p test/dstdir

./main test/dstdir --srcdir=test/srcdir

diff -aur test/dstdir test/golddir
e=$?

fusermount -u test/dstdir

exit $e
