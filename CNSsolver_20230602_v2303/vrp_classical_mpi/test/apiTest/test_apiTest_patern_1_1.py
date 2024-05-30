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
# 距離は1000、時間は1で固定、荷作業時間は1で固定
# *但し拠点と最終訪問先の配送時間枠は1000/1/1 0:00:00～3000/1/1 0:00:00
test_args=[
	# 車両数 : 1、配送先数 : 1(配送時間枠 : 2000/1/1 12:00:00～2000/1/1 17:00:00)とした場合に
	# 12:00前に配送先に到着し、12:00まで待機することを確認
	('1_1_1', '1_1_1_request.json','2x2_cost.json', True),

	# 車両数 : 1、配送先数 : 2(配送時間枠 : 2000/1/1 9:00:00～2000/1/1 12:00:00、2000/1/1 13:00:00～2000/1/1 17:00:00)とした場合に
	# 1番目の配送先→2番目の配送先となることを確認
	('1_1_2', '1_1_2_request.json','3x3_cost.json', True),

	# 車両数 : 1、配送先数 : 2(配送時間枠 : 2000/1/1 9:00:00～2000/1/1 12:00:00、2000/1/1 9:45:00～2000/1/1 10:15:00)とした場合に
	# 2番目の配送先→1番目の配送先となることを確認(1→2では2のdueを超えるため)
	('1_1_3', '1_1_3_request.json','3x3_cost.json', True),

	# 車両数 : 1、配送先数 : 2(配送時間枠 : 2000/1/1 9:00:00～2000/1/1 12:00:00、2000/1/1 11:00:00～2000/1/1 11:30:00)とした場合に
	# 1番目の配送先→2番目の配送先となることを確認(2→1では1のdueを超えるため)
	('1_1_4', '1_1_4_request.json','3x3_cost.json', True),

	# 車両数 : 2、配送先数 : 4(うち2つは配送時間枠 : 2000/1/1 12:00:00～2000/1/1 12:00:00)とした場合に
	# 12:00に配送希望の配送先が別々の車両になることを確認(1台だと片方のdueを超えるため)
	('1_1_5', '1_1_5_request.json','5x5_cost.json', True),

	# 車両数 : 1、配送先数 : 1(配送時間枠 : 2000/1/1 7:00:00～2000/1/1 8:00:00)とした場合に
	# 10:00(到着してすぐ)に配送開始することを確認(max(arr,ready_fs)=arr=10:00なため)
	('1_1_6', '1_1_6_request.json','2x2_cost.json', True),

	# 車両数 : 1、配送先数 : 1(配送時間枠 : 2000/1/1 18:00:00～2000/1/1 19:00:00)とした場合に
	# 18:00前に配送先に到着し、18:00まで待機することを確認(max(arr,ready_fs)=ready_fs=18:00なため)
	('1_1_7', '1_1_7_request.json','2x2_cost.json', True),

	# 車両数 : 1、配送先数 : 2(配送時間枠 : 2000/1/1 9:00:00～2000/1/1 12:00:00、2000/1/1 13:00:00～2000/1/1 17:00:00)、dist(0,1)=99999999999、初期解 : 0→1→2→0とした場合に
	# 0→1→2→0となることを確認(配送時間枠のペナルティよりも距離のペナルティの方が大きいが、時間枠が違反しているため違反していない初期解に戻すため)
	('1_1_8', '1_1_8_request.json','3x3_large-dist_cost.json', True),

	# 車両数 : 1、配送先数 : 2(配送時間枠 : 2000/1/1 9:00:00～2000/1/1 12:00:00、2000/1/1 13:00:00～2000/1/1 17:00:00)、dist(0,1)=99999999999、初期解 : 0→2→1→0とした場合に
	# 0→2→1→0となることを確認(配送時間枠のペナルティよりも距離のペナルティの方が大きいため)
	('1_1_9', '1_1_9_request.json','3x3_large-dist_cost.json', True),
]
