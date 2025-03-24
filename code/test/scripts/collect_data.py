import os, sys, re

sys.dont_write_bytecode = True

out_folder = "../jobs_output"
outs = os.listdir(out_folder)
data_folder = "data/"


def extract_value_from_line(line):
    if "NO FEASIBLE SOLUTION" in line:
        return "NO SOLUTION"
    elif "BEST INCUMBENT:" in line:
        # Extract the value after "BEST INCUMBENT:"
        match = re.search(r'BEST INCUMBENT:\s+(\d+\.?\d*)', line)
        if match:
            return float(match.group(1))
    return None

def main():
    for out in outs:
        with open(out_folder+"/"+out) as f1:
            lines = f1.readlines()

        print(lines[-1])
        #print(extract_value_from_line(lines[-1]))
        #add filename, value in an array
        #read last line lines[-1]
        #create pandas and merge

if __name__ == "__main__":
    main()