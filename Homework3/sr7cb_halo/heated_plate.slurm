#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --partition=training
#SBATCH --time=00:30:00
#SBATCH --mem-per-cpu=1000
#SBATCH --account=parallelcomputing

module load gcc/5.4.0

./heated_plate 10000 10000 10000
