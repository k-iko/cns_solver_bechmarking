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
# テスト47(距離平準化％指定)
#-----------------------------------------------------------------
# 指定のないものは距離は1000、時間は1、電力は3で固定、電力容量は10、初期電力量は10、電力マージンは1、充電レートは1e12、荷作業時間は1(充電スポットは0)、配送時間枠は2000/1/1 9:00:00～2000/1/1 17:00:00で固定
# *但し拠点と最終訪問先の配送時間枠は1000/1/1 0:00:00～3000/1/1 0:00:00

test_args=[
	# ■異常系
	# 1. 車両数 : 1、配送先数 : 1、avedist=-1とした場合に
	# "APE margin of distance-leveling (aka avedist_ape) set to less than 0.0"のエラーメッセージを出力して終了することを確認
	('47_1_1', '47_1_1_request.json','2x2_cost.json', True),

	# 2. 車両数 : 2、配送先数 : 2(移動時間はすべて0)、avedist_ape=0.5とした場合に
	# "failed to calculate APE margin of distance-leveling (aka avedist_ape) due to distance = 0"のエラーメッセージを出力して終了することを確認
	('47_1_2', '47_1_2_request.json','3x3_cost_dtable0.json', True),

	# 3. 車両数 : 1、配送先数 : 1、avedist_ae=-1とした場合に
	# "AE margin of distance-leveling(avedist_ae) set less than 0.0"のエラーメッセージを出力して終了することを確認
	('47_1_3', '47_1_3_request.json','2x2_cost.json', True),

	# 4. 車両数 : 1、配送先数 : 1、avedist_ape=0.5、avedist_ae=1000とした場合に
	# "both APE margin of distance-leveling(avedist_ape_flag) and AE margin of distance-leveling(avedist_ae_flag) set"のエラーメッセージを出力して終了することを確認
	('47_1_4', '47_1_4_request.json','2x2_cost.json', True),

	# 5. 車両数 : 1、配送先数 : 1、距離ばらつきペナルティ重み : -1とした場合に
	# "weight of distance leveling penalty (aka distLevelingPenaltyWeight) set to less than 0.0"のエラーメッセージを出力して終了することを確認
	('47_1_5', '47_1_5_request.json','2x2_cost.json', True),

	# 6. 車両数 : 1、配送先数 : 1、距離ばらつきペナルティ重み : 1e10とした場合に
	# "weight of distance leveling penalty (aka distLevelingPenaltyWeight) set to more than 1000000000"のエラーメッセージを出力して終了することを確認
	('47_1_6', '47_1_6_request.json','2x2_cost.json', True),

	# 7. 車両数 : 1、配送先数 : 1、距離ばらつきペナルティタイプ : normalとした場合に
	# "type of distance leveling penalty (aka distLevelingPenaltyType) set to other than "soft" or "hard""のエラーメッセージを出力して終了することを確認
	('47_1_7', '47_1_7_request.json','2x2_cost.json', True),

	# 8. 車両数 : 2、配送先数 : 2(移動距離 0→2は100)、avedist_ape=0、距離ばらつきペナルティタイプ : hardとした場合に
	# "failed to create solution due to hard constraint"のエラーメッセージを出力して終了することを確認
	('47_1_8', '47_1_8_request.json','3x3_cost_47_1_8.json', True),

	# 9. 車両数 : 2、配送先数 : 2(移動距離 0→2は100)、充電スポット数 : 1、evplan、avedist_ape=0、距離ばらつきペナルティタイプ : hardとした場合に
	# "failed to create solution due to hard constraint"のエラーメッセージを出力して終了することを確認
	('47_1_9', '47_1_9_request.json','4x4_cost_47_1_9.json', True),

	# # ■正常系
	# 10. 車両数 : 1、配送先数 : 2、avedist_ape=0とした場合に
	# 解が作成されることを確認
	('47_1_10', '47_1_10_request.json','3x3_cost.json', True),

	# 11. 車両数 : 3、配送先数 : 6(移動距離はa→bは1000×(a+b))、avedist_ape=10、時間最適化とした場合に
	# 距離のAPE指定なしの解(初期解のまま)と同じ解が作成されることを確認
	('47_1_11', '47_1_11_request.json','7x7_cost_dtable_add.json', True),

	# 12. 車両数 : 3、配送先数 : 6(custno=1,2,3,4,5,7、移動距離はa→bは1000×(a+b))、avedist_ape=0.5、時間最適化とした場合に
	# 距離のAPE<=0.5の解が作成されることを確認
	('47_1_12', '47_1_12_request.json','7x7_cost_dtable_add2.json', True),

	# 13. 車両数 : 3、配送先数 : 6(custno=1,2,3,4,5,7、移動距離はa→bは1000×(a+b))、avedist_ape=0、時間最適化とした場合に
	# 距離のAPE=0の解が作成されることを確認
	('47_1_13', '47_1_13_request.json','7x7_cost_dtable_add2.json', True),

	# 14. 車両数 : 3、配送先数 : 6(custno=1,2,3,4,5,7、移動距離はa→bは1000×a×b)、avedist_ape=0、時間最適化とした場合に
	# 距離のAPEが最小の解が作成されることを確認
	('47_1_14', '47_1_14_request.json','7x7_cost_dtable_mul2.json', True),

	# 15. 車両数 : 3、配送先数 : 6(移動距離は1→*および*→1は100000)、avedist_ape=0、時間最適化とした場合に
	# 1に配送する車両には他の配送先がないことを確認
	('47_1_15', '47_1_15_request.json','7x7_cost_dtable_1x100.json', True),

	# 16. 車両数 : 2、配送先数 : 3(移動距離は0→1および1→0、0→2および2→0は1000、その他は0)、avedist_ape=0.4、時間最適化とした場合に
	# 距離のAPE=0の解が作成されることを確認(1台の走行距離が0になる場合)
	('47_1_16', '47_1_16_request.json','4x4_cost_47_1_16.json', True),

	# 17. 車両数 : 3、配送先数 : 6(移動距離はa→bは1000×(a+b))、
	# 1番目と2番目の配送先に2台目と3台目の車両は立寄不可、5番目と6番目の配送先に1台目と2台目の車両は立寄不可、2→1および4→3は通行不可
	# avedist_ape=0、時間最適化、車両削減モードとした場合に
	# 距離のAPE=0の解が作成されることを確認(車両削減モード)
	('47_1_17', '47_1_17_request.json','7x7_cost_dtable_add3.json', True),

	# 18. 車両数 : 3、配送先数 : 3(移動時間は1→2で0.1)、avedist_ape=0.2、時間最適化、車両削減モードとした場合に
	# 解が変わらないことを確認(距離のAPEを守れない場合は削減しない)
	('47_1_18', '47_1_18_request.json','4x4_cost_47_1_18.json', True),

	# 19. 車両数 : 3、配送先数 : 3(積載量上限 3台目は100、荷物量 全て150)、avedist_ape=0.2、車両削減モードとした場合に
	# 解が変わらないことを確認(距離のAPEを守れない場合は削減しない)
	('47_1_19', '47_1_19_request.json','4x4_cost.json', True),

	# 20. 車両数 : 1、配送先数 : 2、avedist_ae=0とした場合に
	# 解が作成されることを確認
	('47_1_20', '47_1_20_request.json','3x3_cost.json', True),

	# 21. 車両数 : 3、配送先数 : 6(移動距離はa→bは1000×(a+b))、avedist_ae=100000、時間最適化とした場合に
	# 距離のAE指定なしの解(初期解のまま)と同じ解が作成されることを確認
	('47_1_21', '47_1_21_request.json','7x7_cost_dtable_add.json', True),

	# 22. 車両数 : 3、配送先数 : 6(custno=1,2,3,4,5,7、移動距離はa→bは1000×(a+b)、初期解 0→1→2→0および0→5→7→0および0→3→4→0)、avedist_ae=4000、時間最適化とした場合に
	# 距離のAE=2000の解が作成されることを確認
	('47_1_22', '47_1_22_request.json','7x7_cost_dtable_add2.json', True),

	# 23. 車両数 : 3、配送先数 : 6(custno=1,2,3,4,5,7、移動距離はa→bは1000×(a+b)、初期解 0→1→2→0および0→5→7→0および0→3→4→0)、avedist_ae=0、時間最適化とした場合に
	# 距離のAE=0の解が作成されることを確認
	('47_1_23', '47_1_23_request.json','7x7_cost_dtable_add2.json', True),

	# 24. 車両数 : 3、配送先数 : 6(custno=1,2,3,4,5,7、移動距離はa→bは1000×(a+b))、avedist_ae=0、時間最適化とした場合に
	# 距離のAEが最小の解が作成されることを確認
	('47_1_24', '47_1_24_request.json','7x7_cost_dtable_mul2.json', True),

	# 25. 車両数 : 3、配送先数 : 6(移動距離は1→*および*→1は100000)、avedist_ae=2000、時間最適化とした場合に
	# 1に配送する車両には他の配送先がないことを確認
	('47_1_25', '47_1_25_request.json','7x7_cost_dtable_1x100.json', True),

	# 26. 車両数 : 2、配送先数 : 3(移動距離は0→1および1→0、0→2および2→0は1000、その他は0)、avedist_ae=400、時間最適化とした場合に
	# 距離のAE<=400の解が作成されることを確認(1台の走行距離が0になる場合)
	('47_1_26', '47_1_26_request.json','4x4_cost_47_1_26.json', True),

	# 27. 車両数 : 3、配送先数 : 6(移動距離はa→bは1000×(a+b))、
	# 1番目と2番目の配送先に2台目と3台目の車両は立寄不可、5番目と6番目の配送先に1台目と2台目の車両は立寄不可、2→1および4→3は通行不可
	# avedist_ae=0、車両削減モードとした場合に
	# 距離のAE=0の解が作成されることを確認(車両削減モード)
	('47_1_27', '47_1_27_request.json','7x7_cost_dtable_add3.json', True),

	# 28. 車両数 : 3、配送先数 : 3(移動時間は1→2で0.1)、avedist_ae=200、時間最適化、車両削減モードとした場合に
	# 解が変わらないことを確認(距離のAEを守れない場合は削減しない)
	('47_1_28', '47_1_28_request.json','4x4_cost_47_1_28.json', True),

	# 29. 車両数 : 3、配送先数 : 3(積載量上限 3台目は100、荷物量 全て150)、avedist_ae=200、時間最適化、車両削減モードとした場合に
	# 解が変わらないことを確認(距離のAEを守れない場合は削減しない)
	('47_1_29', '47_1_29_request.json','4x4_cost.json', True),

	# 30. 車両数 : 3、配送先数 : 5(SPOTID 1、1、3、4、5、荷物量 150、150、150、150、1、移動距離 0→5および5→3で1500、0→4で3000、荷作業時間はすべて0)、初期解生成モード、avedist_ape=0、時間最適化とした場合に
	# 150、150の荷物で1台の車両(直送便)および0→5→3→0および0→4→0となることを確認(直送便は距離のAPE対象外)
	('47_1_30', '47_1_30_request.json','5x5_cost_47_1_30.json', True),

	# 31. 車両数 : 3、配送先数 : 5(SPOTID 1、1、3、4、5、荷物量 150、150、150、150、1、移動距離 0→5および5→3で1500、0→4で3000、荷作業時間はすべて0)、初期解生成モード、avedist_ae=0、時間最適化とした場合に
	# 150、150の荷物で1台の車両(直送便)および0→5→3→0および0→4→0となることを確認(直送便は距離のAE対象外)
	('47_1_31', '47_1_31_request.json','5x5_cost_47_1_31.json', True),

	# 32. 車両数 : 2、配送先数 : 3(移動時間 3→1は0.1、移動距離 0→3は2000)、avedist_ape=0.1、時間最適化、距離ばらつきペナルティ重み : 0、初期解 0→1→2→0および0→3→0とした場合に
	# 0→2→0および0→3→1→0となることを確認(距離ばらつきペナルティは無視)
	('47_1_32', '47_1_32_request.json','4x4_cost_47_1_32.json', True),

	# 33. 車両数 : 2、配送先数 : 3(移動距離 1→2および2→1は2000、荷物量 150、150、300)、avedist_ae=1500、時間最適化、距離ばらつきペナルティタイプ : hard、初期解 0→1→2→0および0→3→0とした場合に
	# 0→3→2→0および0→1→0となることを確認
	('47_1_33', '47_1_33_request.json','4x4_cost_47_1_33.json', True),

	# 34. 車両数 : 2、配送先数 : 2(配送時間枠 2000/1/1 9:00:00～2000/1/1 12:00:00、2000/1/1 12:00:00～2000/1/1 13:00:00、
	# 移動距離 2000/1/1 9:00:00～2000/1/1 10:00:00はすべて1000、2000/1/1 10:00:00～はすべて2000、荷作業時間はすべて0、初期解 0→1→0、0→2→0)、avedist_ape=0、時間最適化、遅延最適化モードとした場合に
	# 初期解のままとなることを確認(出発遅延により距離のAPEが悪化する場合)
	('47_1_34', '47_1_34_request.json','3x3_cost_47_1_34.json', True),
]

