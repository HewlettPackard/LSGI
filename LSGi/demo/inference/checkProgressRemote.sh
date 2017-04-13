#/bin/bash/
#Read the log file for the main process:0
#How to run it :
# ./checkProgress.sh
#Author, Tere, July  23 2016

echo "Progress for process 0 <master>"
echo "--------------------------------"

file="hosts"
curPath=$(pwd)
p=0;
for host in `cat $file`
do 
	ssh $host "cd $curPath; echo echo progress for $host-$p.log;tail -20 logs/$p.log"
	p=$((p+1));
done


