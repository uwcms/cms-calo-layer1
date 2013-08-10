#!/bin/bash

TEST_STRING="Put that in your pipe and smoke it"

mkfifo in_stream
mkfifo out_stream

./bin/vme2fd in_stream out_stream &

echo -e $TEST_STRING > test.in
cat test.in > in_stream

cat < out_stream > test.out

DIFF=`diff test.in test.out`

if [ "$DIFF" == "" ]
then
    echo "Passed"
else
    echo "Failed"
    cat test.in
    cat test.out
fi

rm in_stream out_stream test.in test.out
