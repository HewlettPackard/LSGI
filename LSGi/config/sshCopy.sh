#!/bin/bash
##Create and Copy SSH passwordless between machines
##Requires: hostfile,
## Tere, Janneth, Fei, Krishna
#Dec 12, 2015
## How to run it: ./sshCcpy.sh 




echo ">> Setting up passwordless key >>"
echo "----------------------------------------------"

##File containing a list of hosts
hostFile="../hosts"

##selectin first node of file host as master node
#masterNode="$(head -n 1 $hostFile)"
masterNode=$(hostname)
curPath=$(pwd)

#create master key
echo "Creating ssh key in from node:"$masterNode 
#ssh $masterNode "cd ~/.ssh/;ssh-keygen -t rsa"

cd ~/.ssh/;ssh-keygen -t rsa
cd $curPath;

##For every host
for host in `cat $hostFile`
do
       echo "SSh-key copy to $host"
       ssh-copy-id -i ~/.ssh/id_rsa.pub $host
done 

echo "-------------------------------------------------"
echo "Done passwordless between master to hosts"

