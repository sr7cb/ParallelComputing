#!/bin/bash
#SBATCH --nodes=1             
#SBATCH --ntasks=1            
#SBATCH --time=00:30:00       
#SBATCH --partition=eqa-cs6414    
#SBATCH --account=parallelcomputing

./original exinput.txt 0.5
./original1 exinput.txt 0.5
./original2 exinput.txt 0.5
./original3 exinput.txt 0.5
