import sys, os, numpy as np, json, matplotlib.pyplot as plt, math #type: ignore
from dotenv import load_dotenv # type: ignore

sys.dont_write_bytecode = True

#EDITABLE PARS
NUM_RANGES = 100

def MIPgap(opt,inc_val):
    if inc_val == None : return None

    if inc_val == "NO SOL" or opt*inc_val<0:
        return 1
    elif opt == inc_val : 
        return 0
    else:
        return abs(opt - inc_val) / max(abs(opt), abs(inc_val))

def createDict(filename, dataDict, numRanges):
    with open(filename, 'r') as file:
            JSdata = json.load(file)
            for inst in JSdata:
                for algo in JSdata[inst]:
                    if algo == "_obj": continue
                    dataDict.update({ algo : np.zeros(numRanges+1)})
                break;

def main(pipeline):

    load_dotenv("../../.ACSenv")
    filename = os.environ.get('ACS_JSOUT_FILENAME')
    outputfile = os.environ.get('ACS_MIP_PP')

    instances = 0

    succDict = {}
    createDict(filename,succDict,NUM_RANGES)

    with open(filename, 'r') as file:
            JSdata = json.load(file)

            for inst in JSdata:
                instances+=1
                obj = JSdata[inst]["_obj"]
                for algo in JSdata[inst]:
                    if algo == "_obj":continue

                    if JSdata[inst][algo][0] == "NO SOL":
                        succDict[algo][NUM_RANGES]+=1
                    else: 
                        discrMIP = math.ceil(MIPgap(obj,JSdata[inst][algo][0])*NUM_RANGES)
                        succDict[algo][discrMIP]+=1
    

    for algo in succDict:
         succDict[algo] = np.cumsum(succDict[algo])/instances

    x_values = np.arange(0,NUM_RANGES+1,1)

    plt.figure(figsize=(12, 8))
    for algo, succ_array in succDict.items():
        plt.plot(x_values, succ_array, label=algo, linewidth=2)

    plt.xlabel('MIG Gap (%)')
    plt.ylabel('Success Rate')
    plt.title('ACS VS CPLEX MIP Gap Success Rate')
    plt.grid(True, linestyle='--', alpha=0.5)
    plt.legend()

    plt.xlim(1, x_values[-1])

    plt.tight_layout()
    
    if not pipeline :
        if input(f"Do you want to collect and save the succes-rate plot on {outputfile} file [y/n]? ") != "y":
            exit(0)

    plt.savefig(outputfile, dpi=300)
    
if __name__ == "__main__":
    if len(sys.argv) == 1:
        main(False)
    else: main(sys.argv[1])
