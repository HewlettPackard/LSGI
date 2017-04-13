#/bin/bash/

fname=$(basename $1)
echo $fname
 grep "Gibbs Run Time(sec)"  logs/$fname.*.log
