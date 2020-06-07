mpicc blur_effect_mpi.c -o blur_effect_mpi -lm
mpirun -np 2 blur_effect_mpi img/test4k.jpg test.png 7