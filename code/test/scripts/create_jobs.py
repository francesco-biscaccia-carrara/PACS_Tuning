import sys,os

sys.dont_write_bytecode = True

import pandas as pd

def main():
    input_csv = "fHard_instances.csv"
    print(f"Input file: {input_csv}")
    if (
            input(
                f"Insert y if the file is correct: "
            )
            != "y"
    ): exit(1)

    df = pd.read_csv(input_csv)

    instances = df["Instance"];
    
    # jobs
    jobs_folder = "../jobs"
    jobs_outputs = "../jobs_output"

    exec_dir = "../../build"

    print(f"Jobs folder: {jobs_folder}")
    print(f"Jobs output: {jobs_outputs}")
    print(f"ACS Executable: {exec_dir}/ACS")
    print(f"CPLEX Executable: {exec_dir}/CPLEXRun")

    if input("Insert y if it's all correct: ") != "y":
            exit(1)

    os.makedirs(jobs_folder, exist_ok=True)
    os.makedirs(jobs_outputs, exist_ok=True)
    
    count =0
    for instance in instances:
        for exe in  ["CPLEXRun","ACS"]:
            if exe == "ACS":
                for rho in [0.25,0.50,0.75]:
                    for seed in [10493847, 83274910, 70938475, 98312048, 19283746]:
                        job_name =f"{instance}_{exe}_{rho}_{seed}"
                        job = f"{jobs_folder}/{job_name}"
                        job_content = f"""#!/bin/bash
#SBATCH --job-name={job_name}
#SBATCH --partition=arrow
#SBATCH --ntasks=1
#SBATCH --mem=14GB
#SBATCH --cpus-per-task=4
#SBATCH --time=00:10:00
#SBATCH --output={jobs_outputs}/{job_name}.out
#SBATCH --error={jobs_outputs}/{job_name}.err

# warm up processors
sudo cpupower frequency-set -g performance
sleep 0.1
stress-ng -c 4 --cpu-ops=100
# set limits
ulimit -v 16777216

#####################
echo "-----"
echo "INFO:"
echo "  SLURM_NODELIST: $SLURM_NODELIST"
echo "  SLURM_CPUS_PER_TASK: $SLURM_CPUS_PER_TASK"
echo "-----"

cd {exec_dir}
./{exe} -f {instance} -tl 300 -th 0.25 -nSMIPs 4 -rh {rho} -sd {seed}

#####################

# back to power saving mode
sudo cpupower frequency-set -g powersave

"""

                        with open(job, "w") as f:
                            f.write(job_content)
                        count+=1
            else: 
                job = f"{jobs_folder}/{instance}_{exe}"
                job_content = f"""#!/bin/bash
#SBATCH --job-name={instance}_{exe}
#SBATCH --partition=arrow
#SBATCH --ntasks=1
#SBATCH --mem=14GB
#SBATCH --cpus-per-task=4
#SBATCH --time=00:10:00
#SBATCH --output={jobs_outputs}/{instance}_{exe}.out
#SBATCH --error={jobs_outputs}/{instance}_{exe}.err

# warm up processors
sudo cpupower frequency-set -g performance
sleep 0.1
stress-ng -c 4 --cpu-ops=100
# set limits
ulimit -v 16777216

#####################
echo "-----"
echo "INFO:"
echo "  SLURM_NODELIST: $SLURM_NODELIST"
echo "  SLURM_CPUS_PER_TASK: $SLURM_CPUS_PER_TASK"
echo "-----"

cd {exec_dir}
./{exe} -f {instance} -tl 300

#####################

# back to power saving mode
sudo cpupower frequency-set -g powersave

"""

                with open(job, "w") as f:
                    f.write(job_content)
                count+=1
    print(f"Generated {count} files")



if __name__ == "__main__":
    main()
