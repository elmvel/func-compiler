#!/bin/sh

set -e

binary="fc"
run=0

if ! [ -z "$1" ]
then
    binary="$1"
    shift
    run=1
fi

cmake --build build/
if [ "$run" -eq 1 ]
then
    ./build/"$binary" "$@"
fi
