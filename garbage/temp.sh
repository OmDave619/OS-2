#!/bin/bash

# Number of times to run the script
n=12 # For example, change this to however many times you want to run the script

# Check if the number of runs is passed as an argument
if [ "$#" -eq 1 ]; then
  n=$1
fi

for ((i=1; i<=n; i++))
do
    echo "Execution $i of $n"
    ./script2.sh
done

echo "Completed $n executions."
