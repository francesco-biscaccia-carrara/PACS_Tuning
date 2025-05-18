import os, sys, json, pandas as pd
from dotenv import load_dotenv # type: ignore

sys.dont_write_bytecode = True

runNameCols = {"0":"ACS_0", "1":"ACS_Th", "2":"ACS_Mef", "3":"ACS_Eje"}

def main(pipeline):
    load_dotenv("../../.ACSenv")
    filename = os.environ.get('ACS_JSOUT_FILENAME')
    perfProf = os.environ.get('ACS_PP')
    outputfile = "out.csv"

    with open(filename, 'r') as file:
        JSdata = json.load(file)
    
    tmpDict={}
    for inst in JSdata:
        tmpDict.update({inst:{}})
        for algo in JSdata[inst]:
            if algo == "_obj": continue
            else:
                tmpDict[inst].update({algo:JSdata[inst][algo]})


    rows = []
    for name, values in tmpDict.items():
        row = {'TMP': name}  # First column is the name
        row.update(values)    # Add all the values (CPLEX, 0, 1, 2, 3)
        rows.append(row)

    df = pd.DataFrame(rows)

    columns = ['TMP', 'CPLEX'] + sorted([col for col in df.columns if col not in ['TMP', 'CPLEX']], key=int)
    df = df[columns]
    df = df.rename(columns={"TMP": str(len(columns)-1)})
    df = df.rename(columns=runNameCols)

    df.to_csv(outputfile, index=False)

    if not pipeline:
        if input("Do you want to perfprof the data [y/n]? ") != "y":
            exit(0)
    
    os.system(f"python3 data/perfprof.py -D ',' -X 'MIP Gap'  -S 2 {outputfile} {perfProf} > /dev/null")
    os.system(f"rm {outputfile}")

if __name__ == "__main__":
    if len(sys.argv) == 1:
        main(False)
    else: main(sys.argv[1])
