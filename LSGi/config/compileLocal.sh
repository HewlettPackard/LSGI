#!/bin/bash
## Compile package on localhost
## parameter: destination path

##Destination path
destPath=$1


#verifying path:
if test -z "$destPath"
then
  cd ..
  destPath=$(pwd)
  echo "Compilation using current path:"$destPath

fi


echo ">>Compiling..."
	cd $destPath/LSGi/source/inference; pwd; make clean; make;
        cd $destPath/LSGi/source/queryService/queryService; pwd; make clean; make;
        cd $destPath/LSGi/source/queryService/clientService; pwd; make clean; make;
echo ">>Done"

