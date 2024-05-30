
■ファイル説明
vrp_classical
 + code
 |  + main_tw.py				配送ソルバメインコード
 + data							コマンドライン実行用テストパラメータ
 |  + xxx.csv					テスト入力データ
 |
 + test
    + apiTest
       + doc
          + 環境構築				python環境構築手順書
          + [x-x]テストパターン.txt		テスト仕様(パターン)説明
       + exp					期待値(例外、配送経路結果)ファイル
          + err_x_x_x.txt			例外出力結果
          + out_x_x_x.txt			配送経路(Vehicle)出力結果
       + log					ログファイル(pytest実行後、結果生成)
       + out					テスト結果(例外、配送経路結果)ファイル(pytest実行後、結果生成)
       + test_data
          + x_x_x_request.json			テスト入力パラメータ
          + cost
             + 	xxx_cost.json			コストテーブル
       + README.txt				本ファイル
          + test_apiTest_main.py		pytest実行ファイル
             + test_apiTest_patern_1_1.py	テストパターンファイル
             |         ・・・
             + test_apiTest_patern_x_x.py	テストパターンファイル
             + test_apiTest_driver.py		テストAPI呼び出し(テストドライバ本体)
             + test_apiTest_create.py		テストAPI入力クラス生成
             + test_apiTest_result.py		テスト結果確認


※docフォルダに記載のテスト仕様とtest_apiTest_patern_x_x.py内で記載するテストパターンは一致


■テスト実行方法
・vrp_classicalフォルダをカレントディレクトリとした状態で下記を実行
----------------------------------
pytest .\test\apiTest\
----------------------------------


[実行例]
PS C:\work\Solver\vrp_classical> pytest .\test\apiTest\
========================================================================================================================================== test session starts ==========================================================================================================================================
platform win32 -- Python 3.9.1, pytest-6.2.1, py-1.10.0, pluggy-0.13.1
rootdir: C:\work\Solver\vrp_classical
plugins: forked-1.3.0, mock-3.5.0, xdist-2.2.0
collected 53 items

test\apiTest\test_apiTest_main.py .....................................................                                                                                                                                                                                                            [100%]

========================================================================================================================================== 53 passed in 2.73s =========================================================================================================================================== 


