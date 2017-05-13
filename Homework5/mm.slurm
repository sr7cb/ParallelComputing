#!/bin/bash
#SBATCH --nodes=1
#SBATCH --partition=main
#SBATCH --nodelist=artemis2
#SBATCH --time=03:30:00
#SBATCH --account=parallelcomputing
#SBATCH --output=threadblock2
#SBATCH --gres=gpu 

./mm 10000
