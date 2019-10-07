
set -o pipefail

cmpout() {
	local base=$1
	shift 1

	"$@" < $base.in | diff -u $base.out -
}

testsdo() {
	local base=$1
	local cmp=$2
	shift 2

	local ret=0
	for i in $base/*.in; do
		$cmp ${i%%.in} "$@" || ret=1
	done

	return $ret
}

testouts() {
	local base=$1
	shift

	testsdo $base cmpout "$@"
}
