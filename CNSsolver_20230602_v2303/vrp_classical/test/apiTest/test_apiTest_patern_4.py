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
# テスト4(タスク平準化％指定)
#-----------------------------------------------------------------
# 記載のないものは距離は1000、時間は1で固定、配送時間枠は配送先、車両ともに1000/1/1 0:00:00～3000/1/1 23:59:00で固定、荷作業時間は0で固定
# *但し拠点と最終訪問先の配送時間枠は1000/1/1 0:00:00～3000/1/1 0:00:00

test_args=[
	# # ■異常系
	# # 1. 車両数 : 1、配送先数 : 1、avetime_ape=-1とした場合に
	# # "APE margin of time-leveling (aka avetime_ape) set to less than 0.0"のエラーメッセージを出力して終了することを確認
	# ('4_1_1', '4_1_1_request.json','2x2_cost.json', True),

	# # 2. 車両数 : 2、配送先数 : 2(移動時間はすべて0)、avetime_ape=0.5とした場合に
	# # "failed to calculate APE margin of time-leveling (aka avetime_ape) due to delivery time = 0"のエラーメッセージを出力して終了することを確認
	# ('4_1_2', '4_1_2_request.json','3x3_cost_ttable0.json', True),

	# # 15. 車両数 : 1、配送先数 : 1、avetime_ae=-1とした場合に
	# # "AE margin of time-leveling(avetime_ae) set less than 0.0"のエラーメッセージを出力して終了することを確認
	# ('4_1_15', '4_1_15_request.json','2x2_cost.json', True),

	# # 16. 車両数 : 2、配送先数 : 2(移動時間はすべて0)、avetime_ae=2とした場合に
	# # "failed to calculate AE margin of time-leveling(avetime_ae) due to delivery time=0"のエラーメッセージを出力して終了することを確認
	# # 削除

	# # 17. 車両数 : 1、配送先数 : 1、avetime_ape=0.5、avetime_ae=2とした場合に
	# # "both APE margin of time-leveling(avetime_ape_flag) and AE margin of time-leveling(avetime_ae_flag) set"のエラーメッセージを出力して終了することを確認
	# ('4_1_17', '4_1_17_request.json','2x2_cost.json', True),

	# # ■正常系
	# # 3. 車両数 : 1、配送先数 : 2、avetime_ape=0とした場合に
	# # 解が作成されることを確認
	# ('4_1_3', '4_1_3_request.json','3x3_cost.json', True),

	# # 4. 車両数 : 3、配送先数 : 6(移動時間はa→bはa+b)、avetime_ape=10とした場合に
	# # 時間のAPE指定なしの解(初期解のまま)と同じ解が作成されることを確認
	# ('4_1_4', '4_1_4_request.json','7x7_cost_ttable_add.json', True),

	# # 5. 車両数 : 3、配送先数 : 6(custno=1,2,3,4,5,7、移動時間はa→bはa+b)、avetime_ape=0.5とした場合に
	# # 時間のAPE<=0.5の解が作成されることを確認
	# ('4_1_5', '4_1_5_request.json','7x7_cost_ttable_add2.json', True),

	# # 6. 車両数 : 3、配送先数 : 6(custno=1,2,3,4,5,7、移動時間はa→bはa+b)、avetime_ape=0とした場合に
	# # 時間のAPE=0の解が作成されることを確認
	# ('4_1_6', '4_1_6_request.json','7x7_cost_ttable_add2.json', True),

	# 7. 車両数 : 3、配送先数 : 6(custno=1,2,3,4,5,7、移動時間はa→bはa×b)、avetime_ape=0とした場合に
	# 時間のAPEが最小の解が作成されることを確認
	('4_1_7', '4_1_7_request.json','7x7_cost_ttable_mul2.json', True),

	# 8. 車両数 : 3、配送先数 : 6(移動時間は1→*および*→1は100)、avetime_ape=0.5とした場合に
	# 1に配送する車両には他の配送先がないことを確認
	('4_1_8', '4_1_8_request.json','7x7_cost_ttable_1x100.json', True),

	# 9. 車両数 : 3、配送先数 : 6(車両営業時間は2000/1/1 0:00:00～2000/1/1 23:59:00、1999/12/31 22:00:00～2000/1/1 21:59:00、2000/1/1 2:00:00～2000/1/2 1:59:00、
	# custno=1,2,3,4,5,7、移動時間はa→bはa+b)、avetime_ape=0とした場合に
	# 時間のAPE=0の解が作成されることを確認
	('4_1_9', '4_1_9_request.json','7x7_cost_ttable_add2.json', True),

	# 10. 車両数 : 2、配送先数 : 3(移動時間は0→1および1→0、0→2および2→0は1、その他は0)、avetime_ape=0.4とした場合に
	# 時間のAPE=0の解が作成されることを確認(1台の配送時間が0になる場合)
	('4_1_10', '4_1_10_request.json','4x4_cost_4_1_10.json', True),

	# 11. 車両数 : 3、配送先数 : 6(drskill=0.6、0.5、0.8、opskill=0.3、0.2、0.1、
	# 移動時間は1→2は0.7、3→4は0.3、5→6は0.9、その他は0、距離は1→2は2000、3→4は1000、5→6は500、その他は0)、avetime_ape=0.7とした場合に
	# 時間のAPE<=0.7の解が作成されることを確認
	('4_1_11', '4_1_11_request.json','7x7_cost_modify1.json', True),

	# 12. 車両数 : 3、配送先数 : 6(移動時間はa→bはa+b)、
	# 1番目と2番目の配送先に2台目と3台目の車両は立寄不可、5番目と6番目の配送先に1台目と2台目の車両は立寄不可、2→1および4→3は通行不可
	# avetime_ape=0、車両削減モードとした場合に
	# 時間のAPE=0の解が作成されることを確認(車両削減モード)
	('4_1_12', '4_1_12_request.json','7x7_cost_ttable_add3.json', True),

	# 13. 車両数 : 3、配送先数 : 3(距離は1→2で100)、avetime_ape=0.2、車両削減モードとした場合に
	# 解が変わらないことを確認(時間のAPEを守れない場合は削減しない)
	('4_1_13', '4_1_13_request.json','4x4_cost_modify6.json', True),

	# 14. 車両数 : 3、配送先数 : 3(積載量上限 3台目は100、荷物量 全て150)、avetime_ape=0.2、車両削減モードとした場合に
	# 解が変わらないことを確認(時間のAPEを守れない場合は削減しない)
	('4_1_14', '4_1_14_request.json','4x4_cost.json', True),

	# 18. 車両数 : 1、配送先数 : 2、avetime_ae=0とした場合に
	# 解が作成されることを確認
	('4_1_18', '4_1_18_request.json','3x3_cost.json', True),

	# 19. 車両数 : 3、配送先数 : 6(移動時間はa→bはa+b)、avetime_ae=100とした場合に
	# 時間のAE指定なしの解(初期解のまま)と同じ解が作成されることを確認
	('4_1_19', '4_1_19_request.json','7x7_cost_ttable_add.json', True),

	# 20. 車両数 : 3、配送先数 : 6(custno=1,2,3,4,5,7、移動時間はa→bはa+b、初期解 0→1→2→0および0→5→7→0および0→3→4→0)、avetime_ae=4とした場合に
	# 時間のAE=2の解が作成されることを確認
	('4_1_20', '4_1_20_request.json','7x7_cost_ttable_add2.json', True),

	# 21. 車両数 : 3、配送先数 : 6(custno=1,2,3,4,5,7、移動時間はa→bはa+b、初期解 0→1→2→0および0→5→7→0および0→3→4→0)、avetime_ae=0とした場合に
	# 時間のAE=0の解が作成されることを確認
	('4_1_21', '4_1_21_request.json','7x7_cost_ttable_add2.json', True),

	# 22. 車両数 : 3、配送先数 : 6(custno=1,2,3,4,5,7、移動時間はa→bはa×b)、avetime_ae=0とした場合に
	# 時間のAEが最小の解が作成されることを確認
	('4_1_22', '4_1_22_request.json','7x7_cost_ttable_mul2.json', True),

	# 23. 車両数 : 3、配送先数 : 6(移動時間は1→*および*→1は100)、avetime_ae=2とした場合に
	# 1に配送する車両には他の配送先がないことを確認
	('4_1_23', '4_1_23_request.json','7x7_cost_ttable_1x100.json', True),

	# 24. 車両数 : 3、配送先数 : 6(車両営業時間は2000/1/1 0:00:00～2000/1/1 23:59:00、1999/12/31 22:00:00～2000/1/1 21:59:00、2000/1/1 2:00:00～2000/1/2 1:59:00、
	# custno=1,2,3,4,5,7、移動時間はa→bはa+b、初期解 0→1→2→0および0→5→7→0および0→3→4→0)、avetime_ae=0とした場合に
	# 時間のAE=0の解が作成されることを確認
	('4_1_24', '4_1_24_request.json','7x7_cost_ttable_add2.json', True),

	# 25. 車両数 : 2、配送先数 : 3(移動時間は0→1および1→0、0→2および2→0は1、その他は0)、avetime_ae=0.4とした場合に
	# 時間のAE<=0.4の解が作成されることを確認(1台の配送時間が0になる場合)
	('4_1_25', '4_1_25_request.json','4x4_cost_4_1_25.json', True),

	# 26. 車両数 : 3、配送先数 : 6(drskill=0.6、0.5、0.8、opskill=0.3、0.2、0.1、
	# 移動時間は1→2は0.7、3→4は0.3、5→6は0.9、その他は0→*および*→0以外は通行不可、距離は1→2は2000、3→4は1000、5→6は500、その他は0)、avetime_ae=0.7とした場合に
	# 時間のAE<=0.7の解が作成されることを確認
	('4_1_26', '4_1_26_request.json','7x7_cost_4_1_26.json', True),

	# 27. 車両数 : 3、配送先数 : 6(移動時間はa→bはa+b)、
	# 1番目と2番目の配送先に2台目と3台目の車両は立寄不可、5番目と6番目の配送先に1台目と2台目の車両は立寄不可、2→1および4→3は通行不可
	# avetime_ae=0、車両削減モードとした場合に
	# 時間のAE=0の解が作成されることを確認(車両削減モード)
	('4_1_27', '4_1_27_request.json','7x7_cost_ttable_add3.json', True),

	# 28. 車両数 : 3、配送先数 : 3(距離は1→2で100)、avetime_ae=0.2、車両削減モードとした場合に
	# 解が変わらないことを確認(時間のAEを守れない場合は削減しない)
	('4_1_28', '4_1_28_request.json','4x4_cost_modify6.json', True),

	# 29. 車両数 : 3、配送先数 : 3(積載量上限 3台目は100、荷物量 全て150)、avetime_ae=0.2、車両削減モードとした場合に
	# 解が変わらないことを確認(時間のAEを守れない場合は削減しない)
	('4_1_29', '4_1_29_request.json','4x4_cost.json', True),

]

