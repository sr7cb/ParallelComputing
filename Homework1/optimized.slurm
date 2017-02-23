#!/bin/bash
#SBATCH --nodes=1             
#SBATCH --ntasks=1            
#SBATCH --time=00:30:00       
#SBATCH --partition=eqa-cs6414    
#SBATCH --account=parallelcomputing

./optimized exinput.txt 0.5 
./optimized1 exinput.txt 0.5
./optimized2 exinput.txt 0.5 
./optimized3 exinput.txt 0.5 
