import sys,os

sys.dont_write_bytecode = True
jobs = os.listdir("../jobs")

if input(f"Are you sure to launch {len(jobs)} jobs [y/n]? ") != "y":
    exit(0)

for job in jobs:
    os.system(f"sbatch --wckey=rop --requeue ../jobs/{job}")

