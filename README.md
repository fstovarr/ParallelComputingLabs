# Blur effect

In this project you will find different implementations of blur effect applied to images in several formats, based on Gaussian Kernel

The directory presents the following structure:

```bash
BlurrEfectPOSIX
│   README.md
│   ejecutar_todo.sh # Script to run all test benchs
|   test-bench.sh # Script to run a single test bench
|   plot.py # Script de python to make plots
|   plot_seq.py # Script de python to make plots of sequential version
|   Analysis.ipynb # python notebook to show how the program works
│
└───BlurEffectSequential
└───BlurEffectBlock
└───BlurEffectBlockCyclic
└───BlurEffectPool
└───img
└───out
```

In each of the following folders with name BlurEffect\*, you will find the problems solution according to a different paradigm for threads work distribution (sequential, block, block-cyclic, thread-pool). In turn, each folder has this structure:

```bash
BlurrEfect*
│   README.md
│   Makefile
|   analysis4.txt # Analysis performed with the tool GProc
└───bin # Executables
└───lib # Libraries
└───src # Source code
```

## Build and run

Each folder has its own Makefile, where there are the necessary steps to make and build the executable. That steps are the following:

```bash
make # Build executable
./bin/blur-effect img/favicon.png out/testing.png 3 8 # Execute the program
```

### Make options

```bash
make clean # Delete objects folder
make fullclean # Delete objects, bin and other non-esencial folders
```

## Run test bench

Perform the test bench for all folders (Sequential, Block, BlockCyclic, Pool)

```
./ejecutar_todo.sh
```

To perfom a test bench for single paradigm, run

```bash
# Run Sequential
./test-bench-sequential.sh $IMG_PATH Results/sequential_data.out

# Run Block
./test-bench.sh BlurEffectBlock/bin/blur-effect $IMG_PATH BlurEffectBlock/out Results/block.out

# Run Block-Cyclic
./test-bench.sh BlurEffectBlockCyclic/bin/blur-effect $IMG_PATH BlurEffectBlockCyclic/out Results/cyclic_data.out

# Runs Pool
./test-bench.sh BlurEffectPool/bin/blur-effect $IMG_PATH BlurEffectPool/out Results/pool_data.out
```
