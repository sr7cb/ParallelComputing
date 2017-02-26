#Sanil Rao 2/25/17
# script to create the jobs for the optimized version of the code 
job_num=10 # total number of jobs
i=1 
#iterates over the number of jobs and creates them for the compute nodes
while [ $i -le $job_num ]
do
sbatch test4.sh $i 
i=$[$i+1]
done

#determines when the script completes running in order to generate the timing report
#looks at the completed star files and compares to the total amount that could be generated
num_frames=0
while [ $num_frames -lt 250 ]
do
num_frames=$(ls star-collapse-* | wc -l) 
sleep 1
done
#Creates the final movive by stitching all the frames together

ls star-collapse-* | xargs -I % mv % %.jpg

# load the video encoder engine
module load ffmpeg

# start number should be 1 as by default the encoder starts looking from file ending with 0
# frame rate and start number options are set before the input files are specified so that the
# configuration is applied for all files going to the output
ffmpeg -framerate 25 -start_number 1 -i "star-collapse-%04d.jpg" -vcodec "output.mpeg4"


