#!/bin/bash

. ../test/lib.sh

make || exit 1

check() {
	local base=$1
	shift 1

	echo ${base}:

	awk '!/^#/' $base.in | \
	"$@" | \
		awk '
			$1 == "reftime" { t = $2 ; next }
			$8 == "reference" { $9 = $9 - t ; print ; next }
			{ print }
		'
}

testsdo test check ./main
