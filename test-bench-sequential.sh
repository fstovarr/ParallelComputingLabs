# 1: image_path - The path of the image to process
# 2: The output file where the times are stored

pip install --user matplotlib
pip install --user numpy
ITERATIONS=5

pwd
rm $2

for K in 3 5 7 9 11 13 15
do
  IN_FILENAME=${1##*/}
  OUT_FILENAME=Blur/out/blur_effect_sequential/${IN_FILENAME%.*}_K${K}.png
  echo "KERN:$K" 
  for (( i=0; i<$ITERATIONS; i++ ))
  do
    echo "TIME: $i"
    COMMAND="./Blur/bin/blur_effect_sequential $1 $OUT_FILENAME $K"
    `echo $COMMAND` >> $2
  done
done

PLOT_PATH=${2%.*}.png
python plot_seq.py $2 $ITERATIONS $PLOT_PATH


