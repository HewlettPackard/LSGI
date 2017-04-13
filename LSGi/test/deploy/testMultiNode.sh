#!/bin/bash
#@Description: 
#		Run test for Multi node launch in remote hosts
#		Verifies if the local execution generates output files
#@Params: <file system> (Optional)
#@Requirements:
#		Make sure 'hosts' file is updated 
#@Authors: Janneth Rivera, Tere Gonzalez
#@Date: June 24 2016
#@How to run it: ./testMultiNode.sh <file system>
#	For example:
#	1) In memory: ./testMultiNode.sh  /dev/shm/
#	2) Using LFS: ./testMultiNode.sh 

LSGI_PATH=$1 #destination path
LSGI_HOME="$LSGI_PATH/LSGi" 
HOSTS_FILE="../../hosts"
LAUNCH_SCRIPT="$LSGI_HOME/demo/inference/launchMultiNode40G.sh"
OUTPUT_DIR="$LSGI_HOME/data/outputStats"
NUM_NODES="$(cat $HOSTS_FILE | grep -v ^$ | wc -l)"
FIRST_HOST="$(head -n 1 $HOSTS_FILE)"
FILE_SYSTEM="/lfs/"


#checking file system to use
#FileSystem

if [ ! -z "$2" ]
then
    FILE_SYSTEM=$2
fi


#Current path
curPath=$(pwd)

#Header
echo ""
echo "LSGi by Hewlett Packard labs"
echo "---------------------------------------------------------"
echo "Test case:         Multi Node LSGi   " 
echo "Script:            $LAUNCH_SCRIPT"
echo "SharedFileSystem:  $FILE_SYSTEM"
echo "Found nodes:       $NUM_NODES (maxNodes=40, available:1,2,4,8,16,32,40)"
echo "---------------------------------------------------------"


##Set ssh from first node to the rest
#if [ "$host" = "$FIRST_HOST"  ]; then
#    ssh $host "cd $LSGI_HOME/config;  pwd; ./sshCopy.sh"
#fi

    ##Removing output files
    echo "Removing output files in $host at:"$FIRST_HOST
    ssh $FIRST_HOST "cd $OUTPUT_DIR;  pwd; rm out*.*"

    ##Launch
    echo "Lauching in $host at:"  
    ssh $FIRST_HOST "cd $LSGI_HOME/demo/inference; pwd; nohup $LAUNCH_SCRIPT $NUM_NODES $FILE_SYSTEM > logs/log.txt 2>&1 &"

    # Maximum wait (40 * 15 seconds = 10 minutes)
    cur_wait=0
    max_wait=40
    wait_len=15

    while [ $cur_wait -lt $max_wait ]
    do
        echo "Verifying output in $host at:"
        ssh $FIRST_HOST "cd $OUTPUT_DIR; pwd; ls -l out*.*"
        if [ $? -eq 0 ]
        then
            echo "---------------------------------------------------------"
            echo "TEST RESULT: Success!!! :)"
            echo "LSGi multi-node has run and output files were generated."
            exit 0
        fi
        echo "waiting $wait_len seconds for log files to appear ..."
        sleep $wait_len
        (( cur_wait=cur_wait+1 ))
        done

    echo "---------------------------------------------------------"
    echo "TEST RESULT: Fail!!! :("
    echo "LSGi has not run and output files were not generated."
    echo "Verify screen log."
    echo "Verify execution log files in each node of host  at $LSGI_HOME/logs/"
    exit 1
