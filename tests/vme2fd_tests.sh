#!/bin/bash

TEST_STRING="Put that in your pipe and smoke it\nNewline"

mkfifo /tmp/$USER/in_stream
mkfifo /tmp/$USER/out_stream

./bin/vme2fd /tmp/$USER/in_stream /tmp/$USER/out_stream &

echo -e $TEST_STRING > test.in
cat test.in > /tmp/$USER/in_stream

cat < /tmp/$USER/out_stream > test.out

DIFF=`diff test.in test.out`

if [ "$DIFF" == "" ]
then
    rm /tmp/$USER/in_stream /tmp/$USER/out_stream test.in test.out
    exit 0
else
    echo "vme2fd_test"
    echo "----------"
    cat test.in
    cat test.out
    rm /tmp/$USER/in_stream /tmp/$USER/out_stream test.in test.out
    echo ""
    exit 1
fi
