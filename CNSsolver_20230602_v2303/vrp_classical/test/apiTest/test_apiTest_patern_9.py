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
# テスト9(時間別車両別距離ファイル対応)
#-----------------------------------------------------------------
# 指定のないものは距離は1000、時間は1で固定、荷作業時間は1、配送時間枠および車両営業時間は2000/1/1 9:00:00～2000/1/1 20:00:00で固定
# *但し拠点と最終訪問先の配送時間枠は1000/1/1 0:00:00～3000/1/1 0:00:00

test_args=[
	# ■異常系
	# 1. 車両数 : 1、配送先数 : 1、management_dtable=[]とした場合に
	# "distance table (aka dtable) does not exist"のエラーメッセージを出力して終了することを確認
	('9_1_1', '9_1_1_request.json','2x2_cost_dtable_empty.json', True),

	# 2. 車両数 : 1、配送先数 : 1、management_dtable : [{vtype : "time_common",start_time : 1,table : デフォルト}]とした場合に
	# "distance table (aka dtable) does not exist"のエラーメッセージを出力して終了することを確認(9:00に参照するテーブルがないため)
	('9_1_2', '9_1_2_request.json','2x2_cost_dtable_time_common_only.json', True),

	# 3. 車両数 : 1、配送先数 : 1、management_dtable : [{vtype : "1",start_time : 0,table : デフォルト}]とした場合に
	# "distance table (aka dtable) does not exist"のエラーメッセージを出力して終了することを確認(vehno="0"が参照するテーブルがないため)
	('9_1_3', '9_1_3_request.json','2x2_cost_vtype.json', True),

	# 4. 車両数 : 1、配送先数 : 1、management_dtable : [{vtype : "time_common",start_time : 1,table : デフォルト},{vtype : "1",start_time : 0,table : デフォルト}]とした場合に
	# "distance table (aka dtable) does not exist"のエラーメッセージを出力して終了することを確認(vehno="0"、9:00が参照するテーブルがないため)
	('9_1_4', '9_1_4_request.json','2x2_cost_time_common1_vtype1.json', True),

	# 5. 車両数 : 1、配送先数 : 3(移動時間はすべて3、初期解 0→1→2→3→0)、
	# management_dtable : [{vtype : "time_common",start_time : 0,table : デフォルト},{vtype : "time_common",start_time : 3,table : 2→3は通行不可}]とした場合に
	# "failed to read initial solution due to cost of -1"のエラーメッセージを出力して終了することを確認(時間別テーブル)
	('9_1_5', '9_1_5_request.json','4x4_cost_time_common0_time_common3.json', True),

	# 6. 車両数 : 2、配送先数 : 4(初期解 0→1→2→0および0→3→4→0)、
	# management_dtable : [{vtype : "0",start_time : 0,table : デフォルト},{vtype : "1",start_time : 0,table : 3→4は通行不可}]とした場合に
	# "failed to read initial solution due to cost of -1"のエラーメッセージを出力して終了することを確認(車両別テーブル)
	('9_1_6', '9_1_6_request.json','5x5_cost_time_common0_time_common0.json', True),

	# 21. 車両数 : 1、配送先数 : 1、management_dtable : [{vtype : "1",start_time : 0,table : デフォルト},{vtype : "1",start_time : 0,table : 全て2000}]とした場合に
	# "same distance tables (aka dtable) exist"のエラーメッセージを出力して終了することを確認(vtype : "1",start_time : 0のテーブルが複数あるため)
	('9_1_21', '9_1_21_request.json','2x2_cost_9_1_21.json', True),

	# 22. 車両数 : 1、配送先数 : 1、management_dtable : [{vtype : "common",start_time : 0,table : デフォルト},{vtype : "common",start_time : 1,table : 全て2000}]とした場合に
	# "same distance tables (aka dtable) exist"のエラーメッセージを出力して終了することを確認(vtype : "common"のテーブルが複数あるため)
	('9_1_22', '9_1_22_request.json','2x2_cost_9_1_22.json', True),

	# ■正常系
	# 7. 車両数 : 1、配送先数 : 3(移動時間はすべて3、初期解 0→1→2→3→0)、
	# management_dtable : 
	#  [{vtype : "time_common",start_time : 0,table : 1→3で10000、0→2で100},
	#   {vtype : "time_common",start_time : 3,table : 1→3で100、0→2で10000}]とした場合に
	# 0→2→1→3→0となることを確認(時間別、commonなし)
	('9_1_7', '9_1_7_request.json','4x4_cost_9_1_7.json', True),

	# 8. 車両数 : 1、配送先数 : 3(移動時間はすべて3、初期解 0→1→2→3→0)、
	# management_dtable : 
	#  [{vtype : "common",start_time : 0,table : 1→3で10000、0→2で100},
	#   {vtype : "time_common",start_time : 3,table : 1→3で100、0→2で10000},
	#   {vtype : "time_common",start_time : 10,table : 1→3および0→2は通行不可}]とした場合に
	# 0→2→1→3→0となることを確認(時間別、commonあり)
	('9_1_8', '9_1_8_request.json','4x4_cost_9_1_8.json', True),

	# 9. 車両数 : 1、配送先数 : 3(移動時間はすべて3、初期解 0→1→2→3→0)、
	# management_dtable : 
	#  [{vtype : "common",start_time : 0,table : 1→3および0→2は通行不可},
	#   {vtype : "time_common",start_time : 0,table : 1→3で10000、0→2で100},
	#   {vtype : "time_common",start_time : 3,table : 1→3で100、0→2で10000}]とした場合に
	# 0→2→1→3→0となることを確認(時間別、commonあり、time_commonが優先される)
	('9_1_9', '9_1_9_request.json','4x4_cost_9_1_9.json', True),

	# 10. 車両数 : 2、配送先数 : 4(初期解 0→1→2→0および0→3→4→0)、
	# management_dtable : 
	#  [{vtype : "0",start_time : 0,table : 1→4で10000、0→3で100},
	#   {vtype : "1",start_time : 0,table : 0→1で100、3→2で10000}]とした場合に
	# 0→3→2→0および0→1→4→0となることを確認(車両別、commonとtime_commonなし)
	('9_1_10', '9_1_10_request.json','5x5_cost_9_1_10.json', True),

	# 11. 車両数 : 2、配送先数 : 4(初期解 0→1→2→0および0→3→4→0)、
	# management_dtable : 
	#  [{vtype : "time_common",start_time : 0,table : 1→4で10000、0→3で100},
	#   {vtype : "1",start_time : 0,table : 0→1で100、3→2で10000},
	#   {vtype : "3",start_time : 0,table : 1→4および0→3は通行不可}]とした場合に
	# 0→3→2→0および0→1→4→0となることを確認(車両別、time_commonあり)
	('9_1_11', '9_1_11_request.json','5x5_cost_9_1_11.json', True),

	# 12. 車両数 : 2、配送先数 : 4(初期解 0→1→2→0および0→3→4→0)、
	# management_dtable : 
	#  [{vtype : "common",start_time : 0,table : 1→4で10000、0→3で100},
	#   {vtype : "1",start_time : 0,table : 0→1で100、3→2で10000},
	#   {vtype : "3",start_time : 0,table : 1→4および0→3は通行不可}]とした場合に
	# 0→3→2→0および0→1→4→0となることを確認(車両別、commonあり)
	('9_1_12', '9_1_12_request.json','5x5_cost_9_1_12.json', True),

	# 13. 車両数 : 2、配送先数 : 6(移動時間はすべて3、初期解 0→1→2→3→0および0→4→5→6→0)、
	# management_dtable : 
	#  [{vtype : "0",start_time : 0,table : 2→4で10000、0→5で100},
	#   {vtype : "1",start_time : 0,table : 0→2で100、5→1で10000},
	#   {vtype : "0",start_time : 3,table : 5→1で100、0→2で10000},
	#   {vtype : "1",start_time : 3,table : 2→4で100、0→5で10000}]とした場合に
	# 0→5→1→3→0および0→2→4→6→0となることを確認(時間別車両別、commonとtime_commonなし)
	('9_1_13', '9_1_13_request.json','7x7_cost_9_1_13.json', True),

	# 14. 車両数 : 2、配送先数 : 6(移動時間はすべて3、vtype=0、1、初期解 0→1→2→3→0および0→4→5→6→0)、
	# management_dtable : 
	#     [{vtype : "common",start_time : 0,table : 2→4で10000、0→5で100},
	#      {vtype : "time_common",start_time : 3,table : 2→4で100、0→5で10000},
	#      {vtype : "0",start_time : 3,table : 5→1で100、0→2で10000},
	#      {vtype : "0",start_time : 10,table : 2→4および0→5は通行不可},
	#      {vtype : "1",start_time : 0,table : 0→2で100、5→1で10000},
	#      {vtype : "3",start_time : 3,table : 2→4および0→5は通行不可}]とした場合に
	# 0→5→1→3→0および0→2→4→6→0となることを確認(時間別車両別、commonとtime_commonあり)
	('9_1_14', '9_1_14_request.json','7x7_cost_9_1_14.json', True),

	# 15. 車両数 : 2、配送先数 : 6(2台目の営業時間枠 2000/1/1 8:00:00～2000/1/1 20:00:00、移動時間はすべて3、初期解 0→1→2→3→0および0→4→5→6→0)、
	# management_dtable : 
	# [{vtype : "common",start_time : 0,table : 2→4で10000、0→5で100},
	#  {vtype : "time_common",start_time : 2,table : 2→4で100、0→5で10000},
	#  {vtype : "0",start_time : 3,table : 5→1で100、0→2で10000},
	#  {vtype : "0",start_time : 10,table : 2→4および0→5は通行不可},
	#  {vtype : "1",start_time : -1,table : 0→2で100、5→1で10000},
	#  {vtype : "2",start_time : 3,table : 2→4および0→5は通行不可}]とした場合に
	# 0→5→1→3→0および0→2→4→6→0となることを確認(時間別車両別、commonとtime_commonあり、配送時間マイナス)
	('9_1_15', '9_1_15_request.json','7x7_cost_9_1_15.json', True),

	# 16. 車両数 : 2、配送先数 : 6(移動時間はすべて3、初期解 0→1→2→3→0および0→4→5→6→0)、
	# management_dtable : 
	# [{vtype : "3",start_time : 3,table : 2→4および0→5は通行不可},
	#  {vtype : "time_common",start_time : 3,table : 2→4で100、0→5で10000},
	#  {vtype : "0",start_time : 10,table : 2→4および0→5は通行不可},
	#  {vtype : "common",start_time : 0,table : 2→4で10000、0→5で100},
	#  {vtype : "0",start_time : 3,table : 5→1で100、0→2で10000},
	#  {vtype : "1",start_time : 0,table : 0→2で100、5→1で10000}]とした場合に
	# 0→5→1→3→0および0→2→4→6→0となることを確認(時間別車両別、commonとtime_commonあり、順不同)
	('9_1_16', '9_1_16_request.json','7x7_cost_9_1_16.json', True),

	# 17. 車両数 : 1、配送先数 : 3(移動時間はすべて3、初期解 0→1→2→3→0)、
	# management_dtable : 
	#  [{vtype : "common",start_time : 0,table : 全て通行不可},
	#   {vtype : "time_common",start_time : 0,table : 1→3は通行不可、0→2で100},
	#   {vtype : "time_common",start_time : 3,table : 1→3で100、0→2は通行不可}]とした場合に
	# 0→2→1→3→0となることを確認(通行不可)
	('9_1_17', '9_1_17_request.json','4x4_cost_9_1_17.json', True),

	# 18. 車両数 : 2、配送先数 : 6(opskill=0.5、1.0、荷作業時間は4のみ2、初期解 0→1→2→3→0および0→4→5→6→0)、
	# management_dtable : 
	#  [{vtype : "common",start_time : 0,table : 2→4で10000、0→5で100},
	#   {vtype : "time_common",start_time : 3,table : 2→4で100、0→5で10000},
	#   {vtype : "0",start_time : 3,table : 5→1で100、0→2で10000},
	#   {vtype : "0",start_time : 10,table : 2→4および0→5は通行不可},
	#   {vtype : "1",start_time : 0,table : 0→2で100、5→1で10000},
	#   {vtype : "3",start_time : 3,table : 2→4および0→5は通行不可}]とした場合に
	# 0→5→1→3→0(opskill=0.5)および0→2→4→6→0(opskill=1.0)となることを確認(車両ごとにテーブルが異なるためスキルシャッフルは機能しない)
	('9_1_18', '9_1_18_request.json','7x7_cost_9_1_18.json', True),

	# 19. 車両数 : 2、配送先数 : 6(opskill=0.5、1.0、荷作業時間は4のみ2、初期解 0→1→2→3→0および0→4→5→6→0)、
	# management_dtable : 
	#  [{vtype : "time_common",start_time : 0,table : 1→3および4→6で10000、0→2および0→5で100},
	#   {vtype : "time_common",start_time : 3,table : 1→3および4→6で100、0→2および0→5で10000}]とした場合に
	# 0→2→1→3→0(opskill=1.0)および0→5→4→6→0(opskill=0.5)となることを確認(同じテーブルを参照するためスキルシャッフル)
	('9_1_19', '9_1_19_request.json','7x7_cost_9_1_19.json', True),

	# # 20. 車両数 : 2、配送先数 : 6(opskill=0.5、1.0、荷作業時間は4のみ2、初期解 0→1→2→3→0および0→4→5→6→0)、
	# # management_dtable : 
	# #  [{vtype : "time_common",start_time : 0,table : 1→3および4→6で10000、0→2および0→5で100},
	# #   {vtype : "0",start_time : 0,table : 1→3および4→6で10000、0→2および0→5で100},
	# #   {vtype : "0",start_time : 3,table : 1→3および4→6で100、0→2および0→5で10000},
	# #   {vtype : "1",start_time : 3,table : 1→3および4→6で100、0→2および0→5で10000}]とした場合に
	# # 0→2→1→3→0(opskill=1.0)および0→5→4→6→0(opskill=0.5)となることを確認(別のテーブルだが中身は同じテーブルを参照するためスキルシャッフル)
	# ('9_1_20', '9_1_20_request.json','7x7_cost_9_1_20.json', True),
	#仕様変更によりテスト不可
]

