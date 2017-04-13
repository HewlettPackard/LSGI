#!/bin/bash
## Copy package into each host from hosts file
## parameter: source path
## parameter: destination path
##@ Tere, Janneth, Fei, Krishna



##Source path
srcPath=$1

##Destination path
destPath=$2

##User to use
user="$(id -u -n)"

##File containing a list of hosts
file="../hosts"

echo ">> Copying files to target nodes."

##For every host
for host in `cat $file`
do
	echo "copy to " $host...
	scp -r $srcPath $user@$host://$destPath
	done

echo "--------------------------------------------------"
echo "Done copy to nodes"

