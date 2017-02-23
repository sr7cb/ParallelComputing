#!/bin/bash
#SBATCH --nodes=10
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=1096
#SBATCH --time=00:15:00 
#SBATCH --part=training    
#SBATCH --account=parallelcomputing

module load blender/2.70

#i=1
#stopvalue=241
#i2=10

#while [ $i -le $stopvalue ]
#do
time blender -b Star-collapse-ntsc.blend -s 1 -e 250 -a
#i=$[$i+10]
#i2=$[i2+10]
#done

