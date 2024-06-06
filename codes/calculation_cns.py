# %% compare solvers with time
# distance_df, conf_df make global

# %%
import os
import re
import subprocess
import datetime as dt
import pandas as pd
import plotly.express as px
import plotly.graph_objects as go
from itertools import permutations
from ortools.constraint_solver import routing_enums_pb2
from ortools.constraint_solver import pywrapcp

# os.chdir(os.path.dirname(os.path.abspath(__file__)))
num_cust = ["100cust", "200cust", "400cust", "600cust", "800cust", "1000cust"]
file_path = {}


# %%
# make instance list
def readTestdata(instance_name):
    instance_file_path = os.path.join(DATA_DIR_PATH, f"{instance_name}.txt")
    file_path["instance_file_path"] = instance_file_path
    instance_df = pd.read_csv(
        instance_file_path, skiprows=8, delim_whitespace=True, header=None
    )
    instance_df.columns = [
        "CUST NO.",
        "XCOORD",
        "YCOORD",
        "DEMAND",
        "READY TIME",
        "DUE DATE",
        "SERVICE TIME",
    ]
    # %% plot instance
    fig = px.scatter(
        instance_df,
        x="XCOORD",
        y="YCOORD",
        text=instance_df["CUST NO."],
        width=1000,
        height=800,
    )
    fig.show()
    # %%
    # save
    img_file_path = os.path.join(SAVE_DIR_PATH, f"{instance_name}.png")
    fig.write_image(img_file_path)
    return instance_df


# make distance data
def makeDistance(instance_df, instance_name):
    distance_df = [[src, dst] for src, dst in permutations(instance_df["CUST NO."], 2)]
    distance_df = pd.DataFrame(distance_df, columns=["START", "END"])
    coords = instance_df.set_index("CUST NO.")
    distance_df["START_x"] = coords.loc[distance_df["START"], "XCOORD"].values
    distance_df["START_y"] = coords.loc[distance_df["START"], "YCOORD"].values
    distance_df["END_x"] = coords.loc[distance_df["END"], "XCOORD"].values
    distance_df["END_y"] = coords.loc[distance_df["END"], "YCOORD"].values
    distance_df["DIFF_x"] = distance_df["END_x"] - distance_df["START_x"]
    distance_df["DIFF_y"] = distance_df["END_y"] - distance_df["START_y"]
    distance_df["METERS"] = 1000 * (
        distance_df["DIFF_x"] ** 2 + distance_df["DIFF_y"] ** 2
    ) ** (1 / 2)
    distance_df["KM"] = (distance_df["METERS"] / 1000).astype(int)
    distance_df.head()
    # save
    distance_file_path = os.path.join(SAVE_DIR_PATH, f"{instance_name}_dist.csv")
    file_path["distance_file_path"] = distance_file_path
    distance_df = distance_df[["START", "END", "METERS"]]
    distance_df.to_csv(distance_file_path, index=False, header=False)
    return distance_df


# make input data
def makeInput(instance_df, instance_name):
    start_date = pd.to_datetime("1/1/2017")
    input_df = instance_df.copy()
    input_df = input_df.rename(
        columns={
            "XCOORD": "LATITUDE",
            "YCOORD": "LONGITUDE",
            "READY TIME": "FROM TIME",
            "DUE DATE": "TO TIME",
        }
    )
    input_df["FROM TIME"] = input_df["FROM TIME"].apply(
        lambda x: (start_date + dt.timedelta(minutes=x)).strftime("%Y/%m/%d %H:%M:%S")
    )
    input_df["TO TIME"] = input_df["TO TIME"].apply(
        lambda x: (start_date + dt.timedelta(minutes=x)).strftime("%Y/%m/%d %H:%M:%S")
    )

    # %%
    # save
    input_file_path = os.path.join(SAVE_DIR_PATH, f"{instance_name}_input.csv")
    input_df.to_csv(input_file_path, index=False)
    file_path["input_file_path"] = input_file_path
    return input_df


# make other cofig data
def makeConfig(instance_name):
    instance_file_path = os.path.join(SAVE_DIR_PATH, f"{instance_name}.txt")
    conf_df = pd.read_csv(
        instance_file_path, skiprows=3, nrows=1, delim_whitespace=True, header=0
    ).T
    # save
    conf_file_path = os.path.join(SAVE_DIR_PATH, f"{instance_name}_conf.csv")
    conf_df.to_csv(conf_file_path, header=False)
    file_path[conf_file_path] = conf_file_path
    return conf_df


# define solver of cns
# TODO check the solver
def CNSsolver(cust_name, instance_name):
    # make distance data
    time_df = distance_df[["START", "END", "METERS"]]
    time_df["HRS"] = time_df["METERS"] / (60 * 1000)
    # save
    time_file_path = os.path.join(SAVE_DIR_PATH, f"{instance_name}_time.csv")
    time_df = time_df[["START", "END", "HRS"]]
    time_df.to_csv(time_file_path, index=False, header=False)

    # make output data
    output_file_path = os.path.join(SAVE_DIR_PATH, f"{instance_name}_output_CARG.csv")
    start_time = input_df["FROM TIME"].min()
    end_time = input_df["TO TIME"].max()
    vehicle_num = conf_df.loc["NUMBER", 0]
    capacity = conf_df.loc["CAPACITY", 0]

    std_out = subprocess.check_output(
        [
            "pypy",
            "CNSsolver_20230602_v2303/vrp_classical/code/main_tw.py",
            "-i",
            os.path.abspath(file_path["input_file_path"]),
            "-d",
            os.path.abspath(file_path["distance_file_path"]),
            "-t",
            os.path.abspath(time_file_path),
            "-o",
            os.path.abspath(output_file_path),
            "-v",
            str(vehicle_num),
            "-rv",  # reduce vehicle
            "-s",
            start_time,
            "-e",
            end_time,
            # "-mt", #multi-trip (rotation)
            "-dem",
            str(capacity),
            "-lpt",
            "hard",  # hard constraint on package weight v.s. vehicle capacity
            # "-lpt", "soft", # hard constraint on package weight v.s. vehicle capacity
            # "-avedistper", "10"
            # "-l" #1.20.2022
            # "-multithread", "0"
            # "-to", str(elapsed_time)
            "ls",
            "-l",
        ]  # , text=True
    )
    # 標準出力をbytes型で受け取るには下記のコードを追加する
    # (['ls', '-l'])
    # 上記にtext=Trueを付けるとstr型になる

    # save std_out as txt file
    with open(
        f"results/simulation/{cust_name}/{instance_name}_log_CARG.txt", "wb"
    ) as file:
        file.write(std_out)

    # %% [markdown]
    # ### Extract Results

    # %%
    result_df = {}

    # %%
    # extract results from detail output file
    detail_output_file_path = f"{output_file_path}.detail.csv"
    with open(detail_output_file_path, "r") as f:
        lines = f.readlines()
    for l in lines:
        if "TOTALCOST," in l:
            result_df["TOTALCOST"] = float(l.split(",")[1].replace("\n", "")) / 1000

    # %%
    # extract results from std output
    pat = re.compile(r"=====.*?=====")
    texts = std_out.decode().split("\n")
    texts = [t for t in texts if pat.match(t)]
    result_row = texts.index("=" * 39)
    texts = texts[result_row:]
    pat = re.compile(r"[\d\.]+")
    result_cols = ["TOTAL_NUMBER_OF VEHICLES", "CONSTRUCTION_TIME", "IMPROVEMENT_TIME"]
    for c in result_cols:
        for t in texts:
            if c in t:
                result_df[c] = float(pat.findall(t)[0])
    result_df["ELAPSED_TIME"] = (
        result_df["CONSTRUCTION_TIME"] + result_df["IMPROVEMENT_TIME"]
    )

    # %%
    result_df = pd.Series(result_df).to_frame()
    result_df

    # %%
    # save
    result_file_path = os.path.join(SAVE_DIR_PATH, f"{instance_name}_result_CARG.csv")
    result_df.to_csv(result_file_path, header=False)

    # %% [markdown]
    # ### Plot Route

    # %%
    # extract routes
    routes_df = []
    # get route
    pat = re.compile(r"[\d\.]+")
    with open(output_file_path, "r") as f:
        lines = f.read().splitlines()
        for line in lines[1:]:
            line = line.split(",")
            v = pat.findall(line[0])[0]
            pre_cust = 0
            for cust in line[1:]:
                cust = int(cust)
                routes_df.append([v, cust, pre_cust])
                pre_cust = cust
    routes_df = pd.DataFrame(routes_df, columns=["vehicle", "customer", "pre_customer"])
    # get order
    routes_df["order"] = 1
    routes_df["order"] = routes_df.groupby("vehicle")["order"].cumsum()
    routes_df["order"] -= 1
    # get coordinate
    coords = instance_df.set_index("CUST NO.")
    routes_df["x"] = coords.loc[routes_df["customer"], "XCOORD"].values
    routes_df["y"] = coords.loc[routes_df["customer"], "YCOORD"].values
    # get demand
    demands = instance_df.set_index("CUST NO.")
    routes_df["demand"] = demands.loc[routes_df["customer"], "DEMAND"].values
    routes_df["total_demand"] = routes_df.groupby("vehicle")["demand"].cumsum()
    # get distance
    dists = distance_df.set_index(["START", "END"])
    dists.loc[(0, 0), "METERS"] = 0
    routes_df["distance"] = dists.loc[
        routes_df.set_index(["customer", "pre_customer"]).index, "METERS"
    ].values
    routes_df["total_distance"] = routes_df.groupby("vehicle")["distance"].cumsum()

    # %%
    # plot routes
    vehicle_num = int(result_df.loc["TOTAL_NUMBER_OF VEHICLES", 0])
    total_cost = result_df.loc["TOTALCOST", 0]
    fig = px.line(
        routes_df,
        x="x",
        y="y",
        color="vehicle",
        hover_data=["order", "x", "y", "demand", "total_distance", "total_demand"],
        text=routes_df["customer"],
        title=f"VEHICLE_NUM:{vehicle_num}\tDISTANCE:{total_cost}",
        width=1000,
        height=800,
    )

    center = instance_df.set_index("CUST NO.").loc[0, ["XCOORD", "YCOORD"]].values
    for vehicle, df in routes_df.groupby("vehicle"):
        no_stop = len(df)
        total_dist = int(df["total_distance"].iloc[-1] / 1000)
        total_dem = int(df["total_demand"].iloc[-1])
        fig.add_trace(
            go.Scatter(
                name=f"{no_stop} Deliveries ({total_dist}km, {total_dem}pkg)",
                x=center[0:1],
                y=center[1:2],
                mode="markers",
                marker=go.scatter.Marker(size=0),
                legendgroup=vehicle,
            )
        )

    fig.add_trace(
        go.Scatter(
            name="Central Depot",
            x=center[0:1],
            y=center[1:2],
            mode="markers",
            marker=go.scatter.Marker(
                size=15, color=px.colors.qualitative.G10[-2], opacity=0.9
            ),
        )
    )

    fig.show()

    # %%
    # save
    img_file_path = os.path.join(SAVE_DIR_PATH, f"{instance_name}_route_CARG.png")
    fig.write_image(img_file_path)
    result = "CNSsolver"
    return result


if __name__ == "__main__":
    for cust in num_cust:
        DATA_DIR_PATH = f"data/cust_data/{cust}/total"
        SAVE_DIR_PATH = f"results/simulation/{cust}"
        INSTANCE_NAMES = []
        for file_name in os.listdir(DATA_DIR_PATH):
            if file_name.endswith(".txt"):
                name_without_extension, extension = os.path.splitext(file_name)
                if extension == ".txt":
                    INSTANCE_NAMES.append(name_without_extension)
        for ins in INSTANCE_NAMES:
            # 各dfの初期化
            instance_df = readTestdata(ins)
            distance_df = makeDistance(instance_df, ins)
            input_df = makeInput(instance_df, ins)
            conf_df = makeConfig(ins)
        os.system("mv *best0.txt results/cns_ini_value/" + cust)
