#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=1096
#SBATCH --time=00:15:00 
#SBATCH --part=training
#SBATCH --account=parallelcomputing
#SBatch --output=script_output

# the number of frames to be used to generate the video
#frame_count=$5

# load the renderer engine that will generate frames for the video 

# -b option is for command line access, i.e., without an output console
# -s option is the starting frame
# -e option is the ending frame
# -a option indicates that all frames should be rendered 
blender -b Star-collapse-ntsc.blend -s 1 -e 1 -a

# need to give the generated frames some extension; otherwise the video encoder will not work
#ls star-collapse-* | xargs -I % mv % %.jpg

# load the video encoder engine
#module load ffmpeg

# start number should be 1 as by default the encoder starts looking from file ending with 0
# frame rate and start number options are set before the input files are specified so that the
# configuration is applied for all files going to the output
#ffmpeg -framerate 25 -start_number 1 -i star-collapse-%04d.jpg -vcodec mpeg4 output.avi


