import sys, os, numpy as np, json, matplotlib.pyplot as plt #type: ignore
#from dotenv import load_dotenv # type: ignore
plt.rcParams.update({ 
    "font.family": "serif",     
})
sys.dont_write_bytecode = True

#EDITABLE PARS
NUM_RANGES = 100

def createDict(filename, dataDict, numRanges):
     with open(filename, 'r') as file:
            JSdata = json.load(file)
            for inst in JSdata:
                for algo in JSdata[inst]:
                    if algo == "_obj": continue
                    dataDict.update({ algo : np.zeros(numRanges+1)})
                break;

def main(pipeline):

    #load_dotenv("../../.ACSenv")
    filename = "data/1_ACS_DYN_Res_v1.2.6/ACS_DYN.json"
    outputfile = "data/1_ACS_DYN_Res_v1.2.6/PACS_DYN-MIPSuccRatePlot.pgf"

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

    x_values = np.arange(DISCR_FACT,TL+DISCR_FACT+1,DISCR_FACT)

    plt.figure(figsize=(12, 8))
    integralValues = []
    for algo, succ_array in succDict.items():
        plt.plot(x_values, succ_array, label=algo, linewidth=2)
        integralValues.append([np.trapezoid(succ_array,x_values),algo])    

    plt.xlabel('Computation Time (sec)')
    plt.ylabel('Success Rate')

    plt.grid(True, linestyle='--', alpha=0.5)
    plt.legend()

    plt.xlim(DISCR_FACT, x_values[-1])

    plt.tight_layout()
    plt.savefig(outputfile, backend="pgf")
    plt.legend(loc='lower right')
    plt.savefig(outputfile[:-4]+".pdf", backend="pdf")

    #integralValues.sort(reverse=True)
    colors = plt.cm.tab10.colors
    txtOut = outputfile[:-4]+".txt"
    with open(txtOut, 'w') as f:
        f.write("--------------------INTEGRAL VALUES--------------------\n")
        for line in integralValues:
            f.write(f"{line[1]}: {line[0]}\n")
        f.write("-------------------------------------------------------\n")
    print(f"Integral values saved on {txtOut}")
     
    algos = [line[1] for line in integralValues]
    values = [line[0] for line in integralValues]

    plt.figure(figsize=(8, 5))
    plt.bar(algos, values, color = colors)
    plt.ylabel("Integral Value")
    plt.grid(axis="y", linestyle="--", alpha=0.5)

    barOut = outputfile[:-4] + "_integrals.pgf"
    plt.tight_layout()
    plt.savefig(barOut, backend="pgf")
    plt.savefig(barOut[:-4]+".pdf", backend="pdf")
    print(f"Bar chart saved on {barOut}")
    
if __name__ == "__main__":
    if len(sys.argv) == 1:
        main(False)
    else: main(sys.argv[1])
