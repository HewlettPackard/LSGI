#!/bin/bash
#Description:
# Launch processes in each node listed on hostfile
# Parameters: Number of process to launch (MAX)
# authors: tere,fei, krishna 
# December 14, 2015

echo "Large Scale  Graph Inference (LSGi)"
echo "Version 1.0-l4tm, @Hewlett Packard labs"
echo "----------------------------------------------------"

#
# - Variable definitions
#
curPath=$(pwd)
#Dataset to run
dataset="/mnt/lsgi/gV1000ME3392337264.alchemy.rnd.bin"
#dataset="../../data/inputGraphs/dns_graph.alchemy.factors.bin"
#dataset="../../data/inputGraphs/dns.g5"
#Configuration file to use
configfile="../../data/config/configure_g1b"

#configfile="../../data/config/configure_gdns"
#File system to use: /lfs/ or /dev/shm
SharedFileSystem="/lfs/"
#node availalbles
file="hosts"

#user parameter: number of process to launch
N=$1

#
# - Input Params Checking 
#
#number of process
if test -z "$N" 
then
    echo "Error, Usage: ./launch..sh <number of process to launch>"
    exit
fi
#number of process
if  [ $N -lt 1 ]
then
   echo "Error, Usage: Number of process should be more than zero"   
   exit
fi

echo Running query service for [ $1 ] processes
echo "----------------------------------------------------"
echo $curPath
#running query service
#kill $(ps -ef | grep  Query | awk '{print $3'})
#sleep 1
./QueryService $dataset $N
echo  "Done query service launched"
echo "Done!"

