import os,time

jobs = os.listdir("../jobs")

if (
            input(
                f"Are you sure to launch {len(jobs)} jobs [y/n]? "
            )
            != "y"
    ): exit(1)

for job in jobs:
    os.system(f"sbatch --wckey=rop --requeue ../jobs/{job}")
    time.sleep(0.5)

