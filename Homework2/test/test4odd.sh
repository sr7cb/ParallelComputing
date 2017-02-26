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
#iterates over non divisible jobs such taht one of the jobs cans garbage collect the last few frames


i=$1 # which job number SR
end=$[$i+160] # which frame that job will end at SR
job_num=$2 # total number of jobs used to determine the next frame to render SR
while [ $i -le $end ]
do
time blender -b Star-collapse-ntsc.blend -s $i -e $i -a
i=$[$i+$job_num]
done

#Garbage collection used to grab all excess jobs to complete the project
if [ $1 -eq $job_num ]; then
time blender -b Star-collapse-ntsc.blend -s 241 -e 250 -a
fi
