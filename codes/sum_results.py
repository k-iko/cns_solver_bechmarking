import os
import pandas as pd
import numpy as np


# In[60]:
num_cust = ["100cust", "200cust", "400cust", "600cust", "800cust", "1000cust"]
RATE_NAMES = ["rate1", "rate2", "rate3"]
DIFFER_NAMES = ["difference1", "difference2", "difference3"]
GROUP_NAMES = ["total", "c", "r", "rc"]


def make_list(group_name, cust_name):
    CUST_DIR_PATH = f"data/cust_data/{cust_name}/{group_name}"
    INSTANCE_NAMES = []
    for file_name in os.listdir(CUST_DIR_PATH):
        if file_name.endswith(".txt"):
            name_without_extension, extension = os.path.splitext(file_name)
            if (
                extension == ".txt"
            ):  # and check_exists(direct, name_without_extension) == True:
                INSTANCE_NAMES.append(name_without_extension)
    return INSTANCE_NAMES


# In[63]:
def total(instance_name, error_name, cust_name):
    DATA_DIR_PATH = f"results/comparison/{cust_name}"
    result_file_path = os.path.join(
        DATA_DIR_PATH, f"{instance_name}_comparison_result.csv"
    )
    if os.path.exists(result_file_path):
        result_df = pd.read_csv(result_file_path, header=0, index_col=0)
        # 'rate1':rate of differences between cns and ortools
        # 'rate2':rate of differences between cns and optimal solution
        # 'rate3':rate of differences between ortools and optimal solution
        rate_df = result_df[error_name]
        rate_dict = rate_df.to_dict()

        # In[62]:
        new_row = {"case": instance_name}
        new_row.update(rate_dict)
        return new_row
    else:
        return 0


# %%

if __name__ == "__main__":
    for cust in num_cust:
        # rateに関しての処理
        for rate in RATE_NAMES:
            for group in GROUP_NAMES:
                INSTANCE_NAMES = make_list(group, cust)
                temp_df = pd.DataFrame(
                    columns=[
                        "case",
                        "TOTALCOST",
                        "TOTAL_NUMBER_OF VEHICLES",
                        "CONSTRUCTION_TIME",
                        "IMPROVEMENT_TIME",
                        "ELAPSED_TIME",
                    ]
                )
                for ins in INSTANCE_NAMES:
                    each_result = total(ins, rate, cust)
                    if each_result != 0:
                        df_add = pd.DataFrame(each_result, index=[0])
                        sum_df = pd.concat([temp_df, df_add], ignore_index=True)
                        temp_df = sum_df
                # In[60]:
                # save
                SAVE_DIR_PATH = f"results/statistics/{cust}/{rate}/{group}"
                result_file_path = os.path.join(
                    SAVE_DIR_PATH, f"{cust}_{group}_{rate}_result.csv"
                )
                sum_df.set_index("case").to_csv(result_file_path, header=True)
                # save
                describe_file_path = os.path.join(
                    SAVE_DIR_PATH, f"{cust}_{group}_{rate}_result_describe.csv"
                )
                sum_df.describe().to_csv(describe_file_path, header=True)

        # differenceに関しての処理
        for differ in DIFFER_NAMES:
            for group in GROUP_NAMES:
                INSTANCE_NAMES = make_list(group, cust)
                temp_df = pd.DataFrame(
                    columns=[
                        "case",
                        "TOTALCOST",
                        "TOTAL_NUMBER_OF VEHICLES",
                        "CONSTRUCTION_TIME",
                        "IMPROVEMENT_TIME",
                        "ELAPSED_TIME",
                    ]
                )
                for ins in INSTANCE_NAMES:
                    each_result = total(ins, differ, cust)
                    if each_result != 0:
                        df_add = pd.DataFrame(each_result, index=[0])
                        sum_df = pd.concat([temp_df, df_add], ignore_index=True)
                        temp_df = sum_df
                # In[60]:
                # save
                SAVE_DIR_PATH = f"results/statistics/{cust}/{differ}/{group}"
                result_file_path = os.path.join(
                    SAVE_DIR_PATH, f"{cust}_{group}_{differ}_result.csv"
                )
                sum_df.set_index("case").to_csv(result_file_path, header=True)
                # save
                describe_file_path = os.path.join(
                    SAVE_DIR_PATH, f"{cust}_{group}_{differ}_result_describe.csv"
                )
                sum_df.describe().to_csv(describe_file_path, header=True)
