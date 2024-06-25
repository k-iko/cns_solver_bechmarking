# Compare results of solvers

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
import chardet

# 作業ディレクトリをスクリプトのあるディレクトリに変更
os.chdir(os.path.dirname(os.path.abspath(__file__)))

# パス・変数の指定
instance_name = "0531"
s = 0  # s=0: 13t>8t>4t>3t, s=1: 3t>4t>8t>13t
data_dir_path = f"../tests/kouei_data_test/kouei_data{instance_name}"
save_dir_path = f"../tests/kouei_data_test/{instance_name}"
request_data_path = f"../tests/kouei_data/検証用データ1/2024{instance_name}元ネタ.csv"

# # ディレクトリの存在確認
# directory_path = save_dir_path
# if os.path.exists(directory_path):
#     print(f'The directory path {directory_path} exists.')
#     for root, dirs, files in os.walk(directory_path):
#         print(f'Current root: {root}')
#         print(f'Directories: {dirs}')
#         print(f'Files: {files}')
# else:
#     print(f'The directory path {directory_path} does not exist.')

# %%
# fig = px.scatter(
#     input_df,
#     x="LATITUDE", y="LONGITUDE",
#     text=input_df['CUST NO.'],
#     width=1000, height=800)
# fig.show()

# %%
# # save
# img_file_path = os.path.join(
#     DATA_DIR_PATH, f'{INSTANCE_NAME}.png'
# )
# fig.write_image(img_file_path)


# クラス化: ファイル内検索 + ファイルパス取得
class FileFinder:
    def __init__(self, directry_path, file_name):
        self.directry_path = directry_path
        self.file_name = file_name

    def find_csv_file_and_return_path(self):

        for root, dirs, files in os.walk(self.directry_path):
            for file in files:
                if file.endswith(f"{self.file_name}.csv"):
                    current_directry_path = os.getcwd()  # 現在のディレクトリを取得
                    relative_path = os.path.relpath(
                        os.path.join(root, file), current_directry_path
                    )  # 指定したディレクトリから現在のディレクトリまでの相対パスを取得
                    return relative_path  # 相対パスを返す


# inputファイルの作成
kouei_input_file_path = FileFinder(
    data_dir_path, "input"
).find_csv_file_and_return_path()
input_df = pd.read_csv(kouei_input_file_path)
input_df.loc[input_df["CUST NO."] != 0, "FROM TIME"] = "04:00"
input_df.loc[input_df["CUST NO."] != 0, "TO TIME"] = "16:00"

# ファイルのエンコーディングを確認する
with open(request_data_path, "rb") as f:
    searched_encoding = chardet.detect(f.read())
    detected_encoding = searched_encoding["encoding"]
if detected_encoding is None:
    possible_encodings = ["shift-jis", "cp932", "utf-8", "latin1"]
    for encoding in possible_encodings:
        try:
            request_df = pd.read_csv(request_data_path, encoding=encoding)
            break  # 成功した場合ループを終了
        except Exception as e:
            pass
            # print(f"Error with encoding {encoding}: {e}")
# request_df = pd.read_csv(request_data_path, encoding='cp932')


# 重複を削除して届先IDごとのユニークな値をリストとして取得
unique_dest_df = request_df.drop_duplicates(subset=["届先ID"])["オーダー重量"]
# inputファイルと要素数を揃えるために0行目に空の値を挿入
new_row = pd.Series(0)
unique_dest_df = pd.concat([new_row, unique_dest_df]).reset_index(drop=True)
input_df["DEMAND"] = unique_dest_df

# save
input_file_path = os.path.join(
    save_dir_path, f"request_cns_solver_{instance_name}_input.csv"
)
input_df.to_csv(input_file_path, index=False)

# %%
# directry_path = "../../tests/kouei_data/検証用データ1/0530データ-20240610-135109304-TES1EDI001-TES1AP001"
# input_file_path = FileFinder(directry_path, "input").find_csv_file_and_return_path()
# distance_file_path = FileFinder(directry_path, "dist").find_csv_file_and_return_path()
# time_file_path = FileFinder(directry_path, "time").find_csv_file_and_return_path()
# vs_file_path = FileFinder(directry_path, "vs").find_csv_file_and_return_path()
# output_file_path = directry_path

# ## 距離ファイルの作成

# %%
distance = FileFinder(data_dir_path, "dist").find_csv_file_and_return_path()
dist_df = pd.read_csv(distance, header=None)
dist_df.columns = [
    "CUST NO.",
    "CUST NO.",
    "distance",
]

# %%
# save
dist_file_path = os.path.join(
    save_dir_path, f"request_cns_solver_{instance_name}_dist.csv"
)
dist_df.to_csv(dist_file_path, index=False)

# 時間ファイルの作成
time = FileFinder(data_dir_path, "time").find_csv_file_and_return_path()
time_df = pd.read_csv(time, header=None)
time_df.columns = [
    "CUST NO.",
    "CUST NO.",
    "time",
]

# save
time_file_path = os.path.join(
    save_dir_path, f"request_cns_solver_{instance_name}_time.csv"
)
time_df.to_csv(time_file_path, index=False)


# ヘテロ車両定義ファイル・ファイルパス作成
# クラス化
class VehicleDataProcessor:
    def __init__(self):
        self.headers = [
            "LOAD_LIMIT",
            "E-CAPACITY",
            "E-INITIAL",
            "E-COST",
            "E-MARGIN",
            "VTYPE",
            "ETYPE",
        ]
        self.vehicle_setting_df = pd.DataFrame(columns=self.headers)

    def create_vehicle_data(self, load_limit, count, vehicle_type=""):
        vehicle_data = {
            "LOAD_LIMIT": [load_limit] * count,
            "E-CAPACITY": [0] * count,
            "E-INITIAL": [0] * count,
            "E-COST": [100] * count,
            "E-MARGIN": [0] * count,
            "VTYPE": [vehicle_type] * count,
            "ETYPE": [""] * count,
        }
        return pd.DataFrame(vehicle_data, columns=self.headers)

    def process_vehicle_data(self, *args):
        all_dfs = [self.vehicle_setting_df] + list(args)
        self.vehicle_setting_df = pd.concat(all_dfs, ignore_index=True)
        self.vehicle_setting_df["VEHICLE NO."] = self.vehicle_setting_df.index
        self.vehicle_setting_df = self.vehicle_setting_df.reindex(
            columns=["VEHICLE NO."] + self.headers
        )

        return self.vehicle_setting_df


processor = VehicleDataProcessor()
vehicle_data_13t = processor.create_vehicle_data(12800, 5)
vehicle_data_8t = processor.create_vehicle_data(7800, 10)
vehicle_data_4t = processor.create_vehicle_data(3800, 10)
vehicle_data_3t = processor.create_vehicle_data(2800, 15)
# 優先して使用する車両の設定を変更するには入力の順番を変えるだけで良い、左にあるほど優先的に使用される?
if s == 0:
    vs_df = processor.process_vehicle_data(
        vehicle_data_13t, vehicle_data_8t, vehicle_data_4t, vehicle_data_3t
    )
else:
    vs_df = processor.process_vehicle_data(
        vehicle_data_3t, vehicle_data_4t, vehicle_data_8t, vehicle_data_13t
    )
# vs_df = processor.process_vehicle_data(
#     (vehicle_data_13t, vehicle_data_8t, vehicle_data_4t, vehicle_data_3t)
#     if s == 0
#     else (vehicle_data_3t, vehicle_data_4t, vehicle_data_8t, vehicle_data_13t)
# )

# save
vs_file_path = os.path.join(save_dir_path, f"request_cns_solver_{instance_name}_vs.csv")
vs_df.to_csv(vs_file_path, index=False)

# 車両別回転数上限及び拠点作業時間ファイルの作成
mtv_df = vs_df.copy()
mtv_df.drop(columns=["E-INITIAL", "E-COST", "E-MARGIN", "VTYPE", "ETYPE"], inplace=True)
mtv_df.columns = ["VEHICLE NO.", "MAXROTATE", "DEPOSERVICETIME"]
mtv_df["MAXROTATE"] = 2
mtv_df

# save
mtv_file_path = os.path.join(
    save_dir_path, f"request_cns_solver_{instance_name}_mtv.csv"
)
mtv_df.to_csv(mtv_file_path, index=False)

# 車両別訪問上限ファイルの作成
maxvisit_df = mtv_df.copy()
maxvisit_df.drop(columns=["DEPOSERVICETIME"], inplace=True)  #
maxvisit_df.columns = ["VEHICLE NO.", "MAXVISIT"]
maxvisit_df["MAXVISIT"] = 10

# save
maxvisit_file_path = os.path.join(
    save_dir_path, f"request_cns_solver_{instance_name}_maxvisit.csv"
)
maxvisit_df.to_csv(maxvisit_file_path, index=False)

# 立寄不可ファイルの作成
# 重複を削除して届先IDごとのユニークな値をリストとして取得
unique_dest_df = request_df.drop_duplicates(subset=["届先ID"])["軒先車格制限"]
# 0行目に挿入
new_row = pd.Series(0)
unique_dest_df = pd.concat([new_row, unique_dest_df]).reset_index(drop=True)

# 比較対象のリスト
comparison_list = [3, 4, 8, 13]

# 比較対象のリスト内の値と比較し、値が大きいものを取得
new_column_values = []
for value in unique_dest_df:
    filtered_values = [x for x in comparison_list if x > value]
    new_column_values.append(filtered_values)

# 新しい列を立寄不可車両としてDataFrameに追加
unique_dest_df = pd.concat(
    [unique_dest_df, pd.Series(new_column_values, name="reject_vehicle")], axis=1
)

# 立寄不可車両と実際の車両IDと紐づける
vehicle_df = vs_df.copy()
vehicle_df["LOAD_LIMIT"] = (
    vehicle_df["LOAD_LIMIT"].apply(lambda x: x / 1000 if x != 0 else x).round()
)
# reject_vehicleとLOAD_LIMITの値を参照して一致するVEHICLENO.を抜き出し、新しい列として追加
result = []
for index, row in unique_dest_df.iterrows():
    vehicles = []
    for v in row["reject_vehicle"]:
        vehicles.extend(
            vehicle_df[vehicle_df["LOAD_LIMIT"] == v]["VEHICLE NO."]
            .astype(str)
            .tolist()
        )
    result.append(",".join(vehicles))

unique_dest_df["matched_vehicles"] = result
# unique_dest_df["matched_vehicles"] = unique_dest_df["matched_vehicles"].apply(
#     lambda x: ",".join(map(str, x))
# )

rej_df = input_df.copy()
rej_df.drop(
    columns=["LONGITUDE", "DEMAND", "FROM TIME", "TO TIME", "SERVICE TIME"],
    inplace=True,
)
rej_df.columns = ["CUST NO.", "REJECT_VEHICLE"]
rej_df["REJECT_VEHICLE"] = unique_dest_df["matched_vehicles"]
rej_df.loc[rej_df["CUST NO."] == 0, "REJECT_VEHICLE"] = ""
# rej_df = rej_df[rej_df["REJECT_VEHICLE"] != ""]

# REJECT_VEHICLE内の要素を分割して新しい列を追加
reject_vehicle_split = rej_df["REJECT_VEHICLE"].str.split(",", expand=True)
reject_vehicle_split.columns = [
    f"REJECT_VEHICLE_{i+1}" for i in range(reject_vehicle_split.shape[1])
]

# 元のDataFrameに追加
rej_df = pd.concat([rej_df, reject_vehicle_split], axis=1)

# df内の整形
rej_df.drop(columns=["REJECT_VEHICLE"], inplace=True)
new_columns = {0: "CUST NO.", 1: "REJECT_VEHICLE"}
rej_df.columns = [new_columns.get(i, None) for i in range(len(rej_df.columns))]
rej_df = rej_df[rej_df["REJECT_VEHICLE"].notna() & (rej_df["REJECT_VEHICLE"] != "")]

# save
rej_file_path = os.path.join(
    save_dir_path, f"request_cns_solver_{instance_name}_rej.csv"
)
rej_df.to_csv(rej_file_path, index=False)

# パスの指定
input_file_path = FileFinder(save_dir_path, "input").find_csv_file_and_return_path()
distance_file_path = FileFinder(save_dir_path, "dist").find_csv_file_and_return_path()
time_file_path = FileFinder(save_dir_path, "time").find_csv_file_and_return_path()
vs_file_path = FileFinder(save_dir_path, "vs").find_csv_file_and_return_path()
mtv_file_path = FileFinder(save_dir_path, "mtv").find_csv_file_and_return_path()
maxvisit_file_path = FileFinder(
    save_dir_path, "maxvisit"
).find_csv_file_and_return_path()
rej_file_path = FileFinder(save_dir_path, "rej").find_csv_file_and_return_path()
output_file_path = os.path.join(save_dir_path, f"cns_solver_{instance_name}_output.csv")

# CNS Solver
std_out = subprocess.check_output(
    [
        "pypy",
        "../CNSsolver_20230602_v2303/vrp_classical/code/main_tw.py",
        "-i",
        input_file_path,
        "-d",
        distance_file_path,
        "-t",
        time_file_path,
        "-o",
        output_file_path,
        "-v",
        str(40),
        "-vs",
        vs_file_path,
        "-rv",  # reduce vehicle
        "-s",
        str(6) + ":00",
        "-e",
        str(16) + ":30",
        "-mt",  # multi-trip (rotation)
        "-rej",
        rej_file_path,
        "-mtv",
        mtv_file_path,
        "-maxvisit",
        maxvisit_file_path,
        # "-dem", str(capacity),
        "-lpt",
        "hard",  # hard constraint on package weight v.s. vehicle capacity
        "-tpt",
        "hard",
        # "-avedistper", "10"
        # "-l" #1.20.2022
        # "-multithread", "0"
        "ls",
        "-l",
    ]  # , text=True
)
# 標準出力をbytes型で受け取るには下記のコードを追加する
# (['ls', '-l'])
# 上記にtext=Trueを付けるとstr型になる

# save std_out as txt file
with open(
    os.path.join(save_dir_path, f"cns_solver_{instance_name}_log.txt"), "wb"
) as file:
    file.write(std_out)

# Extract Results
result_df = {}
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
result_cols = ["TOTAL_NUMBER_OF_VEHICLES", "CONSTRUCTION_TIME", "IMPROVEMENT_TIME"]
for c in result_cols:
    for t in texts:
        if c in t:
            result_df[c] = float(pat.findall(t)[0])
result_df["ELAPSED_TIME"] = (
    result_df["CONSTRUCTION_TIME"] + result_df["IMPROVEMENT_TIME"]
)

result_df = pd.Series(result_df).to_frame()

# save
result_file_path = os.path.join(save_dir_path, f"cns_solver_{instance_name}_result.csv")
result_df.to_csv(result_file_path, header=False)
