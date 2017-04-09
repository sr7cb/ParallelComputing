#!/bin/bash

module load openmpi/gcc/4.8.5/2.0.0
mpicc -O3 -o halo halo.c

sbatch halo.slurm
sbatch halo2.slurm
sbatch halo3.slurm
sbatch halo4.slurm
sbatch halo5.slurm
sbatch halo6.slurm
sbatch halo7.slurm
sbatch halo8.slurm
sbatch halo9.slurm
