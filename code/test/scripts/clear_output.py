import os, sys

sys.dont_write_bytecode = True

out_folder = "../jobs_output"
outs = os.listdir(out_folder)
data_folder = "data/"

def clear_file(filename):
    with open(out_folder+"/"+filename) as f1:
        lines = f1.readlines()

    with open(out_folder+"/"+filename, 'w') as f2:
        f2.writelines(lines[:-2])

def main():
    if (
            input(
                f"You are modifing {len(outs)} files, continue [y/n]? "
            )
            != "y"
    ): exit(1)

    for out in outs:
        clear_file(out)


if __name__ == "__main__":
    main()
