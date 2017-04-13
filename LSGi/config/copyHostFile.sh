
#!/bin/bash
## Compile package remotly for each host in  hosts file
## parameter: destination path
##how to run it ./compile <root path to  LSGi>"
## exmple:./complie /home/root/LSi
##author: tere, april 2016

##Destination path
destPath=$1

echo "despath="$1
##File containing a list of hosts

hostfile=../hosts


#verifying path:
if test -z "$destPath"
then
  destPath=$(pwd)
fi




echo ">> Copy hostfile..."
echo "--------------------------------------------------"
 echo "Hostfile to destination path:"$destPath

##For every host
for host in `cat $hostfile`
do
        echo $host...
	scp $hostfile $host://$destPath/LSGi/
	scp $hostfile $host://$destPath/LSGi/demo/inference/
done
echo "--------------------------------------------------"
echo ">>Done compiling"
