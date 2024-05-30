# OR-Toolsと比較してベンチマーキングする

## 事前準備

- requirements.txtのモジュールをインストール
    ```
    pip install -r requirements.txt
    ```

- CARGのVRPソルバをリポジトリの直下にコピー
    - CNSsolver_20230602_v2303の`vrp_classical`をコピーしてくる

## 使い方

- データディレクトリを準備
    - 適当なところに作成

- ベンチマークデータを取得(**dataファイルに全ケースのデータ有**)
    - 取得先: https://www.sintef.no/projectweb/top/vrptw/100-customers/
    - `インスタンス名.txt`をデータディレクトリに保存


- ベンチマーキングを実行
    - `benchmarking`に移動
    - コード実行
        - `compare_ortools.ipynb`の場合
            - 特定のインスタンスをインタラクティブに実行
            - `DATA_DIR_PATH`にデータディレクトリを指定
            - `INSTANCE_NAME`にインスタンスを指定
        - `compare_ortools.py`の場合
            - 複数のインスタンスをバッチ処理
            - `DATA_DIR_PATH`にデータディレクトリを指定
            - `INSTANCE_NAMES`に対象インスタンス群をリスト形式で指定
    - 出力データ
        - `インスタンス名.png`: インスタンスの配置画像
        - `インスタンス名_result_ソルバ名.csv`: ソルバのベンチマーキング結果
            - TOTALCOST: 総距離
            - TOTAL_NUMBER_OF VEHICLES: 使用車両数
            - ELAPSED_TIME: 計算時間
                - CONSTRUCTION_TIME: 初期解生成時間※CARGソルバのみ
                - IMPROVEMENT_TIME: 近傍探索時間※CARGソルバのみ
        - `インスタンス名_route_ソルバ名.png`: ソルバの最適ルート画像
            - html画像でインタラクティブに見たい場合は`compare_ortools.ipynb`上で確認
    - 追加事項
        - `simu_cns`, `simu_ortools`にソルバーごとに計算するコードを格納(出力データに変更なし)

- 結果を比較・検証および可視化
    - コード実行(各ソルバーの算出解と最適解を比較する)  
    1. まず、各インスタンスの結果を比較するコード実行
        - `compare_results.ipynb`の場合
            - 特定のインスタンスをインタラクティブに実行
            - `DATA_DIR_PATH`にデータディレクトリを指定
            - `INSTANCE_NAME`にインスタンスを指定
        - `compare_results.py`の場合
            - 複数のインスタンスをバッチ処理
            - `num_cust`に対象のcust数を指定
            - `DATA_DIR_PATH`にデータディレクトリを指定
            - `INSTANCE_NAMES`に対象インスタンス群をリスト形式で指定    
        - 出力データ
            - `インスタンス名_comparison_result.csv`:各ソルバーの解と各解との比較結果
                - TOTALCOST: 総距離
                - TOTAL_NUMBER_OF VEHICLES: 使用車両数
                - ELAPSED_TIME: 計算時間
                - CONSTRUCTION_TIME: 初期解生成時間※CARGソルバのみ
                - IMPROVEMENT_TIME: 近傍探索時間※CARGソルバのみ
                `rate1`:rate of differences(difference1) between cns and ortools  
                `rate2`:rate of differences(difference2) between cns and optimal solution  
                `rate3`:rate of differences(difference3) between ortools and optimal solution
    2. 各custにおいてrate毎に`c,r,rc,total`のグループ別に結果をまとめるコード実行
        - `sum_results.ipynb`の場合
            - `num_cust`に対象のcust数を指定
            - `RATE_NAME`に対象のrate名を指定
            - 特定のインスタンスをインタラクティブに実行
            - `DATA_DIR_PATH`にデータディレクトリを指定
            - `INSTANCE_NAME`にインスタンスを指定
        - `sum_results.py`の場合
            - 複数のインスタンスをバッチ処理
            - `num_cust`に対象のcust数を指定
            - `RATE_NAMES`に対象のrate群をリスト形式で指定
            - `GROUP_NAMES`に対象のgroup群をリスト形式で指定
            - `DATA_DIR_PATH`にデータディレクトリを指定
            - `INSTANCE_NAMES`に対象インスタンス群をリスト形式で指定    
        - 出力データ
            - `cust名_group名_rate名_result.csv`:各インスタンスの比較結果の一覧
            - `cust名_group名_rate名_result_describe.csv`:上記のcsvファイルの統計情報
    3. 結果の可視化
        - 各csvファイル上でグラフ作成
        - html画像でインタラクティブに見たい場合は`make_graphs.ipynb`上で確認