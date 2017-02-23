#!/bin/bash
#SBATCH --nodes=10
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=1096
#SBATCH --time=00:15:00 
#SBATCH --part=training
#SBATCH --account=parallelcomputing


#i=1
#stopvalue=225
#i2=25

#while [ $i -le $stopvalue ]
#do
#time blender -b Star-collapse-ntsc.blend -s $i -e $i2 -a
#i=$[$i+25]
#i2=$[i2+25]
#done
time blender -b Star-collapse-ntsc.blend -s 1 -e 25 -a
time blender -b Star-collapse-ntsc.blend -s 26 -e 50 -a
time blender -b Star-collapse-ntsc.blend -s 51 -e 75 -a
time blender -b Star-collapse-ntsc.blend -s 76 -e 100 -a
time blender -b Star-collapse-ntsc.blend -s 101 -e 125 -a
time blender -b Star-collapse-ntsc.blend -s 126 -e 150 -a
time blender -b Star-collapse-ntsc.blend -s 151 -e 175 -a
time blender -b Star-collapse-ntsc.blend -s 176 -e 200 -a
time blender -b Star-collapse-ntsc.blend -s 201 -e 225 -a
time blender -b Star-collapse-ntsc.blend -s 226 -e 250 -a
