■環境構築
	■python3.9.8インストール
	https://www.python.org/downloads/release/python-398/
	　⇒「Windows installer (64-bit)」を選択しインストール

	■ライブラリ統一
	・vrp_classicalフォルダをカレントディレクトリとした状態で下記を実行
	pip install -r ./test/apiTest/doc/環境構築/requirements.txt --proxy http://10.77.8.70:8080

	■TortoiseGit
	1.インストール
	https://qiita.com/SkyLaptor/items/6347f38c8c010f4d5bd2
	2.ソースコード取得
	2-1.GitClone
　	エクスプローラー上を右クリック
	  「Git Clone」押下
	  URL:http://pmts.pmcrd.mei.co.jp/gitlab/2020/qc-solver/vrp_classical.git
	  Directory:C:/xxx
	　　⇒xxx:任意のディレクトリ
	2-2.ブランチ切り替え
	　エクスプローラー上を右クリック
	　TortoiseGit⇒Switch/Checkout...
	　Branch：feature_APITest
	　　⇒上記を選択しOKボタン押下

	■PyPy3.9インストール
	https://doc.pypy.org/en/latest/install.html
	  ⇒「PyPy3.9」をダウンロード
	
	(Windows)
	pypy3.9-x.y.z-win64.zipを解凍
	pypy.exeおよびpypy3.exeを環境変数Pathに追加
	下記を実行
  	----------------------------------
	./pypy -m ensurepip
	./pypy -mpip install -U pip wheel
	./pypy -mpip install texttable
	----------------------------------
	(Linux)
	下記を実行
  	----------------------------------
	tar xf pypy-x.y.z.tar.bz2
	ln -s pypy3.9-v7.3.8-linux64/bin/pypy .
	ln -s pypy3.9-v7.3.8-linux64/bin/pypy3 .
	./pypy -m ensurepip
	./pypy -mpip install -U pip wheel
	./pypy -mpip install texttable
	----------------------------------

■ファイル説明
vrp_classical
 + code							外部提供時に削除
 |  + main_tw.py					配送ソルバメインコード
 |  + cross_dtable.cp39-xxx.pyd				Windowsのdllファイル（Windows、Python）
 |  + cross_dtable.pypy39-xxx.pyd			Windowsのdllファイル（Windows、PyPy）
 |  + cross_dtable.cpython-39-xxx.so			Linuxの共有ライブラリ（Linux、Python）
 |  + cross_dtable.pypy39-xxx.so			Linuxの共有ライブラリ（Linux、PyPy）
 |
 + exe_python						外部提供用フォルダ(Python)
 |  + main_tw.pyc
 |  + cross_dtable.xxx.pyd
 |  + cross_dtable.xxx.so
 |
 + exe_pypy						外部提供用フォルダ(PyPy)
 |  + main_tw.pyc
 |  + cross_dtable.xxx.pyd
 |  + cross_dtable.xxx.so
 |
 + cmake.bat						C++コードコンパイル用batファイル（Windowsの場合）
 |
 + cmake.sh						C++コードコンパイル用shファイル（Linuxの場合）
 |
 + README.txt						本ファイル


※docフォルダに記載のテスト仕様とtest_apiTest_patern_x_x.py内で記載するテストパターンは一致


■コマンドライン実行方法
(Python)
・vrp_classical/codeフォルダをカレントディレクトリとした状態で下記を実行(詳細はvrp_classical/ソルバ説明書_yyyymmdd.docを参照)
----------------------------------
python main_tw.py -i ../data/DSS_FORMAT_02-0input_50.csv -d ../data/DSS_FORMAT_02-0dist.csv -t ../data/DSS_FORMAT_02-0time.csv -v 4 -vend ../data/vendcust2.csv -si ../data/spotid2.csv -s "2000/1/1 9:00:00" -e "2000/1/1 17:00:00" -o ./1-1/DSS_FORMAT_02-0output.csv > ./1-1/result.txt
----------------------------------

(PyPy)
・vrp_classical/codeフォルダをカレントディレクトリとした状態で下記を実行(詳細はvrp_classical/ソルバ説明書_yyyymmdd.docを参照)
----------------------------------
/path/to/pypy main_tw.py -i ../data/DSS_FORMAT_02-0input_50.csv -d ../data/DSS_FORMAT_02-0dist.csv -t ../data/DSS_FORMAT_02-0time.csv -v 4 -vend ../data/vendcust2.csv -si ../data/spotid2.csv -s "2000/1/1 9:00:00" -e "2000/1/1 17:00:00" -o ./1-1/DSS_FORMAT_02-0output.csv > ./1-1/result.txt
----------------------------------

■テスト実行方法
・vrp_classicalフォルダをカレントディレクトリとした状態で下記を実行
----------------------------------
pytest ./test/apiTest/
----------------------------------


[実行例]
PS C:/work/Solver/vrp_classical> pytest ./test/apiTest/
========================================================================================================================================== test session starts ==========================================================================================================================================
platform win32 -- Python 3.9.1, pytest-6.2.1, py-1.10.0, pluggy-0.13.1
rootdir: C:/work/Solver/vrp_classical
plugins: forked-1.3.0, mock-3.5.0, xdist-2.2.0
collected 53 items

test/apiTest/test_apiTest_main.py .....................................................                                                                                                                                                                                                            [100%]

========================================================================================================================================== 53 passed in 2.73s =========================================================================================================================================== 


■C++化コード実行手順
・C++化コードをコンパイルする準備
1-1. Microsoft Visual Studio 2015 Update 3 or newer(Windows)またはGCC 4.8 or newer(Linux)をインストール
1-2. https://cmake.org/download/もしくはpip install cmakeでcmakeのversion3.9以上をインストール
1-3. Gitリポジトリからvrp_classical/code/pybind11/をpullするかhttps://github.com/pybind11/cmake_example/pybind11/をvrp_classical/code/にコピー(すでに存在する場合はこの手順は不要)
1-4. Gitリポジトリからvrp_classical/code/pybind11/CMakeLists.txtをpull(すでに存在する場合はこの手順は不要)

・C++化コードをコンパイル（cmake.bat, cmake.shを使用する場合）
1-5-1. cmake.bat (Windows)またはcmake.sh (Linux)をvrp_classical直下で実行、エラーが発生した場合、以下を実行

・C++化コードをコンパイル（cmake.bat, cmake.shを使用しない場合）
1-5-2. vrp_classical/code/pybind11/にbuildフォルダを作成
1-5-3. 1-5-2.で作成したbuildフォルダに移動し下記を実行
(Windows、Python)
----------------------------------
cmake ..
cmake --build . --config Release
----------------------------------

(Windows、PyPy)
----------------------------------
cmake -DPYTHON_EXECUTABLE=/path/to/pypy ..
cmake --build . --config Release
----------------------------------

"cmake .."を実行した際に"Python config failure: Python is 64-bit, chosen compiler is 32-bit"が発生した場合、代わりに
cmake -G "Visual Studio XX 20YY Win64" ..
を実行（XX, YYはバージョン名、"cmake -G"にて確認可能）
----------------------------------
Generators
  Visual Studio 17 2022        = Generates Visual Studio 2022 project files.
                                 Use -A option to specify architecture.
  Visual Studio 16 2019        = Generates Visual Studio 2019 project files.
                                 Use -A option to specify architecture.
* Visual Studio 15 2017 [arch] = Generates Visual Studio 2017 project files.
                                 Optional [arch] can be "Win64" or "ARM".
----------------------------------

(Linux、Python)
----------------------------------
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
----------------------------------

(Linux、PyPy)
----------------------------------
cmake -DCMAKE_BUILD_TYPE=Release -DPYTHON_EXECUTABLE=/path/to/pypy ..
cmake --build .
----------------------------------

1-5-4. 1-5-3でbuild/Release/直下に.pydファイル(Windows)またはbuild/直下に.soファイル(Linux)が作成されるのでvrp_classical/code/にコピー


・C++化コード(OpenMP)を実行
2-1. 並列数を設定
  ・API実行の場合
    Argvsクラスのmultithread_numに並列数を設定(デフォルト値 1)

  ・コマンドライン実行の場合
    -multithreadオプション(コマンドライン実行時に指定)で並列数を設定(デフォルト値 1)


2-2. ■コマンドライン実行方法 に従い実行

■pyc作成手順
(Python)
vrp_classical/codeフォルダをカレントディレクトリとした状態で下記を実行
----------------------------------
python -m compileall main_tw.py
----------------------------------

__pycache__/にmain_tw.cpython-39.pycが作成されるのでファイル名を変更

(PyPy)
vrp_classical/codeフォルダをカレントディレクトリとした状態で下記を実行
----------------------------------
pypy -m compileall main_tw.py
----------------------------------

__pycache__/にmain_tw.pypy39.pycが作成されるのでファイル名を変更