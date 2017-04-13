#!/bin/bash
##Check SSH passwordless between machines
##Otherwise, set passwordless
## Janneth, Tere, Fei, Krishna
#Dec 12, 2015


echo ">>Checking SSH connections"

##File containing a list of hosts
hostfile="../hosts"



##For every host
while read host
do

##Do ssh and get status
status=$(ssh -o BatchMode=yes -o ConnectTimeout=5 $host echo ok 2>&1  <<'SSHBLOCK'
        exit
SSHBLOCK
)

#echo "status:" $status

##check ssh response status
if [[ $status == ok ]] ; then
        echo "ssh to " $host: $status

elif [[ $status == "Permission denied"* ]] ; then
        echo "ssh to " $host: $status
        echo "setting ssh passworless..."
        ssh-copy-id -i ~/.ssh/id_rsa.pub $host
else
        echo "ssh to " $host: $status
fi

done < $hostfile
echo ">>Done"

