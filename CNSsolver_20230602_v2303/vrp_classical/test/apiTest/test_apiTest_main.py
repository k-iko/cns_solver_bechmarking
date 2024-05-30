# -*- coding: utf-8 -*-
# !/usr/bin/env python3

# ---------------------------------------------------------------------
# ---------------------------------------------------------------------
#
# APIテスト環境
#

import sys
import os
import pprint

import json
import traceback
import random
import time
import datetime
import logging
from io import StringIO
import re
import numpy as np
import copy
import pytest
import test_apiTest_driver as driver
import test_apiTest_result as result
import test_apiTest_patern_1_1
import test_apiTest_patern_1_2
import test_apiTest_patern_2
import test_apiTest_patern_3
import test_apiTest_patern_4
import test_apiTest_patern_5
import test_apiTest_patern_6
import test_apiTest_patern_7
import test_apiTest_patern_8
import test_apiTest_patern_9
import test_apiTest_patern_10
import test_apiTest_patern_11
import test_apiTest_patern_12
import test_apiTest_patern_13
import test_apiTest_patern_14
import test_apiTest_patern_15
import test_apiTest_patern_16
import test_apiTest_patern_17
import test_apiTest_patern_18
import test_apiTest_patern_19
import test_apiTest_patern_20
import test_apiTest_patern_21
import test_apiTest_patern_23
import test_apiTest_patern_24
import test_apiTest_patern_25_2
import test_apiTest_patern_27
import test_apiTest_patern_28
import test_apiTest_patern_29
import test_apiTest_patern_30
import test_apiTest_patern_31
import test_apiTest_patern_32
import test_apiTest_patern_33
import test_apiTest_patern_35
import test_apiTest_patern_36
import test_apiTest_patern_37
import test_apiTest_patern_38_1
import test_apiTest_patern_38_2
import test_apiTest_patern_39_1
import test_apiTest_patern_39_2
import test_apiTest_patern_40
import test_apiTest_patern_41
import test_apiTest_patern_42
import test_apiTest_patern_43
import test_apiTest_patern_44
import test_apiTest_patern_45
import test_apiTest_patern_46
import test_apiTest_patern_47
import test_apiTest_patern_48
import test_apiTest_patern_49
import test_apiTest_patern_50
import test_apiTest_patern_51
import test_apiTest_random
import test_apiTest_random_bugfix
import test_apiTest_patern_arpa_1

path = 'test/apiTest/test_data/'
cost_dir = './cost/'

random.seed(0)

# -----------------------------------------------------------------
# テスト1-1(配送時間枠指定)
# -----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_1_1.test_args)
def test_APItest_1_1(test_no, request_file, cost_file, logon_flag, caplog):
	#APItest_common(test_no, request_file, cost_file, logon_flag, caplog)
	APItest_common(test_no, request_file, cost_file, True, None)

#-----------------------------------------------------------------
# テスト1-2(荷作業時間設定)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_1_2.test_args)
def test_APItest_1_2(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# テスト2(初期解入力)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_2.test_args)
def test_APItest_2(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# テスト3(積載量指定)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_3.test_args)
def test_APItest_3(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# テスト4(タスク平準化％指定)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_4.test_args)
def test_APItest_4(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog, result.AdditionalCheckFunc_4)

#-----------------------------------------------------------------
# テスト5(最適化指標)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_5.test_args)
def test_APItest_5(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# テスト6(非巡回モード)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_6.test_args)
def test_APItest_6(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# テスト7(通行不可)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_7.test_args)
def test_APItest_7(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# テスト8(車両別営業時間枠)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_8.test_args)
def test_APItest_8(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# テスト9(時間別車両別距離ファイル対応)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_9.test_args)
def test_APItest_9(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# テスト10(時間別車両別時間ファイル対応)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_10.test_args)
def test_APItest_10(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# テスト11(ヘテロ車両対応)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_11.test_args)
def test_APItest_11(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# テスト12(SPOTID指定)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_12.test_args)
def test_APItest_12(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# テスト13(最終訪問先指定)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_13.test_args)
def test_APItest_13(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# テスト14(車両立寄制限)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_14.test_args)
def test_APItest_14(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# テスト15(スキル設定)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_15.test_args)
def test_APItest_15(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

# -----------------------------------------------------------------
# テスト16(車両数固定・削減モード)
# -----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_16.test_args)
def test_APItest_16(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

# -----------------------------------------------------------------
# テスト17(出発遅延)
# -----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_17.test_args)
def test_APItest_17(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

# -----------------------------------------------------------------
# テスト18(打ち切り時間指定)
# -----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_18.test_args)
def test_APItest_18(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog, result.AdditionalCheckFunc_18)

# -----------------------------------------------------------------
# テスト19(初期解生成)
# -----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_19.test_args)
def test_APItest_19(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

# -----------------------------------------------------------------
# テスト20(巡回モード)
# -----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_20.test_args)
def test_APItest_20(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog, result.AdditionalCheckFunc_20)

# -----------------------------------------------------------------
# テスト21(直送便)
# -----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_21.test_args)
def test_APItest_21(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog, result.AdditionalCheckFunc_21)

# -----------------------------------------------------------------
# テスト23(正常解BackUp & 回復機能)
# -----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_23.test_args)
def test_APItest_23(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog, result.AdditionalCheckFunc_23)

# -----------------------------------------------------------------
# テスト24(最適化指標（時間・距離の係数を自由に設定）)
# -----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_24.test_args)
def test_APItest_24(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

# -----------------------------------------------------------------
# テスト25-2(機能オプション最多組み合わせ)
# -----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_25_2.test_args)
def test_APItest_25_2(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog, result.AdditionalCheckFunc_25_2)

# -----------------------------------------------------------------
# テスト27(積載量指定（第1積載、第2積載）)
# -----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_27.test_args)
def test_APItest_27(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog, result.AdditionalCheckFunc_27)

# -----------------------------------------------------------------
# テスト28(訪問数上限)
# -----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_28.test_args)
def test_APItest_28(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

# -----------------------------------------------------------------
# テスト29(集荷)
# -----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_29.test_args)
def test_APItest_29(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog, result.AdditionalCheckFunc_29)

# -----------------------------------------------------------------
# テスト30(大量輸送モード bulk-shipping)
# -----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_30.test_args)
def test_APItest_30(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

# -----------------------------------------------------------------
# テスト31(EV Plan全般)
# -----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_31.test_args)
def test_APItest_31(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog, result.AdditionalCheckFunc_31)

#-----------------------------------------------------------------
# テスト32(経験コスト全般)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_32.test_args)
def test_APItest_32(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# テスト33(ltime, utime指定全般)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_33.test_args)
def test_APItest_33(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# テスト35(1台車両の配送途中で再計画)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_35.test_args)
def test_APItest_35(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)
	oParamJSON = create_JSON(request_file, cost_file)
	test_no, request_file, cost_file = driver.generate_file_for_replanning(test_no, json.dumps(oParamJSON))
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog, result.AdditionalCheckFunc_35)

#-----------------------------------------------------------------
# テスト36(配送件数平準化)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_36.test_args)
def test_APItest_36(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog, result.AdditionalCheckFunc_36)

#-----------------------------------------------------------------
# テスト37(コマンドライン実行)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file", test_apiTest_patern_37.test_args)
def test_APItest_37(test_no, request_file, caplog):
	APItest_command_line(test_no, request_file, caplog)

#-----------------------------------------------------------------
# テスト38-1(回転機能)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_38_1.test_args)
def test_APItest_38_1(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# テスト38-2(回転機能、車両の拠点作業時間)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_38_2.test_args)
def test_APItest_38_2(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# テスト39-1(駐車時間)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_39_1.test_args)
def test_APItest_39_1(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# テスト39-2(駐車時間)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_39_2.test_args)
def test_APItest_39_2(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# テスト40(スレッド数指定)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_40.test_args)
def test_APItest_40(test_no, request_file, cost_file, logon_flag, caplog):
	start_time=time.time()
	APItest_common('40_1_0', '40_1_0_request.json','cost_40_1_0.json', logon_flag, caplog, continuity_test_flag=True)
	end_time=time.time()
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog, result.AdditionalCheckFunc_40, calc_time=end_time-start_time)

#-----------------------------------------------------------------
# テスト41(重み係数)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_41.test_args)
def test_APItest_41(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# テスト42(ハード制約切替)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_42.test_args)
def test_APItest_42(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# テスト43(集荷最適化)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_43.test_args)
def test_APItest_43(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# テスト44(遅延最適化)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_44.test_args)
def test_APItest_44(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# テスト45(飛び地機能)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_45.test_args)
def test_APItest_45(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# テスト46(荷物自動分割)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_46.test_args)
def test_APItest_46(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# テスト47(距離平準化)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_47.test_args)
def test_APItest_47(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog, result.AdditionalCheckFunc_47)

#-----------------------------------------------------------------
# テスト48(台数削減優先)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_48.test_args)
def test_APItest_48(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# テスト49(台数削減優先)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_49.test_args)
def test_APItest_49(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# テスト50(休憩機能)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_50.test_args)
def test_APItest_50(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# テスト51(未割り当て荷物最適化)
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_51.test_args)
def test_APItest_51(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# 中国ユースケースの追加テスト
#-----------------------------------------------------------------
@pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_patern_arpa_1.test_args)
def test_APItest_arpa_1(test_no, request_file, cost_file, logon_flag, caplog):
	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

# -----------------------------------------------------------------
# ランダムテスト
# -----------------------------------------------------------------
# @pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_random.test_args)
# def test_APItest_random(test_no, request_file, cost_file, logon_flag, caplog):
# 	timeout_check = test_apiTest_random.Create_Random_Test(test_no, request_file, cost_file, logon_flag)
# 	APItest_common(test_no, 'random/' + request_file, '../random/cost/' + cost_file, logon_flag, caplog,  result.AdditionalCheckFunc_random, continuity_test_flag=True, timeout_flag=timeout_check)

# # -----------------------------------------------------------------
# # ランダムテスト（バグ修正）
# # -----------------------------------------------------------------
# @pytest.mark.parametrize("test_no, request_file, cost_file, logon_flag", test_apiTest_random_bugfix.test_args)
# def test_APItest_random_bugfix(test_no, request_file, cost_file, logon_flag, caplog):
# 	APItest_common(test_no, request_file, cost_file, logon_flag, caplog)

#-----------------------------------------------------------------
# 共通関数
#-----------------------------------------------------------------
def create_JSON(request_file, cost_file):
	with open(path + request_file, 'r', encoding='utf-8') as f:
		sRequestJson = f.read()
	with open(path + cost_dir + cost_file, 'r', encoding='utf-8') as f:
		sCostJson = f.read()
	oParamJSON = {"requestJSON": json.loads(sRequestJson),
				  "costJSON": json.loads(sCostJson)
				  }
	return oParamJSON

def APItest_common(test_no, request_file, cost_file, logon_flag, caplog, addCheckFunc=None, continuity_test_flag=False, timeout_flag=False, calc_time=-1):
	now = datetime.datetime.now()
	print(sys._getframe().f_code.co_name + ' : ' + test_no, ' StartTime:', now)
	oParamJSON = create_JSON(request_file, cost_file)
	driver.driver(test_no, json.dumps(oParamJSON), logon_flag, caplog, addCheckFunc, continuity_test_flag, timeout_flag, calc_time)
	
def APItest_command_line(test_no, request_file, caplog, continuity_test_flag=False):
	now = datetime.datetime.now()
	print(sys._getframe().f_code.co_name + ' : ' + test_no, ' StartTime:', now)
	with open(path + request_file, 'r', encoding='utf-8') as f:
		sRequestJson = json.loads(f.read())
	driver.driver_command_line(test_no, sRequestJson, caplog, continuity_test_flag)
	
#-----------------------------------------------------------------
# メイン関数(Pytest実行しない場合は下記'__main__'から呼び出す)
#-----------------------------------------------------------------
if __name__ == '__main__':
	print("pid =",os.getpid())

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_1_1.test_args:
	# 	test_APItest_1_1(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_1_2.test_args:
	# 	test_APItest_1_2(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_2.test_args:
	# 	test_APItest_2(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_3.test_args:
	# 	test_APItest_3(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_4.test_args:
	# 	test_APItest_4(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_5.test_args:
	# 	test_APItest_5(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_6.test_args:
	# 	test_APItest_6(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_7.test_args:
	# 	test_APItest_7(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_8.test_args:
	# 	test_APItest_8(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_9.test_args:
	# 	test_APItest_9(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_10.test_args:
	# 	test_APItest_10(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_11.test_args:
	# 	test_APItest_11(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_12.test_args:
	# 	test_APItest_12(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_13.test_args:
	# 	test_APItest_13(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_14.test_args:
	# 	test_APItest_14(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_15.test_args:
	# 	test_APItest_15(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_16.test_args:
	# 	test_APItest_16(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_17.test_args:
	# 	test_APItest_17(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_18.test_args:
	# 	test_APItest_18(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_19.test_args:
	# 	test_APItest_19(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_20.test_args:
	# 	test_APItest_20(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_21.test_args:
	# 	test_APItest_21(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_23.test_args:
	# 	test_APItest_23(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_24.test_args:
	# 	test_APItest_24(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_25_2.test_args:
	# 	test_APItest_25_2(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_27.test_args:
	# 	test_APItest_27(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_28.test_args:
	# 	test_APItest_28(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_29.test_args:
	# 	test_APItest_29(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_30.test_args:
	# 	test_APItest_30(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_31.test_args:
	# 	test_APItest_31(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_32.test_args:
	# 	test_APItest_32(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_33.test_args:
	# 	test_APItest_33(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_35.test_args:
	# 	test_APItest_35(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_36.test_args:
	# 	test_APItest_36(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file in test_apiTest_patern_37.test_args:
	# 	test_APItest_37(test_no, request_file, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_38_1.test_args:
	# 	test_APItest_38_1(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_38_2.test_args:
	# 	test_APItest_38_2(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_39_1.test_args:
	# 	test_APItest_39_1(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_39_2.test_args:
	# 	test_APItest_39_2(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_40.test_args:
	# 	test_APItest_40(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_41.test_args:
	# 	test_APItest_41(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_42.test_args:
	# 	test_APItest_42(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_43.test_args:
	# 	test_APItest_43(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_44.test_args:
	# 	test_APItest_44(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_45.test_args:
	# 	test_APItest_45(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_46.test_args:
	# 	test_APItest_46(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_47.test_args:
	# 	test_APItest_47(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_48.test_args:
	# 	test_APItest_48(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_49.test_args:
	# 	test_APItest_49(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_50.test_args:
	# 	test_APItest_50(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_51.test_args:
	# 	test_APItest_51(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_patern_arpa_1.test_args:
	# 	test_APItest_arpa_1(test_no, request_file, cost_file, logon_flag, None)

	# for test_no, request_file, cost_file, logon_flag in test_apiTest_random.test_args:
	# 	test_APItest_random(test_no, request_file, cost_file, logon_flag, None)
	
	# for test_no, request_file, cost_file, logon_flag in test_apiTest_random_bugfix.test_args:
	# 	test_APItest_random_bugfix(test_no, request_file, cost_file, logon_flag, None)
	

