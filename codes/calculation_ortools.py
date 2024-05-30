# %% compare solvers with time
# distance_df, conf_df make global

# %%
import os
import re
import subprocess
import time
import datetime as dt
import pandas as pd
import plotly.express as px
import plotly.graph_objects as go
from itertools import permutations
from ortools.constraint_solver import routing_enums_pb2
from ortools.constraint_solver import pywrapcp

# os.chdir(os.path.dirname(os.path.abspath(__file__)))
file_path = {}
num_cust = ["100cust", "200cust", "400cust", "600cust", "800cust", "1000cust"]

# # %% make instance list
# # RESULT_DIR_PATH = 'result/100cust'
# DATA_DIR_PATH = "data/cust_data/100cust/total"
# SAVE_DIR_PATH = "data/cust_data/100cust"
# INSTANCE_NAMES = []
# for file_name in os.listdir(DATA_DIR_PATH):
#     print(file_name)
#     if file_name.endswith(".txt"):
#         name_without_extension, extension = os.path.splitext(file_name)
#         if extension == ".txt":
#             INSTANCE_NAMES.append(name_without_extension)
# print(INSTANCE_NAMES)


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


# define the function of plotting routes
"""
def draw_solution(data, manager, routing, solution, fig_name='output.png'):
    G = nx.DiGraph()
    nodes = data['nodes']
    G.add_nodes_from([node['name'] for node in nodes])
    for vehicle_id in range(data['num_vehicles']):
        # 各日（「各トラック」と対応）のルートを描画
        index = routing.Start(vehicle_id)
        while not routing.IsEnd(index):
            previous_node_index = manager.IndexToNode(index)
            index = solution.Value(routing.NextVar(index))
            node_index = manager.IndexToNode(index)
            G.add_edge(
                nodes[previous_node_index]['name'],
                nodes[node_index]['name'],
                color=data['colors'][vehicle_id])
    pos = {
        node['name']: node['coord'] for node in data['nodes']
    }
    edge_color = [edge['color'] for edge in G.edges.values()]
    nx.draw_networkx(G, pos=pos, arrowsize=15, edge_color=edge_color, node_color='c')
    plt.savefig(fig_name)
    print(f'Result: {fig_name}')]
"""


# define solver of or tools
def ORtools(instance_name, cust_name):
    # %% [markdown]
    data = {}
    data["distance_matrix"] = distance_df.pivot_table(
        index=["START"], columns=["END"], values=["METERS"], aggfunc="first"
    ).fillna(0)
    data["distance_matrix"] = data["distance_matrix"].astype(int).values
    data["num_vehicles"] = int(conf_df.loc["NUMBER", 0])
    capacity = conf_df.loc["CAPACITY", 0]
    # data['num_vehicles'] = int(vehicle_num_carg)
    data["depot"] = 0
    data["demands"] = input_df.set_index("CUST NO.")["DEMAND"].tolist()
    data["vehicle_capacities"] = [capacity] * data["num_vehicles"]
    data["time_windows"] = (instance_df[["READY TIME", "DUE DATE"]] * 1000).values
    data["time_matrix"] = data["distance_matrix"]
    data["service_time"] = (instance_df["SERVICE TIME"] * 1000).values

    # %%
    # Create the routing index manager.
    manager = pywrapcp.RoutingIndexManager(
        len(data["distance_matrix"]), data["num_vehicles"], data["depot"]
    )
    # Create Routing Model.
    routing = pywrapcp.RoutingModel(manager)

    # %%
    # Create and register a transit callback.
    def distance_callback(from_index, to_index):
        """Returns the distance between the two nodes."""
        # Convert from routing variable Index to distance matrix NodeIndex.
        from_node = manager.IndexToNode(from_index)
        to_node = manager.IndexToNode(to_index)
        return data["distance_matrix"][from_node][to_node]

    transit_callback_index = routing.RegisterTransitCallback(distance_callback)

    # Define cost of each arc.
    routing.SetArcCostEvaluatorOfAllVehicles(transit_callback_index)

    # %%
    # Demand Constraint
    def demand_callback(from_index):
        """Returns the demand of the node."""
        # Convert from routing variable Index to demands NodeIndex.
        from_node = manager.IndexToNode(from_index)
        return data["demands"][from_node]

    demand_callback_index = routing.RegisterUnaryTransitCallback(demand_callback)

    routing.AddDimensionWithVehicleCapacity(
        demand_callback_index,
        0,  # null capacity slack
        data["vehicle_capacities"],  # vehicle maximum capacities
        True,  # start cumul to zero
        "Capacity",
    )

    # %%
    # Time Constraint
    def time_callback(from_index, to_index):
        """Returns the travel time between the two nodes."""
        # Convert from routing variable Index to time matrix NodeIndex.
        from_node = manager.IndexToNode(from_index)
        to_node = manager.IndexToNode(to_index)
        return data["time_matrix"][from_node][to_node] + data["service_time"][from_node]

    time_callback_index = routing.RegisterTransitCallback(time_callback)

    dimension_name = "Time"
    routing.AddDimension(
        time_callback_index,
        int(data["time_windows"].max()),  # allow waiting time
        int(data["time_windows"].max()),  # maximum time per vehicle
        False,  # Don't force start cumul to zero.
        dimension_name,
    )
    time_dimension = routing.GetDimensionOrDie(dimension_name)
    # Add time window constraints for each location except depot.
    for location_idx, time_window in enumerate(data["time_windows"]):
        if location_idx == data["depot"]:
            continue
        index = manager.NodeToIndex(location_idx)
        time_dimension.CumulVar(index).SetRange(
            int(time_window[0]), int(time_window[1])
        )
    # Add time window constraints for each vehicle start node.
    depot_idx = data["depot"]
    for vehicle_id in range(data["num_vehicles"]):
        index = routing.Start(vehicle_id)
        time_dimension.CumulVar(index).SetRange(
            int(data["time_windows"][depot_idx][0]),
            int(data["time_windows"][depot_idx][1]),
        )
    for i in range(data["num_vehicles"]):
        routing.AddVariableMinimizedByFinalizer(
            time_dimension.CumulVar(routing.Start(i))
        )
        routing.AddVariableMinimizedByFinalizer(time_dimension.CumulVar(routing.End(i)))

    # %%
    # Setting first solution heuristic.
    search_parameters = pywrapcp.DefaultRoutingSearchParameters()
    search_parameters.first_solution_strategy = (
        routing_enums_pb2.FirstSolutionStrategy.LOCAL_CHEAPEST_INSERTION
    )
    search_parameters.log_search = True

    # log_searchの内容を取得
    log_search_content = str(search_parameters).encode("utf-8")

    # log_searchの内容をtxtファイルに保存する
    with open(
        f"/results/simulation/{cust_name}/{instance_name}_log_ortools.txt", "wb"
    ) as file:
        file.write(log_search_content)
    # %%
    # Solve the problem.
    start_time = time.time()
    solution = routing.SolveWithParameters(search_parameters)
    elapsed_time = time.time() - start_time

    # %% [markdown]
    # ### Extract Results

    # %%
    # extract route
    routes_df = []
    max_route_distance = 0
    for vehicle in range(data["num_vehicles"]):
        index = routing.Start(vehicle)
        previous_index = index
        route_distance = 0
        while not routing.IsEnd(index):
            cust = manager.IndexToNode(index)
            pre_cust = manager.IndexToNode(previous_index)
            routes_df.append([vehicle, cust, pre_cust])
            previous_index = index
            index = solution.Value(routing.NextVar(index))
            route_distance += routing.GetArcCostForVehicle(
                previous_index, index, vehicle
            )
        cust = manager.IndexToNode(index)
        pre_cust = manager.IndexToNode(previous_index)
        routes_df.append([vehicle, cust, pre_cust])
        max_route_distance = max(route_distance, max_route_distance)
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
    # drop non used vehicles
    active_vehicles = (routes_df.groupby("vehicle")["distance"].sum() > 0).to_dict()
    active_vehicles = [k for k, v in active_vehicles.items() if v]
    routes_df = routes_df[routes_df["vehicle"].isin(active_vehicles)]
    reindex = {
        v: new_v for v, new_v in zip(active_vehicles, range(len(active_vehicles)))
    }
    routes_df["vehicle"] = routes_df["vehicle"].map(reindex)
    routes_df

    # %%
    # make result
    result_df = {}
    result_df["TOTALCOST"] = (
        routes_df.groupby("vehicle")["total_distance"].last().sum() / 1000
    )
    result_df["TOTAL_NUMBER_OF VEHICLES"] = len(routes_df["vehicle"].unique())
    result_df["ELAPSED_TIME"] = elapsed_time
    result_df = pd.Series(result_df).to_frame()
    result_df

    # %%
    # save
    result_file_path = os.path.join(
        SAVE_DIR_PATH, f"{instance_name}_result_ortools.csv"
    )
    result_df.to_csv(result_file_path, header=False)

    # %%
    # make route output
    output_file_path = os.path.join(
        SAVE_DIR_PATH, f"{instance_name}_output_ortools.csv"
    )
    with open(output_file_path, "w", newline="\n") as f:
        f.write("route#,Id\n")
        for v, df in routes_df.groupby("vehicle"):
            line = ",".join([f"route{v}"] + df["customer"].astype(str).to_list())
            line += "\n"
            f.write(line)

    # %%
    result_df

    # %% [markdown]
    # ### Plot Route
    """
    solution = routing.SolveWithParameters(search_parameters)
    if solution:
        draw_solution(data, manager, routing, solution)
    """
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
    img_file_path = os.path.join(SAVE_DIR_PATH, f"{instance_name}_route_ortools.png")
    fig.write_image(img_file_path)
    result = "ORtools"
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
            ORtools(ins, cust)
