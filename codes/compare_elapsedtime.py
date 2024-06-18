# %%
import os
import pandas as pd

# %%
# make instance list of cust
num_cust = ["100cust", "200cust", "400cust", "600cust", "800cust", "1000cust"]


# %%
def total_elapsedtime(cust_name, instance_name):
    """
    Calculate the total elapsed time for a given customer and instance.

    Args:
        cust_name (str): The name of the customer.
        instance_name (str): The name of the instance.

    Returns:
        dict: A dictionary containing the elapsed time values.
    """
    data_dir_path = f"results/simulation/{cust_name}"
    # read CARG result
    result_carg_file_path = os.path.join(
        data_dir_path, f"{instance_name}_result_CARG.csv"
    )
    if os.path.exists(result_carg_file_path):
        result_carg_df = pd.read_csv(
            result_carg_file_path, header=None, index_col=0, names=["CARG"]
        )
        result_carg_df = result_carg_df.rename(
            index={
                "ELAPSED_TIME": "ELAPSED_TIME_CARG",
                "CONSTRUCTION_TIME": "CONSTRUCTION_TIME_CARG",
                "IMPROVEMENT_TIME": "IMPROVEMENT_TIME_CARG",
            }
        )
        result_carg_dict = (
            result_carg_df["CARG"]
            .drop(["TOTALCOST", "TOTAL_NUMBER_OF VEHICLES"], axis=0)
            .to_dict()
        )
        # read ORTools result
        result_ortools_file_path = os.path.join(
            data_dir_path, f"{instance_name}_result_ortools.csv"
        )
        result_ortools_df = pd.read_csv(
            result_ortools_file_path, header=None, index_col=0, names=["ORTools"]
        )
        result_ortools_df = result_ortools_df.rename(
            index={"ELAPSED_TIME": "ELAPSED_TIME_ORTools"}
        )
        result_ortools_dict = (
            result_ortools_df["ORTools"]
            .drop(["TOTALCOST", "TOTAL_NUMBER_OF VEHICLES"], axis=0)
            .to_dict()
        )
        # make elapsed time dict per instance_name
        case_dict = {"case": instance_name}
        combined_dict = {**case_dict, **result_carg_dict, **result_ortools_dict}
        return combined_dict
    return 0


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
        temp_df = pd.DataFrame(
            columns=[
                "case",
                "CONSTRUCTION_TIME_CARG",
                "IMPROVEMENT_TIME_CARG",
                "ELAPSED_TIME_CARG",
                "ELAPSED_TIME_ORTools",
            ]
        )
        for ins in INSTANCE_NAMES:
            each_result = total_elapsedtime(cust, ins)
            if each_result != 0:
                df_add = pd.DataFrame(each_result, index=[0])
                temp_df = pd.concat([temp_df, df_add], ignore_index=True)
        temp_df = temp_df.set_index("case")
        # save
        SAVE_DIR_PATH = f"results/statistics/{cust}/elapsed_time"
        result_file_path = os.path.join(
            SAVE_DIR_PATH, f"{cust}_elapsed_time_result.csv"
        )
        temp_df.to_csv(result_file_path, header=True)
