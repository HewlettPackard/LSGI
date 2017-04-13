#!/bin/bash
#Script to run query for specific  vertex id results
#Done by tere
#How to run id

#search status of vertex id=5
#./queryByVertexId.sh 5
#search status of vertex id=5 connected to port-58002
#./queryByVertexId.sh 5 58002

vid=$1
port=58000

#check if the por it is specified
#otherwise will use defaul value
if [ ! -z "$2" ]
then
    port=$2
fi


./QueryClient 1 $vid $port

echo ""
echo "Done query for [ " $vid  " ]"
