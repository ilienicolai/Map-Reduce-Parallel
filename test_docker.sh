#!/bin/bash

for i in {1..500}
do
    echo "Running test $i"
    docker exec -w /apd/checker -it apd_container /apd/checker/checker.sh | grep "Total"
done