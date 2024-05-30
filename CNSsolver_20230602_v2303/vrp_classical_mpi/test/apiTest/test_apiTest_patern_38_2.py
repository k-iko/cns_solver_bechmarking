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
# 指定のないものは距離は1000、時間は1、荷作業時間は1、配送時間枠は2000/1/1 9:00:00～2000/1/1 17:00:00で固定
# *但し拠点と最終訪問先の配送時間枠は1000/1/1 0:00:00～3000/1/1 0:00:00
test_args=[
	#■異常系
	# 1. 車両数 : 1(拠点作業時間 -1)、配送先数 : 1、回転モードとした場合に
	# "vehicle depot service time(aka depotservt) set to less than 0"のエラーメッセージを出力して終了することを確認(車両の拠点作業時間が負)
	('38_2_1', '38_2_1_request.json','2x2_cost.json', True),

	# ■正常系
	# 2. 車両数 : 1(積載量 300、拠点作業時間 60)、配送先数 : 2(荷物量 300、300、配送時間枠 2000/1/1 9:00:00～2000/1/1 17:00:00、2000/1/1 9:00:00～2000/1/1 14:00:00)、初期解 0→1→0→2→0、回転モードとした場合に
	# 0→2→0→1→0となることを確認(車両の拠点作業時間)
	('38_2_2', '38_2_2_request.json','3x3_cost.json', True),

	# 3. 車両数 : 1(積載量 300、拠点作業時間 60、opskill 0.2)、配送先数 : 2(荷物量 300、300、配送時間枠 2000/1/1 9:00:00～2000/1/1 17:00:00、2000/1/1 9:00:00～2000/1/1 14:00:00)、初期解 0→1→0→2→0、回転モードとした場合に
	# 初期解のままとなることを確認(車両の拠点作業時間、スキル指定)
	('38_2_3', '38_2_3_request.json','3x3_cost.json', True),

	# 4. 車両数 : 1(積載量 300、拠点作業時間 60、車両営業時間枠 2000/1/1 9:00:00～2000/1/1 17:00:00)、配送先数 : 2(荷物量 300、300、配送時間枠 2000/1/1 12:00:00～2000/1/1 17:00:00、2000/1/1 9:00:00～2000/1/1 17:00:00)、初期解 0→1→0→2→0、回転モード、巡回モードとした場合に
	# 初期解のままとなることを確認(最終訪問先は車両の拠点作業時間を計上しない)
	('38_2_4', '38_2_4_request.json','3x3_cost.json', True),

	# 5. 車両数 : 1(積載量 300、拠点作業時間 30)、配送先数 : 2(荷物量 300、300、配送時間枠 2000/1/1 9:00:00～2000/1/1 17:00:00、2000/1/1 9:00:00～2000/1/1 14:00:00、拠点作業時間 30、30)、初期解 0→1→0→2→0、回転モードとした場合に
	# 0→2→0→1→0となることを確認(車両および配送先の拠点作業時間)
	('38_2_5', '38_2_5_request.json','3x3_cost.json', True),

	# 6. 車両数 : 1(積載量 300、拠点作業時間 60)、配送先数 : 2(荷物量 300、300、配送時間枠 2000/1/1 9:00:00～2000/1/1 17:00:00、2000/1/1 9:00:00～2000/1/1 12:00:00)、初期解 0→1→2→0とした場合に
	# 0→2→1→0となることを確認(車両の拠点作業時間、非回転モード)
	('38_2_6', '38_2_6_request.json','3x3_cost.json', True),
]
