#!/bin/bash

mkdir -p /tmp/$USER
rm -f /tmp/$USER/in_stream /tmp/$USER/out_stream
mkfifo /tmp/$USER/in_stream
mkfifo /tmp/$USER/out_stream

#export LD_LIBRARY_PATH=/opt/xdaq/lib:$LD_LIBRARY_PATH
export VME_CONTROLLER=CAEN # TESTECHO
echo "Transmitting dictionary"
#cat /usr/share/dict/words > /tmp/$USER/in_stream &
cat /tmp/tapas/dict_words > /tmp/$USER/in_stream &
INPUT_PID=$!

echo "Reading output"
cat < /tmp/$USER/out_stream > test.out &
OUTPUT_PID=$!

echo "Spawning vme2fd"
./bin/vme2fd /tmp/$USER/in_stream /tmp/$USER/out_stream &
VME2FD_PID=$!

sleep 2
echo "Waiting for completion of input stream"
wait $INPUT_PID

echo "...done. Sleeping for a second to let things get buffered out"
sleep 5

echo "Shutting down processes"
## Shutdown processes cleanly
kill $VME2FD_PID
kill $OUTPUT_PID

DIFF=`diff /usr/share/dict/words test.out`

RESULT=2

if [ "$DIFF" == "" ]
then
    rm /tmp/$USER/in_stream /tmp/$USER/out_stream test.out
    RESULT=0
else
    echo "vme2fd_test failed"
    #diff /usr/share/dict/words test.out
    rm /tmp/$USER/in_stream /tmp/$USER/out_stream 
    echo ""
    RESULT=1
fi

exit $RESULT
