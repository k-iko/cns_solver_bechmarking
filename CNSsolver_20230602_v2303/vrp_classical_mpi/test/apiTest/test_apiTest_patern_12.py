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
# テスト12(SPOTID指定)
#-----------------------------------------------------------------
#指定のないものは距離は1000、時間は1で固定、荷作業時間は1、配送時間枠は2000/1/1 9:00:00～2000/1/1 17:00:00で固定
# *但し拠点と最終訪問先の配送時間枠は1000/1/1 0:00:00～3000/1/1 0:00:00

test_args=[
	# ■異常系
	# 1. 車両数 : 1、配送先数 : 2(SPOTID 1、2、距離テーブルのサイズ 2×2)とした場合に
	# "distance table (aka dtable) does not match spotid"のエラーメッセージを出力して終了することを確認
	('12_1_1', '12_1_1_request.json','2x2_dtable_3x3_ttable_cost.json', True),

	# 2. 車両数 : 1、配送先数 : 2(SPOTID 1、1、距離テーブルのサイズ 3×3)とした場合に
	# "distance table (aka dtable) does not match spotid"のエラーメッセージを出力して終了することを確認
	('12_1_2', '12_1_2_request.json','3x3_dtable_2x2_ttable_cost.json', True),

	# 3. 車両数 : 1、配送先数 : 2(SPOTID 1、2、時間テーブルのサイズ 2×2)とした場合に
	# "eta table (aka ttable) does not match spotid"のエラーメッセージを出力して終了することを確認
	('12_1_3', '12_1_3_request.json','3x3_dtable_2x2_ttable_cost.json', True),

	# 4. 車両数 : 1、配送先数 : 2(SPOTID 1、1、時間テーブルのサイズ 3×3)とした場合に
	# "eta table (aka ttable) does not match spotid"のエラーメッセージを出力して終了することを確認
	('12_1_4', '12_1_4_request.json','2x2_dtable_3x3_ttable_cost.json', True),

	# 5. 車両数 : 1、配送先数 : 1(SPOTID 0)とした場合に
	# "spotid = "0" must be set to depot or last spot"のエラーメッセージを出力して終了することを確認
	('12_1_5', '12_1_5_request.json','2x2_cost.json', True),

	# ■正常系
	# 6. 車両数 : 2、配送先数 : 4(距離 1→2および3→4で10000、1→4および3→2で100、SPOTID 156、158、161、163、初期解 0→1→2→0および0→3→4→0)とした場合に
	# 0→1→4→0および0→3→2→0となることを確認(custnoとspotidが異なり、連番でなくても動作することを確認)
	('12_1_6', '12_1_6_request.json','5x5_cost_modify1.json', True),

	# 7. 車両数 : 2、配送先数 : 4(距離 1→2および3→4で10000、1→4および3→2で100、SPOTID 2、10、11、103、初期解 0→4→1→0および0→3→2→0)とした場合に
	# 0→2→1→0および0→3→4→0となることを確認(辞書式順序でコストテーブルにアクセスすることを確認。
	# 距離 1→2の1はテーブルの1行目(SPOTID 10)を、0→4→1→0の1は配送先の1番目(SPOTID 2)を指しており、異なる配送先を表していることに注意)
	('12_1_7', '12_1_7_request.json','5x5_cost_modify1.json', True),

	# 8. 車両数 : 1、配送先数 : 3(SPOTID 1、2、1、初期解 0→1→2→3→0)とした場合に
	# 1と3が連続することを確認(同一SPOTID)
	('12_1_8', '12_1_8_request.json','3x3_cost.json', True),

	# 9. 車両数 : 1、配送先数 : 3(SPOTID 1、2、1、配送時間枠 2000/1/1 9:00:00～2000/1/1 10:00:00、2000/1/1 10:00:00～2000/1/1 12:00:00、2000/1/1 11:00:00～2000/1/1 13:00:00、初期解 0→3→1→2→0)とした場合に
	# 0→1→2→3となることを確認(同一SPOTID、異なる時間枠)
	('12_1_9', '12_1_9_request.json','3x3_cost.json', True),

	# 10. 車両数 : 1、配送先数 : 4(SPOTID 1、2、1、3、距離 1→2で-1、2→1および1→3で10000、初期解 0→2→1→3→4→0)とした場合に
	# 0→2→4→1→3→0または0→2→4→3→1→0となることを確認(同一SPOTID、通行不可)
	('12_1_10', '12_1_10_request.json','4x4_cost_modify2.json', True),

]

