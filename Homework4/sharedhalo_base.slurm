#!/bin/bash
#SBATCH --nodes=3
#SBATCH --ntasks-per-node=4
#SBATCH --partition=main
#SBATCH --nodelist=hermes[1,2,3]
#SBATCH --time=04:30:00
#SBATCH --account=parallelcomputing
#SBATCH --output=basedata

./sharedhalo_base 10000 10000 10000 1
./sharedhalo_base 10000 10000 10000 2
./sharedhalo_base 10000 10000 10000 4
./sharedhalo_base 10000 10000 10000 8
./sharedhalo_base 10000 10000 10000 16
./sharedhalo_base 10000 10000 10000 32
./sharedhalo_base 10000 10000 10000 64
