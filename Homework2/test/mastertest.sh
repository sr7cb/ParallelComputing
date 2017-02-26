
job_num=1
i=1
start_frame=1;
end_frame=13;
while [ $i -le $job_num ]
do
sbatch test3.sh $start_frame $end_frame
i=$[$i+1]
start_frame=$[$start_frame+25]
end_frame=$[$end_frame+25]
done

num_frames= $(jobq -u sr7cb | wc -l)
echo $num_frames
