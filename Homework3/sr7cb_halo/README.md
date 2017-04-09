To run the makefile first make sure you first do a module load openmpi/gcc/4.8.5/2.0.0
The haloX.slurm files are currently set to 200 processors to change that amount to whatever you want to run it on.
The run cmds file should run all 9 jobs at once and all you have to change is processor amount
if you want to use a different executable you would have to do change halo optimized to halo snap
to run the serial code use the serial slurm file. 

