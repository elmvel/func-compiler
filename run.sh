#!/bin/sh

binary="fc"

if ! [ -z "$1" ]
then
    binary="$1"
fi

set -xe

cmake --build build/
./build/$binary
