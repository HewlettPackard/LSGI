#!/bin/bash
#Kill the query service job in all the connecting nodes
#Author Tere, July 1, 2016
#Params: the hosts file should contain the list of remote nodes
#How to run it:
#./killInferenceAll.sh


#host file

file="hosts"

echo "killing QueryService.."
kill $(pgrep -l QueryService | awk '{print $1'}) 

for host in `cat $file`
   do
	echo "killing gibbs on host:" $host 
	ssh $host  "kill $(pgrep -l gibbs | awk '{print $1'})"
done
echo "Done!"
