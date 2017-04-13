#!/bin/bash
#Search vertices that are greater or equal to the  input threshold.
#The threshold should be put as integer between [0-100]
#Tere, Fei, Krishna,
#Done Dec 18, 2015
 
#params
#user threshold
threshold=$1 
queryType=3

#triger the searchs
./QueryClient $queryType $threshold
