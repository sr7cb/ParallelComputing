#!/bin/bash
#SBATCH --nodes=10
#SBATCH --ntasks-per-node=20
#SBATCH --partition=eqa-cs6414
#SBATCH --time=01:30:00
#SBATCH --mem-per-cpu=1000
#SBATCH --account=parallelcomputing
#SBATCH --output=200procs1ipc1bt
module	load openmpi/gcc/4.8.5/2.0.0

mpiexec ./halo_optimized 10000 10000 1 1 10000 1
