mpicc blur_effect_mpi.c -o blur_effect_mpi -lm
mpirun -np 8 blur_effect_mpi img/test4j.jpg test.png 7