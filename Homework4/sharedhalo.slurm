#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=4
#SBATCH --partition=main
#SBATCH --nodelist=hermes1
#SBATCH --time=03:30:00
#SBATCH --account=parallelcomputing
#SBATCH --output=finalalldata2

#./sharedhalo_optimized 10000 10000 10000 1
#./sharedhalo_optimized 10000 10000 10000 2
#./sharedhalo_optimized 10000 10000 10000 4
#./sharedhalo_optimized 10000 10000 10000 8
#./sharedhalo_optimized 10000 10000 10000 16
#./sharedhalo_optimized 10000 10000 10000 32
./sharedhalo_optimized 10000 10000 10000 64
