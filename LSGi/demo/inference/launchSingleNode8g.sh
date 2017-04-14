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

#@Description: 
#		Launch single  process in the local system
#@Params: 
#		<SharedFileSystem> File system to use (Optional)
#@Authors: Tere Gonzalez, Fei, Krishna 
#@Date: December 17, 2015
#@How to run it: ./launchSingleNode8g.sh <SharedFileSystem>
#	For example:
#	1) In memory: ./launchSingleNode8g.sh  /dev/shm/
#	2) Using LFS: ./launchSingleNode8g.sh 

echo "Large Scale  Graph Inference (LSGi)"
echo "Version 1.0-l4tm, @Hewlett Packard Labs"
echo "----------------------------------------------------"

#
# - Variable definitions
#
curPath=$(pwd)
#Dataset to run
#dataset="../../data/inputGraphs/dns_graph.alchemy.factors.bin"
dataset="../../data/inputGraphs/graph8.alchemy.bin"
#Configuration file to use
configfile="../../data/config/configure_g8"
#configfile="../../data/config/configure_gdns"
#File system to use: /lfs/ or /dev/shm
SharedFileSystem="/dev/shm/"
#node availalbles

#FileSystem
if [ ! -z "$1" ]
then
    SharedFileSystem=$1
fi

echo "Lauching single-local process"

./gibbsDNSApp $configfile $dataset 0 1 -FS=$SharedFileSystem > logs/0.log

echo "done"
