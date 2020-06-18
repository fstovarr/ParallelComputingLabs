# Blur effect

In this project you will find different implementations of blur effect applied to images in several formats, based on Gaussian Kernel

The directory presents the following structure:

```bash
BlurCPU
│ └───Blur
│     └───img
│     └───lib
│     └───src
│     Makefile
│     README.md
│  run_all.sh       # Script to run all test benchs
|  test-bench.sh    # Script to run a single test bench
|  plot.py          # Script de python to make plots
|  plot_seq.py      # Script de python to make plots of sequential version
|  Analysis.ipynb   # python notebook to show how the program works
│
BlurGPU
│ └───Blur
│     └───img
│     └───lib
│     └───src
│  plot_cuda.py # Script to plot the data produced in the GPU execution
Results
│ └───CUDA          # Data and plots for the execution with CUDA
│ └───OMP           # Data and plots for the execution with OMP
│ └───POSIX         # Data and plots for the execution with POSIX
│ └───Sequential    # Data and plots for the execution
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

## Run test bench (CPU)

Perform the test bench for all folders (Sequential, Block, BlockCyclic, Pool)

```
./run_all.sh
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

## Run test bench (GPU)

For the `BlurGPU` folder, the test bench is contained in the notebook `Blur_effect.ipynb`.

## Run test bench (MPI)

Execute in the `BlurMPI` folder:

```bash
./run.sh
```
