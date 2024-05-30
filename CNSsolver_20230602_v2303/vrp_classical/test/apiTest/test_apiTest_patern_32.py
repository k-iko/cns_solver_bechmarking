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
# テスト32(経験コスト全般)
#-----------------------------------------------------------------
# 指定のないものは距離は1000、時間は1、経験コストは1000、電力は1で固定、電力容量は10、初期電力量は10、電力マージンは1、充電レートは1e12、荷作業時間は1(充電スポットは0)、配送時間枠および車両営業時間は2000/1/1 9:00:00～2000/1/1 20:00:00で固定
# *但し拠点と最終訪問先の配送時間枠は1000/1/1 0:00:00～3000/1/1 0:00:00

test_args=[
    # ■異常系
    # 1. 車両数 : 1、配送先数 : 1、management_xtable : [{vtype : "time_common",start_time : 1,table : デフォルト}]とした場合に
    # "empCost table (aka xtable) does not exist"のエラーメッセージを出力して終了することを確認(9:00に参照するテーブルがないため)
    ('32_1_1', '32_1_1_request.json','2x2_cost_32_1_1.json', True),

    # 2. 車両数 : 1、配送先数 : 1、management_xtable : [{vtype : "1",start_time : 0,table : デフォルト}]とした場合に
    # "empCost table (aka xtable) does not exist"のエラーメッセージを出力して終了することを確認(vehno="0"が参照するテーブルがないため)
    ('32_1_2', '32_1_2_request.json','2x2_cost_32_1_2.json', True),

    # 3. 車両数 : 1、配送先数 : 1、management_xtable : [{vtype : "time_common",start_time : 1,table : デフォルト},{vtype : "1",start_time : 0,table : デフォルト}]とした場合に
    # "empCost table (aka xtable) does not exist"のエラーメッセージを出力して終了することを確認(vehno="0"、9:00が参照するテーブルがないため)
    ('32_1_3', '32_1_3_request.json','2x2_cost_32_1_3.json', True),

    # 4. 車両数 : 1、配送先数 : 3(移動時間はすべて3、初期解 0→1→2→3→0)、
    # management_xtable : [{vtype : "time_common",start_time : 0,table : デフォルト},{vtype : "time_common",start_time : 3,table : 2→3は通行不可、その他はデフォルト}]とした場合に
    # "failed to read initial solution due to cost of -1"のエラーメッセージを出力して終了することを確認(時間別テーブル)
    ('32_1_4', '32_1_4_request.json','4x4_cost_32_1_4.json', True),

    # 5. 車両数 : 2、配送先数 : 4(初期解 0→1→2→0および0→3→4→0)、
    # management_xtable : [{vtype : "0",start_time : 0,table : デフォルト},{vtype : "1",start_time : 0,table : 3→4は通行不可、その他はデフォルト}]とした場合に
    # "failed to read initial solution due to cost of -1"のエラーメッセージを出力して終了することを確認(車両別テーブル)
    ('32_1_5', '32_1_5_request.json','5x5_cost_32_1_5.json', True),

    # 6. 車両数 : 1、配送先数 : 1、management_xtable : [{vtype : "1",start_time : 0,table : デフォルト},{vtype : "1",start_time : 0,table : 全て2}]とした場合に
    # "same empCost tables (aka xtable) exist"のエラーメッセージを出力して終了することを確認(vtype : "1",start_time : 0のテーブルが複数あるため)
    ('32_1_6', '32_1_6_request.json','2x2_cost_32_1_6.json', True),

    # 7. 車両数 : 1、配送先数 : 1、management_xtable : [{vtype : "common",start_time : 0,table : デフォルト},{vtype : "common",start_time : 1,table : 全て2}]とした場合に
    # "same empCost tables (aka xtable) exist"のエラーメッセージを出力して終了することを確認(vtype : "common"のテーブルが複数あるため)
    ('32_1_7', '32_1_7_request.json','2x2_cost_32_1_7.json', True),

    # 8. 車両数 : 1、配送先数 : 1、opt_x_weight=-1とした場合に
    # "empirical cost for optimization metric (aka opt_x_weight) set to less than 0.0"のエラーメッセージを出力して終了することを確認
    ('32_1_8', '32_1_8_request.json','2x2_cost_32_1_8.json', True),

    # 9. 車両数 : 1、配送先数 : 1、opt_x_weight=2とした場合に
    # "empirical cost for optimization metric (aka opt_x_weight) set to more than 1.0"のエラーメッセージを出力して終了することを確認
    ('32_1_9', '32_1_9_request.json','2x2_cost_32_1_9.json', True),

    # 10. 車両数 : 1、配送先数 : 2(SPOTID 1、2、経験コストテーブルのサイズ 2×2)とした場合に
    # "empCost table (aka xtable) does not match spotid"のエラーメッセージを出力して終了することを確認
    ('32_1_10', '32_1_10_request.json','3x3_cost_32_1_10.json', True),

    # 11. 車両数 : 1、配送先数 : 2(SPOTID 1、1、経験コストテーブルのサイズ 3×3)とした場合に
    # "empCost table (aka xtable) does not match spotid"のエラーメッセージを出力して終了することを確認
    ('32_1_11', '32_1_11_request.json','2x2_cost_32_1_11.json', True),

    # 12. 車両数 : 1、充電スポット数 : 1、配送先数 : 2(SPOTID 1、2、経験コストテーブルのサイズ 3×3)とした場合に
    # "empCost table (aka xtable) does not match spotid"のエラーメッセージを出力して終了することを確認
    ('32_1_12', '32_1_12_request.json','4x4_cost_32_1_12.json', True),


    # ■正常系
    # 13. 車両数 : 1、配送先数 : 3(移動時間はすべて3、初期解 0→1→2→3→0)、
    # management_xtable : [{vtype : "time_common",start_time : 0,table : 1→3で10000、0→2で100},{vtype : "time_common",start_time : 3,table : 1→3で100、0→2で10000}]、経験コストで最適化とした場合に
    # 0→2→1→3→0となることを確認(時間別、commonなし)
    ('32_1_13', '32_1_13_request.json','4x4_cost_32_1_13.json', True),

    # 14. 車両数 : 1、配送先数 : 3(移動時間はすべて3、初期解 0→1→2→3→0)、
    # management_xtable : [{vtype : "common",start_time : 0,table : 1→3で10000、0→2で100},{vtype : "time_common",start_time : 3,table : 1→3で100、0→2で10000},{vtype : "time_common",start_time : 10,table : 1→3および0→2は通行不可}]、経験コストで最適化とした場合に
    # 0→2→1→3→0となることを確認(時間別、commonあり)
    ('32_1_14', '32_1_14_request.json','4x4_cost_32_1_14.json', True),

    # 15. 車両数 : 1、配送先数 : 3(移動時間はすべて3、初期解 0→1→2→3→0)、
    # management_xtable : [{vtype : "common",start_time : 0,table : 1→3および0→2は通行不可},{vtype : "time_common",start_time : 0,table : 1→3で10000、0→2で100},{vtype : "time_common",start_time : 3,table : 1→3で100、0→2で10000}]、経験コストで最適化とした場合に
    # 0→2→1→3→0となることを確認(時間別、commonあり、time_commonが優先される)
    ('32_1_15', '32_1_15_request.json','4x4_cost_32_1_15.json', True),

    # 16. 車両数 : 2、配送先数 : 4(初期解 0→1→2→0および0→3→4→0)、
    # management_xtable : [{vtype : "0",start_time : 0,table : 1→4で10000、0→3で100},{vtype : "1",start_time : 0,table : 0→1で100、3→2で10000}]、経験コストで最適化とした場合に
    # 0→3→2→0および0→1→4→0となることを確認(車両別、commonとtime_commonなし)
    ('32_1_16', '32_1_16_request.json','5x5_cost_32_1_16.json', True),

    # 17. 車両数 : 2、配送先数 : 4(初期解 0→1→2→0および0→3→4→0)、
    # management_xtable : [{vtype : "time_common",start_time : 0,table : 1→4で10000、0→3で100},{vtype : "1",start_time : 0,table : 0→1で100、3→2で10000},{vtype : "3",start_time : 0,table : 1→4および0→3は通行不可}]、経験コストで最適化とした場合に
    # 0→3→2→0および0→1→4→0となることを確認(車両別、time_commonあり)
    ('32_1_17', '32_1_17_request.json','5x5_cost_32_1_17.json', True),

    # 18. 車両数 : 2、配送先数 : 4(初期解 0→1→2→0および0→3→4→0)、
    # management_xtable : [{vtype : "common",start_time : 0,table : 1→4で10000、0→3で100},{vtype : "1",start_time : 0,table : 0→1で100、3→2で10000},{vtype : "3",start_time : 0,table : 1→4および0→3は通行不可}]、経験コストで最適化とした場合に
    # 0→3→2→0および0→1→4→0となることを確認(車両別、commonあり)
    ('32_1_18', '32_1_18_request.json','5x5_cost_32_1_18.json', True),

    # 19. 車両数 : 2、配送先数 : 6(移動時間はすべて3、初期解 0→1→2→3→0および0→4→5→6→0)、
    # management_xtable : [{vtype : "0",start_time : 0,table : 2→4で10000、0→5で100},{vtype : "1",start_time : 0,table : 0→2で100、5→1で10000},{vtype : "0",start_time : 3,table : 5→1で100、0→2で10000},{vtype : "1",start_time : 3,table : 2→4で100、0→5で10000}]、経験コストで最適化とした場合に
    # 0→5→1→3→0および0→2→4→6→0となることを確認(時間別車両別、commonとtime_commonなし)
    ('32_1_19', '32_1_19_request.json','7x7_cost_32_1_19.json', True),

    # 20. 車両数 : 2、配送先数 : 6(移動時間はすべて3、初期解 0→1→2→3→0および0→4→5→6→0)、
    # management_xtable : [{vtype : "common",start_time : 0,table : 2→4で10000、0→5で100},{vtype : "time_common",start_time : 3,table : 2→4で100、0→5で10000},{vtype : "0",start_time : 3,table : 5→1で100、0→2で10000},{vtype : "0",start_time : 10,table : 2→4および0→5は通行不可},{vtype : "1",start_time : 0,table : 0→2で100、5→1で10000},{vtype : "3",start_time : 3,table : 2→4および0→5は通行不可}]、経験コストで最適化とした場合に
    # 0→5→1→3→0および0→2→4→6→0となることを確認(時間別車両別、commonとtime_commonあり)
    ('32_1_20', '32_1_20_request.json','7x7_cost_32_1_20.json', True),

    # 21. 車両数 : 2、配送先数 : 6(2台目の営業時間枠 2000/1/1 8:00:00～2000/1/1 20:00:00、移動時間はすべて3、初期解 0→1→2→3→0および0→4→5→6→0)、
    # management_xtable : [{vtype : "common",start_time : 0,table : 2→4で10000、0→5で100},{vtype : "time_common",start_time : 2,table : 2→4で100、0→5で10000},{vtype : "0",start_time : 3,table : 5→1で100、0→2で10000},{vtype : "0",start_time : 10,table : 2→4および0→5は通行不可},{vtype : "1",start_time : -1,table : 0→2で100、5→1で10000},{vtype : "2",start_time : 3,table : 2→4および0→5は通行不可}]、経験コストで最適化とした場合に
    # 0→5→1→3→0および0→2→4→6→0となることを確認(時間別車両別、commonとtime_commonあり、配送時間マイナス)
    ('32_1_21', '32_1_21_request.json','7x7_cost_32_1_21.json', True),

    # 22. 車両数 : 2、配送先数 : 6(移動時間はすべて3、初期解 0→1→2→3→0および0→4→5→6→0)、
    # management_xtable : [{vtype : "3",start_time : 3,table : 2→4および0→5は通行不可},{vtype : "time_common",start_time : 3,table : 2→4で100、0→5で10000},{vtype : "0",start_time : 10,table : 2→4および0→5は通行不可},{vtype : "common",start_time : 0,table : 2→4で10000、0→5で100},{vtype : "0",start_time : 3,table : 5→1で100、0→2で10000},{vtype : "1",start_time : 0,table : 0→2で100、5→1で10000}]、経験コストで最適化とした場合に
    # 0→5→1→3→0および0→2→4→6→0となることを確認(時間別車両別、commonとtime_commonあり、順不同)
    ('32_1_22', '32_1_22_request.json','7x7_cost_32_1_22.json', True),

    # 23. 車両数 : 1、配送先数 : 3(移動時間はすべて3、初期解 0→1→2→3→0)、
    # management_xtable : [{vtype : "common",start_time : 0,table : 全て通行不可},{vtype : "time_common",start_time : 0,table : 1→3は通行不可、0→2で100},{vtype : "time_common",start_time : 3,table : 1→3で100、0→2は通行不可}]、経験コストで最適化とした場合に
    # 0→2→1→3→0となることを確認(通行不可)
    ('32_1_23', '32_1_23_request.json','4x4_cost_32_1_23.json', True),

    # 24. 車両数 : 2、配送先数 : 6(opskill=0.5、1.0、荷作業時間は4のみ2、初期解 0→1→2→3→0および0→4→5→6→0)、
    # management_xtable : [{vtype : "common",start_time : 0,table : 2→4で10000、0→5で100},{vtype : "time_common",start_time : 3,table : 2→4で100、0→5で10000},{vtype : "0",start_time : 3,table : 5→1で100、0→2で10000},{vtype : "0",start_time : 10,table : 2→4および0→5は通行不可},{vtype : "1",start_time : 0,table : 0→2で100、5→1で10000},{vtype : "3",start_time : 3,table : 2→4および0→5は通行不可}]、経験コストで最適化とした場合に
    # 0→5→1→3→0(opskill=0.5)および0→2→4→6→0(opskill=1.0)となることを確認(車両ごとにテーブルが異なるためスキルシャッフルは機能しない)
    ('32_1_24', '32_1_24_request.json','7x7_cost_32_1_24.json', True),

    # 25. 車両数 : 2、配送先数 : 6(opskill=0.5、1.0、荷作業時間は4のみ2、初期解 0→1→2→3→0および0→4→5→6→0)、
    # management_xtable : [{vtype : "time_common",start_time : 0,table : 1→3および4→6で10000、0→2および0→5で100},{vtype : "time_common",start_time : 3,table : 1→3および4→6で100、0→2および0→5で10000}]、経験コストで最適化とした場合に
    # 0→2→1→3→0(opskill=1.0)および0→5→4→6→0(opskill=0.5)となることを確認(同じテーブルを参照するためスキルシャッフル)
    ('32_1_25', '32_1_25_request.json','7x7_cost_32_1_25.json', True),

    # 26. 車両数 : 2、配送先数 : 6(drskill=0.5、1.0、荷作業時間は4のみ2、初期解 0→1→2→3→0および0→4→5→6→0)、
    # management_xtable : [{vtype : "time_common",start_time : 0,table : 1→3および4→6で10000、0→2および0→5で100},{vtype : "1",start_time : 0,table : 1→3および4→6で10000、0→2および0→5で100},{vtype : "1",start_time : 3,table : 1→3および4→6で100、0→2および0→5で10000},{vtype : "2",start_time : 3,table : 1→3および4→6で100、0→2および0→5で10000}]、経験コストで最適化とした場合に
    # 0→2→1→3→0(opskill=1.0)および0→5→4→6→0(opskill=0.5)となることを確認(別のテーブルだが中身は同じテーブルを参照するためスキルシャッフル)
    # ('32_1_26', '32_1_26_request.json','7x7_cost_32_1_26.json', True),
    #仕様変更によりテスト不可

    # 27. 車両数 : 1、配送先数 : 3(初期解 0→1→2→3→0)、
    # management_dtable : [{vtype : "time_common",start_time : 0,table : 1→3で10000、0→2で100},{vtype : "time_common",start_time : 5,table : 1→3で100}]、
    # management_ttable : [{vtype : "time_common",start_time : 0,table : 1→3で10、0→2で3、その他は4},{vtype : "time_common",start_time : 3,table : 0→2で10、その他は4}]、
    # management_etable : [{vtype : "time_common",etype : "time_common",start_time : 0,table : 1→3で10、0→2で3、その他は4},{vtype : "time_common",etype : "time_common",start_time : 2,table : 0→2で10、その他は4}]、
    # management_xtable : [{vtype : "time_common",start_time : 0,table : 1→3で10000、0→2で100},{vtype : "time_common",start_time : 1,table : 1→3で100}]、経験コストで最適化とした場合に
    # 0→2→1→3→0となることを確認(dtableとttableとetableとxtableの時間枠が異なる場合)
    ('32_1_27', '32_1_27_request.json','4x4_cost_32_1_27.json', True),

    # 28. 車両数 : 2、配送先数 : 4(移動距離　1→3および2→4で1001、移動時間　1→3および2→4で2、消費電力　1→3および2→4で4、経験コスト　1→3および2→4で998)、opt_d_weight=0.2、opt_t_weight=0.2、opt_e_weight=0.2、opt_x_weight=0.4、初期解を0→1→2→0および0→3→4→0とした場合に
    # 0→1→3→0および0→2→4→0となることを確認(距離の悪化と時間の悪化と消費電力の悪化の合計より経験コストの改善の方が影響が大きいため)
    ('32_1_28', '32_1_28_request.json','5x5_cost_32_1_28.json', True),

    # 29. 車両数 : 1(初期電力 5)、充電スポット数 : 1、配送先数 : 2(消費電力はすべて3、距離 e0→1は100、経験コスト e0→1は10000、初期解 0→1→2→0)、evplan=True、経験コストで最適化とした場合に
    # 0→e0→2→1→0(e0で8充電)となることを確認(経験コストが最適になるよう充電)
    ('32_1_29', '32_1_29_request.json','4x4_cost_32_1_29.json', True),

    # 30. 車両数 : 1(初期電力 5)、充電スポット数 : 1、配送先数 : 2(消費電力はすべて3、初期解 0→1→2→0)、evplan=True、
    # management_xtable : [{vtype : "0",start_time : 0,table : e0→2で10000},{vtype : "0",start_time : 1,table : e0→2で100},{vtype : "1",start_time : 0,table : e0→2で10000}]、経験コストで最適化とした場合に
    # 0→e0→2→1→0(e0で8充電)となることを確認(時間別車両別経験コストテーブル)
    ('32_1_30', '32_1_30_request.json','4x4_cost_32_1_30.json', True),

]
