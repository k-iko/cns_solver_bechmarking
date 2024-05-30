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
# テスト5(最適化指標)
#-----------------------------------------------------------------
# 記載のないものは距離は1000、時間は1で固定、配送時間枠は1000/1/1 0:00:00～3000/1/1 0:00:00で固定、荷作業時間は1で固定
# *但し拠点と最終訪問先の配送時間枠は1000/1/1 0:00:00～3000/1/1 0:00:00

test_args=[
	# ■異常系
	# 1. 車両数 : 1、配送先数 : 1、opt_d_weight=-1とした場合に
	# "distance for optimization metirc (aka opt_d_weight) set to less than 0.0"のエラーメッセージを出力して終了することを確認
	('5_1_1', '5_1_1_request.json','2x2_cost.json', True),
	# 2. 車両数 : 1、配送先数 : 1、opt_d_weight=2とした場合に
	# "distance for optimization metric (aka opt_d_weight) set to more than 1.0"のエラーメッセージを出力して終了することを確認
	('5_1_2', '5_1_2_request.json','2x2_cost.json', True),
	# 3. 車両数 : 1、配送先数 : 1、opt_t_weight=-1とした場合に
	# "time for optimization metric (aka opt_t_weight) set to less than 0.0"のエラーメッセージを出力して終了することを確認
	('5_1_3', '5_1_3_request.json','2x2_cost.json', True),
	# 4. 車両数 : 1、配送先数 : 1、opt_t_weight=2とした場合に
	# "time for optimization metric (aka opt_t_weight) set to more than 1.0"のエラーメッセージを出力して終了することを確認
	('5_1_4', '5_1_4_request.json','2x2_cost.json', True),
	# 5. 車両数 : 1、配送先数 : 1、opt_e_weight=-1とした場合に
	# "epower for optimization metric (aka opt_e_weight) set to less than 0.0"のエラーメッセージを出力して終了することを確認
	('5_1_5', '5_1_5_request.json','2x2_cost.json', True),
	# 6. 車両数 : 1、配送先数 : 1、opt_e_weight=2とした場合に
	# "epower for optimization metric (aka opt_e_weight) set to more than 1.0"のエラーメッセージを出力して終了することを確認
	('5_1_6', '5_1_6_request.json','2x2_cost.json', True),
	# 7. 車両数 : 1、配送先数 : 1、opt_x_weight=-1とした場合に
	# "empirical cost for optimization metric (aka opt_x_weight) set to less than 0.0"のエラーメッセージを出力して終了することを確認
	('5_1_7', '5_1_7_request.json','2x2_cost.json', True),
	# 8. 車両数 : 1、配送先数 : 1、opt_x_weight=2とした場合に
	# "empirical cost for optimization metric (aka opt_x_weight) set to more than 1.0"のエラーメッセージを出力して終了することを確認
	('5_1_8', '5_1_8_request.json','2x2_cost.json', True),
	# ■正常系
	# 9. 車両数 : 2、配送先数 : 4(移動距離　1→3および2→4で100、移動時間　1→3および2→4で1000)、
	# opt_d_weight=1、初期解を0→1→2→0および0→3→4→0とした場合に
	# 距離が短くなるような改善解(0→1→3→0および0→2→4→0)となることを確認
	('5_1_9', '5_1_9_request.json','5x5_cost_opt_d.json', True),
	# 10. 車両数 : 2、配送先数 : 4(移動距離　1→3および2→4で10000、移動時間　1→3および2→4で0.5)、
	# opt_t_weight=1、初期解を0→1→2→0および0→3→4→0とした場合に
	# 時間が短くなるような改善解(0→1→3→0および0→2→4→0)となることを確認
	('5_1_10', '5_1_10_request.json','5x5_cost_opt_t.json', True),
]

