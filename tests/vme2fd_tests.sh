#!/bin/bash

TEST_STRING="Put that in your pipe and smoke it\nNewline"

mkfifo in_stream
mkfifo out_stream

./bin/vme2fd in_stream out_stream &

echo -e $TEST_STRING > test.in
cat test.in > in_stream

cat < out_stream > test.out

DIFF=`diff test.in test.out`

if [ "$DIFF" == "" ]
then
    rm in_stream out_stream test.in test.out
    exit 0
else
    echo "vme2fd_test"
    echo "----------"
    cat test.in
    cat test.out
    rm in_stream out_stream test.in test.out
    echo ""
    exit 1
fi
