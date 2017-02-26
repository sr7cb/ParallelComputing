#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=1096
#SBATCH --time=00:15:00 
#SBATCH --part=training    
#SBATCH --account=parallelcomputing

#Sanil Rao 2/25/17
#instance of jobs that will be run on the compute nodes for the sequential part of the assignment SR

module load blender/2.70

time blender -b Star-collapse-ntsc.blend -s $1 -e $2 -a #runs the job using command line parameters SR
