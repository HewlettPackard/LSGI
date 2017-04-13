#!/bin/bash
#@Description: 
#		Launch processes in each node listed on hostfile
#@Params: 
#		<N> Number of process to launch (MAX)
#		<dataset> Dataset to run
#		<port> QueryService port (Optional)
#@Requirements:
#		Make sure 'hosts' file is updated 
#@Authors: Tere Gonzalez, Fei, Krishna 
#@Date: December 14, 2015
#@How to run it: ./launchMultiNodeDemo.sh <N> <dataset> <port>
#	For example:
#	1) Using default QS port: ./launchMultiNodeDemo.sh 2 ../../data/inputGraphs/dns_graph.alchemy.factors.bin
#	2) Using custom QS port: ./launchMultiNodeDemo.sh 2 ../../data/inputGraphs/dns_graph.alchemy.factors.bin 580003

echo "Large Scale  Graph Inference (LSGi)"
echo "Version 1.0-l4tm, @Hewlett Packard Labs"
echo "----------------------------------------------------"

#
# - Variable definitions
#
curPath=$(pwd)
#Configuration file to use
configfile="../../data/config/configure_gdns"
#File system to use: /lfs/ or /dev/shm
SharedFileSystem="/lfs/"
#SharedFileSystem="/dev/shm/"
#node availalbles
file="hosts"
#default QueryService port
port=58000

#user parameter: number of process to launch
N=$1
#user parameter: Dataset to run
dataset=$2

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

#host file exist
if [ ! -f $file ]
then
	echo "Error, host file does not exist, please check configuration"
fi 

#checking QueryService port to use
if [ ! -z "$3" ]
then
    port=$3
fi

#
# - remote launch
#
echo Running experiment for [ $1 ] processes
echo "----------------------------------------------------"

echo "killing previous job.."
kill $(pgrep -l QueryService | awk '{print $1'})

echo $curPath

p=0;
while [ $p -lt $N ]
do
	for host in `cat $file`
		    do
	     echo "lauching on host:" $host  
	     ssh $host  "cd $curPath; ./gibbsDNSApp $configfile $dataset $p $N -FS=$SharedFileSystem > logs/$p.log 2>&1 &"
	     p=$((p + 1))	
	     sleep 1
	     if [ $p -eq $N ]
	     then
		break 	
	     fi
	done
done
echo "Launch done for [$p] nodes"
sleep 30
#running query service
if [ $p -gt 0 ] 
then

	./QueryService $dataset $N -FS=$SharedFileSystem -port=$port > logs/QS.$p.txt 2>&1 &
	echo  "Done query service launched"
fi 
echo "Done!"

