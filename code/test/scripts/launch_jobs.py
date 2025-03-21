import os

jobs = os.listdir("../job_outputs")

for job in jobs:
    print(f"sbatch --wckey=rop --requeue {job}")
#    os.system(f"sbatch --wckey=rop --requeue {job}")