import os, sys, re, pandas as pd, numpy as np

sys.dont_write_bytecode = True

data_folder = "data/"
data_file = data_folder + "res.csv"
out_file = data_folder + "out.csv"

def prepare_perfplot(df):
    processed_df = df.copy()

    if 'Objective' in processed_df.columns:
        processed_df = processed_df.drop(columns=['Objective'])

    processed_df.to_csv(out_file,index=False)


def main():
    fil_in = pd.read_csv(data_file)
    prepare_perfplot(fil_in)

if __name__ == "__main__":
    main()