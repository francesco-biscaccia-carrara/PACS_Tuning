import os, sys, re, pandas as pd, numpy as np

sys.dont_write_bytecode = True

inst_file = "fHard_instances.csv"
out_folder = "../jobs_output"
outs = os.listdir(out_folder)
data_folder = "data/"
data_file = data_folder + "res.csv"
test_file = data_folder + "test.csv"



def extract_value_from_line(line : str):
    if "NO FEASIBLE SOLUTION" in line:
        return "NO SOL"
    elif "BEST INCUMBENT:" in line:
        match = re.search(r"BEST INCUMBENT:\s+(-?\d+\.\d+)", line)
        if match:
            return float(match.group(1))
    return None


def process_dataframe(df):
    # Create a copy of the dataframe to avoid modifying the original
    processed_df = df.copy()

    # Get the objective column name
    obj_col = "Objective"

    # Columns to process (ACS columns and CPLEX)
    cols_to_process = [
        col for col in df.columns if col.startswith("ACS") or col == "CPLEX"
    ]

    for col in cols_to_process:
        # Create a mask for 'NO SOL' values
        no_sol_mask = df[col] == "NO SOL"

        # Convert column to numeric, replacing 'NO SOL' with NaN
        processed_df[col] = pd.to_numeric(df[col], errors="coerce")

        # Create masks for different conditions
        equal_mask = processed_df[col] == processed_df[obj_col]
        opposite_sign_mask = processed_df[obj_col] * processed_df[col] < 0

        # Calculate the relative error
        def relative_error(obj_val, col_val):
            if pd.isna(obj_val) or pd.isna(col_val):
                return 1
            if col_val == obj_val:
                return 0
            return abs(obj_val - col_val) / max(abs(obj_val), abs(col_val))

        # Apply transformations
        processed_df[col] = processed_df.apply(
            lambda row: (
                1
                if (no_sol_mask.loc[row.name] or opposite_sign_mask.loc[row.name])
                else (
                    0
                    if equal_mask.loc[row.name]
                    else relative_error(row[obj_col], row[col])
                )
            ),
            axis=1,
        )

    return processed_df

def geo_mean(df):
    # Create a copy of the dataframe to avoid modifying the original
    processed_df = df.copy()
    
    # Get the objective column name
    obj_col = 'Objective'
    
    # Columns to process (ACS columns)
    acs_cols = [col for col in df.columns if col.startswith('ACS')]
    
    # Group columns by their decimal value
    decimal_groups = {}
    for col in acs_cols:
        decimal = col.split('_')[1]
        if decimal not in decimal_groups:
            decimal_groups[decimal] = []
        decimal_groups[decimal].append(col)
    
    # Compute geometric mean for each group of ACS columns with same decimal
    for decimal, group_cols in decimal_groups.items():
        # Create a new column name for the geometric mean
        geomean_col = f'ACS_{decimal}_geomean'
        
        # Compute geometric mean
        def safe_geomean(row):
            # Filter out 'NO SOL' values
            valid_values = [val for val in row[group_cols]]

            # Compute geometric mean
            try:
                return np.prod(valid_values) ** (1/len(valid_values))
            except Exception:
                return 1.0
        
        processed_df[geomean_col] = processed_df.apply(safe_geomean, axis=1)

        processed_df.drop(columns=group_cols, inplace=True)
    
    return processed_df


def main():
    i = 0

    existing_df = pd.read_csv(inst_file)
    outs.sort()

    cols = {}
    for rho in [0.25,0.5,0.75]:
        for seed in [10493847, 83274910, 70938475, 98312048, 19283746]:
            col = f"ACS_{rho}_{seed}"
            cols[col] = []
    cols["CPLEX"] = []

    for out in outs:
        with open(out_folder + "/" + out) as f1:
            for line in f1:
                pass
            last_line = line
            value = extract_value_from_line(last_line)
            if "CPLEXRun" in out:
                cols["CPLEX"].append(value)
            match = re.search(r"ACS_\d+\.\d+_\d+", out)
            if match:
                col = match.group()
                cols[col].append(value)

    data = pd.DataFrame.from_dict(cols, orient="index").T
    merged_df = pd.concat([existing_df, data], axis=1)
    merged_df.to_csv(test_file, index=False)

    processed_df = geo_mean(process_dataframe(merged_df))

    processed_df.to_csv(data_file, index=False)


if __name__ == "__main__":
    main()
