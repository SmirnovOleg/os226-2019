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
			$(NF-1) == "reference" { $NF = $NF - t ; print ; next }
			{ print }
		'
}

testsdo test check ./main
