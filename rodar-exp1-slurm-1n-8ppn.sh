#!/bin/bash
#SBATCH --exclusive
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=8
#SBATCH --job-name=mpimulti_partition_job
echo -----------------------------------
echo -----------------------------------
echo -----------------------------------
echo ---------- ATENCAO: usar --exclusive na linha de comando
echo -----------------------------------

#SBATCH --output=saida.out

echo "$0 rodando no host " `hostname`  
echo "$0 rodando no host " `hostname` >saida.txt
echo "$0 rodando no host " `hostname` >output.txt

echo "SLURM_JOB_NAME: " $SLURM_JOB_NAME 
echo "SLURM_NODELIST: " $SLURM_NODELIST 
echo "SLURM_JOB_NODELIST: " $SLURM_JOB_NODELIST
echo "SLURM_JOB_CPUS_PER_NODE: " $SLURM_JOB_CPUS_PER_NODE

# Load necessary modules (adjust paths as needed)
#module load openmpi/4.1.4
NTIMES=10
echo "nt " $NTIMES
MAX_EXECS=5
echo "MAX_EXECS " $MAX_EXECS

    for j in $(seq 1 $NTIMES);
    do
        if [ $j -le $MAX_EXECS ];
        then 
            # Run the MPI program
            #mpirun ./multi-partition 8000000
            outuput=$(mpirun ./multi-partition 8000000)
            #tempo=$(mpirun ./multi-partition 8000000 | awk '/Total time:/ {print $3}')
            tempo=$(echo $outuput | awk '/Total time:/ {print $3}')
            throughput=$(echo -e "$outuput" | awk '/Throughput:/ {print $2}')
            echo "$outuput " >> output.txt
            echo "$tempo " >> saida.txt
            echo "$tempo segundos, $throughput MEPS"

        else
          echo "nao" | tee -a saida.txt 
        fi  
    done

echo "O tempo total dessa shell foi de" $SECONDS "segundos"
echo "SLURM_JOB_NAME: " $SLURM_JOB_NAME 
echo "SLURM_NODELIST: " $SLURM_NODELIST 
echo "SLURM_JOB_NODELIST: " $SLURM_JOB_NODELIST
echo "SLURM_JOB_CPUS_PER_NODE: " $SLURM_JOB_CPUS_PER_NODE
#imprime infos do job slurm (e.g. TEMPO at       aqui no fim do job)
squeue -j $SLURM_JOBID
