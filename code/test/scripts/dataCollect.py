import sys, os, numpy as np, json
from dotenv import load_dotenv # type: ignore

sys.dont_write_bytecode = True


def dataSanitizing(filename):
    with open(filename, 'r') as file:
        JSdata = json.load(file)

        for inst in JSdata:
            obj = JSdata[inst]["_obj"]
            for algo in JSdata[inst]:
                if algo == "_obj": continue
                if algo == "CPLEX":
                    CPLEXTime = JSdata[inst]["CPLEX"][1]
                    JSdata[inst]["CPLEX"][1] = CPLEXTime if CPLEXTime<300 else 300
                else:
                    for seed in JSdata[inst][algo]:
                        ACSTime = JSdata[inst][algo][seed][1]
                        JSdata[inst][algo][seed][1] = ACSTime if ACSTime<300 else 300
    
    with open(filename, 'w', encoding='utf-8') as f:
        json.dump(JSdata, f, ensure_ascii=False, indent=4)
    print(f"Sanitized {filename} JSON file")

def MIPgap(opt,inc_val):
    if inc_val == None : return None

    if inc_val == "NO SOL" or opt*inc_val<0:
        return 1
    elif opt == inc_val : 
        return 0
    else:
        return abs(opt - inc_val) / max(abs(opt), abs(inc_val))

def mean(iterable):
    a = np.array(iterable)
    return a.mean()

def main(pipeline):

    load_dotenv("../../.ACSenv")
    filename = os.environ.get('ACS_JSON_FILENAME')
    outputfile = os.environ.get('ACS_JSOUT_FILENAME')

   #FIXME : update the plotting 
    with open(filename, 'r') as file:
            JSdata = json.load(file)

            for inst in JSdata:
                #obj = JSdata[inst]["_obj"]
                for algo in JSdata[inst]:
                    if algo == "_obj": continue
                    if algo == "CPLEX": continue
                    else:
                        values = []
                        times = []
                        for seed in JSdata[inst][algo]:
                            values.append(JSdata[inst][algo][seed][0])
                            times.append(JSdata[inst][algo][seed][1])
                        
                        nosol=0
                        for val in values:
                            if val == "NO SOL":
                                nosol+=1
                        
                        if nosol < len(values)/2:
                            for val in values:
                                if val =="NO SOL":
                                    values.remove(val)

                        avgVal = "NO SOL" if nosol >= len(values)/2 else mean(values)
                        avgTime = mean(times)
                        JSdata[inst][algo]=[avgVal,avgTime]
    
    if not pipeline :
        if input(f"Do you want to collect and save the result on {outputfile} file [y/n]? ") != "y":
            exit(0)
    
    with open(outputfile, 'w', encoding='utf-8') as f:
        json.dump(JSdata, f, ensure_ascii=False, indent=4)
    print(f"Generated {outputfile} JSON file")
    
if __name__ == "__main__":
    if len(sys.argv) == 1:
        main(False)
    else: main(sys.argv[1])
