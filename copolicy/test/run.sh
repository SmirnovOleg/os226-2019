#!/bin/bash

. ../test/lib.sh

make || exit 1

testouts test ./main
