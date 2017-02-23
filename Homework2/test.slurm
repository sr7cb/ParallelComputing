#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=1096
#SBATCH --time=00:15:00 
#SBATCH --part=training
#SBATCH --account=parallelcomputing
#SBATCH --output=script-output

module load blender/2.70


./slurm.sh
