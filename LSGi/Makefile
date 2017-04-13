#@Description: <target>:<description>
#	deployment: Deploys LSGi package into remote hosts
#	configure: Compiles LSGi package in remote hosts
#	configureSSH: Configure SSH passwordless among the master node (first host), the master itself and the rest of the nodes
#	configureHosts: Copy hostfile to remote nodes
#	testSingleNode: Runs a test for single node in remote hosts
#	testMultiNode: Runs a test for multi node in remote hosts
#@Params: <target>
#@Requirements:
#	Edit 'hosts' file with the remote hostnames before running this Makefile
#	SSH passwordless connection needed from localhost to remote hosts
#@Authors: Janneth Rivera, Tere Gonzalez
#@Date: June 24 2016
#@How to run it: make <target>. For example: make deployment

# The base directory is where the LSGi files are installed,
# assume that is one directory above where the Makefile is.
LSGI_NAME = LSGi
LSGI_BASE_DIR = $(shell (cd .. ; pwd))

$(info LSGI_BASE_DIR: $(LSGI_BASE_DIR))

run:
	pwd;
	cd config/;
	pwd;

deployment:
	#Deploy LSGi package to remote hosts
	#[Step #1]
	cd deploy/;./setPermissions.sh $(LSGI_BASE_DIR)
	#[Step #2]
	cd deploy/; ./copyPkg.sh $(LSGI_BASE_DIR) $(LSGI_BASE_DIR)

configure:
	#Copy hostfile to remote nodes
	cd config/; ./copyHostFile.sh $(LSGI_BASE_DIR)
	#Compile LSGi package in remote hosts
	cd config/; ./compile.sh $(LSGI_BASE_DIR)

configureSSH:
	#Configure SSH passwordless among the master node (first host), the master itself and the rest of the nodes
	cd config; ./sshCopy.sh

configureHosts:
	#Copy hostfile to remote nodes
	cd config/; ./copyHostFile.sh $(LSGI_BASE_DIR)

testSingleNode:
	#Run test for SingleNode in remote hosts
	cd test/deploy/; ./testSingleNode.sh $(LSGI_BASE_DIR)

testMultiNode:
	#Run test for MultiNode in master node
	cd test/deploy/; ./testMultiNode.sh $(LSGI_BASE_DIR)
