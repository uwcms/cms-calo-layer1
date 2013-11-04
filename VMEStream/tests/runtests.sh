echo "------------------"
echo "Running Unit Tests"
echo "------------------"

for i in tests/*_tests #tests/*_tests.sh
do
    if [ -f $i ]
    then
        if $VALGRIND ./$i #>> tests/tests.log
        then
            echo $i PASS
        else
            echo "ERROR in tests $i: see tests/tests.log"
            echo "-----"
            #tail tests/tests.log
            exit 1
        fi
    fi
done
