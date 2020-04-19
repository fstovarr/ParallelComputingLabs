# Run like ./ejecutar_todo.sh IMG_PATH

pip install --user matplotlib
pip install --user numpy
pip install --user pandas
IMG_PATH=$1

make -C Blur

# Runs Sequential
echo "------- SEQUENTIAL------"
./test-bench-sequential.sh $IMG_PATH Blur/results/blur_effect_sequential/sequential.out
# Runs Block data
echo "--------- BLOCK --------"
./test-bench.sh Blur/bin/blur_effect_blockwise $IMG_PATH Blur/out/blur_effect_blockwise Blur/results/blur_effect_blockwise/blockwise.out
# Runs cyclic data
echo "--------- CYCLIC--------"
./test-bench.sh Blur/bin/blur_effect_block_cyclic $IMG_PATH Blur/out/blur_effect_block_cyclic Blur/results/blur_effect_block_cyclic/bloc_cyclic.out 
# Runs pool
echo "---------- POOL ---------"
./test-bench.sh Blur/bin/blur_effect_pool $IMG_PATH Blur/out/blur_effect_pool Blur/results/blur_effect_pool/pool.out 
