# !/bin/bash

for K in 3 5 7 9 11 13 15
do
  for THREADS in 2 4 8 16 32
  do
    IN_FILENAME=${1##*/}
    OUT_FILENAME=./BlurEffect/out/${IN_FILENAME%.*}_K${K}_T${THREADS}.jpg
    echo "#THREADS $THREADS - KERN: $K - IN: ${1} - OUT: ${OUT_FILENAME}"
    (/usr/bin/time -f %e ./BlurEffect/bin/blur-effect $1 $OUT_FILENAME $K $THREADS) 2>>out.out
  done
done