#Sanil Rao sr7cb 2/25/17 
#runs the nondivisble jobs that require extra garbage collection
job_num=80 #type of jobs
i=1 #iterator
#creates the jobs on the compute node
while [ $i -le $job_num ]
do
sbatch test4odd.sh $i $job_num
i=$[$i+1]

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


