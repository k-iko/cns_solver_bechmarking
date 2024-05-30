import os
import streamlit as st
import pandas as pd
import plotly.express as px
import plotly.graph_objects as go

# import numpy as np
# import matplotlib.pyplot as plt


# diff, rate, group, c_area_group, r_area_groupの辞書作成
diff_dict = {
    "CNS-ORTools": "difference1",
    "CNS-OptSol": "difference2",
    "ORTools-OptSol": "difference3",
}
rate_dict = {"CNS-ORTools": "rate1", "CNS-OptSol": "rate2", "ORTools-OptSol": "rate3"}
group_dict = {
    "Total": "total",
    "Clustering": "c",
    "Random": "r",
    "Random Clustering": "rc",
}
c_area_group = {
    "100cust": ["c101", "c208"],
    "200cust": ["C1_2_1", "C2_2_9"],
    "400cust": ["C1_4_1", "C2_4_9"],
    "600cust": ["C1_6_1", "C2_6_2"],
    # TODO 800, 1000custの場合も追加
}
r_area_group = {
    "100cust": ["r101", "r211"],
    "200cust": ["R1_2_1", "R2_2_9"],
    "400cust": ["R1_4_1", "R2_4_9"],
    "600cust": ["R1_6_1", "R2_6_2"],
    # TODO 800, 1000custの場合も追加
}


# 各ケースのインスタンス数を読み込むクラス作成
class Num_Instance:
    def __init__(self, cust, group):
        self.__cust = cust
        self.__gourp = group
        self.__numInstance = self.__caluculate_number()

    def __caluculate_number(self):
        DATA_DIR_PATH = f"data/cust_data/{self.__cust}/{self.__gourp}"  # f'results/simulation/{num_cust}/'
        count = 0
        for file_name in os.listdir(DATA_DIR_PATH):
            if file_name.endswith(".txt"):
                count += 1
        return count

    def get_num(self):
        return self.__numInstance


# 折れ線グラフ作成
def ROC_TotalCost_graph(cust_name, rate_name, group_name):
    # CSVファイルを読み込む
    CSV_DIR_PATH = f"results/statistics/{cust_name}/{rate_dict[rate_name]}/{group_dict[group_name]}"
    data_file_path = os.path.join(
        CSV_DIR_PATH,
        f"{cust_name}_{group_dict[group_name]}_{rate_dict[rate_name]}_result.csv",
    )
    data_df = pd.read_csv(data_file_path)
    x_data = data_df["case"]
    y_data = data_df["TOTALCOST"]

    # プロットの作成
    trace = go.Scatter(x=x_data, y=y_data, mode="lines+markers")

    # レイアウトの作成
    layout = go.Layout(
        title="ROC of Total Cost[%]",
        xaxis=dict(title="Case"),
        yaxis=dict(title="Total Cost"),
    )

    # 図全体の定義
    fig = go.Figure(data=trace, layout=layout)

    if group_name == "Total":
        # axvspanに相当する箇所を追加(Clustering)
        fig.add_vrect(
            x0=c_area_group[cust_name][0],
            x1=c_area_group[cust_name][1],
            fillcolor="LightSalmon",
            opacity=0.5,
            layer="below",
            line_width=0,
        )
        # axvspanに相当する箇所を追加(Random)
        fig.add_vrect(
            x0=r_area_group[cust_name][0],
            x1=r_area_group[cust_name][1],
            fillcolor="LightSkyBlue",
            opacity=0.5,
            layer="below",
            line_width=0,
        )

    st.plotly_chart(fig)


# 箱ひげ図作成
def BoxHideDiagram_graph(cust_name, rate_name, group_name):
    # CSVファイルを読み込む
    TOTAL_CSV_DIR_PATH = f"results/statistics/{cust_name}/{rate_dict[rate_name]}/total"
    total_data_file_path = os.path.join(
        TOTAL_CSV_DIR_PATH, f"{cust_name}_total_{rate_dict[rate_name]}_result.csv"
    )
    C_CSV_DIR_PATH = f"results/statistics/{cust_name}/{rate_dict[rate_name]}/c"
    c_data_file_path = os.path.join(
        C_CSV_DIR_PATH, f"{cust_name}_c_{rate_dict[rate_name]}_result.csv"
    )
    R_CSV_DIR_PATH = f"results/statistics/{cust_name}/{rate_dict[rate_name]}/r"
    r_data_file_path = os.path.join(
        R_CSV_DIR_PATH, f"{cust_name}_r_{rate_dict[rate_name]}_result.csv"
    )
    RC_CSV_DIR_PATH = f"results/statistics/{cust_name}/{rate_dict[rate_name]}/rc"
    rc_data_file_path = os.path.join(
        RC_CSV_DIR_PATH, f"{cust_name}_rc_{rate_dict[rate_name]}_result.csv"
    )
    dataTOTAL_df = pd.read_csv(total_data_file_path)
    dataC_df = pd.read_csv(c_data_file_path)
    dataR_df = pd.read_csv(r_data_file_path)
    dataRC_df = pd.read_csv(rc_data_file_path)

    fig = go.Figure()

    fig.add_trace(
        go.Box(
            y=dataTOTAL_df["TOTALCOST"],
            name="Total",
            marker_color="lightcoral",
            boxmean=True,
        )
    )
    fig.add_trace(
        go.Box(
            y=dataC_df["TOTALCOST"],
            name="Clustering",
            marker_color="indianred",
            boxmean=True,
        )
    )
    fig.add_trace(
        go.Box(
            y=dataR_df["TOTALCOST"],
            name="Random",
            marker_color="lightsalmon",
            boxmean=True,
        )
    )
    fig.add_trace(
        go.Box(
            y=dataRC_df["TOTALCOST"],
            name="Random Clustering",
            marker_color="lightgreen",
            boxmean=True,
        )
    )

    fig.update_layout(
        title="Boxplot of TOTALCOST", xaxis_title="Categories", yaxis_title="Total Cost"
    )

    st.plotly_chart(fig)


# ヒストグラム作成
def Histogram(cust_name, rate_name, group_name):
    # CSVファイルを読み込む
    CSV_DIR_PATH = f"results/statistics/{cust_name}/{diff_dict[rate_name]}/{group_dict[group_name]}"
    data_file_path = os.path.join(
        CSV_DIR_PATH,
        f"{cust_name}_{group_dict[group_name]}_{diff_dict[rate_name]}_result.csv",
    )
    data_df = pd.read_csv(data_file_path)

    fig = px.histogram(
        data_df,
        x="TOTAL_NUMBER_OF VEHICLES",
        title="Histogram of Total Number of Vehicles",
    )

    st.plotly_chart(fig)


# 棒グラフ作成
def Bar_graph(cust_name):
    # CSVファイルを読み込む
    CSV_DIR_PATH = f"results/statistics/{cust_name}/elapsed_time"
    data_file_path = os.path.join(CSV_DIR_PATH, f"{cust_name}_elapsed_time_result.csv")
    data_df = pd.read_csv(data_file_path)
    # ORToolsの計算時間
    fig1 = px.bar(
        data_df, x="case", y="ELAPSED_TIME_ORTools", title="Elapsed Time of ORTools"
    )
    st.plotly_chart(fig1)
    # CNSの計算時間
    fig2 = px.bar(
        data_df,
        x="case",
        y=["CONSTRUCTION_TIME_CARG", "IMPROVEMENT_TIME_CARG"],
        title="Elapsed Time of CNS solver",
        color_discrete_map={
            "CONSTRUCTION_TIME_CARG": "blue",
            "IMPROVEMENT_TIME_CARG": "lightsalmon",
        },
    )
    st.plotly_chart(fig2)


# タイトル表示
st.title("Benchmarking")

with st.sidebar:
    # custをリスト作成
    cust_list = (
        "100cust",
        "200cust",
        "400cust",
        "600cust",
        "800cust",
        "1000cust",
        "all custs",
    )
    rate_list = ("CNS-ORTools", "CNS-OptSol", "ORTools-OptSol")
    # cust名,group名のセレクトボックスを作成
    sel_cust = st.selectbox("select number of custmers:", cust_list)
    sel_rate = st.selectbox("select number of rate:", rate_list)

st.write(f"selected cust : {sel_cust}")
st.write(f"selected group : {sel_rate}")

group_list = ("Total", "Clustering", "Random", "Random Clustering")
tab1, tab2, tab3, tab4 = st.tabs(group_list)

with tab1:
    ROC_TotalCost_graph(sel_cust, sel_rate, "Total")
    BoxHideDiagram_graph(sel_cust, sel_rate, "Total")
    Histogram(sel_cust, sel_rate, "Total")
    Bar_graph(sel_cust)
    # Bar_graph(sel_cust, sel_rate, 'Total')
    with st.expander("Route", expanded=False):
        # 複数選択ボックスのオプションを定義
        DATA_DIR_PATH = f"data/cust_data/{sel_cust}/total"
        INSTANCE_NAMES = []
        for file_name in os.listdir(DATA_DIR_PATH):
            if file_name.endswith(".txt"):
                name_without_extension, extension = os.path.splitext(file_name)
                if extension == ".txt":
                    INSTANCE_NAMES.append(name_without_extension)
        # 複数選択ボックスを作成し、ユーザーの選択を取得
        choice = st.multiselect(
            "Select the case",
            INSTANCE_NAMES,
            INSTANCE_NAMES[:1],
            max_selections=1,
            placeholder="Select",
        )
        for selected_case in choice:
            ROUTE_DIR_PATH = f"results/simulation/{sel_cust}"
            carg_route_file_path = os.path.join(
                ROUTE_DIR_PATH, f"{selected_case}_route_CARG.png"
            )
            ortools_route_file_path = os.path.join(
                ROUTE_DIR_PATH, f"{selected_case}_route_ortools.png"
            )
            # ファイルパスが存在する場合、画像を表示
            if os.path.exists(carg_route_file_path):
                st.image(carg_route_file_path, caption=f"CNS Route for {selected_case}")
            else:
                st.write(f"Image not found for {selected_case}: {carg_route_file_path}")
            if os.path.exists(ortools_route_file_path):
                st.image(
                    ortools_route_file_path, caption=f"ORTols Route for {selected_case}"
                )
            else:
                st.write(f"Image not found for {selected_case}: {carg_route_file_path}")
with tab2:
    ROC_TotalCost_graph(sel_cust, sel_rate, "Clustering")
    Histogram(sel_cust, sel_rate, "Clustering")
with tab3:
    ROC_TotalCost_graph(sel_cust, sel_rate, "Random")
    Histogram(sel_cust, sel_rate, "Random")
with tab4:
    ROC_TotalCost_graph(sel_cust, sel_rate, "Random Clustering")
    Histogram(sel_cust, sel_rate, "Random Clustering")
