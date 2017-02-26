#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=1096
#SBATCH --time=00:15:00 
#SBATCH --part=training    
#SBATCH --account=parallelcomputing

module load blender/2.70

#Sanil Rao 2/25/17
#Creating specific frames for load balancing in this case the 10  job trial SR
i=$1 #which job to be run SR
end=$[$i+240] # frame at which job will finish SR

while [ $i -le $end ] #loop over all possible frames creating each specific one SR
do
time blender -b Star-collapse-ntsc.blend -s $i -e $i -a
i=$[$i+10] #iterates by the total number of jobs
done
