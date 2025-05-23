import sys, os, numpy as np, json, matplotlib.pyplot as plt #type: ignore
from dotenv import load_dotenv # type: ignore

sys.dont_write_bytecode = True

#EDITABLE PARS
NUM_RANGES = 100

def createDict(filename, dataDict, numRanges):
     with open(filename, 'r') as file:
            JSdata = json.load(file)
            for inst in JSdata:
                for algo in JSdata[inst]:
                    if algo == "_obj": continue
                    dataDict.update({ algo : np.zeros(numRanges)})
                break;

def main(pipeline):

    load_dotenv("../../.ACSenv")
    filename = os.environ.get('ACS_JSOUT_FILENAME')
    outputfile = os.environ.get('ACS_SUCCRATE_PP')

    TL = 300
    DISCR_FACT = TL/NUM_RANGES

    instances = 0

    succDict = {}
    createDict(filename,succDict,NUM_RANGES)


    with open(filename, 'r') as file:
            JSdata = json.load(file)

            for inst in JSdata:
                instances+=1
                for algo in JSdata[inst]:
                    if algo == "_obj": continue
                    if JSdata[inst][algo][0] == "NO SOL": continue
                    else: 
                        discTime = int(JSdata[inst][algo][1]/DISCR_FACT)
                        succDict[algo][discTime]+=1

    for algo in succDict:
         succDict[algo] = np.cumsum(succDict[algo])/instances

    x_values = np.arange(DISCR_FACT,TL+DISCR_FACT,DISCR_FACT)

    plt.figure(figsize=(12, 8))
    for algo, succ_array in succDict.items():
        plt.plot(x_values, succ_array, label=algo, linewidth=2)

    plt.xlabel('Time Steps')
    plt.ylabel('Success Rate')
    plt.title('ACS vs CPLEX Success Rate')
    plt.grid(True, linestyle='--', alpha=0.5)
    plt.legend()

    plt.xlim(DISCR_FACT, x_values[-1])

    plt.tight_layout()
    

    if not pipeline :
        if input(f"Do you want to collect and save the succes-rate plot on {outputfile} file [y/n]? ") != "y":
            exit(0)

    plt.savefig(outputfile, dpi=300)
    
if __name__ == "__main__":
    if len(sys.argv) == 1:
        main(False)
    else: main(sys.argv[1])
