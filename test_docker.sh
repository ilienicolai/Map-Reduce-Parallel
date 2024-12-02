#!/bin/bash

for i in {1..100}
do
    echo "Running test $i"
    ./run_with_docker.sh | grep "Total"
done