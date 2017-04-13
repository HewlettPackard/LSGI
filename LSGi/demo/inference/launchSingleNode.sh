#!/bin/bash
#@Description: 
#		Launch single  process in the local system
#@Authors: Tere Gonzalez, Fei, Krishna 
#@Date: December 17, 2015
#@How to run it: ./launchSingleNode.sh


echo "Large Scale  Graph Inference (LSGi)"
echo "Version 1.0-l4tm, @Hewlett Packard Labs"
echo "----------------------------------------------------"

#
# - Variable definitions
#
curPath=$(pwd)
#Dataset to run
dataset="../../data/inputGraphs/dns_graph.alchemy.factors.bin"
#dataset="../../data/inputGraphs/graph8.alchemy.bin"
#Configuration file to use
#configfile="../../data/config/configure_g8"
configfile="../../data/config/configure_gdns"
#File system to use: /lfs/ or /dev/shm
SharedFileSystem="/lfs/"
#node availalbles



echo "Lauching single-local process"

./gibbsDNSApp $configfile $dataset 0 1 -FS=$SharedFileSystem > logs/0.log

echo "done"
