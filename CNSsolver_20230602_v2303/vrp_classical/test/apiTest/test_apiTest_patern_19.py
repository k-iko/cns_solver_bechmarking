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
# テスト19(初期解生成)
#-----------------------------------------------------------------
# 記載のないものは距離は1000、時間は1で固定、配送時間枠は2000/1/1 9:00:00～2000/1/1 17:00:00で固定、荷作業時間は1で固定
# *但し拠点と最終訪問先の配送時間枠は1000/1/1 0:00:00～3000/1/1 0:00:00

test_args=[
	# ■異常系
	# 1. 車両数 : 1、配送先数 : 1(ivnum=0)とした場合に
	# "vehicle number (aka ivnum) set to other than 0 or positive integer"のエラーメッセージを出力して終了することを確認
	#('19_1_1', '19_1_1_request.json','2x2_cost.json', True),
	# ⇒ivnum変更のテストが不能であるため削除

	# 2. 車両数 : 1、配送先数 : 1(ivnum="1")とした場合に
	# "vehicle number (aka ivnum) set to other than 0 or positive integer"のエラーメッセージを出力して終了することを確認
	#('19_1_2', '19_1_2_request.json','2x2_cost.json', True),
	# ⇒ivnum変更のテストが不能であるため削除

	# 3. 車両数 : 1、配送先数 : 1(ivnum=1.5)とした場合に
	# "vehicle number (aka ivnum) set to other than 0 or positive integer"のエラーメッセージを出力して終了することを確認
	#('19_1_3', '19_1_3_request.json','2x2_cost.json', True),
	# ⇒ivnum変更のテストが不能であるため削除

	# 4. 車両数 : 1、配送先数 : 1(ivnum=2)とした場合に
	# "vehicle number differs from vehicle(class) length"のエラーメッセージを出力して終了することを確認
	#('19_1_4', '19_1_4_request.json','2x2_cost.json', True),
	# ⇒ivnum変更のテストが不能であるため削除

	# 5. 車両数 : 1、配送先数 : 1(opskill=-1,skillfile="dummy")とした場合に
	# "delivery skill (aka opskill) set to less than 0.0"のエラーメッセージを出力して終了することを確認
	('19_1_5', '19_1_5_request.json','2x2_cost.json', True),

	# 6. 車両数 : 1、配送先数 : 1(drskill=-1,skillfile="dummy")とした場合に
	# "driving skill (aka drskill) set to less than 0.0"のエラーメッセージを出力して終了することを確認
	('19_1_6', '19_1_6_request.json','2x2_cost.json', True),

	# 7. 車両数 : 1、配送先数 : 1(drskill=0.5,skillfile=None)とした場合に
	# "skills argument must be set to none when skillfile field of Argvs object set none"のエラーメッセージを出力して終了することを確認
	('19_1_7', '19_1_7_request.json','2x2_cost.json', True),

	# 8. 車両数 : 1、配送先数 : 1(配送先に車両は立寄不可)とした場合に
	# "failed to create initial solution due to cost of -1 or unvisitable vehicles or max visits or fixed vehicles"のエラーメッセージを出力して終了することを確認
	('19_1_8', '19_1_8_request.json','2x2_cost.json', True),

	# 9. 車両数 : 1、配送先数 : 2(1→2および2→1は通行不可)とした場合に
	# "failed to create initial solution due to cost of -1 or unvisitable vehicles or max visits or fixed vehicles"のエラーメッセージを出力して終了することを確認
	('19_1_9', '19_1_9_request.json','3x3_cost_19_1_9.json', True),

	# 10. 車両数 : 2、配送先数 : 2(1→2および2→1は通行不可、1番目の配送先及び2番目の配送先に1台目の車両は立寄不可)とした場合に
	# "failed to create initial solution due to cost of -1 or unvisitable vehicles or max visits or fixed vehicles"のエラーメッセージを出力して終了することを確認
	('19_1_10', '19_1_10_request.json','3x3_cost_19_1_10.json', True),

	# 25. 車両数 : 2、配送先数 : 2(1番目および2番目の配送先に2台目の車両は立寄不可)とした場合に
	# "failed to create initial solution due to cost of -1 or unvisitable vehicles or max visits or fixed vehicles"のエラーメッセージを出力して終了することを確認
	('19_1_25', '19_1_25_request.json','3x3_cost.json', True),

	# ■正常系
	# 11. 車両数 : 2、配送先数 : 3(荷物量 300、300、150)とした場合に
	# 300の荷物で1台、300+150の荷物で1台となることを確認(積載量違反解しか作れなくても初期解作成)
	('19_1_11', '19_1_11_request.json','4x4_cost.json', True),

	# 12. 車両数 : 2、配送先数 : 3(荷物量 400、150、150)とした場合に
	# 400の荷物で1台、150+150の荷物で1台となることを確認(積載量違反解(1つの配送先)しか作れなくても初期解作成)
	('19_1_12', '19_1_12_request.json','4x4_cost.json', True),

	# 13. 車両数 : 2、配送先数 : 3(配送時間枠 2000/1/1 9:00:00～2000/1/1 10:00:00、2000/1/1 9:00:00～2000/1/1 10:00:00、2000/1/1 9:30:00～2000/1/1 10:30:00)とした場合に
	# 2000/1/1 9:00:00～2000/1/1 10:00:00の配送先で1台、
	# 2000/1/1 9:00:00～2000/1/1 10:00:00+2000/1/1 9:30:00～2000/1/1 10:30:00の配送先で1台となることを確認(配送時間枠違反解しか作れなくても初期解作成)
	('19_1_13', '19_1_13_request.json','4x4_cost.json', True),

	# 14. 車両数 : 2、配送先数 : 3(配送時間枠 2000/1/1 18:00:00～2000/1/1 19:00:00、2000/1/1 19:00:00～2000/1/1 20:00:00、2000/1/1 10:00:00～2000/1/1 12:00:00)とした場合に
	# 2000/1/1 10:00:00～2000/1/1 12:00:00の配送先で1台、
	#  2000/1/1 18:00:00～2000/1/1 19:00:00+2000/1/1 19:00:00～2000/1/1 20:00:00の配送先で1台となることを確認(車両営業時間枠違反解しか作れなくても初期解作成)
	('19_1_14', '19_1_14_request.json','4x4_cost.json', True),

	# 15. 車両数 : 2、配送先数 : 2(移動時間 0→1で5)、avetime_ape=0.1とした場合に
	# それぞれの車両で配送先が1つずつとなることを確認(時間のAPEに違反する解しか作れなくても初期解作成)
	('19_1_15', '19_1_15_request.json','3x3_cost_19_1_15.json', True),

	# 16. 車両数 : 2、配送先数 : 1とした場合に
	# 車両が1台となること(残りの1台は空)を確認(車両数>配送先数)
	# 正常なルート(配送先が含まれるルート)が車両台数未満となり、配送計画出力要件を満たさないが、本テストパターンではエラーとしない
	('19_1_16', '19_1_16_request.json','2x2_cost.json', True),

	# 17. 車両数 : 2、配送先数 : 6(移動時間はすべて3、opskill=0.5、1.0、荷作業時間は4のみ2、1番目、3番目、5番目の配送先に2台目の車両は立寄不可、2番目、4番目、6番目の配送先に1台目の車両は立寄不可)、
	# management_dtable : 
	#  [{vtype : "common",start_time : 0,table : 2→4で10000、0→5で100},
	#   {vtype : "time_common",start_time : 3,table : 2→4で100、0→5で10000},
	#   {vtype : "0",start_time : 3,table : 5→1で100、0→2で10000},
	#   {vtype : "0",start_time : 10,table : 2→4および0→5は通行不可},
	#   {vtype : "1",start_time : 0,table : 0→2で100、5→1で10000},
	#   {vtype : "3",start_time : 3,table : 2→4および0→5は通行不可}]とした場合に
	# 0→5→1→3→0(opskill=0.5)および0→2→6→4→0(opskill=1.0)となることを確認(車両ごとにテーブルが異なるためスキルシャッフルは機能しない)
	('19_1_17', '19_1_17_request.json','7x7_cost_19_1_17.json', True),
# 
	# 18. 車両数 : 2、配送先数 : 6(移動時間はすべて3、opskill=0.5、1.0、荷作業時間は4のみ2)、
	# management_dtable : 
	#  [{vtype : "time_common",start_time : 0,table : 1→3および4→6で10000、0→2および0→5で100},
	#   {vtype : "time_common",start_time : 3,table : 1→3および4→6で100、0→2および0→5で10000}]とした場合に
	# 0→2→1→3→0(opskill=1.0)および0→5→4→6→0(opskill=0.5)となることを確認(同じテーブルを参照するためスキルシャッフル)
	('19_1_18', '19_1_18_request.json','7x7_cost_19_1_18.json', True),

	# 19. 車両数 : 2、配送先数 : 6(移動時間はすべて3、opskill=0.5、1.0、荷作業時間は4のみ2、1番目、2番目、3番目の配送先に2台目の車両は立寄不可、4番目、5番目、6番目の配送先に1台目の車両は立寄不可)、
	# management_dtable : 
	#  [{vtype : "time_common",start_time : 0,table : 1→3および4→6で10000、0→2および0→5で100},
	#   {vtype : "0",start_time : 0,table : 1→3および4→6で10000、0→2および0→5で100},
	#   {vtype : "0",start_time : 3,table : 1→3および4→6で100、0→2および0→5で10000},
	#   {vtype : "1",start_time : 3,table : 1→3および4→6で100、0→2および0→5で10000}]とした場合に
	# 0→2→1→3→0(opskill=1.0)および0→5→4→6→0(opskill=0.5)となることを確認(別のテーブルだが中身は同じテーブルを参照するためスキルシャッフル)
	# ('19_1_19', '19_1_19_request.json','7x7_cost_19_1_19.json', True),
	#仕様変更によりテスト不可

	# 20. 車両数 : 2、配送先数 : 4(1→3および3→1および2→3および3→2および1→2および2→1および4→3および1→4および2→4および4→2を通行不可)とした場合に
	# 0→3→4→1→0および0→2→0となることを確認(通行不可)
	('19_1_20', '19_1_20_request.json','5x5_cost_19_1_20.json', True),

	# 21. 車両数 : 2、配送先数 : 3(1番目および2番目の配送先に2台目の車両は立寄不可)とした場合に
	# 1台目に2つの配送先、2台目に1つの配送先となることを確認(立寄不可)
	('19_1_21', '19_1_21_request.json','4x4_cost.json', True),

	# 22. 車両数 : 2、配送先数 : 2(最終訪問先 custno="2"およびspotid="2"、custno="3"およびspotid="3")とした場合に
	# 0→1→2および0→2→3となることを確認(最終訪問先指定)
	('19_1_22', '19_1_22_request.json','4x4_cost.json', True),

	# 23. 車両数 : 2、配送先数 : 2(移動時間 1→2で10、2→1で5)、車両削減モードとした場合に
	# 0→2→1→0となることを確認(車両削減モード)
	('19_1_23', '19_1_23_request.json','3x3_cost_19_1_23.json', True),

	# 24. 車両数 : 2、配送先数 : 2(1番目および2番目の配送先に2台目の車両は立寄不可)、車両削減モードとした場合に
	# 車両が1台となることを確認(立寄不可、車両削減モード)
	('19_1_24', '19_1_24_request.json','3x3_cost.json', True),

	# 26. 車両数 : 2、配送先数 : 3(1台目の訪問数上限2、3番目の配送先に2台目の車両は立寄不可)とした場合に
 	# 0→3→0および0→2→1→0となることを確認(立寄不可が多い車両から生成)
	('19_1_26', '19_1_26_request.json','4x4_cost.json', True),

	# 27. 車両数 : 3、配送先数 : 4(2台目の訪問数上限1、1番目の配送先に1、2台目の車両は立寄不可、2番目の配送先に1台目の車両は立寄不可、3番目の配送先に1、3台目の車両は立寄不可、4番目の配送先に2台目の車両は立寄不可)とした場合に
 	# 0→4→0および0→3→0および0→2→1→0となることを確認(立寄不可が多い車両から生成(最も良いものは選べない))
	('19_1_27', '19_1_27_request.json','5x5_cost.json', True),

]

