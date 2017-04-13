#!/bin/bash
#Search vertices that are greater or equal to the  input threshold.
#The threshold should be put as integer between [0-100]
#Tere, Fei, Krishna,
#Done Dec 18, 2015
#how to run it: 
# ./queryByProbThreshold.sh 90 
#specific port for the query service
# ./queryByProbThreshold.sh 90 58001

#params
#user threshold
threshold=$1 
queryType=3
port=58000


#checking QueryService port to use, otherwise
#otherwise will use defaul value
if [ ! -z "$2" ]
then
    port=$2
fi
#triger the searchs
./QueryClient $queryType $threshold $port
