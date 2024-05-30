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
# テスト1-1(配送時間枠指定)
#-----------------------------------------------------------------
# 指定のないものは距離は1000、時間は1、電力は3で固定、電力容量は10、初期電力量は10、電力マージンは1、充電レートは1e12、荷作業時間は1(充電スポットは0)、配送時間枠は2000/1/1 9:00:00～2000/1/1 17:00:00で固定
# *但し拠点と最終訪問先の配送時間枠は1000/1/1 0:00:00～3000/1/1 0:00:00
test_args=[
	#■異常系
	# 1. 車両数 : 1、配送先数 : 1、時間ペナルティタイプ : normalとした場合に
	# "type of time penalty (aka timePenaltyType) set to other than "soft" or "hard""のエラーメッセージを出力して終了することを確認
	('42_1_1', '42_1_1_request.json','2x2_cost.json', True),

	# 2. 車両数 : 1、配送先数 : 1、積載量ペナルティタイプ : normalとした場合に
	# "type of load penalty (aka loadPenaltyType) set to other than "soft" or "hard""のエラーメッセージを出力して終了することを確認
	('42_1_2', '42_1_2_request.json','2x2_cost.json', True),

	# 3. 車両数 : 1、配送先数 : 1、第2積載量ペナルティタイプ : normalとした場合に
	# "type of load2 penalty (aka load2PenaltyType) set to other than "soft" or "hard""のエラーメッセージを出力して終了することを確認
	('42_1_3', '42_1_3_request.json','2x2_cost.json', True),

	# 4. 車両数 : 1、配送先数 : 1、時間ばらつきペナルティタイプ : normalとした場合に
	# "type of time leveling penalty (aka timeLevelingPenaltyType) set to other than "soft" or "hard""のエラーメッセージを出力して終了することを確認
	('42_1_4', '42_1_4_request.json','2x2_cost.json', True),

	# 5. 車両数 : 1、配送先数 : 1、件数ばらつきペナルティタイプ : normalとした場合に
	# "type of visits leveling penalty (aka visitsLevelingPenaltyType) set to other than "soft" or "hard""のエラーメッセージを出力して終了することを確認
	('42_1_5', '42_1_5_request.json','2x2_cost.json', True),

	# 6. 車両数 : 1、配送先数 : 1(配送時間枠 2000/1/1 8:00:00～2000/1/1 9:00:00)、時間ペナルティタイプ : hardとした場合に
	# "failed to create solution due to hard constraint"のエラーメッセージを出力して終了することを確認
	('42_1_6', '42_1_6_request.json','2x2_cost.json', True),

	# 7. 車両数 : 1、配送先数 : 1(荷物量 400)、積載量ペナルティタイプ : hardとした場合に
	# "failed to create solution due to hard constraint"のエラーメッセージを出力して終了することを確認
	('42_1_7', '42_1_7_request.json','2x2_cost.json', True),

	# 8. 車両数 : 1、配送先数 : 1(第2荷物量 400)、第2積載量ペナルティタイプ : hardとした場合に
	# "failed to create solution due to hard constraint"のエラーメッセージを出力して終了することを確認
	('42_1_8', '42_1_8_request.json','2x2_cost.json', True),

	# 9. 車両数 : 2、配送先数 : 2(配送時間枠 2000/1/1 11:00:00～2000/1/1 12:00:00、2000/1/1 9:00:00～2000/1/1 17:00:00)、avetime_ape=0、時間ばらつきペナルティタイプ : hardとした場合に
	# "failed to create solution due to hard constraint"のエラーメッセージを出力して終了することを確認
	('42_1_9', '42_1_9_request.json','3x3_cost.json', True),

	# 10. 車両数 : 2、配送先数 : 3、avevisits_ape=0、件数ばらつきペナルティタイプ : hardとした場合に
	# "failed to create solution due to hard constraint"のエラーメッセージを出力して終了することを確認
	('42_1_10', '42_1_10_request.json','4x4_cost.json', True),

	# 11. 車両数 : 1、配送先数 : 1(配送時間枠 2000/1/1 8:00:00～2000/1/1 9:00:00)、充電スポット数 : 1、evplan、時間ペナルティタイプ : hardとした場合に
	# "failed to create solution due to hard constraint"のエラーメッセージを出力して終了することを確認
	('42_1_11', '42_1_11_request.json','3x3_cost_42_1_11.json', True),

	# 12. 車両数 : 1、配送先数 : 1(荷物量 400)、充電スポット数 : 1、evplan、積載量ペナルティタイプ : hardとした場合に
	# "failed to create solution due to hard constraint"のエラーメッセージを出力して終了することを確認
	('42_1_12', '42_1_12_request.json','3x3_cost_42_1_12.json', True),

	# 13. 車両数 : 1、配送先数 : 1(第2荷物量 400)、充電スポット数 : 1、evplan、第2積載量ペナルティタイプ : hardとした場合に
	# "failed to create solution due to hard constraint"のエラーメッセージを出力して終了することを確認
	('42_1_13', '42_1_13_request.json','3x3_cost_42_1_13.json', True),

	# 14. 車両数 : 2、配送先数 : 2(配送時間枠 2000/1/1 11:00:00～2000/1/1 12:00:00、2000/1/1 9:00:00～2000/1/1 17:00:00)、充電スポット数 : 1、evplan、avetime_ape=0、時間ばらつきペナルティタイプ : hardとした場合に
	# "failed to create solution due to hard constraint"のエラーメッセージを出力して終了することを確認
	('42_1_14', '42_1_14_request.json','4x4_cost_42_1_14.json', True),

	# 15. 車両数 : 2、配送先数 : 3、充電スポット数 : 1、evplan、avevisits_ape=0、件数ばらつきペナルティタイプ : hardとした場合に
	# "failed to create solution due to hard constraint"のエラーメッセージを出力して終了することを確認
	('42_1_15', '42_1_15_request.json','5x5_cost_42_1_15.json', True),

	# ■正常系
	# 16. 車両数 : 2、配送先数 : 3(移動時間 1→2および2→1は10、荷物量 150、150、300)、時間ペナルティタイプ : hard、初期解 0→1→2→0および0→3→0とした場合に
	# 0→3→2→0および0→1→0となることを確認
	('42_1_16', '42_1_16_request.json','4x4_cost_42_1_16.json', True),

	# 17. 車両数 : 2、配送先数 : 3(移動時間 1→3および3→1は100、荷物量 150、151、150)、積載量ペナルティタイプ : hard、初期解 0→1→2→0および0→3→0とした場合に
	# 0→1→3→0および0→2→0となることを確認
	('42_1_17', '42_1_17_request.json','4x4_cost_42_1_17.json', True),

	# 18. 車両数 : 2、配送先数 : 3(移動時間 1→3および3→1は100、第2荷物量 150、151、150)、第2積載量ペナルティタイプ : hard、初期解 0→1→2→0および0→3→0とした場合に
	# 0→1→3→0および0→2→0となることを確認
	('42_1_18', '42_1_18_request.json','4x4_cost_42_1_18.json', True),

	# 19. 車両数 : 2、配送先数 : 3(移動時間 1→2および2→1は2、荷物量 150、150、300)、avetime_ae=1.5、時間ばらつきペナルティタイプ : hard、初期解 0→1→2→0および0→3→0とした場合に
	# 0→3→2→0および0→1→0となることを確認
	('42_1_19', '42_1_19_request.json','4x4_cost_42_1_19.json', True),

	# 20. 車両数 : 2、配送先数 : 4(荷物量 120、120、60、300)、avevisits_ae=0、件数ばらつきペナルティタイプ : hard、初期解 0→1→2→3→0および0→4→0とした場合に
	# 0→4→3→0および0→1→2→0となることを確認
	('42_1_20', '42_1_20_request.json','5x5_cost.json', True),

	# 21. 車両数 : 2(訪問数上限 : 2、2)、配送先数 : 4(移動時間 下記(*)のテーブル参照、荷物量 100、100、200、200)、時間ペナルティタイプ : hard、積載量ペナルティタイプ : hard、初期解 0→1→2→0および0→3→4→0とした場合に
	# 0→4→1→0および0→2→3→0(車両は順不同)となることを確認(複数hard制約がある場合)
	('42_1_21', '42_1_21_request.json','5x5_cost_42_1_21.json', True),

	# 22. 車両数 : 1(初期電力 8)、充電スポット数 : 1、配送先数 : 2(消費電力はすべて3、移動時間 : 1→2および2→1は10、初期解 0→1→2→0)、evplan=True、時間ペナルティタイプ : hardとした場合に
	# 0→1→e0→2→0となることを確認(電力解の場合は電力なし解の段階ではエラーとならない)
	('42_1_22', '42_1_22_request.json','4x4_cost_42_1_22.json', True),

	# 23. 車両数 : 2、配送先数 : 4(距離 0→1および1→2および2→3は100、移動時間 4→*および*→4は1→4および4→3以外10)、時間ペナルティタイプ : hard、初期解生成モード、巡回モードとした場合に
	# 0→1→4→3→0および0→2→0となることを確認(初期解生成で違反があってもエラーとしない)
	('42_1_23', '42_1_23_request.json','5x5_cost_42_1_23.json', True),


	# (*)
	# [[ 0, 1, 1, 1, 1],
	#  [ 1, 0,10,11,11],
	#  [ 1,10, 0, 1,11],
	#  [ 1,11,11, 0,10],
	#  [ 1, 1,11,10, 0]]
]
