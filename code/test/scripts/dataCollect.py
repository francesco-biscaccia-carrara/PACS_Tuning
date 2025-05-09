import sys, os, numpy as np, json
from dotenv import load_dotenv

sys.dont_write_bytecode = True

def MIPgap(opt,inc_val):
    if inc_val == None : return None

    if inc_val == "NO SOL" or opt*inc_val<0:
        return 1
    elif opt == inc_val : 
        return 0
    else:
        return abs(opt - inc_val) / max(abs(opt), abs(inc_val))

def geo_mean(iterable):
    a = np.array(iterable)
    return a.prod()**(1.0/len(a))

def main():

    load_dotenv("../../.env")
    filename = os.environ.get('ACS_JSON_FILENAME')
    outputfile = os.environ.get('ACS_JSOUT_FILENAME')

    with open(filename, 'r') as file:
        JSdata = json.load(file)
    
    for inst in JSdata:
        obj = JSdata[inst]["_obj"]
        for algo in JSdata[inst]:
            if algo == "_obj": continue

            #Compare with optimal
            if algo == "CPLEX":
                    CPLEXVal = JSdata[inst]["CPLEX"]
                    if MIPgap(obj,CPLEXVal) != None:
                        JSdata[inst]["CPLEX"] = MIPgap(obj,CPLEXVal)
            else:
                values = []
                for seed in JSdata[inst][algo]:
                    ACSVal = JSdata[inst][algo][seed]
                    if MIPgap(obj,ACSVal) != None:
                        JSdata[inst][algo][seed] = MIPgap(obj,ACSVal)
                        values.append(JSdata[inst][algo][seed])
                    
                JSdata[inst][algo] = geo_mean(values)
    
    if input("Do you want to update the JSON file [y/n]? ") != "y":
        exit(0)
    
    with open(outputfile, 'w', encoding='utf-8') as f:
        json.dump(JSdata, f, ensure_ascii=False, indent=4)
    print(f"Generated {outputfile} JSON file")
    
if __name__ == "__main__":
    main()
