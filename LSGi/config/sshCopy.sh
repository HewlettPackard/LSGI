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

