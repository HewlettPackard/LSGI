#!/bin/bash
#@Description: 
#		Run test for Single node launch in remote hosts
#		Verifies if the local execution generates output files
#@Params: <file system> (Optional)
#@Requirements:
#		Make sure 'hosts' file is updated 
#@Authors: Janneth Rivera, Tere Gonzalez
#@Date: June 24 2016
#@How to run it: ./testSingleNode.sh <file system>
#	For example:
#	1) In memory: ./testSingleNode.sh  /dev/shm/
#	2) Using LFS: ./testSingleNode.sh 

LSGI_PATH=$1
LSGI_HOME="$LSGI_PATH/LSGi" 
HOSTS_FILE="../../hosts"
LAUNCH_SCRIPT="$LSGI_HOME/demo/inference/launchSingleNode8g.sh"
OUTPUT_DIR="$LSGI_HOME/data/outputStats"
FILE_SYSTEM="/lfs/"
FAIL_COUNTER=0
FAIL_NODES_LIST=""

#checking file system to use
#FileSystem

if [ ! -z "$2" ]
then
    FILE_SYSTEM=$2
fi


#host file exist
if [ ! -f $HOSTS_FILE ] 
then
        echo "Error, host file does not exist, please check configuration"
        exit
fi


#Current path
curPath=$(pwd)

#Header
echo ""
echo "LSGi by Hewlett Packard Labs"
echo "---------------------------------------------------------"
echo "Test case:         LSGi Single Node"
echo "Script:            $LAUNCH_SCRIPT"
echo "SharedFileSystem:  $FILE_SYSTEM"
echo "---------------------------------------------------------"

for host in `cat $HOSTS_FILE`
do

	##Removing output files
        echo "Removing output files in "$host at
	ssh $host "cd $OUTPUT_DIR; pwd; rm out*.*"
        echo "Lauching in $host at:"
        ssh $host "cd $LSGI_HOME/demo/inference; pwd; $LAUNCH_SCRIPT $FILE_SYSTEM > logs/log.txt"
        ##Verify if output files 'out*.*' exists
        echo "Verifying output in $host at:"
        ssh $host "cd $OUTPUT_DIR; pwd; ls -l out*.*"
       

        if [  $? -eq 0 ]; then
	    echo "---------------------------------------------------------"
	    echo "TEST RESULT: Success!!! :)"
	    echo "LSGi single node has run and output files were generated."
        else
	     echo "---------------------------------------------------------"
	     echo "TEST RESULT: Fail!!! :("
	     echo "LSGi has not run and output files were not generated."
	     echo "Verify screen log."
	     FAIL_COUNTER=$((FAIL_COUNTER+1))
	     FAIL_NODES_LIST="$FAIL_NODES_LIST $host "
        fi
done 

#Fail resume
if [ $FAIL_COUNTER -gt 0 ]; then
	echo ""
	echo "---------------------------------------------------------"
	echo "Test case:	LSGi Single Node"
	echo "Total Failures: $FAIL_COUNTER"
	echo "Faliure nodes:	$FAIL_NODES_LIST"
	echo "---------------------------------------------------------"
fi
