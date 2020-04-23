# 1: which binary to run - Pick between the tests
# 2: image_path - The path of the image to process
# 3: output_folder_path - Where to place the generated images
# 4: The output file where the times are stored
# Example: ./test-bench.sh BlurEffectBlockCyclic/bin/blur-effect img/test720.jpg BlurEffectBlockCyclic/out out.out
# IF THIS DOES NOT RUN. MAKE sure to use bash test-bench.sh instead

ITERATIONS=5

pwd
rm $4

for K in 3 5 7 9 11 13 15
do
  for THREADS in 1 2 4 8 16 32
  do
    IN_FILENAME=${2##*/}
    OUT_FILENAME=${3}/${IN_FILENAME%.*}_K${K}_T${THREADS}.png
    echo "KERN:$K THR:$THREADS" 
    for (( i=0; i<$ITERATIONS; i++ ))
    do
      echo "Iteration: $i"
      COMMAND="$1 $2 $OUT_FILENAME $K $THREADS"
      `echo $COMMAND` >> $4
    done
  done
done

PLOT_PATH=${4%.*}.png
CSV_PATH=${4%.*}.csv
python plot.py $4 $ITERATIONS $PLOT_PATH $CSV_PATH

