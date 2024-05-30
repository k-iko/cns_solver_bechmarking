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
# 中国ユースケースの追加テスト
#-----------------------------------------------------------------
test_args=[
	# ■正常系
	# テスト１ 平準化なし (初期解 0⇒1⇒2⇒0, 0⇒3⇒4⇒5⇒0)
	('Arpa_1_1', 'Arpa_1_1_request.json','6x6_cost_Arpa_1_1.json', True),
	# テスト１ 平準化なし (初期解 0⇒5⇒4⇒3⇒0, 0⇒2⇒1⇒0)
	('Arpa_1_1b', 'Arpa_1_1b_request.json','6x6_cost_Arpa_1_1b.json', True),
	# テスト１ 平準化あり(10%)
	('Arpa_1_2', 'Arpa_1_2_request.json','6x6_cost_Arpa_1_2.json', True),
	# テスト２ 厳しい優先度
	('Arpa_2_1', 'Arpa_2_1_request.json','6x6_cost_Arpa_2_1.json', True),
	# テスト２ 緩い優先度
	('Arpa_2_2', 'Arpa_2_2_request.json','6x6_cost_Arpa_2_2.json', True),
	# テスト３ 受入時刻の違反解でも違反量最小の解になるか (初期解なし)
	('Arpa_3_1', 'Arpa_3_1_request.json','6x6_cost_Arpa_3_1.json', True),
	# テスト３ 受入時刻の違反解でも違反量最小の解になるか (初期解 0⇒2⇒5⇒3⇒1⇒4⇒0)
	('Arpa_3_2', 'Arpa_3_2_request.json','6x6_cost_Arpa_3_2.json', True),
]

