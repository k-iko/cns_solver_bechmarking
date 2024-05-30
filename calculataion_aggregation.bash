#!/bin/bash
set -e

# 仮想環境の確認および必要なライブラリのインストール
VENV_DIR=".venv"
REQUIREMENTS_FILE="requirements.txt"

# .venvフォルダが存在するか確認
if [ -d "$VENV_DIR" ]; then
    echo ".venvフォルダが見つかりました。仮想環境を有効にします。"
    source "$VENV_DIR/bin/activate"
    echo "仮想環境が起動しました"
else
    echo ".venvフォルダが見つからないため、新しい仮想環境を作成します。"
    python3 -m venv "$VENV_DIR"
    source "$VENV_DIR/bin/activate"
    echo "新しい仮想環境を作成しました。"
    
    if [ -f  "$REQUIREMENTS_FILE" ]; then
        echo "requirements.txtファイルが見つかったため、必要なパッケージをインストールします。"
        pip install -r "$REQUIREMENTS_FILE"
        echo "必要なパッケージをインストールしました。"
    else
        echo "requirements.txtファイルが見つからないため、パッケージのインストールは行いません。"
    fi
fi

# cnssolverで最適化計算を実行
python codes/calculation_cns.py
#TODO: 初期解出力場所および移動先の確認、指定 
mv simu_cns/*.txt simu_cns/ini_result
#TODO:logの出力先の指定

# ortoolsで最適化計算を実行
python codes/calculation_ortools.py
#TODO:logの出力先の指定

# ソルバーごとの計算結果および最適化解との比較(総距離、台数削減、計算時間)を実行
python codes/compare_results.py
echo "最適化計算結果比較:完了"

# ソルバーごとの計算時間比較を実行
python codes/compare_elapsedtime.py
echo "計算時間比較:完了"

# 比較結果の集計を実行
python codes/sum_results.py
echo "比較結果集計:完了"

# 仮想環境からのdeactivate
deactivate
