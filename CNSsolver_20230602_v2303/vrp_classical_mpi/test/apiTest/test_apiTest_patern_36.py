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
# テスト36(配送件数平準化)
#-----------------------------------------------------------------
# 記載のないものは距離は1000、時間は1、電力は1で固定、電力容量は10、初期電力量は10、電力マージンは1、充電レートは1e12で固定、配送時間枠は配送先、車両ともに1000/1/1 0:00:00～3000/1/1 23:59:00で固定、荷作業時間は0で固定
# *但し拠点と最終訪問先の配送時間枠は1000/1/1 0:00:00～3000/1/1 0:00:00

test_args=[
    # ■異常系
    # 1. 車両数 : 1、配送先数 : 1、avevisits_ape=-1とした場合に
    # "APE margin of visits-leveling(avevisits_ape) is less than 0.0"のエラーメッセージを出力して終了することを確認
    ('36_1_1', '36_1_1_request.json','2x2_cost.json', True),

    # 2. 車両数 : 1、配送先数 : 1、avevisits_ae=-1とした場合に
    # "AE margin of visits-leveling(avevisits_ae) is less than 0.0"のエラーメッセージを出力して終了することを確認
    ('36_1_2', '36_1_2_request.json','2x2_cost.json', True),

    # 3. 車両数 : 1、配送先数 : 1、avevisits_ape=1、avevisits_ae=1とした場合に
    # "APE margin of visits-leveling(avevisits_ape_flag) and AE margin of visits-leveling(avevisits_ae_flag) set"のエラーメッセージを出力して終了することを確認
    ('36_1_3', '36_1_3_request.json','2x2_cost.json', True),

    # ■正常系
    # 4. 車両数 : 1、配送先数 : 2、avevisits_ape=0とした場合に
    # 解が作成されることを確認
    ('36_1_4', '36_1_4_request.json','3x3_cost.json', True),

    # 5. 車両数 : 2、配送先数 : 6、avevisits_ape=10、初期解は1台目に1つ、2台目に5つの配送先、
    # management_dtable : [{vtype : "0",start_time : 0,table : デフォルト},{vtype : "1",start_time : 0,table : 全て100}]とした場合に
    # avevisits_ape指定なしの解(初期解のまま)と同じ解が作成されることを確認
    ('36_1_5', '36_1_5_request.json','7x7_cost_36_1_5.json', True),

    # 6. 車両数 : 2、配送先数 : 6、avevisits_ape=0.5、初期解は1台目に1つ、2台目に5つの配送先、
    # management_dtable : [{vtype : "0",start_time : 0,table : デフォルト},{vtype : "1",start_time : 0,table : 全て100}]とした場合に
    # 1台目に2つ、2台目に4つの配送先の解が作成されることを確認
    ('36_1_6', '36_1_6_request.json','7x7_cost_36_1_6.json', True),

    # 7. 車両数 : 2、配送先数 : 6、avevisits_ape=0、初期解は1台目に1つ、2台目に5つの配送先、
    # management_dtable : [{vtype : "0",start_time : 0,table : デフォルト},{vtype : "1",start_time : 0,table : 全て100}]とした場合に
    # 1台目に3つ、2台目に3つの配送先の解が作成されることを確認
    ('36_1_7', '36_1_7_request.json','7x7_cost_36_1_7.json', True),

    # 8. 車両数 : 2、配送先数 : 7、avevisits_ape=0、初期解は1台目に1つ、2台目に6つの配送先、
    # management_dtable : [{vtype : "0",start_time : 0,table : デフォルト},{vtype : "1",start_time : 0,table : 全て100}]とした場合に
    # 1台目に3つ、2台目に4つの配送先の解が作成されることを確認(守れない場合)
    ('36_1_8', '36_1_8_request.json','8x8_cost_36_1_8.json', True),

    # 9. 車両数 : 2、配送先数 : 7(2台目の初期ルートの配送先には1台目は立寄不可)、avevisits_ape=0、初期解は1台目に2つ、2台目に5つの配送先、
    # management_dtable : [{vtype : "0",start_time : 0,table : デフォルト},{vtype : "1",start_time : 0,table : 全て100}]とした場合に
    # 初期解のままとなることを確認
    ('36_1_9', '36_1_9_request.json','8x8_cost_36_1_9.json', True),

    # 10. 車両数 : 2、配送先数 : 6、avevisits_ape=0、初期解は1台目に1つ、2台目に5つの配送先、台数削減モード、
    # management_dtable : [{vtype : "0",start_time : 0,table : デフォルト},{vtype : "1",start_time : 0,table : 全て100}]とした場合に
    # 2台目に6つの配送先の解が作成されることを確認(台数削減モード)
    ('36_1_10', '36_1_10_request.json','7x7_cost_36_1_10.json', True),

    # 11. 車両数 : 3、配送先数 : 6(1番目と3番目の配送先に1台目の車両は立寄不可、2番目の配送先に2台目の車両は立寄不可)、avevisits_ape=0、初期解は1台目に1つ、2台目に1つ、3台目に4つの配送先、台数削減モード、
    # management_dtable : [{vtype : "0",start_time : 0,table : デフォルト},{vtype : "1",start_time : 0,table : 全て100},{vtype : "2",start_time : 0,table : 全て10}]とした場合に
    # 1台目に2つ、2台目に2つ、3台目に2つの配送先の解が作成されることを確認(avevisits_apeを守れない場合は削減しない)
    ('36_1_11', '36_1_11_request.json','7x7_cost_36_1_11.json', True),

    # 12. 車両数 : 2、配送先数 : 6(SPOTID 1、2、2、2、2、2)、avevisits_ape=0、初期解 0→1→0および0→2→3→4→5→6→0、
    # management_dtable : [{vtype : "0",start_time : 0,table : デフォルト},{vtype : "1",start_time : 0,table : 全て100}]とした場合に
    # 1台目に3つ、2台目に3つの配送先の解が作成されることを確認(SPOTID)
    ('36_1_12', '36_1_12_request.json','3x3_cost_36_1_12.json', True),

    # 13. 車両数 : 2、配送先数 : 6(SPOTID 1、2、2、3、3、3)、avevisits_ape=0、初期解 0→1→0および0→2→3→4→5→6→0、大量輸送モード、
    # management_dtable : [{vtype : "0",start_time : 0,table : デフォルト},{vtype : "1",start_time : 0,table : 全て100}]とした場合に
    # 1台目にSPOTID3の配送先、2台目にSPOTIDが1と2の配送先の解が作成されることを確認(大量輸送モード)
    ('36_1_13', '36_1_13_request.json','4x4_cost_36_1_13.json', True),

    # 14. 車両数 : 3、配送先数 : 6(SPOTID 1、1、1、1、5、6、荷物量 100、100、100、100、1、1、積載量 400、300、300)、avevisits_ape=0、初期解生成モードとした場合に
    # 0→1→2→3→4→0(直送便)および0→6→0および0→5→0となることを確認(直送便は対象外)
    ('36_1_14', '36_1_14_request.json','4x4_cost.json', True),

    # 15. 車両数 : 2、配送先数 : 4(移動距離 1→2で10000000000、2→1および3→2および4→2および1→3は通行不可)、初期解を0→1→2→0および0→3→4→0、avevisits_ape=0とした場合に
    # merge処理が実行されてから初期解に戻ることを確認(初期解違反なし、avevisits_ape違反)
    ('36_1_15', '36_1_15_request.json','5x5_cost_36_1_15.json', True),

    # 16. 車両数 : 2、配送先数 : 6(消費電力はすべて3)、充電スポット数 : 1、avevisits_ape=0.4、初期解は1台目に2つ、2台目に4つの配送先、evplanとした場合に
    # 1台目に2つ、2台目に4つの配送先(2つ目と3つ目の配送先の間で9充電)の解が作成されることを確認(充電スポットはカウントしない)
    ('36_1_16', '36_1_16_request.json','8x8_cost_36_1_16.json', True),

    # 17. 車両数 : 1、配送先数 : 2、avevisits_ae=0とした場合に
    # 解が作成されることを確認
    ('36_1_17', '36_1_17_request.json','3x3_cost.json', True),

    # 18. 車両数 : 2、配送先数 : 6、avevisits_ae=10、初期解は1台目に1つ、2台目に5つの配送先、
    # management_dtable : [{vtype : "0",start_time : 0,table : デフォルト},{vtype : "1",start_time : 0,table : 全て100}]とした場合に
    # avevisits_ae指定なしの解(初期解のまま)と同じ解が作成されることを確認
    ('36_1_18', '36_1_18_request.json','7x7_cost_36_1_18.json', True),

    # 19. 車両数 : 2、配送先数 : 6、avevisits_ae=2、初期解は1台目に1つ、2台目に5つの配送先、
    # management_dtable : [{vtype : "0",start_time : 0,table : デフォルト},{vtype : "1",start_time : 0,table : 全て100}]とした場合に
    # 1台目に2つ、2台目に4つの配送先の解が作成されることを確認
    ('36_1_19', '36_1_19_request.json','7x7_cost_36_1_19.json', True),

    # 20. 車両数 : 2、配送先数 : 6、avevisits_ae=0、初期解は1台目に1つ、2台目に5つの配送先、
    # management_dtable : [{vtype : "0",start_time : 0,table : デフォルト},{vtype : "1",start_time : 0,table : 全て100}]とした場合に
    # 1台目に3つ、2台目に3つの配送先の解が作成されることを確認
    ('36_1_20', '36_1_20_request.json','7x7_cost_36_1_20.json', True),

    # 21. 車両数 : 2、配送先数 : 7、avevisits_ae=0、初期解は1台目に1つ、2台目に6つの配送先、
    # management_dtable : [{vtype : "0",start_time : 0,table : デフォルト},{vtype : "1",start_time : 0,table : 全て100}]とした場合に
    # 1台目に3つ、2台目に4つの配送先の解が作成されることを確認(守れない場合)
    ('36_1_21', '36_1_21_request.json','8x8_cost_36_1_21.json', True),

    # 22. 車両数 : 2、配送先数 : 7(2台目の初期ルートの配送先には1台目は立寄不可)、avevisits_ae=0、初期解は1台目に2つ、2台目に5つの配送先、
    # management_dtable : [{vtype : "0",start_time : 0,table : デフォルト},{vtype : "1",start_time : 0,table : 全て100}]とした場合に
    # 初期解のままとなることを確認
    ('36_1_22', '36_1_22_request.json','8x8_cost_36_1_22.json', True),

    # 23. 車両数 : 2、配送先数 : 6、avevisits_ae=0、初期解は1台目に1つ、2台目に5つの配送先、台数削減モード、
    # management_dtable : [{vtype : "0",start_time : 0,table : デフォルト},{vtype : "1",start_time : 0,table : 全て100}]とした場合に
    # 2台目に6つの配送先の解が作成されることを確認(台数削減モード)
    ('36_1_23', '36_1_23_request.json','7x7_cost_36_1_23.json', True),

    # 24. 車両数 : 3、配送先数 : 6(1番目と3番目の配送先に1台目の車両は立寄不可、2番目の配送先に2台目の車両は立寄不可)、avevisits_ae=0、初期解は1台目に1つ、2台目に1つ、3台目に4つの配送先、台数削減モード、
    # management_dtable : [{vtype : "0",start_time : 0,table : デフォルト},{vtype : "1",start_time : 0,table : 全て100},{vtype : "2",start_time : 0,table : 全て10}]とした場合に
    # 1台目に2つ、2台目に2つ、3台目に2つの配送先の解が作成されることを確認(avevisits_aeを守れない場合は削減しない)
    ('36_1_24', '36_1_24_request.json','7x7_cost_36_1_24.json', True),

    # 25. 車両数 : 2、配送先数 : 6(SPOTID 1、2、2、2、2、2)、avevisits_ae=0、初期解 0→1→0および0→2→3→4→5→6→0、
    # management_dtable : [{vtype : "0",start_time : 0,table : デフォルト},{vtype : "1",start_time : 0,table : 全て100}]とした場合に
    # 1台目に3つ、2台目に3つの配送先の解が作成されることを確認(SPOTID)
    ('36_1_25', '36_1_25_request.json','3x3_cost_36_1_25.json', True),

    # 26. 車両数 : 2、配送先数 : 6(SPOTID 1、2、2、3、3、3)、avevisits_ae=0、初期解 0→1→0および0→2→3→4→5→6→0、大量輸送モード、
    # management_dtable : [{vtype : "0",start_time : 0,table : デフォルト},{vtype : "1",start_time : 0,table : 全て100}]とした場合に
    # 1台目にSPOTID3の配送先、2台目にSPOTIDが1と2の配送先の解が作成されることを確認(大量輸送モード)
    ('36_1_26', '36_1_26_request.json','4x4_cost_36_1_26.json', True),

    # 27. 車両数 : 3、配送先数 : 6(SPOTID 1、1、1、1、5、6、荷物量 100、100、100、100、1、1、積載量 400、300、300)、avevisits_ae=0、初期解生成モードとした場合に
    # 0→1→2→3→4→0(直送便)および0→5→0および0→6→0となることを確認(直送便は対象外)
    ('36_1_27', '36_1_27_request.json','4x4_cost.json', True),

    # 28. 車両数 : 2、配送先数 : 4(移動距離 1→2で10000000000、2→1および3→2および4→2および1→3は通行不可)、初期解を0→1→2→0および0→3→4→0、avevisits_ae=0とした場合に
    # merge処理が実行されてから初期解に戻ることを確認(初期解違反なし、avevisits_ae違反)
    ('36_1_28', '36_1_28_request.json','5x5_cost_36_1_28.json', True),

    # 29. 車両数 : 2、配送先数 : 6(消費電力はすべて3)、充電スポット数 : 1、avevisits_ae=2、初期解は1台目に2つ、2台目に4つの配送先、evplanとした場合に
    # 1台目に2つ、2台目に4つの配送先(2つ目と3つ目の配送先の間で9充電)の解が作成されることを確認(充電スポットはカウントしない)
    ('36_1_29', '36_1_29_request.json','8x8_cost_36_1_29.json', True),

]
