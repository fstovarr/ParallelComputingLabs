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
# echo "--------- BLOCK --------"
./test-bench.sh Blur/bin/blur_effect_blockwise $IMG_PATH Blur/out/blur_effect_blockwise Blur/results/blur_effect_blockwise/blockwise.out

# Runs cyclic data
#echo "--------- CYCLIC--------"
./test-bench.sh Blur/bin/blur_effect_block_cyclic $IMG_PATH Blur/out/blur_effect_block_cyclic Blur/results/blur_effect_block_cyclic/block_cyclic.out 

# Runs pool
# echo "---------- POOL ---------"
./test-bench.sh Blur/bin/blur_effect_pool $IMG_PATH Blur/out/blur_effect_pool Blur/results/blur_effect_pool/pool.out 

# Runs Block OMP
echo "---------- BLOCK - OMP ---------"
./test-bench.sh Blur/bin/blur_effect_blockwise_omp $IMG_PATH Blur/out/blur_effect_blockwise_omp Blur/results/blur_effect_blockwise_omp/blockwise_omp.out 

# Runs Cyclic OMP
echo "---------- CYCLIC - OMP ---------"
./test-bench.sh Blur/bin/blur_effect_block_cyclic_omp $IMG_PATH Blur/out/blur_effect_block_cyclic_omp Blur/results/blur_effect_block_cyclic_omp/block_cyclic_omp.out 

# Run CUDA
echo "---------- CYCLIC - OMP ---------"
./test-bench.sh BlurEffectCUDA/bin/blur_effect_block_cyclic_omp $IMG_PATH BlurEffectCUDA/out/blur_effect_block_cyclic_omp BlurEffectCUDA/results/blur_effect_block_cyclic_omp/block_cyclic_omp.out 