#!/bin/bash

for i in {1..100}
do
    echo "Running test $i"
    ./checker.sh | grep "Total"
done