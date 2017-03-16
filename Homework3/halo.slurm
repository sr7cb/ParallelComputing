#!/bin/bash
#SBATCH --nodes=4
#SBATCH --ntasks-per-node=1
#SBATCH --partition=training
#SBATCH --time=00:30:00
#SBATCH --mem-per-cpu=1000
#SBATCH --account=parallelcomputing

module	load openmpi/gcc/4.8.5/2.0.0

mpiexec ./halo 16 16 1 1 1 1 
