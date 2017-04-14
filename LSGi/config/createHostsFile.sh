#!/bin/bash

#“© Copyright 2017  Hewlett Packard Enterprise Development LP

# Redistribution and use in source and binary forms, with or without modification, 
# are permitted provided that the following conditions are met:
# 1. Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice, 
# this list of conditions and the following disclaimer in the documentation and/or
# other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the names of its contributors
# may be used to endorse or promote products derived from this software without 
# specific prior written permission.

# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
# IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
# USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.”
#*/

#@Description: 
#	Create local 'hosts' file from from ansible inventory file for specific group
#	Distribute local 'hosts' file to each host
#@Parameters:
#	<input file>
#	<group name>
#	<output file>
#	<path to distribute>
#@Results: 
#	'hosts' file containing hosts names
#	updated 'hosts'file in remote hosts
#@Requirements:
#	Edit 'ansible_hosts' file with the remote hostnames before running this script
#@Authors: Janneth Rivera, Tere Gonzalez
#@Date: June 24 2016
#@How to run it: ./createHostsFile.sh <input file> <group name> <output file> <path to distribute>
#	For example: ./createHostsFile.sh ansible_hosts lsgi_nodes hosts ~/LSGi

INPUT_FILE=$1
GROUP_NAME=$2
OUTPUT_FILE=$3
LSGI_PATH=$4

#
# - Input Params Checking
#
#input file exist
if [ ! -f $INPUT_FILE ]
then
	echo "Error, input file does not exist, please check configuration"
	exit
fi
#group name not empty
if [ -z $GROUP_NAME ]
then
	echo "Error, group name is empty, please check configuration"
	exit
fi
#output file name not empty
if [ -z $OUTPUT_FILE ]
then
        echo "Error, output filename is empty, please check configuration"
        exit
fi
#path not empty
if [ -z $LSGI_PATH ] 
then
	echo "Error, path is empty, please check configuration"
	exit
fi

#Get total number of lines in input file
totalLines=$(cat $INPUT_FILE | wc -l)
#echo "total lines: $totalLines"

#Find group name in input file
groupLine=$(grep -n $GROUP_NAME $INPUT_FILE | cut -d ":" -f1)
#echo "group found at line: $groupLine"

#Calculate number of hosts
numLines=$(($totalLines-$groupLine))
#echo "getting $numLines lines"

#Write to output file, ignore empty lines if any
cat $INPUT_FILE | tail -n $numLines |  grep -v ^$ > $OUTPUT_FILE

#Copy 'hosts' file in local host
cp $OUTPUT_FILE $LSGI_PATH/LSGi/config
cp $OUTPUT_FILE $LSGI_PATH/LSGi/demo/inference

#Copy 'hosts' file in remote hosts
for host in `cat $OUTPUT_FILE`
do
	echo $host...
	scp $OUTPUT_FILE $host://$LSGI_PATH/LSGi/config
        scp $OUTPUT_FILE $host://$LSGI_PATH/LSGi/demo/inference
	scp $OUTPUT_FILE $host://$LSGI_PATH/LSGi/deploy
done


echo "done"

