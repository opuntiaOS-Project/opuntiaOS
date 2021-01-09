#!/bin/bash
if [ "$1" != "${ONEOS_TARGET}" ];
then
    export ONEOS_TARGET="$1"
    make clean
fi
