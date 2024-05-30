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
# テスト3(積載量指定)
#-----------------------------------------------------------------
#記載のないものは距離は1000、時間は1で固定、配送時間枠は2000/1/1 9:00:00～2000/1/1 17:00:00で固定、荷作業時間は1で固定
# *但し拠点と最終訪問先の配送時間枠は1000/1/1 0:00:00～3000/1/1 0:00:00

test_args=[
	#■異常系
	#1. 車両数 : 1、配送先数 : 1(車両の積載量上限 -1)とした場合に
	# "max loading weight of vehicles (aka cap) set to less than 0"のエラーメッセージを出力して終了することを確認
	('3_1_1', '3_1_1_request.json','2x2_cost.json', True),

	#9. 車両数 : 1、配送先数 : 1(荷物量 -1)とした場合に
	# "loading weight of package (aka dem) set to less than 0"のエラーメッセージを出力して終了することを確認
	('3_1_9', '3_1_9_request.json','2x2_cost.json', True),

	#10. 車両数 : 1、配送先数 : 1(荷物の種類 DELIVER)とした場合に
	# "request type of load (aka requestType) set to other than "DELIVERY" or "PICKUP""のエラーメッセージを出力して終了することを確認
	('3_1_10', '3_1_10_request.json','2x2_cost.json', True),

	#■正常系
	#2. 車両数 : 2、配送先数 : 3(車両の積載量上限 100,200、各配送先の荷物量 100)とした場合に
	#積載量上限100の車両に1つの配送先、積載量上限200の車両に2つの配送先となることを確認
	# (初期解として、1台目に2箇所、2台目に1箇所の配送先数を指定→積載量上限に合わせて配送先数が改善されることを確認)
	('3_1_2', '3_1_2_request.json','4x4_cost.json', True),
	#3. 車両数 : 2、配送先数 : 2(車両の積載量上限 0,300、各配送先の荷物量 300,0)とした場合に
	#積載量上限0の車両に荷物量0の配送先、積載量上限300の車両に荷物量300の配送先となることを確認
	# (初期解として、1台目に300の荷物、2台目に0の荷物を配送先として指定→積載量上限に合わせて配送先数が改善されることを確認)
	('3_1_3', '3_1_3_request.json','3x3_cost.json', True),
	#4. 車両数 : 2、配送先数 : 4(車両の積載量上限 200,400、各配送先の荷物量 全て150)とした場合に
	#解が作成されることを確認(積載量上限の合計=荷物量の合計 だが、積載量違反となるパターン)
	# (初期解として、1台目に3箇所(150,150,150)の荷物、2台目に1箇所(150)の荷物を配送先として指定→1台目に1箇所(150)の荷物、2台目に3箇所(150,150,150)となることを確認
	('3_1_4', '3_1_4_request.json','5x5_cost.json', True),
	#5. 車両数 : 1、配送先数 : 1(車両の積載量上限 300、配送先の荷物量 全て100、拠点および最終訪問先の荷物量 1000)とした場合に
	#積載量が100となることを確認(拠点と最終配送先のdemは無視)
	# (初期解として、1台目に1箇所(150)の荷物を配送先として指定→改善会での最初に拠点で積む荷物量(vehicleのload_onbrd)が100になることを確認。
	('3_1_5', '3_1_5_request.json','2x2_cost.json', True),
	#6. 車両数 : 2、配送先数 : 4(車両の積載量上限 300,300、各配送先の荷物量 149,151,150,150、
	#配送時間枠 2000/1/1 9:00:00～2000/1/1 10:00:00,2000/1/1 9:00:00～2000/1/1 10:00:00,2000/1/1 13:00:00～2000/1/1 17:00:00,2000/1/1 13:00:00～2000/1/1 17:00:00)とした場合に
	# 各車両の配送先が0→1→3→0、0→2→4→0(車両については順不同)を確認(積載量を守るよりも配送時間枠を守ったほうがペナルティが小さい場合)
	('3_1_6', '3_1_6_request.json','5x5_cost.json', True),
	#7. 車両数 : 2、配送先数 : 4(車両の積載量上限 300,300、各配送先の荷物量 148,152,150,150、
	#配送時間枠 2000/1/1 9:00:00～2000/1/1 10:00:00,2000/1/1 9:00:00～2000/1/1 10:00:00,2000/1/1 13:00:00～2000/1/1 17:00:00,2000/1/1 13:00:00～2000/1/1 17:00:00)とした場合に
	#各車両の配送先が0→1→2→0、0→3→4→0(車両については順不同、1と2は順不同、3と4は順不同)を確認(配送時間枠を守るよりも積載量を守ったほうがペナルティが小さい場合)
	('3_1_7', '3_1_7_request.json','5x5_cost.json', True),
	#8. 車両数 : 2、配送先数 : 4(車両の積載量上限 300,300、各配送先の荷物量 50,100,200,300)とした場合に
	#1と4が同じ車両、2と3が同じ車両となることを確認(一番ペナルティが小さい組み合わせ)
	('3_1_8', '3_1_8_request.json','5x5_cost.json', True),
]
