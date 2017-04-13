#!/bin/bash
## Compile package remotly for each host in  hosts file
## parameter: destination path
##how to run it ./compile <root path to  LSGi>"
## exmple:./complie /home/root/LSi
##author: tere, april 2016

##Destination path
destPath=$1

echo "despaht="$destPath
##File containing a list of hosts

file=../hosts


#verifying path:
if test -z "$destPath"
then
  destPath=$(pwd)
fi




echo ">> Compiling..."
echo "--------------------------------------------------"
 echo "Compilation using current path:"$destPath

##For every host
for host in `cat $file`
do

        echo $host...
        ssh $host "cd $destPath/LSGi/source/inference; pwd; make clean; make"
        ssh $host "cd $destPath/LSGi/source/queryService/queryService; pwd; make clean; make"
        ssh $host "cd $destPath/LSGi/source/queryService/clientService; pwd; make clean; make"

done
echo "--------------------------------------------------"
echo ">>Done compiling"
