# Run like ./ejecutar_todo.sh IMG_PATH

pip install --user matplotlib
pip install --user numpy
pip install --user pandas
IMG_PATH=$1

# I make all because why not
make -C BlurEffect
make -C BlurEffectPool
make -C BlurEffectSequential
make -C BlurEffectBlock

# Runs Sequential
echo "------- SEQUENTIAL------"
./test-bench-sequential.sh $IMG_PATH Results/sequential_data.out
# Runs Block data
echo "--------- BLOCK --------"
./test-bench.sh BlurEffectBlock/bin/blur-effect $IMG_PATH BlurEffectBlock/out Results/block.out
# Runs cyclic data
echo "--------- CYCLIC--------"
./test-bench.sh BlurEffectBlockCyclic/bin/blur-effect $IMG_PATH BlurEffectBlockCyclic/out Results/cyclic_data.out 
# Runs pool
echo "---------- POOL ---------"
./test-bench.sh BlurEffectPool/bin/blur-effect $IMG_PATH BlurEffectPool/out Results/pool_data.out 
