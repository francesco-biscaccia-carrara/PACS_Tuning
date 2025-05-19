import sys, os, numpy as np, json, matplotlib.pyplot as plt #type: ignore
from dotenv import load_dotenv # type: ignore

sys.dont_write_bytecode = True

#EDITABLE PARS
algorithm_names={
         "CPLEX":"CPLEX",
         "0":"ACS_Rho_0.1",
         "1":"ACS_Rho_0.25",
         "2":"ACS_Rho_0.5",
         "3":"ACS_Rho_0.75",
         "4":"ACS_Rho_0.9"
    }

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
    instances = 0
    with open(filename, 'r') as file:
            JSdata = json.load(file)
            for inst in JSdata: instances+=1

    with open(filename, 'r') as file:
            JSdata = json.load(file)
            for inst in JSdata:
                for algo in JSdata[inst]:
                    if algo == "_obj": continue
                    dataDict.update({ algo : np.ones((instances, numRanges))})
                break;

def main(pipeline):

    load_dotenv("../../.ACSenv")
    filename = os.environ.get('ACS_JSOUT_FILENAME')
    outputfile = os.environ.get('ACS_MIP_PP')
    TL = 300
    DISCR_FACT = TL/NUM_RANGES

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

                    if JSdata[inst][algo][0] == "NO SOL": continue
                    else: 
                        discTime = int(JSdata[inst][algo][1]/DISCR_FACT)
                        succDict[algo][instances][discTime] = MIPgap(obj,JSdata[inst][algo][0])
    

    for algo in succDict:
        for row in succDict[algo]:
            retInd = 0
            for i in range(0,len(row)):
                if row[i] == 1 : continue
                else: 
                    retInd = i
            for i in range(retInd,len(row)):
                row[i]= row[retInd]          
                
        succDict[algo] = np.mean(succDict[algo],axis=0) #mean by col
    
    x_values = np.arange(DISCR_FACT,TL+DISCR_FACT,DISCR_FACT)

    plt.figure(figsize=(12, 8))
    for algo, succ_array in succDict.items():
        plt.plot(x_values, succ_array, label=algorithm_names[algo], linewidth=2)

    plt.xlabel('Time Steps')
    plt.ylabel('Average MIP Gap')
    plt.title('ACS VS CPLEX MIP Gap')
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
