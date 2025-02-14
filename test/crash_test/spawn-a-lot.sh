#!/bin/sh

for d in $(seq 1 $1)
do
    $DEBUG ./test/crash_test/write-a-lot.sh $2
done

exit 0