#Sanil Rao sr7cb 2/25/17 
#initial job launcher for a single 250 blender job and creation of the movie
job_num=1 #number of jobs
i=1 #iterator
start_frame=1; #starting frame for each blend job
end_frame=250; #ending frame for each blend job
while [ $i -le $job_num ] #iterate over number of jobs in this case 1 job
do
sbatch test3.sh $start_frame $end_frame #run the command to create the job
i=$[$i+1]
#start_frame=$[$start_frame+25] #icrementor if spreading frames out for both starting and ending frame
#end_frame=$[$end_frame+25] 
done

#determines when the script completes running in order to generate the timing report
#looks at the completed star files and compares to the total amount that could be generated
num_frames=0
while [ $num_frames -lt 249 ]
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
ffmpeg -framerate 25 -start_number 1 -i "star-collapse-%04d.jpg" -vcodec ".mpeg4" output.avi


