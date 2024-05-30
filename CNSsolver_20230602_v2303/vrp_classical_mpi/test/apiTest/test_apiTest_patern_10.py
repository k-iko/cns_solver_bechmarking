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
# テスト10(時間別車両別時間ファイル対応)
#-----------------------------------------------------------------
# 指定のないものは距離は1000、時間は1で固定、荷作業時間は1、配送時間枠および車両営業時間は2000/1/1 9:00:00～2000/1/1 20:00:00で固定
# *但し拠点と最終訪問先の配送時間枠は1000/1/1 0:00:00～3000/1/1 0:00:00

test_args=[
	# ■異常系
	# 1. 車両数 : 1、配送先数 : 1、management_ttable=[]とした場合に
	# "eta table (aka ttable) does not exist"のエラーメッセージを出力して終了することを確認
	('10_1_1', '10_1_1_request.json','2x2_cost_ttable_empty.json', True),

	# 2. 車両数 : 1、配送先数 : 1、management_ttable : [{vtype : "time_common",start_time : 1,table : デフォルト}]とした場合に
	# "eta table (aka ttable) does not exist"のエラーメッセージを出力して終了することを確認(9:00に参照するテーブルがないため)
	('10_1_2', '10_1_2_request.json','2x2_cost_ttable_time_common_only.json', True),

	# 3. 車両数 : 1、配送先数 : 1、management_ttable : [{vtype : "1",start_time : 0,table : デフォルト}]とした場合に
	# "eta table (aka ttable) does not exist"のエラーメッセージを出力して終了することを確認(vehno="0"が参照するテーブルがないため)
	('10_1_3', '10_1_3_request.json','2x2_cost_ttable_vtype.json', True),

	# 4. 車両数 : 1、配送先数 : 1、management_ttable : [{vtype : "time_common",start_time : 1,table : デフォルト},{vtype : "1",start_time : 0,table : デフォルト}]とした場合に
	# "eta table (aka ttable) does not exist"のエラーメッセージを出力して終了することを確認(vehno="0"、9:00が参照するテーブルがないため)
	('10_1_4', '10_1_4_request.json','2x2_cost_10_1_4.json', True),

	# 5. 車両数 : 1、配送先数 : 3(初期解 0→1→2→3→0)、
	# management_ttable : [{vtype : "time_common",start_time : 0,table : 全て３},{vtype : "time_common",start_time : 3,table : 2→3は通行不可、その他は3}]とした場合に
	# "failed to read initial solution due to cost of -1"のエラーメッセージを出力して終了することを確認(時間別テーブル)
	('10_1_5', '10_1_5_request.json','4x4_cost_10_1_5.json', True),

	# 6. 車両数 : 2、配送先数 : 4(初期解 0→1→2→0および0→3→4→0)、
	# management_ttable : [{vtype : "0",start_time : 0,table : 全て３},{vtype : "1",start_time : 0,table : 3→4は通行不可、その他は3}]とした場合に
	# "failed to read initial solution due to cost of -1"のエラーメッセージを出力して終了することを確認(車両別テーブル)
	('10_1_6', '10_1_6_request.json','5x5_cost_10_1_6.json', True),

	# 22. 車両数 : 1、配送先数 : 1、management_ttable : [{vtype : "1",start_time : 0,table : デフォルト},{vtype : "1",start_time : 0,table : 全て2}]とした場合に
	# "same eta tables (aka ttable) exist"のエラーメッセージを出力して終了することを確認(vtype : "1",start_time : 0のテーブルが複数あるため)
	('10_1_22', '10_1_22_request.json','2x2_cost_10_1_22.json', True),

	# 23. 車両数 : 1、配送先数 : 1、management_ttable : [{vtype : "common",start_time : 0,table : デフォルト},{vtype : "common",start_time : 1,table : 全て2}]とした場合に
	# "same eta tables (aka ttable) exist"のエラーメッセージを出力して終了することを確認(vtype : "common"のテーブルが複数あるため)
	('10_1_23', '10_1_23_request.json','2x2_cost_10_1_23.json', True),

	# ■正常系
	# 7. 車両数 : 1、配送先数 : 3(初期解 0→1→2→3→0)、
	# management_ttable : 
	#  [{vtype : "time_common",start_time : 0,table : 1→3で10、0→2で3、その他は4},
	#   {vtype : "time_common",start_time : 3,table : 1→3で3、0→2で10、その他は4}]とした場合に
	# 0→2→1→3→0となることを確認(時間別、commonなし)
	('10_1_7', '10_1_7_request.json','4x4_cost_10_1_7.json', True),

	# 8. 車両数 : 1、配送先数 : 3(初期解 0→1→2→3→0)、
	# management_ttable : 
	#  [{vtype : "common",start_time : 0,table : 1→3で10、0→2で3、その他は4},
	#   {vtype : "time_common",start_time : 3,table : 1→3で3、0→2で10、その他は4},
	#   {vtype : "time_common",start_time : 10,table : 1→3および0→2は通行不可、その他は4}]とした場合に
	# 0→2→1→3→0となることを確認(時間別、commonあり)
	('10_1_8', '10_1_8_request.json','4x4_cost_10_1_8.json', True),

	# 9. 車両数 : 1、配送先数 : 3(初期解 0→1→2→3→0)、
	# management_ttable : 
	#  [{vtype : "common",start_time : 0,table : 1→3および0→2は通行不可},
	#   {vtype : "time_common",start_time : 0,table : 1→3で10、0→2で3、その他は4},
	#   {vtype : "time_common",start_time : 3,table : 1→3で3、0→2で10、その他は4}]とした場合に
	# 0→2→1→3→0となることを確認(時間別、commonあり、time_commonが優先される)
	('10_1_9', '10_1_9_request.json','4x4_cost_10_1_9.json', True),

	# 10. 車両数 : 2、配送先数 : 4(初期解 0→1→2→0および0→3→4→0)、
	# management_ttable : 
	#  [{vtype : "0",start_time : 0,table : 1→4で10、0→3で3、その他は6},
	#   {vtype : "1",start_time : 0,table : 0→1で3、3→2で10、その他は6}]とした場合に
	# 0→3→2→0および0→1→4→0となることを確認(車両別、commonとtime_commonなし)
	('10_1_10', '10_1_10_request.json','5x5_cost_10_1_10.json', True),

	# 11. 車両数 : 2、配送先数 : 4(初期解 0→1→2→0および0→3→4→0)、
	# management_ttable : 
	#  [{vtype : "time_common",start_time : 0,table : 1→4で10、0→3で3、その他は6},
	#   {vtype : "1",start_time : 0,table : 0→1で3、3→2で10、その他は6},
	#   {vtype : "3",start_time : 0,table : 1→4および0→3は通行不可、その他は6}]とした場合に
	# 0→3→2→0および0→1→4→0となることを確認(車両別、time_commonあり)
	('10_1_11', '10_1_11_request.json','5x5_cost_10_1_11.json', True),

	# 12. 車両数 : 2、配送先数 : 4(初期解 0→1→2→0および0→3→4→0)、
	# management_ttable : 
	#  [{vtype : "common",start_time : 0,table : 1→4で10、0→3で3、その他は6},
	#   {vtype : "1",start_time : 0,table : 0→1で3、3→2で10、その他は6},
	#   {vtype : "3",start_time : 0,table : 1→4および0→3は通行不可、その他は6}]とした場合に
	# 0→3→2→0および0→1→4→0となることを確認(車両別、commonあり)
	('10_1_12', '10_1_12_request.json','5x5_cost_10_1_12.json', True),

	# 13. 車両数 : 2、配送先数 : 6(初期解 0→1→2→3→0および0→4→5→6→0)、
	# management_ttable : 
	#  [{vtype : "0",start_time : 0,table : 2→4で10、0→5で3、その他は4},
	#   {vtype : "1",start_time : 0,table : 0→2で3、5→1で10、その他は4},
	#   {vtype : "0",start_time : 3,table : 5→1で3、0→2で10、その他は4},
	#   {vtype : "1",start_time : 3,table : 2→4で3、0→5で10、その他は4}]とした場合に
	# 0→5→1→3→0および0→2→4→6→0となることを確認(時間別車両別、commonとtime_commonなし)
	('10_1_13', '10_1_13_request.json','7x7_cost_10_1_13.json', True),

	# 14. 車両数 : 2、配送先数 : 6(初期解 0→1→2→3→0および0→4→5→6→0)、
	# management_ttable : 
	#  [{vtype : "common",start_time : 0,table : 2→4で10、0→5で3、その他は4},
	#   {vtype : "time_common",start_time : 3,table : 2→4で3、0→5で10、その他は4},
	#   {vtype : "0",start_time : 3,table : 5→1で3、0→2で10、その他は4},
	#   {vtype : "0",start_time : 10,table : 2→4および0→5は通行不可、その他は4},
	#   {vtype : "1",start_time : 0,table : 0→2で3、5→1で10、その他は4},
	#   {vtype : "3",start_time : 3,table : 2→4および0→5は通行不可、その他は4}]とした場合に
	# 0→5→1→3→0および0→2→4→6→0となることを確認(時間別車両別、commonとtime_commonあり)
	('10_1_14', '10_1_14_request.json','7x7_cost_10_1_14.json', True),

	# 15. 車両数 : 2、配送先数 : 6(2台目の営業時間枠 2000/1/1 8:00:00～2000/1/1 20:00:00、初期解 0→1→2→3→0および0→4→5→6→0)、
	# management_ttable : 
	#  [{vtype : "common",start_time : 0,table : 2→4で10、0→5で3、その他は4},
	#   {vtype : "time_common",start_time : 2,table : 2→4で3、0→5で10、その他は4},
	#   {vtype : "0",start_time : 3,table : 5→1で3、0→2で10、その他は4},
	#   {vtype : "0",start_time : 10,table : 2→4および0→5は通行不可、その他は4},
	#   {vtype : "1",start_time : 0,table : 0→2で3、5→1で10、その他は4},
	#   {vtype : "2",start_time : 3,table : 2→4および0→5は通行不可、その他は4}]とした場合に
	# 0→5→1→3→0および0→2→4→6→0となることを確認(時間別車両別、commonとtime_commonあり、配送時間マイナス)
	('10_1_15', '10_1_15_request.json','7x7_cost_10_1_15.json', True),

	# 16. 車両数 : 2、配送先数 : 6(初期解 0→1→2→3→0および0→4→5→6→0)、
	# management_ttable : 
	#  [{vtype : "3",start_time : 3,table : 2→4および0→5は通行不可、その他は4},
	#   {vtype : "time_common",start_time : 3,table : 2→4で3、0→5で10、その他は4},
	#   {vtype : "0",start_time : 10,table : 2→4および0→5は通行不可、その他は4},
	#   {vtype : "common",start_time : 0,table : 2→4で10、0→5で3、その他は4},
	#   {vtype : "0",start_time : 3,table : 5→1で3、0→2で10、その他は4},
	#   {vtype : "1",start_time : 0,table : 0→2で3、5→1で10、その他は4}]とした場合に
	# 0→5→1→3→0および0→2→4→6→0となることを確認(時間別車両別、commonとtime_commonあり、順不同)
	('10_1_16', '10_1_16_request.json','7x7_cost_10_1_16.json', True),

	# 17. 車両数 : 1、配送先数 : 3(初期解 0→1→2→3→0)、
	# management_ttable : 
	#  [{vtype : "common",start_time : 0,table : 全て通行不可},
	#   {vtype : "time_common",start_time : 0,table : 1→3は通行不可、0→2で3、その他は4},
	#   {vtype : "time_common",start_time : 3,table : 1→3で3、0→2は通行不可、その他は4}]とした場合に
	# 0→2→1→3→0となることを確認(通行不可)
	('10_1_17', '10_1_17_request.json','4x4_cost_10_1_17.json', True),

	# 18. 車両数 : 2、配送先数 : 6(opskill=0.5、1.0、荷作業時間は4のみ2、初期解 0→1→2→3→0および0→4→5→6→0)、
	# management_ttable : 
	#  [{vtype : "common",start_time : 0,table : 2→4で10、0→5で3、その他は4},
	#   {vtype : "time_common",start_time : 3,table : 0→5で10、その他は4},
	#   {vtype : "0",start_time : 3,table : 5→1で3、0→2で10、その他は4},
	#   {vtype : "0",start_time : 10,table : 2→4および0→5は通行不可、その他は4},
	#   {vtype : "1",start_time : 0,table : 0→2で2、5→1で10、その他は4},
	#   {vtype : "3",start_time : 3,table : 2→4および0→5は通行不可、その他は4}]とした場合に
	# 0→5→1→3→0(opskill=0.5)および0→2→4→6→0(opskill=1.0)となることを確認(車両ごとにテーブルが異なるためスキルシャッフルは機能しない)
	('10_1_18', '10_1_18_request.json','7x7_cost_10_1_18.json', True),

	# 19. 車両数 : 2、配送先数 : 6(opskill=0.5、1.0、荷作業時間は4のみ2、初期解 0→1→2→3→0および0→4→5→6→0)、
	# management_ttable : 
	#  [{vtype : "time_common",start_time : 0,table : 1→3および4→6で10、0→2および0→5で3、その他は4},
	#   {vtype : "time_common",start_time : 3,table : 1→3および4→6で3、0→2および0→5で10、その他は4}]とした場合に
	# 0→2→1→3→0(opskill=1.0)および0→5→4→6→0(opskill=0.5)となることを確認(同じテーブルを参照するためスキルシャッフル)
	('10_1_19', '10_1_19_request.json','7x7_cost_10_1_19.json', True),

	# # 20. 車両数 : 2、配送先数 : 6(opskill=0.5、1.0、荷作業時間は4のみ2、初期解 0→1→2→3→0および0→4→5→6→0)、
	# # management_ttable : 
	# #  [{vtype : "time_common",start_time : 0,table : 1→3および4→6で10、0→2および0→5で3、その他は4},
	# #   {vtype : "0",start_time : 0,table : 1→3および4→6で10、0→2および0→5で3、その他は4},
	# #   {vtype : "0",start_time : 3,table : 1→3および4→6で3、0→2および0→5で10、その他は4},
	# #   {vtype : "1",start_time : 3,table : 1→3および4→6で3、0→2および0→5で10、その他は4}]とした場合に
	# # 0→2→1→3→0(opskill=1.0)および0→5→4→6→0(oprskill=0.5)となることを確認(別のテーブルだが中身は同じテーブルを参照するためスキルシャッフル)
	# ('10_1_20', '10_1_20_request.json','7x7_cost_10_1_20.json', True),
	#仕様変更によりテスト不可

	# 21. 車両数 : 1、配送先数 : 3(初期解 0→1→2→3→0)、
	# management_dtable : 
	#  [{vtype : "time_common",start_time : 0,table : 1→3で10000、0→2で100},
	#   {vtype : "time_common",start_time : 5,table : 1→3で100}]とした場合に
	# management_ttable : 
	#  [{vtype : "time_common",start_time : 0,table : 1→3で10、0→2で3、その他は4},
	#   {vtype : "time_common",start_time : 3,table : 0→2で10、その他は4}]とした場合に
	# 0→2→1→3→0となることを確認(dtableとttableの時間枠が異なる場合)
	('10_1_21', '10_1_21_request.json','4x4_cost_10_1_21.json', True),
	
]

