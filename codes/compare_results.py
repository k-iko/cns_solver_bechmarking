#!/usr/bin/env python
# coding: utf-8

# In[53]:
import os
import pandas as pd
import numpy as np


# In[54]:
# make instance list of cust
num_cust = ["100cust", "200cust", "400cust", "600cust", "800cust", "1000cust"]


# In[55]:
def compare_results(instance_name, cust_name):
    """
    Compare the results of CARG and ORTools for a given instance and customer.

    Args:
        instance_name (str): The name of the instance.
        cust_name (str): The name of the customer.
    """
    # Rest of the code...
    # read results of cns
    result_carg_file_path = os.path.join(
        f"results/simulation/{cust_name}", f"{instance_name}_result_CARG.csv"
    )
    # Check for existence of result file
    if os.path.exists(result_carg_file_path):
        result_carg_df = pd.read_csv(
            result_carg_file_path, header=None, index_col=0, names=["CARG"]
        )
        # In[56]:
        # read results of ortools
        result_ortools_file_path = os.path.join(
            f"results/simulation/{cust_name}", f"{instance_name}_result_ortools.csv"
        )
        result_ortools_df = pd.read_csv(
            result_ortools_file_path, header=None, index_col=0, names=["ORTools"]
        )
        # In[57]:
        # merge results of cns and ortools
        result_df = pd.concat([result_carg_df, result_ortools_df], axis=1)
        # In[58]:
        # read optimal results
        result_optimal_file_path = os.path.join(
            f"data/optimal_solution/Best_known_results_for_{cust_name}.csv"
        )
        optimal_result_df = pd.read_csv(result_optimal_file_path, header=0)
        # 200cust, 400cust
        if cust_name == "100cust":
            # 100cust
            distance_value = optimal_result_df.loc[
                optimal_result_df["Instance"] == f"{instance_name.upper()}", "Distance"
            ].values[
                0
            ]  # if you need, use '.upper() or .lower()'
            vehicle_value = optimal_result_df.loc[
                optimal_result_df["Instance"] == f"{instance_name.upper()}", "Vehicles"
            ].values[0]
        else:
            # 200cust~
            distance_value = optimal_result_df.loc[
                optimal_result_df["Instance"] == f"{instance_name.lower()}", "Distance"
            ].values[
                0
            ]  # if you need, use '.upper() or .lower()'
            vehicle_value = optimal_result_df.loc[
                optimal_result_df["Instance"] == f"{instance_name.lower()}", "Vehicles"
            ].values[0]
        result_df["Optimal Sol"] = [distance_value, vehicle_value, 999, 999, 999]
        result_df = result_df.replace(999, np.nan)
        # In[59]:
        # result_df['difference1'] = result_df['ORTools'] - result_df['CARG']
        # result_df['rate1'] = result_df['difference1'] / result_df['CARG'] *100
        result_df["difference1"] = result_df["CARG"] - result_df["ORTools"]
        result_df["rate1"] = result_df["difference1"] / result_df["ORTools"] * 100
        result_df["difference2"] = result_df["CARG"] - result_df["Optimal Sol"]
        result_df["rate2"] = result_df["difference2"] / result_df["Optimal Sol"] * 100
        result_df["difference3"] = result_df["ORTools"] - result_df["Optimal Sol"]
        result_df["rate3"] = result_df["difference3"] / result_df["Optimal Sol"] * 100
        # In[60]:
        # save
        result_file_path = os.path.join(
            f"results/comparison/{cust_name}",
            f"{instance_name}_comparison_result.csv",
        )
        result_df.to_csv(result_file_path, header=True)


# %%

if __name__ == "__main__":
    for cust in num_cust:
        DATA_DIR_PATH = f"results/simulation/{cust}"
        file_names = os.listdir(DATA_DIR_PATH)
        file_names.sort()
        INSTANCE_NAMES = []
        for file_name in file_names:
            if file_name.endswith("_output_CARG.csv"):
                name_without_extension = file_name.replace("_output_CARG.csv", "")
                INSTANCE_NAMES.append(name_without_extension)
        for ins in INSTANCE_NAMES:
            compare_results(ins, cust)
