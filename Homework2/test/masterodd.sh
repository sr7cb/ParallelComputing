#Sanil Rao 2/25/2017 
#Creates the frames for an undivisable number of jobs by having a grabage collection of the last few frames
job_num=80 #total numbers of jobs
i=1 #iterator 
start_frame=1; #beginning frame
end_frame=3; #ending frame
while [ $i -le $job_num ] #creates the frame
do
sbatch test3.sh $start_frame $end_frame
i=$[$i+1]
start_frame=$[$start_frame+3] #iterates by the floor of total number of frames by total number of jobs
end_frame=$[$end_frame+3] 
done
sbatch test3.sh 241 250 #extra frames
num_frames=0


#determines when the script completes running in order to generate the timing report
#looks at the completed star files and compares to the total amount that could be generated

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
ffmpeg -framerate 25 -start_number 1 -i star-collapse-%%04d.jpg -vcodec output.mpeg4 


