#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=10
#SBATCH --ntasks-per-node=10
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=1096
#SBATCH --time=00:15:00 
#SBATCH --part=training    
#SBATCH --account=parallelcomputing

module load blender/2.70

time blender -b Star-collapse-ntsc.blend -s 1 -e 250 -a

