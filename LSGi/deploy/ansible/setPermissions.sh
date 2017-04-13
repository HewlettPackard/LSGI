#!/bin/bash

##
LSGI_PATH=$1

##File containing a list of hosts
file="./hosts"


##For every host
for host in `cat $file`
do

        echo $host...
	#Set permission to scripts *.sh
	ssh $host "cd $LSGI_PATH/LSGi/config; pwd; chmod 755 *.sh"
	ssh $host "cd $LSGI_PATH/LSGi/demo/inference; pwd; chmod 755 *.sh"
	ssh $host "cd $LSGI_PATH/LSGi/demo/graphQuery; pwd; chmod 755 *.sh"
	ssh $host "cd $LSGI_PATH/LSGi/deploy; pwd; chmod 755 *.sh"
	ssh $host "cd $LSGI_PATH/LSGi/test/deploy; pwd; chmod 755 *.sh"

	#Set permission to executables
	ssh $host "cd $LSGI_PATH/LSGi/demo/inference; pwd; find . -type f -not -name '*.*' -exec chmod 755 \\{\\} \\;"
	ssh $host "cd $LSGI_PATH/LSGi/demo/graphQuery; pwd; find . -type f -not -name '*.*' -exec chmod 755 \\{\\} \\;"
done
