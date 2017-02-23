#!/bin/bash
#SBATCH --nodes=1             
#SBATCH --ntasks=1            
#SBATCH --time=00:30:00       
#SBATCH --partition=eqa-cs6414    
#SBATCH --account=parallelcomputing

./generate_input 20000 > exinput.txt
