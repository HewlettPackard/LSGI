#!/bin/bash
#This script put correct permissions to the files and
#folder incase the checkout has remove them
#It avoids the configuration to fail because of permissions errors
#Author Janneth, Tere
##July
#How to rund
#./setPermissions.sh <LSGI root path>
#./setPermissions.sh /home/user/
#Use
LSGI_PATH=$1

echo ">> Setting permisions to local package"
	#Set permission to scripts *.sh
	cd $LSGI_PATH/LSGi/config; pwd; chmod 755 *.sh
	cd $LSGI_PATH/LSGi/demo/inference; pwd; chmod 755 *.sh
	cd $LSGI_PATH/LSGi/demo/graphQuery; pwd; chmod 755 *.sh
	cd $LSGI_PATH/LSGi/deploy; pwd; chmod 755 *.sh
	cd $LSGI_PATH/LSGi/test/deploy; pwd; chmod 755 *.sh

	#Set permission to executables
	cd $LSGI_PATH/LSGi/demo/inference; pwd; find . -type f -not -name '*.*' -exec chmod 755 \{\} \; 
	cd $LSGI_PATH/LSGi/demo/graphQuery; pwd; find . -type f -not -name '*.*' -exec chmod 755 \{\} \; 

echo "--------------------"
echo "Done with permissions"
