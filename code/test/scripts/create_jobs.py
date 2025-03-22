import sys,os

sys.dont_write_bytecode = True

import pandas as pd
from pathlib import Path

def clear_dir(dir: Path):
    for item in dir.iterdir():
        if item.is_file() or item.is_symlink():
            item.unlink()
        elif item.is_dir():
            shutil.rmtree(item)

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

    programms = ["ACS"]

    # jobs
    jobs_folder = "../jobs"
    jobs_outputs = "../jobs_output"

    exec_dir = "../../build"

    print(f"Jobs folder: {jobs_folder}")
    print(f"Jobs output: {jobs_outputs}")
    print(f"ACS Executable: {exec_dir}/CPLEXRun")
    print(f"CPLEX Executable: {exec_dir}/CPLEXRun")

    if input("Insert y if it's all correct: ") != "y":
            exit(1)

    os.makedirs(jobs_folder, exist_ok=True)
    os.makedirs(jobs_outputs, exist_ok=True)
    clear_dir(Path(jobs_outputs))
    
    count =0
    for instance in instances:
        for exe in  ["CPLEXRun","ACS"]:
            if exe == "ACS":
                for rho in [0.10,0.25,0.50,0.75,0.9]:
                    for seed in [2120934,3409212,240931]:
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
#SBATCH --error={jobs_outputs}/{job_name}.out

# warm up processors
sudo cpupower frequency-set -g performance
sleep 0.1
stress-ng -c 4 --cpu-ops=100
# set limits
ulimit -v 16777216

#####################
cd {exec_dir}
./{exe} -f {instance} -tl 300 -th 0.25 -nSMIPs 4 -rh {rho} -sd {seed}

#####################

# back to power saving mode
sudo cpupower frequency-set -g powersave"""

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
#SBATCH --error={jobs_outputs}/{instance}_{exe}.out

# warm up processors
sudo cpupower frequency-set -g performance
sleep 0.1
stress-ng -c 4 --cpu-ops=100
# set limits
ulimit -v 16777216

#####################
cd {exec_dir}
./{exe} -f {instance} -tl 300

#####################

# back to power saving mode
sudo cpupower frequency-set -g powersave"""

                with open(job, "w") as f:
                    f.write(job_content)
                count+=1
    print(f"Generated {count} files")



if __name__ == "__main__":
    main()


















































'''

def clear_dir(dir: Path):
    for item in dir.iterdir():
        if item.is_file() or item.is_symlink():
            item.unlink()
        elif item.is_dir():
            shutil.rmtree(item)


def main():
    # verify input correctness
    if len(sys.argv) <= 1 or sys.argv[1] in ["-h", "--h", "-help", "--help"]:
        print(
            f"Usage: >> python3 {os.path.basename(__file__)} <instances_folder> <all parameters to pass to the execution>."
        )
        exit(0)

    # read inputs
    instances_folder = os.path.abspath(sys.argv[1])
    execution_parameters = " ".join(sys.argv[2:])

    # verify inputs
    if (
        input(
            f"The instances will be taken from the folder {instances_folder}\nInsert y if it's correct: "
        )
        != "y"
    ):
        exit(0)
    if (
        input(
            f"The execution will have the following structure:\n>> ./exec -f <instance> <pars>\nInsert y if it's correct: "
        )
        != "y"
    ):
        exit(0)

    instances_list = os.listdir(instances_folder)

    code_dir = os.path.abspath("../..")

    # jobs
    jobs_folder = os.path.join(code_dir, "test/jobs")
    jobs_outputs = os.path.join(code_dir, "test/jobs_output")

    exec_dir = os.path.join(code_dir, "build")

    print(f"Jobs folder: {jobs_folder}")
    print(f"Jobs output: {jobs_outputs}")
    print(f"Executable: {exec_dir}/CPLEXRun")
    if input("Check those paths.\nInsert y if it's all correct: ") != "y":
        exit(0)

    os.makedirs(jobs_folder, exist_ok=True)
    os.makedirs(jobs_outputs, exist_ok=True)
    clear_dir(Path(jobs_outputs))

    for inst in instances_list:

        job = f"{instances_folder}/{inst.replace('.mps', '_job.sh')}"
        job_content = f"""#!/bin/bash
#SBATCH --job-name={inst}_CPLEXRun
#SBATCH --partition=arrow
#SBATCH --ntasks=1
#SBATCH --mem=20GB
#SBATCH --time=00:10:00
#SBATCH --output={jobs_outputs}/{inst}.out
#SBATCH --error={jobs_outputs}/{inst}.out

# warm up processors
sudo cpupower frequency-set -g performance
sleep 0.1
stress-ng -c 4 --cpu-ops=100
# set limits
ulimit -v 16777216

#####################

{exec_dir}/./CPLEXRun -f {instances_folder}/{inst} -t 300 

#####################

# back to power saving mode
sudo cpupower frequency-set -g powersave"""

        with open(job, "w") as f:
            f.write(job_content)


if __name__ == "__main__":
    main()
    '''
