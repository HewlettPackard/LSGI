#!/bin/bash
#script to launch services
#nov 9, 2015

echo "To start query service for graph inference"

#dataset=datapw/gV20ME55912986.Alchemy.map.bin
dataset=dns_graph.alchemy.factors.bin
numberOfProcess=2

./QueryService $dataset $numberOfProcess
