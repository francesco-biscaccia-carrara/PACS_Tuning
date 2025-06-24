import sys, pandas as pd, json, datetime, os
from dotenv import load_dotenv # type: ignore

sys.dont_write_bytecode = True

#EDITABLE PARS
ALGOS = (0, 1, 2)
SEEDS = (38472910, 56473829, 27384910, 91827364, 83746592)

def generate_env_file(env_vars, file_path="../../.ACSenv"):
    with open(file_path, "w") as f:
        for key, value in env_vars.items():
            f.write(f"{key}={value}\n")
    
    print(f"Environment file created at {file_path}")


def main():
    date = datetime.datetime.now()

    dataFolder = "data/"
    newData = dataFolder+"{m}_{d}_{y}".format(m=date.strftime("%m"),d=date.strftime("%d"),y=date.strftime("%Y"))
    os.makedirs(newData, exist_ok=True)
    
    envVars = {
        "ACS_JSON_FILENAME": newData+"/{m}_{d}_{y}-raw.json".format(m=date.strftime("%m"),d=date.strftime("%d"),y=date.strftime("%Y")),
        "ACS_JSOUT_FILENAME": newData+"/{m}_{d}_{y}.json".format(m=date.strftime("%m"),d=date.strftime("%d"),y=date.strftime("%Y")),
        "ACS_SUCCRATE_PP":newData+"/{m}_{d}_{y}-SuccRatePlot.svg".format(m=date.strftime("%m"),d=date.strftime("%d"),y=date.strftime("%Y")),
        "ACS_MIP_PP": newData+"/{m}_{d}_{y}-MIPGapPlot.svg".format(m=date.strftime("%m"),d=date.strftime("%d"),y=date.strftime("%Y"))
    }
    print("--------------------------")
    generate_env_file(env_vars=envVars)
    print("--------------------------")

    load_dotenv("../../.ACSenv")
    filename = os.environ.get('ACS_JSON_FILENAME')
    input_csv = "data/fHard_instances.csv"

    df = pd.read_csv(input_csv)

    instances = df["Instance"]
    objs = df["Objective"]
    
    jobs_folder = "../jobs"
    jobs_outputs = "../jobs_output"
    exec_dir = "../../build"

    print(f"Input file: {input_csv}")
    print(f"Jobs folder: {jobs_folder}")
    print(f"Jobs output: {jobs_outputs}")
    print(f"ACS Executable: {exec_dir}/ACS")
    print(f"CPLEX Executable: {exec_dir}/CPLEXRun")

    if input("Is this info correct [y/n]? ") != "y":
        exit(0)
    print("--------------------------")

    if input(f"Are you sure to clean up jobs folder ({len(os.listdir(jobs_folder))} jobs) [y/n]? ") == "y":
        os.system(f"rm {jobs_folder}/*")

    if len(os.listdir(jobs_folder)) == 0:
        print(f"Folder `{jobs_folder}` cleaned up!")
    print("--------------------------")

    os.makedirs(jobs_folder, exist_ok=True)
    os.makedirs(jobs_outputs, exist_ok=True)
    
    if input("Do you want to generate the jobs [y/n]? ") != "y":
        exit(0)
    
    count =0
    indexObj= 0
    jsonInst ={}
    for instance in instances:
        jsonInst.update({instance:{}})
        
        for exe in  ["CPLEXRun","ACS"]:
            if exe == "ACS":
                for rho in ALGOS:
                    jsonInst[instance].update({"_obj": objs[indexObj], rho: {}})
                    for seed in SEEDS:
                        jsonInst[instance][rho].update({seed: [None,None]})
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
sudo cpupower frequency-set -g performance > /dev/null
sleep 0.1
stress-ng -c 4 --cpu-ops=100
# set limits
ulimit -v 16777216

#####################
echo "----------INFO----------"
echo "NODE: $SLURM_NODELIST"
echo "CPUS: $SLURM_CPUS_PER_TASK"
echo "------------------------"

cd {exec_dir}
./{exe} -f {instance} -tl 300 -th 0.25 -nSMIPs 4 -rh 0.25 -sd {seed} -ag {rho}

#####################

# back to power saving mode
sudo cpupower frequency-set -g powersave > /dev/null
"""

                        with open(job, "w") as f:
                            f.write(job_content)
                        count+=1
            else: 
                jsonInst[instance]={
                        'CPLEX': [None,None]
                }
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
sudo cpupower frequency-set -g performance > /dev/null
sleep 0.1
stress-ng -c 4 --cpu-ops=100
# set limits
ulimit -v 16777216

#####################
echo "----------INFO----------"
echo "NODE: $SLURM_NODELIST"
echo "CPUS: $SLURM_CPUS_PER_TASK"
echo "------------------------"

cd {exec_dir}
./{exe} -f {instance} -tl 300

#####################

# back to power saving mode
sudo cpupower frequency-set -g powersave > /dev/null

"""

                with open(job, "w") as f:
                    f.write(job_content)
                count+=1
        indexObj+=1
    
    print(f"Generated {count} files")

    tmpFolder = "tmp"
    os.makedirs(tmpFolder, exist_ok=True)
    print(f"Generated `{tmpFolder}` folder")

    with open(filename, 'w', encoding='utf-8') as f:
         json.dump(jsonInst, f, ensure_ascii=False, indent=4)
    print(f"Generated `{filename}` JSON file")
    print("--------------------------")
 


if __name__ == "__main__":
    main()
