#!/bin/bash
#Kill the inference job in the local host
#Author: Tere, July 1st, 2016
#Params:none
#How to run it:
#./killInferenceLocal.sh

#kill all local jobs 
kill $(pgrep -l gibbs | awk '{print $1'})

echo "done..."
