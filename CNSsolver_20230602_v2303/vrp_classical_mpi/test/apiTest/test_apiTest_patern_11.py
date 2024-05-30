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
# テスト11(ヘテロ車両対応)
#-----------------------------------------------------------------
# 指定のないものは距離は1000、時間は1で固定、荷作業時間は1、配送時間枠は2000/1/1 9:00:00～2000/1/1 17:00:00で固定
# *但し拠点と最終訪問先の配送時間枠は1000/1/1 0:00:00～3000/1/1 0:00:00

test_args=[
	# ■正常系
	# 1. 車両数 : 2、配送先数 : 2(vtype "0"、"0"、初期解 0→1→0および0→2→0)、
	# management_dtable : 
	#  [{vtype : "0",start_time : 0,table : デフォルト},
	#   {vtype : "1",start_time : 0,table : 全て通行不可}]とした場合に
	# 解が作成されることを確認(vehnoではなくvtypeで参照することを確認)
	('11_1_1', '11_1_1_request.json','3x3_cost_11_1_1.json', True),

	# 2. 車両数 : 2、配送先数 : 2(vtype "0"、"0"、drskill=0.5、1.0、初期解 0→1→0および0→2→0)、
	# management_dtable : 
	#  [{vtype : "0",start_time : 0,table : 0→2で10000},
	#   {vtype : "1",start_time : 0,table : 全て通行不可}]
	# management_ttable : 
	#  [{vtype : "common",start_time : 0,table : 0→2で3}]とした場合に
	# 0→1→0(drskill=1.0)および0→2→0(drskill=0.5)となることを確認(vtypeが同じ場合スキルシャッフル)
	('11_1_2', '11_1_2_request.json','3x3_cost_11_1_2.json', True),

	# 3. 車両数 : 2、配送先数 : 6(vtype "common"、"common"、移動時間はすべて3、初期解 0→1→2→3→0および0→4→5→6→0)、
	# management_dtable : 
	#  [{vtype : "common",start_time : 0,table : 0→2で100、0→5で100、2→4で10000、5→1で10000},
	#   {vtype : "time_common",start_time : 3,table : 2→4で100、5→1で100、0→2で10000、0→5で10000},
	#   {vtype : "0",start_time : 0,table : 2→4および0→5は通行不可},
	#   {vtype : "1",start_time : 0,table : 2→4および0→5は通行不可}]とした場合に
	# 0→5→1→3→0および0→2→4→6→0となることを確認(vtype="common"の車両は"common"および"time_common"のテーブルのみ参照)
	('11_1_3', '11_1_3_request.json','7x7_cost_11_1_3.json', True),

]

