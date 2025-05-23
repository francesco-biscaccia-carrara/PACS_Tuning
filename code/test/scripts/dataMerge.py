import sys, os, numpy as np, json
from dotenv import load_dotenv # type: ignore

sys.dont_write_bytecode = True

def main(pipeline):

    load_dotenv("../../.ACSenv")
    filename = os.environ.get('ACS_JSON_FILENAME')

    with open(filename, 'r') as file:
        JSdata = json.load(file)

    tmpfolder = "tmp"
    JSouts = os.listdir(tmpfolder)
    for out in JSouts:
        with open("tmp/"+out, 'r') as file:
            dataOut = json.load(file)

            for inst in dataOut:
                for algo in dataOut[inst]:
                    if algo == "CPLEX":
                        JSdata[inst][algo]=dataOut[inst][algo]
                    else:
                        for seed in dataOut[inst][algo]:
                            JSdata[inst][algo][seed]=dataOut[inst][algo][seed]
    
    if not pipeline :
        if input(f"Do you want to merge the data and save the result on {filename} file [y/n]? ") != "y":
            exit(0)
    
    with open(filename, 'w', encoding='utf-8') as f:
        json.dump(JSdata, f, ensure_ascii=False, indent=4)
    print(f"Updated {filename} JSON file")

    if not pipeline :
        if input(f"Do you want to clear `tmp` folder [y/n]? ") != "y":
            exit(0)

    os.system(f"rm {tmpfolder}/*")
    if len(os.listdir(tmpfolder)) == 0:
        print(f"Folder `{tmpfolder}` cleaned up!")
    
    
if __name__ == "__main__":
    if len(sys.argv) == 1:
        main(False)
    else: main(sys.argv[1])
