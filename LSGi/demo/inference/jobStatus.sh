#!/bin/bash
#search for inference jobs running on the connecting nodes
#author: tere
#january 2016

echo "Running inference jobs by Node"
echo "----------------------------------"
#ssh node04 "top -c -n 2"

file="hosts"
for host in `cat $file`
    do
    ssh $host "hostname; pgrep -l gibbs|wc -l;"
    done
echo "done";


