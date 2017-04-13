#/bin/bash/

fname=$(basename $1)
echo $fname

tail -20 logs/$fname.0.log
