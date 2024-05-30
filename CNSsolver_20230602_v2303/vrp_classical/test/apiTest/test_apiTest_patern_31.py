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
# テスト31(EV Plan全般)
#-----------------------------------------------------------------
# 指定のないものは距離は1000、時間は1、電力は1で固定、電力容量は10、初期電力量は10、電力マージンは1、充電レートは1e12、荷作業時間は1(充電スポットは0)、配送時間枠および車両営業時間は2000/1/1 9:00:00～2000/1/1 20:00:00で固定
# *但し拠点と最終訪問先の配送時間枠は1000/1/1 0:00:00～3000/1/1 0:00:00
# *打ち切り時間 X秒の精度はX～1.4X以内とする(それ以上の場合、"Accuracy of Time-Out is low."のエラーメッセージを出力して終了)

test_args=[
    # ■異常系
    # 1. 車両数 : 1、配送先数 : 1、management_etable : [{vtype : "time_common",etype : "time_common",start_time : 1,table : デフォルト}]とした場合に
    # "epower table (aka etable) does not exist"のエラーメッセージを出力して終了することを確認(9:00に参照するテーブルがないため)
    ('31_1_1', '31_1_1_request.json','2x2_cost_31_1_1.json', True),

    # 2. 車両数 : 1、配送先数 : 1、management_etable : [{vtype : "1",etype : "1",start_time : 0,table : デフォルト}]とした場合に
    # "epower table (aka etable) does not exist"のエラーメッセージを出力して終了することを確認(vehno="0"が参照するテーブルがないため)
    ('31_1_2', '31_1_2_request.json','2x2_cost_31_1_2.json', True),

    # 3. 車両数 : 1、配送先数 : 1、management_etable : [{vtype : "time_common",etype : "time_common",start_time : 1,table : デフォルト},{vtype : "1",etype : "1",start_time : 0,table : デフォルト}]とした場合に
    # "epower table (aka etable) does not exist"のエラーメッセージを出力して終了することを確認(vehno="0"、9:00が参照するテーブルがないため)
    ('31_1_3', '31_1_3_request.json','2x2_cost_31_1_3.json', True),

    # 4. 車両数 : 1、配送先数 : 3(移動時間はすべて3、初期解 0→1→2→3→0)、
    # management_etable : [{vtype : "time_common",etype : "time_common",start_time : 0,table : デフォルト},{vtype : "time_common",etype : "time_common",start_time : 3,table : 2→3は通行不可、その他はデフォルト}]とした場合に
    # "failed to read initial solution due to cost of -1"のエラーメッセージを出力して終了することを確認(時間別テーブル)
    ('31_1_4', '31_1_4_request.json','4x4_cost_31_1_4.json', True),

    # 5. 車両数 : 2、配送先数 : 4(初期解 0→1→2→0および0→3→4→0)、
    # management_etable : [{vtype : "0",etype : "0",start_time : 0,table : デフォルト},{vtype : "1",etype : "1",start_time : 0,table : 3→4は通行不可、その他はデフォルト}]とした場合に
    # "failed to read initial solution due to cost of -1"のエラーメッセージを出力して終了することを確認(車両別テーブル)
    ('31_1_5', '31_1_5_request.json','5x5_cost_31_1_5.json', True),

    # 6. 車両数 : 1、配送先数 : 1、management_etable : [{vtype : "1",etype : "1",start_time : 0,table : デフォルト},{vtype : "1",etype : "1",start_time : 0,table : 全て2}]とした場合に
    # "same epower tables (aka etable) exist"のエラーメッセージを出力して終了することを確認(vtype : "1",etype : "1",start_time : 0のテーブルが複数あるため)
    ('31_1_6', '31_1_6_request.json','2x2_cost_31_1_6.json', True),

    # 7. 車両数 : 1、配送先数 : 1、management_etable : [{vtype : "common",etype : "common",start_time : 0,table : デフォルト},{vtype : "common",etype : "common",start_time : 1,table : 全て2}]とした場合に
    # "same epower tables (aka etable) exist"のエラーメッセージを出力して終了することを確認(vtype : "common",etype : "common"のテーブルが複数あるため)
    ('31_1_7', '31_1_7_request.json','2x2_cost_31_1_7.json', True),

    # 8. 車両数 : 1、配送先数 : 1、opt_e_weight=-1とした場合に
    # "epower for optimization metric (aka opt_e_weight) set to less than 0.0"のエラーメッセージを出力して終了することを確認
    ('31_1_8', '31_1_8_request.json','2x2_cost.json', True),

    # 9. 車両数 : 1、配送先数 : 1、opt_e_weight=2とした場合に
    # "epower for optimization metric (aka opt_e_weight) set to more than 1.0"のエラーメッセージを出力して終了することを確認
    ('31_1_9', '31_1_9_request.json','2x2_cost.json', True),

    # 10. 車両数 : 1、配送先数 : 2(SPOTID 1、2、電力テーブルのサイズ 2×2)とした場合に
    # "epower table (aka etable) does not match spotid"のエラーメッセージを出力して終了することを確認
    ('31_1_10', '31_1_10_request.json','3x3_cost_31_1_10.json', True),

    # 11. 車両数 : 1、配送先数 : 2(SPOTID 1、1、電力テーブルのサイズ 3×3)とした場合に
    # "epower table (aka etable) does not match spotid"のエラーメッセージを出力して終了することを確認
    ('31_1_11', '31_1_11_request.json','2x2_cost_31_1_11.json', True),

    # 12. 車両数 : 1、充電スポット数 : 1、配送先数 : 2(SPOTID 1、2、距離テーブルのサイズ 3×3)とした場合に
    # "distance table (aka dtable) does not match spotid"のエラーメッセージを出力して終了することを確認
    ('31_1_12', '31_1_12_request.json','4x4_cost_31_1_12.json', True),

    # 13. 車両数 : 1、充電スポット数 : 1、配送先数 : 2(SPOTID 1、2、時間テーブルのサイズ 3×3)とした場合に
    # "eta table (aka ttable) does not match spotid"のエラーメッセージを出力して終了することを確認
    ('31_1_13', '31_1_13_request.json','4x4_cost_31_1_13.json', True),

    # 14. 車両数 : 1、充電スポット数 : 1、配送先数 : 2(SPOTID 1、2、電力テーブルのサイズ 3×3)とした場合に
    # "epower table (aka etable) does not match spotid"のエラーメッセージを出力して終了することを確認
    ('31_1_14', '31_1_14_request.json','4x4_cost_31_1_14.json', True),

    # 15. 車両数 : 1、充電スポット数 : 1(e_rate -1)、配送先数 : 1とした場合に
    # "charging rate of charging stations (aka e_rate) set to less than 0"のエラーメッセージを出力して終了することを確認
    ('31_1_15', '31_1_15_request.json','3x3_cost.json', True),

    # 16. 車両数 : 1、充電スポット数 : 1(SPOTID 2)、配送先数 : 1とした場合に
    # "spotid of CS_spot must be the same as customer id (aka custno) of CS_spot"のエラーメッセージを出力して終了することを確認
    ('31_1_16', '31_1_16_request.json','3x3_cost.json', True),

    # 17. 車両数 : 1(電力容量 8)、充電スポット数 : 1、配送先数 : 1、evplan=Trueとした場合に
    # "max epower (aka e_capacity) < init epower (aka e_initial)"のエラーメッセージを出力して終了することを確認
    ('31_1_17', '31_1_17_request.json','3x3_cost.json', True),

    # 18. 車両数 : 1(電力マージン 10)、充電スポット数 : 1、配送先数 : 1、evplan=Trueとした場合に
    # "max epower (aka e_capacity) <= marginal epower (aka e_margin)"のエラーメッセージを出力して終了することを確認
    ('31_1_18', '31_1_18_request.json','3x3_cost.json', True),

    # 19. 車両数 : 1(電力マージン 11)、充電スポット数 : 1、配送先数 : 1、evplan=Trueとした場合に
    # "max epower (aka e_capacity) <= marginal epower (aka e_margin)"のエラーメッセージを出力して終了することを確認
    ('31_1_19', '31_1_19_request.json','3x3_cost.json', True),

    # 20. 車両数 : 1(初期電力 8、電力マージン 8)、充電スポット数 : 1、配送先数 : 1、evplan=Trueとした場合に
    # "init epower (aka e_initial) <= marginal epower (aka e_margin)"のエラーメッセージを出力して終了することを確認
    ('31_1_20', '31_1_20_request.json','3x3_cost.json', True),

    # 21. 車両数 : 1(初期電力 8、電力マージン 9)、充電スポット数 : 1、配送先数 : 1、evplan=Trueとした場合に
    # "init epower (aka e_initial) <= marginal epower (aka e_margin)"のエラーメッセージを出力して終了することを確認
    ('31_1_21', '31_1_21_request.json','3x3_cost.json', True),

    # 22. 車両数 : 1、充電スポット数 : 1(SPOTID 2、custno "2")、配送先数 : 1とした場合に
    # "CS_spots include something not "e" + positive integer"のエラーメッセージを出力して終了することを確認
    ('31_1_22', '31_1_22_request.json','3x3_cost.json', True),

    # 23. 車両数 : 1、充電スポット数 : 1(SPOTID 2、custno "2")、配送先数 : 1、初期解生成モードとした場合に
    # "CS_spots include something not "e" + positive integer"のエラーメッセージを出力して終了することを確認
    ('31_1_23', '31_1_23_request.json','3x3_cost.json', True),


    # ■正常系
    # 24. 車両数 : 1、配送先数 : 3(移動時間はすべて3、初期解 0→1→2→3→0)、
    # management_etable : [{vtype : "time_common",etype : "time_common",start_time : 0,table : 1→3で10、0→2で3、その他は4},{vtype : "time_common",etype : "time_common",start_time : 3,table : 1→3で3、0→2で10、その他は4}]、電力で最適化とした場合に
    # 0→2→1→3→0となることを確認(時間別、commonなし)
    ('31_1_24', '31_1_24_request.json','4x4_cost_31_1_24.json', True),

    # 25. 車両数 : 1、配送先数 : 3(移動時間はすべて3、初期解 0→1→2→3→0)、
    # management_etable : [{vtype : "common",etype : "common",start_time : 0,table : 1→3で10、0→2で3、その他は4},{vtype : "time_common",etype : "time_common",start_time : 3,table : 1→3で3、0→2で10、その他は4},{vtype : "time_common",etype : "time_common",start_time : 10,table : 1→3および0→2は通行不可、その他は4}]、電力で最適化とした場合に
    # 0→2→1→3→0となることを確認(時間別、commonあり)
    ('31_1_25', '31_1_25_request.json','4x4_cost_31_1_25.json', True),

    # 26. 車両数 : 1、配送先数 : 3(移動時間はすべて3、初期解 0→1→2→3→0)、
    # management_etable : [{vtype : "common",etype : "common",start_time : 0,table : 1→3および0→2は通行不可},{vtype : "time_common",etype : "time_common",start_time : 0,table : 1→3で10、0→2で3、その他は4},{vtype : "time_common",etype : "time_common",start_time : 3,table : 1→3で3、0→2で10、その他は4}]、電力で最適化とした場合に
    # 0→2→1→3→0となることを確認(時間別、commonあり、time_commonが優先される)
    ('31_1_26', '31_1_26_request.json','4x4_cost_31_1_26.json', True),

    # 27. 車両数 : 2、配送先数 : 4(移動時間はすべて3、初期解 0→1→2→0および0→3→4→0)、
    # management_etable : [{vtype : "0",etype : "0",start_time : 0,table : 1→4で10、0→3で3、その他は6},{vtype : "1",etype : "1",start_time : 0,table : 0→1で3、3→2で10、その他は6}]、電力で最適化とした場合に
    # 0→3→2→0および0→1→4→0となることを確認(車両別、commonとtime_commonなし)
    ('31_1_27', '31_1_27_request.json','5x5_cost_31_1_27.json', True),

    # 28. 車両数 : 2、配送先数 : 4(移動時間はすべて3、初期解 0→1→2→0および0→3→4→0)、
    # management_etable : [{vtype : "time_common",etype : "time_common",start_time : 0,table : 1→4で10、0→3で3、その他は6},{vtype : "1",etype : "1",start_time : 0,table : 0→1で3、3→2で10、その他は6},{vtype : "3",etype : "3",start_time : 0,table : 1→4および0→3は通行不可、その他は6}]、電力で最適化とした場合に
    # 0→3→2→0および0→1→4→0となることを確認(車両別、time_commonあり)
    ('31_1_28', '31_1_28_request.json','5x5_cost_31_1_28.json', True),

    # 29. 車両数 : 2、配送先数 : 4(移動時間はすべて3、初期解 0→1→2→0および0→3→4→0)、
    # management_etable : [{vtype : "common",etype : "common",start_time : 0,table : 1→4で10、0→3で3、その他は6},{vtype : "1",etype : "1",start_time : 0,table : 0→1で3、3→2で10、その他は6},{vtype : "3",etype : "3",start_time : 0,table : 1→4および0→3は通行不可、その他は6}]、電力で最適化とした場合に
    # 0→3→2→0および0→1→4→0となることを確認(車両別、commonあり)
    ('31_1_29', '31_1_29_request.json','5x5_cost_31_1_29.json', True),

    # 30. 車両数 : 2、配送先数 : 6(移動時間はすべて3、初期解 0→1→2→3→0および0→4→5→6→0)、
    # management_etable : [{vtype : "0",etype : "0",start_time : 0,table : 2→4で10、0→5で3、その他は4},{vtype : "1",etype : "1",start_time : 0,table : 0→2で3、5→1で10、その他は4},{vtype : "0",etype : "0",start_time : 3,table : 5→1で3、0→2で10、その他は4},{vtype : "1",etype : "1",start_time : 3,table : 2→4で3、0→5で10、その他は4}]、電力で最適化とした場合に
    # 0→5→1→3→0および0→2→4→6→0となることを確認(時間別車両別、commonとtime_commonなし)
    ('31_1_30', '31_1_30_request.json','7x7_cost_31_1_30.json', True),

    # 31. 車両数 : 2(vtype 0、0、etype 0、1)、配送先数 : 6(移動時間はすべて3、初期解 0→1→2→3→0および0→4→5→6→0)、
    # management_etable : [{vtype : "0",etype : "0",start_time : 0,table : 2→4で10、0→5で3、その他は4},{vtype : "0",etype : "1",start_time : 0,table : 0→2で3、5→1で10、その他は4},{vtype : "0",etype : "0",start_time : 3,table : 5→1で3、0→2で10、その他は4},{vtype : "0",etype : "1",start_time : 3,table : 2→4で3、0→5で10、その他は4}]、電力で最適化とした場合に
    # 0→5→1→3→0および0→2→4→6→0となることを確認(時間別車両別、commonとtime_commonなし、vtype!=etype)
    ('31_1_31', '31_1_31_request.json','7x7_cost_31_1_31.json', True),

    # 32. 車両数 : 2(vtype 0、1、etype 0、0)、配送先数 : 6(移動時間はすべて3、初期解 0→1→2→3→0および0→4→5→6→0)、
    # management_etable : [{vtype : "0",etype : "0",start_time : 0,table : 2→4で10、0→5で3、その他は4},{vtype : "1",etype : "0",start_time : 0,table : 0→2で3、5→1で10、その他は4},{vtype : "0",etype : "0",start_time : 3,table : 5→1で3、0→2で10、その他は4},{vtype : "1",etype : "0",start_time : 3,table : 2→4で3、0→5で10、その他は4}]、電力で最適化とした場合に
    # 0→5→1→3→0および0→2→4→6→0となることを確認(時間別車両別、commonとtime_commonなし、vtype!=etype)
    ('31_1_32', '31_1_32_request.json','7x7_cost_31_1_32.json', True),

    # 33. 車両数 : 2、配送先数 : 6(移動時間はすべて3、初期解 0→1→2→3→0および0→4→5→6→0)、
    # management_etable : [{vtype : "common",etype : "common",start_time : 0,table : 2→4で10、0→5で3、その他は4},{vtype : "time_common",etype : "time_common",start_time : 3,table : 2→4で3、0→5で10、その他は4},{vtype : "0",etype : "0",start_time : 3,table : 5→1で3、0→2で10、その他は4},{vtype : "0",etype : "0",start_time : 10,table : 2→4および0→5は通行不可、その他は4},{vtype : "1",etype : "1",start_time : 0,table : 0→2で3、5→1で10、その他は4},{vtype : "3",etype : "0",start_time : 3,table : 2→4および0→5は通行不可、その他は4}]、電力で最適化とした場合に
    # 0→5→1→3→0および0→2→4→6→0となることを確認(時間別車両別、commonとtime_commonあり)
    ('31_1_33', '31_1_33_request.json','7x7_cost_31_1_33.json', True),

    # 34. 車両数 : 2、配送先数 : 6(移動時間はすべて3、2台目の営業時間枠 2000/1/1 8:00:00～2000/1/1 20:00:00、初期解 0→1→2→3→0および0→4→5→6→0)、
    # management_etable : [{vtype : "common",etype : "common",start_time : 0,table : 2→4で10、0→5で3、その他は4},{vtype : "time_common",etype : "time_common",start_time : 2,table : 2→4で3、0→5で10、その他は4},{vtype : "0",etype : "0",start_time : 3,table : 5→1で3、0→2で10、その他は4},{vtype : "0",etype : "0",start_time : 10,table : 2→4および0→5は通行不可、その他は4},{vtype : "1",etype : "1",start_time : -1,table : 0→2で3、5→1で10、その他は4},{vtype : "2",etype : "0",start_time : 3,table : 2→4および0→5は通行不可、その他は4}]、電力で最適化とした場合に
    # 0→5→1→3→0および0→2→4→6→0となることを確認(時間別車両別、commonとtime_commonあり、配送時間マイナス)
    ('31_1_34', '31_1_34_request.json','7x7_cost_31_1_34.json', True),

    # 35. 車両数 : 2、配送先数 : 6(移動時間はすべて3、初期解 0→1→2→3→0および0→4→5→6→0)、
    # management_etable : [{vtype : "3",etype : "0",start_time : 3,table : 2→4および0→5は通行不可、その他は4},{vtype : "time_common",etype : "time_common",start_time : 3,table : 2→4で3、0→5で10、その他は4},{vtype : "0",etype : "0",start_time : 10,table : 2→4および0→5は通行不可、その他は4},{vtype : "common",etype : "common",start_time : 0,table : 2→4で10、0→5で3、その他は4},{vtype : "0",etype : "0",start_time : 3,table : 5→1で3、0→2で10、その他は4},{vtype : "1",etype : "1",start_time : 0,table : 0→2で3、5→1で10、その他は4}]、電力で最適化とした場合に
    # 0→5→1→3→0および0→2→4→6→0となることを確認(時間別車両別、commonとtime_commonあり、順不同)
    ('31_1_35', '31_1_35_request.json','7x7_cost_31_1_35.json', True),

    # 36. 車両数 : 1、配送先数 : 3(移動時間はすべて3、初期解 0→1→2→3→0)、
    # management_etable : [{vtype : "common",etype : "common",start_time : 0,table : 全て通行不可},{vtype : "time_common",etype : "time_common",start_time : 0,table : 1→3は通行不可、0→2で3、その他は4},{vtype : "time_common",etype : "time_common",start_time : 3,table : 1→3で3、0→2は通行不可、その他は4}]、電力で最適化とした場合に
    # 0→2→1→3→0となることを確認(通行不可)
    ('31_1_36', '31_1_36_request.json','4x4_cost_31_1_36.json', True),

    # 37. 車両数 : 2、配送先数 : 6(移動時間はすべて3、opskill=0.5、1.0、荷作業時間は4のみ2、初期解 0→1→2→3→0および0→4→5→6→0)、
    # management_etable : [{vtype : "common",etype : "common",start_time : 0,table : 2→4で10、0→5で3、その他は4},{vtype : "time_common",etype : "time_common",start_time : 3,table : 0→5で10、その他は4},{vtype : "0",etype : "0",start_time : 3,table : 5→1で3、0→2で10、その他は4},{vtype : "0",etype : "0",start_time : 10,table : 2→4および0→5は通行不可、その他は4},{vtype : "1",etype : "1",start_time : 0,table : 0→2で2、5→1で10、その他は4},{vtype : "3",etype : "0",start_time : 3,table : 2→4および0→5は通行不可、その他は4}]、電力で最適化とした場合に
    # 0→5→1→3→0(opskill=0.5)および0→2→4→6→0(opskill=1.0)となることを確認(車両ごとにテーブルが異なるためスキルシャッフルは機能しない)
    ('31_1_37', '31_1_37_request.json','7x7_cost_31_1_37.json', True),

    # 38. 車両数 : 2、配送先数 : 6(移動時間はすべて3、opskill=0.5、1.0、荷作業時間は4のみ2、初期解 0→1→2→3→0および0→4→5→6→0)、
    # management_etable : [{vtype : "time_common",etype : "time_common",start_time : 0,table : 1→3および4→6で10、0→2および0→5で3、その他は4},{vtype : "time_common",etype : "time_common",start_time : 3,table : 1→3および4→6で3、0→2および0→5で10、その他は4}]、電力で最適化とした場合に
    # 0→2→1→3→0(opskill=1.0)および0→5→4→6→0(opskill=0.5)となることを確認(同じテーブルを参照するためスキルシャッフル)
    ('31_1_38', '31_1_38_request.json','7x7_cost_31_1_38.json', True),

    # 39. 車両数 : 2、配送先数 : 6(移動時間はすべて3、opskill=0.5、1.0、荷作業時間は4のみ2、初期解 0→1→2→3→0および0→4→5→6→0)、
    # management_etable : [{vtype : "time_common",etype : "time_common",start_time : 0,table : 1→3および4→6で10、0→2および0→5で3、その他は4},{vtype : "0",etype : "0",start_time : 0,table : 1→3および4→6で10、0→2および0→5で3、その他は4},{vtype : "0",etype : "0",start_time : 3,table : 1→3および4→6で3、0→2および0→5で10、その他は4},{vtype : "1",etype : "1",start_time : 3,table : 1→3および4→6で3、0→2および0→5で10、その他は4}]、電力で最適化とした場合に
    # 0→2→1→3→0(opskill=1.0)および0→5→4→6→0(oprskill=0.5)となることを確認(別のテーブルだが中身は同じテーブルを参照するためスキルシャッフル)
    # ('31_1_39', '31_1_39_request.json','7x7_cost_31_1_39.json', True),
    #仕様変更によりテスト不可

    # 40. 車両数 : 1、配送先数 : 3(初期解 0→1→2→3→0)、
    # management_dtable : [{vtype : "time_common",start_time : 0,table : 1→3で10000、0→2で100},{vtype : "time_common",start_time : 5,table : 1→3で100}]、
    # management_ttable : [{vtype : "time_common",start_time : 0,table : 1→3で10、0→2で3、その他は4},{vtype : "time_common",start_time : 3,table : 0→2で10、その他は4}]、
    # management_etable : [{vtype : "time_common",etype : "time_common",start_time : 0,table : 1→3で10、0→2で3、その他は4},{vtype : "time_common",etype : "time_common",start_time : 2,table : 0→2で10、その他は4}]、電力で最適化とした場合に
    # 0→2→1→3→0となることを確認(dtableとttableとetableの時間枠が異なる場合)
    ('31_1_40', '31_1_40_request.json','4x4_cost_31_1_40.json', True),

    # 41. 車両数 : 2、配送先数 : 4(移動距離　1→3および2→4で1001、移動時間　1→3および2→4で2、消費電力　1→3および2→4で0)、opt_d_weight=0.2、opt_t_weight=0.2、opt_e_weight=0.6、初期解を0→1→2→0および0→3→4→0とした場合に
    # 0→1→3→0および0→2→4→0となることを確認(距離の悪化と時間の悪化の合計より消費電力の改善の方が影響が大きいため)
    ('31_1_41', '31_1_41_request.json','5x5_cost_31_1_41.json', True),

    # 42. 車両数 : 1(初期電力 5)、充電スポット数 : 1、配送先数 : 2(消費電力はすべて3、初期解 0→1→2→0)、evplan=Trueとした場合に
    # 0→e0→1→2→0(e0で8充電)となることを確認
    ('31_1_42', '31_1_42_request.json','4x4_cost_31_1_42.json', True),

    # 43. 車両数 : 1(電力容量 8、初期電力 7)、充電スポット数 : 1、配送先数 : 2(消費電力はすべて3、初期解 0→1→2→0)、evplan=Trueとした場合に
    # 0→1→e0→2→0(e0で6充電)となることを確認(電力容量を超えないよう充電)
    ('31_1_43', '31_1_43_request.json','4x4_cost_31_1_43.json', True),

    # 44. 車両数 : 1(初期電力 8)、充電スポット数 : 1、配送先数 : 2(消費電力はすべて3、距離 e0→2は100、初期解 0→1→2→0)、evplan=Trueとした場合に
    # 0→1→e0→2→0(e0で5充電)となることを確認(距離が最適になるよう充電)
    ('31_1_44', '31_1_44_request.json','4x4_cost_31_1_44.json', True),

    # 45. 車両数 : 1(電力容量 5、初期電力 5)、充電スポット数 : 1、配送先数 : 2(消費電力はすべて3、初期解 0→1→2→0)、evplan=Trueとした場合に
    # 0→e0→1→2→0(e0で3充電)となることを確認(電欠)
    ('31_1_45', '31_1_45_request.json','4x4_cost_31_1_45.json', True),

    # 46. 車両数 : 1(初期電力 5、電費 1)、充電スポット数 : 1、配送先数 : 2(距離 全て3000、初期解 0→1→2→0)、evplan=True、電力テーブルなしとした場合に
    # 0→e0→1→2→0(e0で8充電)となることを確認(電力テーブルがない場合は電費で計算)
    ('31_1_46', '31_1_46_request.json','4x4_cost_31_1_46.json', True),

    # 47. 車両数 : 1(電力容量 8、初期電力 7)、充電スポット数 : 1(配送時間枠 2000/1/1 9:00:00～2000/1/1 11:00:00)、配送先数 : 2(消費電力はすべて3、初期解 0→1→2→0)、evplan=Trueとした場合に
    # 0→e0→1→2→0(e0で4充電)となることを確認(配送時間枠、電欠)
    ('31_1_47', '31_1_47_request.json','4x4_cost_31_1_47.json', True),

    # 48. 車両数 : 1(初期電力 8)、充電スポット数 : 1(配送時間枠 2000/1/1 11:00:00～2000/1/1 12:00:00)、配送先数 : 2(消費電力はすべて3、初期解 0→1→2→0)、evplan=Trueとした場合に
    # 0→1→e0→2→0(e0で5充電)となることを確認(配送時間枠)
    ('31_1_48', '31_1_48_request.json','4x4_cost_31_1_48.json', True),

    # 49. 車両数 : 1(初期電力 8)、充電スポット数 : 1(荷作業時間 60)、配送先数 : 2(消費電力はすべて3、配送時間枠 2000/1/1 9:00:00～2000/1/1 11:30:00、2000/1/1 9:00:00～2000/1/1 17:00:00、初期解 0→1→2→0)、evplan=Trueとした場合に
    # 0→1→e0→2→0(e0で5充電)となることを確認(荷作業時間)
    ('31_1_49', '31_1_49_request.json','4x4_cost_31_1_49.json', True),

    # 50. 車両数 : 1(初期電力 8)、充電スポット数 : 1(充電レート 8)、配送先数 : 2(消費電力はすべて3、配送時間枠 2000/1/1 9:00:00～2000/1/1 11:30:00、2000/1/1 9:00:00～2000/1/1 17:00:00、初期解 0→1→2→0)、evplan=Trueとした場合に
    # 0→1→e0→2→0(e0で5充電)となることを確認(充電レート)
    ('31_1_50', '31_1_50_request.json','4x4_cost_31_1_50.json', True),

    # 51. 車両数 : 1(初期電力 5)、充電スポット数 : 2、配送先数 : 2(電力 0→e1は2、消費電力はすべて3、初期解 0→1→2→0)、evplan=Trueとした場合に
    # 0→e1→1→2→0(e1で7充電)となることを確認(複数充電スポット、電力が最適になるよう充電)
    ('31_1_51', '31_1_51_request.json','5x5_cost_31_1_51.json', True),

    # 52. 車両数 : 1(電力容量 8、初期電力 7)、充電スポット数 : 2(配送時間枠 2000/1/1 9:00:00～2000/1/1 11:00:00、2000/1/1 10:00:00～2000/1/1 12:00:00)、配送先数 : 2(消費電力はすべて3、初期解 0→1→2→0)、evplan=Trueとした場合に
    # 0→1→e1→2→0(e1で6充電)となることを確認(複数充電スポット、配送時間枠)
    ('31_1_52', '31_1_52_request.json','5x5_cost_31_1_52.json', True),

    # 53. 車両数 : 2(電力容量 7、100、初期電力 5、100)、充電スポット数 : 1、配送先数 : 4(消費電力はすべて3、初期解 0→1→2→0および0→3→4→0)、evplan=Trueとした場合に
    # 0→e0→1→0および0→2→3→4→0(1、2、3、4は順不同、1台目はe0で5充電)となることを確認(電欠となる場合は他の車両に配送先を移動)
    ('31_1_53', '31_1_53_request.json','6x6_cost_31_1_53.json', True),

    # 54. 車両数 : 2(電力容量 7、100、初期電力 5、5)、充電スポット数 : 1、配送先数 : 4(消費電力はすべて3、初期解 0→1→2→0および0→3→4→0)、evplan=Trueとした場合に
    # 0→e0→1→0および0→e0→2→3→4→0(1、2は交換可能、2、3、4は順不同、1台目はe0で5充電、2台目はe0で11充電)となることを確認(電欠となる場合は他の車両に配送先を移動、充電スポットあり)
    ('31_1_54', '31_1_54_request.json','6x6_cost_31_1_54.json', True),

    # 55. 車両数 : 1(初期電力 8、営業時間枠 2000/1/1 9:00:00～2000/1/1 13:00:00)、充電スポット数 : 2(充電レート 4、8)、配送先数 : 2(消費電力はすべて3、配送時間枠 2000/1/1 9:00:00～2000/1/1 11:30:00、2000/1/1 9:00:00～2000/1/1 17:00:00、初期解 0→1→2→0)、evplan=Trueとした場合に
    # 0→1→e1→2→0(e1で5充電)となることを確認(車両営業時間枠)
    ('31_1_55', '31_1_55_request.json','5x5_cost_31_1_55.json', True),

    # 56. 車両数 : 2(電力容量 7、100、初期電力 5、100)、充電スポット数 : 1、配送先数 : 4(消費電力はすべて3、荷物量(第1および第2) 100、200、100、100、初期解 0→1→2→0および0→3→4→0)、evplan=Trueとした場合に
    # 0→e0→2→0および0→1→3→4→0(1、3、4は順不同、1台目はe0で5充電)となることを確認(電欠となる場合は他の車両に配送先を移動、積載量(第1および第2))
    ('31_1_56', '31_1_56_request.json','6x6_cost_31_1_56.json', True),

    # 57. 車両数 : 2(初期電力 9、5)、充電スポット数 : 1(充電レート 8、配送時間枠 2000/1/1 9:00:00～2000/1/1 11:00:00
    # )、配送先数 : 4(移動時間 4→3は0.5、消費電力はすべて3、1番目と2番目の配送先に2台目の車両は立寄不可、3番目と4番目の配送先に1台目の車両は立寄不可、初期解 0→1→2→0、0→3→4→0)、evplan=True、avetime_ape=0.0とした場合に
    # 0→e0→1→2→0および0→e0→4→3→0(1台目はe0で4充電、2台目はe0で8充電)となることを確認(時間のAPE)
    ('31_1_57', '31_1_57_request.json','6x6_cost_31_1_57.json', True),

    # 58. 車両数 : 1(初期電力 5)、充電スポット数 : 1、配送先数 : 2(消費電力はすべて3、距離 e0→1は100、時間e0→1は2、初期解 0→1→2→0)、evplan=True、時間最適化とした場合に
    # 0→e0→2→1→0(e0で8充電)となることを確認(時間が最適になるよう充電)
    ('31_1_58', '31_1_58_request.json','4x4_cost_31_1_58.json', True),

    # 59. 車両数 : 1(初期電力 8)、充電スポット数 : 1、配送先数 : 2(消費電力はすべて3、0→e0は通行不可、初期解 0→1→2→0)、evplan=Trueとした場合に
    # 0→1→e0→2→0(e0で5充電)となることを確認(通行不可)
    ('31_1_59', '31_1_59_request.json','4x4_cost_31_1_59.json', True),

    # 60. 車両数 : 2(電力容量 7、100、初期電力 5、100)、充電スポット数 : 1、配送先数 : 4(消費電力はすべて3、2→3および4→2は通行不可、初期解 0→1→2→0および0→3→4→0)、evplan=Trueとした場合に
    # 0→e0→1→0および0→3→2→4→0(1、2は交換可能、1台目はe0で5充電)となることを確認(電欠となる場合は他の車両に配送先を移動、通行不可)
    ('31_1_60', '31_1_60_request.json','6x6_cost_31_1_60.json', True),

    # 61. 車両数 : 1(初期電力 5)、充電スポット数 : 1、配送先数 : 2(消費電力はすべて3、初期解 0→1→2→0)、evplan=True、
    # management_dtable : [{vtype : "0",start_time : 0,table : e0→2で10000},{vtype : "0",start_time : 1,table : e0→2で100},{vtype : "1",start_time : 0,table : e0→2で10000}]とした場合に
    # 0→e0→2→1→0(e0で8充電)となることを確認(時間別車両別距離テーブル)
    ('31_1_61', '31_1_61_request.json','4x4_cost_31_1_61.json', True),

    # 62. 車両数 : 1(初期電力 5)、充電スポット数 : 2、配送先数 : 2(消費電力はすべて3、初期解 0→1→2→0)、evplan=True、
    # management_ttable : [{vtype : "0",start_time : 0,table : 0→e0で20},{vtype : "0",start_time : 1,table : 0→e1および1→e1および1→e0で20},{vtype : "1",start_time : 0,table : 0→e1および1→e1および1→e0で20}]とした場合に
    # 0→e1→1→2→0(e1で8充電)となることを確認(時間別車両別時間テーブル)
    ('31_1_62', '31_1_62_request.json','5x5_cost_31_1_62.json', True),

    # 63. 車両数 : 1(初期電力 8)、充電スポット数 : 2、配送先数 : 2(消費電力はすべて3、e0に1台目の車両は立寄不可、初期解 0→1→2→0)、evplan=Trueとした場合に
    # 0→e1→1→2→0(e1で8充電)となることを確認(立寄不可)
    ('31_1_63', '31_1_63_request.json','5x5_cost_31_1_63.json', True),

    # 64. 車両数 : 2(電力容量 7、100、初期電力 5、100)、充電スポット数 : 1、配送先数 : 4(消費電力はすべて3、2番目の配送先に2台目の車両は立寄不可、初期解 0→1→2→0および0→3→4→0)、evplan=Trueとした場合に
    # 0→e0→2→0および0→1→3→4→0(1、3、4は順不同、1台目はe0で5充電)となることを確認(電欠となる場合は他の車両に配送先を移動、立寄不可)
    ('31_1_64', '31_1_64_request.json','6x6_cost_31_1_64.json', True),

    # 65. 車両数 : 2(電力容量 7、7、初期電力 5、5)、充電スポット数 : 1、配送先数 : 2(消費電力はすべて3、初期解 0→1→0および0→2→0)、evplan=True、台数削減モードとした場合に
    # 0→e0→1→2→0となることを確認(e0で5充電、電欠、台数削減モード)
    ('31_1_65', '31_1_65_request.json','4x4_cost_31_1_65.json', True),

    # 66. 車両数 : 1(初期電力 5)、充電スポット数 : 1(配送時間枠 2000/1/1 11:00:00～2000/1/1 11:00:00)、配送先数 : 2(消費電力はすべて3、初期解 0→1→2→0)、evplan=True、出発遅延モードとした場合に
    # 0→e0→1→2→0(e0で8充電、拠点の出発時刻は2000/1/1/10:00:00)となることを確認(出発遅延)
    ('31_1_66', '31_1_66_request.json','4x4_cost_31_1_66.json', True),

    # # 67. 初期解生成モード、evplan=True、打ち切り時間 3秒(電力解計算中)とした場合に
    # # 配送計画出力要件およびハード制約を守る解(充電初期解より良く、打ち切り時間なしより悪い)が3秒で得られることを確認
    # ('31_1_67', '31_1_67_request.json','29x29_cost_31_1_67.json', True),

    # 68. 車両数 : 1(初期電力 5)、充電スポット数 : 2、配送先数 : 2(配送時間 0→e1は2、消費電力はすべて3、初期解 0→1→2→0)、evplan=True、
    # management_dtable : [{vtype : "0",start_time : 0,table : 2→0で10000、2→1は通行不可},{vtype : "0",start_time : 4,table : 2→0で100}]、巡回モードとした場合に
    # 0→e1→1→2→0(e1で8充電)となることを確認(巡回モード)
    ('31_1_68', '31_1_68_request.json','5x5_cost_31_1_68.json', True),

    # 69. 車両数 : 1(初期電力 5)、充電スポット数 : 1、配送先数 : 2(SPOTID 1、1、荷物量 150、150、消費電力はすべて3)、初期解生成モード、evplan=Trueとした場合に
    # 0→e0→1→2→0(直送便、e0で5充電)となることを確認(直送便は単体で電力解を作成)
    ('31_1_69', '31_1_69_request.json','3x3_cost_31_1_69.json', True),

    # 70. 車両数 : 2(電力容量 100、7、初期電力 100、5)、充電スポット数 : 1、配送先数 : 4(SPOTID 1、1、2、3、荷物量 150、150、1、1、消費電力はすべて3)、初期解生成モード、evplan=Trueとした場合に
    # 0→1→2→0(直送便)および0→e0→3→4→0(3、4は順不同、2台目はe0で5充電、2台目は電欠)となることを確認(直送便は電力解の対象外)
    ('31_1_70', '31_1_70_request.json','5x5_cost_31_1_70.json', True),

    # 71. 車両数 : 1(初期電力 5、訪問数上限 2)、充電スポット数 : 1、配送先数 : 2(消費電力はすべて3、初期解 0→1→2→0)、evplan=Trueとした場合に
    # 0→e0→1→2→0(e0で8充電)となることを確認(訪問数上限、充電スポットはカウントしない)
    ('31_1_71', '31_1_71_request.json','4x4_cost_31_1_71.json', True),

    # 72. 車両数 : 2(電力容量 7、100、初期電力 5、100、訪問数上限 0、2)、充電スポット数 : 1、配送先数 : 4(消費電力はすべて3、初期解 0→1→2→0および0→3→4→0)、evplan=Trueとした場合に
    # 0→e0→1→2→0および0→3→4→0(1台目はe0で5充電、1台目は電欠)となることを確認(訪問数上限)
    ('31_1_72', '31_1_72_request.json','6x6_cost_31_1_72.json', True),

    # 73. 車両数 : 2(電力容量 7、100、初期電力 5、100)、充電スポット数 : 1、配送先数 : 4(消費電力はすべて3、荷物量(第1および第2) 100(集荷)、200(集荷)、100(集荷)、100(集荷)、初期解 0→1→2→0および0→3→4→0)、evplan=Trueとした場合に
    # 0→e0→2→0および0→1→3→4→0(1、3、4は順不同、1台目はe0で5充電)となることを確認(電欠となる場合は他の車両に配送先を移動、集荷(第1および第2))
    ('31_1_73', '31_1_73_request.json','6x6_cost_31_1_73.json', True),

    # 74. 車両数 : 1(初期電力 5)、充電スポット数 : 1、配送先数 : 2(SPOTID 1、1、消費電力はすべて3)、初期解生成モード、大量輸送モード、evplan=Trueとした場合に
    # 0→e0→1→2→0(1と2はtie-up、e0で5充電)となることを確認(大量輸送モード)
    ('31_1_74', '31_1_74_request.json','3x3_cost_31_1_74.json', True),

    # 75. 車両数 : 1(初期電力 5)、充電スポット数 : 1、配送先数 : 2(消費電力はすべて3、配送時間枠 2000/1/1 9:00:00～2000/1/1 11:00:00、2000/1/1 10:00:00～2000/1/1 12:00:00、初期解 0→1→2→0)、evplan=Trueとした場合に
    # 0→1→2→0となることを確認(配送時間枠違反)
    ('31_1_75', '31_1_75_request.json','4x4_cost_31_1_75.json', True),

    # 76. 車両数 : 1(初期電力 5)、充電スポット数 : 1、配送先数 : 2(消費電力はすべて3、配送時間枠 2000/1/1 8:00:00～2000/1/1 9:00:00、2000/1/1 9:00:00～2000/1/1 10:00:00、初期解 0→1→2→0)、evplan=Trueとした場合に
    # 0→1→2→0となることを確認(配送時間枠違反、電欠)
    ('31_1_76', '31_1_76_request.json','4x4_cost_31_1_76.json', True),

    # 77. 車両数 : 1(初期電力 5)、充電スポット数 : 1(配送時間枠 2000/1/1 11:00:00～2000/1/1 11:00:00)、配送先数 : 2(初期解 0→1→2→0)、evplan=True、
    # management_etable : [{vtype : "0",etype : "0",start_time : 0,table : 全て3},{vtype : "0",etype : "0",start_time : 1,table : 全て5}],{vtype : "0",etype : "0",start_time : 2,table : 全て3}]、出発遅延モードとした場合に
    # 0→e0→2→1→0(e0で8充電、拠点の出発時刻は2000/1/1/9:00:00)となることを確認(出発遅延後に電欠となる場合は遅延しない)
    ('31_1_77', '31_1_77_request.json','4x4_cost_31_1_77.json', True),

    # 78. 車両数 : 1(初期電力 5)、充電スポット数 : 1(配送時間枠 2000/1/1 11:00:00～2000/1/1 11:00:00)、配送先数 : 2(初期解 0→1→2→0)、evplan=True、
    # management_etable : [{vtype : "0",etype : "0",start_time : 0,table : 全て3},{vtype : "0",etype : "0",start_time : 1,table : 全て4}],{vtype : "0",etype : "0",start_time : 2,table : 全て4}]、出発遅延モードとした場合に
    # 0→e0→1→2→0(e0で9充電、拠点の出発時刻は2000/1/1/10:00:00)となることを確認(遅延前から電欠の場合は場合は遅延する)
    ('31_1_78', '31_1_78_request.json','4x4_cost_31_1_78.json', True),

]
