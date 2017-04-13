#!/bin/bash/
#@Description: <target>:<description>
#	deploy: Deploys LSGi package into remote hosts
#	configure: Compiles LSGi package in remote hosts
#	testSingleNode: Runs a test for single node in remote hosts
#	testMultiNode: Runs a test for multi node in remote hosts
#	testPing: Tests connectivity by pinging remote hosts
#@Params: <target>
#@Requirements:
#	Edit 'ansible_hosts' file with the remote hostnames before running this Makefile
#	SSH passwordless connection needed from localhost to remote hosts
#@Authors: Janneth Rivera, Tere Gonzalez
#@Date: June 24 2016
#@How to run it: make <target>. For example: make testPing


#Ansible inventory file
ANSIBLE_INVENTORY=./ansible_hosts

#Ansible hosts group name
ANSIBLE_GROUP=lsgi_nodes

#Hosts file name for configure
HOSTS_FILE=./hosts

#Path to deploy LSGi package
LSGI_PATH=~/LSGiTest9


deploy:
	#Deploy LSGi package to remote hosts
	ansible-playbook deploy.yml -i $(ANSIBLE_INVENTORY) --extra-vars "ANSIBLE_GROUP=$(ANSIBLE_GROUP) LSGI_PATH=$(LSGI_PATH)"

configure:
	#Create 'hosts' file from ansible inventory file for specific group of hosts
	#and distribute it on each host
	./createHostsFile.sh $(ANSIBLE_INVENTORY) $(ANSIBLE_GROUP) $(HOSTS_FILE) $(LSGI_PATH)

	./setPermissions.sh $(LSGI_PATH)

	#Configure SSH passwordless among the master node (first host), the master itself and the rest of the nodes
#	../config/sshCopy.sh
	 ansible-playbook sshCopy.yml -i $(ANSIBLE_INVENTORY) --extra-vars "ANSIBLE_GROUP=$(ANSIBLE_GROUP) LSGI_PATH=$(LSGI_PATH)"

	#Compile LSGi package in remote hosts
	../config/compile.sh $(LSGI_PATH)


testSingleNode:
	#Run test for SingleNode in remote hosts
	cd ../test/deploy/; ./testSingleNode.sh

testMultiNode:
	#Run test for MultiNode in master node
	cd ../test/deploy/; ./testMultiNode.sh

testPing:
	#Test connectivity between localhost and remote hosts
	ansible -i $(ANSIBLE_INVENTORY) $(ANSIBLE_GROUP) -m ping

