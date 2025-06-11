import sys, json

sys.dont_write_bytecode = True


def main(filename):

    with open(filename, 'r') as file:
            JSdata = json.load(file)

            for inst in JSdata:
                obj = JSdata[inst]["_obj"]
                for algo in JSdata[inst]:
                    if algo == "_obj": continue
                    if JSdata[inst][algo][0] == "NO SOL": continue
                    else: 
                        if JSdata[inst][algo][0]<obj: print(f"ERROR HERE: {inst} -- Obj: {obj} | Heu: {JSdata[inst][algo][0]}")

    
if __name__ == "__main__":
    if len(sys.argv) == 1:
        print("No filename passed to the main function.")
        exit(1)
    else: main(sys.argv[1])
 
