#!/bin/bash


#Dataset to run
dataset="../../data/inputGraphs/dns_graph.alchemy.factors.bin"
#Number of process to bind
N=$1

#running query service
echo "lauchng service for " $dataset on $N

if [ $N -gt 0 ]
then

        kill $(ps -ef | grep  Query | awk '{print $2'})

        ./QueryService $dataset $N
        echo  "Done query service launched"
fi
echo "Done!"
