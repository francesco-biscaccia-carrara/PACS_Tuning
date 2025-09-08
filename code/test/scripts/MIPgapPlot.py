
import sys, os, numpy as np, json, matplotlib.pyplot as plt, math #type: ignore
#from dotenv import load_dotenv # type: ignore
plt.rcParams.update({ 
    "font.family": "serif",     
})
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
                break

def main(pipeline):

    #load_dotenv("../../.ACSenv")
    filename = "data/6_ACS_WalkMIP_Res_v1.2.11/ACS_WalkMIP.json"
    outputfile = "data/6_ACS_WalkMIP_Res_v1.2.11/PACS_WalkMIP-MIPGapPlot.pgf"

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

    integralValues = []
    for algo, succ_array in succDict.items():
        plt.plot(x_values, succ_array, label=algo, linewidth=2)
        integralValues.append([np.trapezoid(succ_array, x_values), algo])

    plt.xlabel('MIP Gap (%)')
    plt.ylabel('Success Rate')
    plt.grid(True, linestyle='--', alpha=0.5)
    plt.legend(loc='upper left')
    plt.xlim(1, x_values[-1])
    plt.tight_layout()
    plt.savefig(outputfile, backend="pgf")
    plt.savefig(outputfile[:-4]+".pdf", backend="pdf")

    # --- Save integral values to file ---
    colors = plt.cm.tab10.colors
    #integralValues.sort(reverse=True)
    txtOut = outputfile[:-4] + ".txt"
    with open(txtOut, 'w') as f:
        f.write("--------------------INTEGRAL VALUES--------------------\n")
        for line in integralValues:
            f.write(f"{line[1]}: {line[0]}\n")
        f.write("-------------------------------------------------------\n")
    print(f"Integral values saved on {txtOut}")

    # --- Create a bar chart of integrals ---
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
