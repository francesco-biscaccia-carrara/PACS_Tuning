import sys, os, numpy as np, json

sys.dont_write_bytecode = True

def main():

    
    filename1 = "std.json"
    filename2 = "dyn.json"
    outputfile = "new.json"

    with open(filename2, 'r') as file:
            JSdata2 = json.load(file)

    with open(filename1, 'r') as file:
            JSdata1 = json.load(file)

            for inst in JSdata1:
                JSdata1[inst].update(JSdata2[inst])
                JSdata1[inst].pop("ACS_Rho_0.9")     
                JSdata1[inst].pop("ACS_Rho_0.75")   
                JSdata1[inst].pop("ACS_Rho_0.5")   
                JSdata1[inst].pop("ACS_Dyn_0.5")         

    with open(outputfile, 'w', encoding='utf-8') as f:
        json.dump(JSdata1, f, ensure_ascii=False, indent=4)
    print(f"Generated {outputfile} JSON file")

if __name__ == "__main__":
    main()