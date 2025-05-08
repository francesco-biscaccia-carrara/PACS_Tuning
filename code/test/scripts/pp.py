import os, sys, json, pandas as pd

sys.dont_write_bytecode = True

filename = "TMP_DATA_PR.json"
outputfile = "out.csv"

def main(filename):

    with open(filename, 'r') as file:
        JSdata = json.load(file)
    
    tmpDict={}
    for inst in JSdata:
        tmpDict.update({inst:{}})
        for algo in JSdata[inst]:
            if algo == "_obj": continue
            else:
                tmpDict[inst].update({algo:JSdata[inst][algo]})

    #TODO: CHECK BELOW

    # Convert the nested dictionary to a list of dictionaries
    rows = []
    algos = 0
    for name, values in tmpDict.items():
        row = {'TMP': name}  # First column is the name
        row.update(values)    # Add all the values (CPLEX, 0, 1, 2, 3)
        algos = len(values)
        rows.append(row)

    # Create DataFrame
    df = pd.DataFrame(rows)

    # Reorder columns to ensure 'Name' is first, followed by 'CPLEX', then numeric columns
    columns = ['TMP', 'CPLEX'] + sorted([col for col in df.columns if col not in ['TMP', 'CPLEX']], key=int)
    df = df[columns]
    df = df.rename(columns={"TMP": str(algos)})
    df.to_csv(outputfile, index=False)

    if input("Do you want to perfprof the data [y/n]? ") != "y":
        exit(0)
    
    os.system(f"python3 data/perfprof.py -D ',' -X 'MIP Gap'  -S 2 {outputfile} RENAME_DATE.svg")

if __name__ == "__main__":
    main(filename)