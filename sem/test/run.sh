#!/bin/bash

. ../test/lib.sh

make || exit 1

check() {
	local base=$1
	shift 1

	echo ${base}:

	awk '!/^#/' $base.in | \
		timeout 60 "$@" 2>&1 | awk '
			$5 != 1 { e = 1}
			{ print }
			END { exit e }'

	[  ${PIPESTATUS[1]} = 124 ] && [ ${PIPESTATUS[2]} = 0 ]
}

testsdo test check ./main
