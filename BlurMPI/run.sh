ITERATIONS=3

pwd
rm $3

for K in 3 5 7 9 11 13
do
  for THREADS in 1 2 4 8
  do
    echo "THREADS: $THREADS and Kernel $K"
    for (( i=0; i<$ITERATIONS; i++ ))
    do
      echo "Iteration: $i"
      COMMAND="mpirun -np $THREADS --hostfile ../../mpi_hosts blur_effect_mpi $1 $2 $K"
      `echo $COMMAND` >> $3
      echo $THREADS >> $3
      echo $K >> $3
      rm $2
    done
  done
done

# mpirun -np 8 --hostfile mpi_hosts blur_effect_mpi img/test720.jpg out/test720.png 3