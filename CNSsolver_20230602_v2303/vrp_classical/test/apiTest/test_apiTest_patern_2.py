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

#-----------------------------------------------------------------
# テスト2(初期解入力)
#-----------------------------------------------------------------
# 記載のないものは距離は1000、時間は1で固定、配送時間枠は2000/1/1 9:00:00～2000/1/1 17:00:00で固定、荷作業時間は1で固定
# *但し拠点と最終訪問先の配送時間枠は1000/1/1 0:00:00～3000/1/1 0:00:00
test_args=[
	# ■異常系
	# 1. 車両数 : 1、配送先数 : 1(初期解の訪問先のID 実数の文字列)とした場合に
	# "customer id (aka custno) set to not as string of integer"のエラーメッセージを出力して終了することを確認
	('2_1_1', '2_1_1_request.json','2x2_cost.json', True),

	# 2. 車両数 : 1、配送先数 : 1(初期解の訪問先のID e等を含む文字列)とした場合に
	# "customer id (aka custno) set to not as string of integer"のエラーメッセージを出力して終了することを確認
	('2_1_2', '2_1_2_request.json','2x2_cost.json', True),

	# 3. 車両数 : 1、配送先数 : 1(初期解の訪問先のID 負の整数の文字列)とした場合に
	# "customer id (aka custno) set to not as string of integer"のエラーメッセージを出力して終了することを確認
	('2_1_3', '2_1_3_request.json','2x2_cost.json', True),

	# 4. 車両数 : 1、配送先数 : 2(初期解の訪問先のIDが配送先のcustnoと一致しない(個数は同じ))とした場合に
	# "all custno of customer class (aka _custs) must include all customer id (aka custno) of iniroutes"のエラーメッセージを出力して終了することを確認
	('2_1_4', '2_1_4_request.json','2x2_cost.json', True),

	# 5. 車両数 : 1、配送先数 : 2(初期解の訪問先のIDが過剰)とした場合に
	# "all custno of customer class (aka _custs) must include all customer id (aka custno) of iniroutes"のエラーメッセージを出力して終了することを確認
	('2_1_5', '2_1_5_request.json','2x2_cost.json', True),

	# 7. 車両数 : 1、配送先数 : 1(初期解に空の車両のみ含まれている(initroutes=[[]]))とした場合に
	# "initial solution (aka iniroutes) includes plan without customers"のエラーメッセージを出力して終了することを確認
	('2_1_7', '2_1_7_request.json','2x2_cost.json', True),

	# 9. 車両数 : 2、配送先数 : 3(初期解の台数>_vehsの台数)とした場合に
	# "initial solution (aka iniroutes) length is more than length of vehicle class (aka _vehs)"のエラーメッセージを出力して終了することを確認
	('2_1_9', '2_1_9_request.json','3x3_cost.json', True),

	# 10. 車両数 : 2、配送先数 : 1(初期解の訪問先数が2(拠点と最終訪問先のみ))とした場合に
	# "initial solution (aka iniroutes) includes plan without customers"のエラーメッセージを出力して終了することを確認
	('2_1_10', '2_1_10_request.json','2x2_cost.json', True),

	# 11. 車両数 : 1、配送先数 : 2(初期解が0→1→2→0だが1→2が通行不可)とした場合に
	# "failed to read initial solution due to cost of -1"のエラーメッセージを出力して終了することを確認
	('2_1_11', '2_1_11_request.json','3x3_cost_Impassable.json', True),

	# 12. 車両数 : 2、配送先数 : 2(初期解で1番目の配送先が1台目に含まれているが立寄不可)とした場合に
	# "failed to read initial solution due to unvisitable vehicle"のエラーメッセージを出力して終了することを確認
	('2_1_12', '2_1_12_request.json','3x3_cost.json', True),

	# 13. 車両数 : 2、配送先数 : 3(初期解で配送数上限が1の車両に2つの配送先が存在する)とした場合に
	# "initial solution (aka iniroutes) includes plan with over max visits (aka maxvisit)"のエラーメッセージを出力して終了することを確認
	('2_1_13', '2_1_13_request.json','4x4_cost.json', True),


	# ■正常系
	# #16. 車両数 : 2、配送先数 : 2(初期解ファイルの1列目が上から"first_vehicle","second_vehicle")とした場合に
	# #1列目の文字列に関係なく上から順に1台目の車両にfirst_vehicleの配送先が格納されることを確認
	('2_1_16', '2_1_16_request.json','3x3_cost.json', True),
	# #17. 車両数 : 1、配送先数 : 1(積載量 500)とした場合に
	# #初期解が作成されることを確認(積載量違反の解しか作れなくても初期解を作成する)
	('2_1_17', '2_1_17_request.json','2x2_cost.json', True),
]
