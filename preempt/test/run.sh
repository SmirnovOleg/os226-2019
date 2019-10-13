#!/bin/bash

. ../test/lib.sh

make || exit 1

check() {
	local base=$1
	shift 1

	echo ${base}:

	awk '!/^#/' $base.in | \
		timeout 10 "$@"
	[ $? == 124 ]
}

testsdo test check ./main
