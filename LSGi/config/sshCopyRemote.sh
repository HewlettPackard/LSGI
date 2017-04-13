#!/bin/bash
##Check SSH passwordless between machines
##Otherwise, set passwordless
## Janneth, Tere, Fei, Krishna
#Dec 12, 2015


echo ">>Setting up passwordless key"
echo "----------------------------------------------"

##File containing a list of hosts
hostFile="../hosts"
masterNode="$(head -n 1 $hostFile)"

echo "Master Node:"$masterNode
##For every host



for host in `cat $hostFile`
do
	echo "from:"$masterNode" key to $host"
#        ssh $masterNode "echo "ssh in"; hostname;ssh-copy-id -i ~/.ssh/id_rsa.pub $host"
	ssh $masterNode "cat ~/.ssh/id_rsa.pub | ssh $host 'cat >> .ssh/authorized_keys'"
done 
echo "-------------------------------------------------"
echo "Done passwordless between master to hosts"

