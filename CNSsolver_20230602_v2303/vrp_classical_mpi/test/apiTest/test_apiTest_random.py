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
import test_apiTest_result as result
import test_apiTest_patern_1_1
import test_apiTest_patern_1_2
import test_apiTest_patern_2
import test_apiTest_patern_3
import test_apiTest_patern_4
import test_apiTest_patern_5
import test_apiTest_patern_6

unique_no = 0 # テスト固有番号(テストの乱数シードを変更したい場合この値を任意の整数へ変更)

path = 'test/apiTest/test_data/'
cost_dir = './'

testno = ""

if testno == "":
	sample_num = 10000 #テスト数
	TimeOut = 60 * 15 #最大打ち切り時間[秒](フリーズ回避)
else:
	sample_num = 1 #テスト数
	TimeOut = 1000000 #最大打ち切り時間[秒](フリーズ回避)
	#TimeOut = 60 * 15 #最大打ち切り時間[秒](フリーズ回避)

# Args
avetime_ape_rate = 0.1 #平準化オプション(avetime_ape)が指定される確率
avetime_ae_rate = 0.1 #平準化オプション(avetime_ae)が指定される確率
max_avetime_ape = 10 #最大avetime_ape
max_avetime_ae = 10 #最大avetime_ae
avevisits_ape_rate = 0.1 #平準化オプション(avevisits_ape)が指定される確率
avevisits_ae_rate = 0.1 #平準化オプション(avevisits_ae)が指定される確率
max_avevisits_ape = 10 #最大avevisits_ape
max_avevisits_ae = 10 #最大avevisits_ae
avedist_ape_rate = 0.1 #平準化オプション(avedist_ape)が指定される確率
avedist_ae_rate = 0.1 #平準化オプション(avedist_ae)が指定される確率
max_avedist_ape = 10 #最大avedist_ape
max_avedist_ae = 10 #最大avedist_ae
decr_vnum_rate = 0.1 #車両台数削減モードになる確率
evplan_rate = 0.1 #充電解作成モードになる確率
lastc_flag_rate = 0.5 #非巡回モードになる確率
ltime_utime_flag_rate = 0.2 #ltime, utime指定になる確率
max_ltime_utime = 10 #最大ltime, utime
skill_rate = 0.5 #スキル設定モードになる確率
timeout_rate = 0.2 #打ち切り時間指定になる確率
max_timeout = 60 #最大打ち切り時間[秒](打ち切り時間指定)
rotation_rate = 0.1 #回転機能が使用される確率
max_parallel_num = 4 #最大並列数
max_timePenaltyWeight = 1e9 #最大時間違反量重み
max_loadPenaltyWeight = 1e9 #最大積載量違反量重み
max_load2PenaltyWeight = 1e9 #最大第2積載量違反量重み
max_timeLevelingPenaltyWeight = 1e9 #最大時間ばらつき違反量重み
max_visitsLevelingPenaltyWeight = 1e9 #最大件数ばらつき違反量重み
max_distLevelingPenaltyWeight = 1e9 #最大距離ばらつき違反量重み
max_sparePenaltyWeight =1e12 #最大未割り当て荷物最適化違反重み
timePenaltyType_hard_rate = 0.2 #時間違反がハード制約になる確率
loadPenaltyType_hard_rate = 0.2 #積載量違反がハード制約になる確率
load2PenaltyType_hard_rate = 0.2 #第2積載量違反がハード制約になる確率
timeLevelingPenaltyType_hard_rate = 0.2 #時間ばらつき違反がハード制約になる確率
visitsLevelingPenaltyType_hard_rate = 0.2 #件数ばらつき違反がハード制約になる確率
distLevelingPenaltyType_hard_rate = 0.2 #距離ばらつき違反がハード制約になる確率
delaystart_rate = 0.1 #出発遅延モードになる確率
delaystart2_rate = 0.1 #遅延最適化モードになる確率
addtw_rate = 0.2 #飛び地が設定される確率
max_addtw_num = 5 #配送先ごとに設定される飛び地の最大数
autoloaddivide_rate = 0.2 #荷物自動分割を使用する確率
init_changecust_rate = 0.5 #割当荷物で条件2を使用する確率
# 大量輸送モード
bulkshipping_rate = 0.2 #大量輸送モードになる確率
bulkshipping_generate_rate = 0.5 #大量輸送モードにおいてTiedCustomerとなり得る解を生成する確率
max_TiedCustomer = 10 #大量輸送モードにおいて同じspotidとなる最大ID数
max_same_id_customers_bulk = 10 #大量輸送モードにおいて同じspotidとなる最大配送先数
# 直送便
direct_rate = 0.2 #直送便となり得る解を生成する確率
direct_generate_rate = 0.5 #直送便になりやすい解を生成する確率
max_direct_customers = 10 #直送便において同じspotidとなる最大ID数（直送便の数）
max_same_id_customers_direct = 10 #直送便において同じspotidとなる最大配送先数（直送便内の配送先数）
# 休憩機能
max_break_interval = 480
max_break_time = 120
max_break_interval_rate = 0.7 #0になる確率
max_break_time_rate = 0.7 #0になる確率
max_takebreak_rate = 0.8 #0になる確率
#未割り当て荷物最適化
max_opt_unassigned_flag_rate = 0.8
max_deny_unass_rate = 0.7 #Falseになる確率
max_reserved_v_flag_rate = 0.7 #Falseになる確率


# NormalCustomer
max_customer_num = 20 #最大配送先数
max_custno = 100 #最大顧客番号
max_spotid = 100 #最大スポットID
max_dem = 300.0 #最大荷物量
max_dem2 = 300.0 #最大第2荷物量
dem_rate = 0.5 #集荷になる確率
max_servt = 60.0 #最大荷作業時間
reject_ve_rate = 0.01 #立寄不可車両が選ばれる確率
max_parkt_open = 60.0 #最大駐車時間
max_parkt_arrive = 60.0 #最大駐車時間
max_cust_depotservt = 60 #最大拠点作業時間
max_loaddivideunit = 10 #最大荷物分割単位

# takebreak
max_takebreak_num = 5 #最大配送先数
max_takebreak_spotno_rate = 0.5 #spotid=""の発生確率

# Vehicles
max_vehicle_num = 10 #最大車両数
max_vehno = 100 #最大車両番号
max_vtype = 10 #最大車両種別数 vtype
max_etype = 10 #最大車両種別数 etype
max_cap = 500.0 #最大積載量
max_cap2 = 500.0 #最大第2積載量
max_visit = 100 #最大訪問数
max_drskill = 10 #最大drskill
max_opskill = 10 #最大opskill
max_rotate = 10
max_veh_depotservt = 60 #最大拠点作業時間
# evehicle_dict
max_ecapacity = 100.0 #最大電力容量
min_ecapacity = 1.0 #最小電力容量
min_einitial = 1.0 #最小初期電力量
max_ecost = 10.0 #最大電費
min_ecost = 1.0 #最小電費
min_emargin = 1.0 #最小残電力マージン

# ESpots
max_espot_num = 9 # 最大充電スポット数
min_espot_num = 1 # 最小スポット数
max_erate = 1e12 # 1時間に充電できる最大充電量
min_erate = 1 # 1時間に充電できる最小充電量

# InitialRoutes
init_read_rate = 0.5 #初期解読み込みモードになる確率

# Table
min_starttime = -24.0 #テーブル参照の際の相対時刻の最小値
max_starttime = 24.0 #テーブル参照の際の相対時刻の最大値
impassable_rate = 0.01 #通行不可になる確率
# dtable
max_dtable_num = 10 #最大距離テーブル数
max_dist = 10000.0 #最大移動距離
# ttable
max_ttable_num = 10 #最大時間テーブル数
max_time = 10.0 #最大移動時間
# etable
etable_rate = 0.3 #電力テーブルを作成する確率
max_etable_num = 10 #最大電力テーブル数
max_e = 10.0 #最大電力
# xtable
xtable_rate = 0.3 #経験コストテーブルを作成する確率
max_xtable_num = 10 #最大経験コストテーブル数
max_x = 10000.0 #最大経験コスト

if testno != "":
	test_args=[("random_"+testno,"request_random_"+testno+".json","cost_random_"+testno+".json",True)]
else:
	test_args=[('random_'+str(i), 'request_random_'+str(i) + '.json','cost_random_'+str(i) + '.json', True) for i in range(1,sample_num+1)]

#-----------------------------------------------------------------
# ランダムテストパターン作成
#-----------------------------------------------------------------
def Create_Random_Test(test_no, request_file, cost_file, logon_flag):
	random.seed(int(re.sub(r"\D","",test_no)) + sample_num * unique_no)

	print(sys._getframe().f_code.co_name + ' : ' + test_no)
	oParamJSON = create_JSON('request_base.json', 'base_cost.json')
	oParamJSON = json.dumps(oParamJSON)
	oParamJSON = json.loads(oParamJSON)
	oRequestJSON = oParamJSON["requestJSON"]
	oCostJSON = oParamJSON["costJSON"]

	# 充電スポット追加
	oRequestJSON['ESpots'] = copy.deepcopy(oRequestJSON['NormalCustomer'])

	args = oRequestJSON['Args']
	v = oRequestJSON['Vehicles'][0]
	es = oRequestJSON['EndSpots'][0]
	c = oRequestJSON['NormalCustomer'][0]
	e = oRequestJSON['ESpots'][0]
	dtable = oCostJSON['management_dtable'][0]
	ttable = oCostJSON['management_ttable'][0]

	vnum = random.randint(1,max_vehicle_num) #車両数
	oRequestJSON['Args']["ivnum"] = vnum
	cnum = random.randint(vnum,max_customer_num) #配送先数(拠点除く)
	enum = random.randint(min_espot_num,max_espot_num) #充電スポット数

	addtw_num_list = [random.randint(0,max_addtw_num) if random.uniform(0.0,1.0) < addtw_rate else 0 for i in range(cnum)]
	addtw_num = sum(addtw_num_list)

	if random.uniform(0.0,1.0) < autoloaddivide_rate:
		loaddiivideunit_list = [random.randint(0,max_loaddivideunit) for i in range(cnum)]
	else:
		loaddiivideunit_list = [1 for i in range(cnum)]

	if random.uniform(0.0,1.0) < init_read_rate:
		init_read = True #初期解読み込み
	else:
		init_read = False #初期解生成

	if random.uniform(0.0,1.0) < avetime_ape_rate and random.uniform(0.0,1.0) < avetime_ae_rate:
		args["avetime_ape_flag"] = False #平準化オプション指定なし
		args["avetime_ae_flag"] = False #平準化オプション指定なし
	elif random.uniform(0.0,1.0) < avetime_ape_rate:
		args["avetime_ape_flag"] = True #平準化オプション指定あり
		args["avetime_ape"] = random.uniform(0.0,max_avetime_ape)
	elif random.uniform(0.0,1.0) < avetime_ae_rate:
		args["avetime_ape_flag"] = True #平準化オプション指定あり
		args["avetime_ape"] = random.uniform(0.0,max_avetime_ae)
	else:
		args["avetime_ape_flag"] = False #平準化オプション指定なし
		args["avetime_ae_flag"] = False #平準化オプション指定なし

	if random.uniform(0.0,1.0) < avevisits_ape_rate and random.uniform(0.0,1.0) < avevisits_ae_rate:
		args["avevisits_ape_flag"] = False #平準化オプション指定なし
		args["avevisits_ae_flag"] = False #平準化オプション指定なし
	elif random.uniform(0.0,1.0) < avetime_ape_rate:
		args["avevisits_ape_flag"] = True #平準化オプション指定なし
		args["avevisits_ape"] = random.uniform(0.0,max_avetime_ape)
	elif random.uniform(0.0,1.0) < avetime_ae_rate:
		args["avevisits_ae_flag"] = True #平準化オプション指定なし
		args["avevisits_ae"] = random.randint(0.0,max_avetime_ae)
	else:
		args["avevisits_ape_flag"] = False #平準化オプション指定なし
		args["avevisits_ae_flag"] = False #平準化オプション指定なし

	if random.uniform(0.0,1.0) < avedist_ape_rate and random.uniform(0.0,1.0) < avedist_ae_rate:
		args["avedist_ape_flag"] = False #平準化オプション指定なし
		args["avedist_ae_flag"] = False #平準化オプション指定なし
	elif random.uniform(0.0,1.0) < avedist_ape_rate:
		args["avedist_ape_flag"] = True #平準化オプション指定あり
		args["avedist_ape"] = random.uniform(0.0,max_avedist_ape)
	elif random.uniform(0.0,1.0) < avedist_ae_rate:
		args["avedist_ape_flag"] = True #平準化オプション指定あり
		args["avedist_ape"] = random.uniform(0.0,max_avedist_ae)
	else:
		args["avedist_ape_flag"] = False #平準化オプション指定なし
		args["avedist_ae_flag"] = False #平準化オプション指定なし

	if random.uniform(0.0,1.0) < decr_vnum_rate:
		args["decr_vnum"] = True #車両台数削減モード
	else:
		args["decr_vnum"] = False #車両台数固定モード

	if random.uniform(0.0,1.0) < lastc_flag_rate:
		args["lastc_flag"] = True #非巡回モード
	else:
		args["lastc_flag"] = False #巡回モード

	if random.uniform(0.0,1.0) < ltime_utime_flag_rate:
		args["lowertime"] = random.uniform(0.0, max_ltime_utime) #ltime指定あり
		args["uppertime"] = random.uniform(args["lowertime"], max_ltime_utime) #utime指定あり
	else:
		args["lowertime"] = 0.0 #ltime指定なし
		args["uppertime"] = 0.0 #utime指定なし

	if random.uniform(0.0,1.0) < evplan_rate:
		args["evplan"] = True #充電解作成
	else:
		args["evplan"] = False #充電解作成なし

	if random.uniform(0.0,1.0) < max_break_interval_rate:
		args["break_interval"] = 0.0 #休憩間隔なし
	else:
		args["break_interval"] = random.uniform(0.0,max_break_interval) #休憩間隔指定有り

	if random.uniform(0.0,1.0) < max_break_time_rate:
		args["break_time"] = 0.0 #休憩時間なし
	else:
		args["break_time"] = random.uniform(0.0,max_break_time) #休憩時間指定有り

	if random.uniform(0.0,1.0) < max_opt_unassigned_flag_rate:
		args["opt_unassigned_flag"] = False #未割り当て荷物最適化無し
	else:
		args["opt_unassigned_flag"] = True #未割り当て荷物最適化有り

	d_rate = random.uniform(0.0,1.0)
	t_rate = random.uniform(0.0,1.0)
	e_rate = random.uniform(0.0,1.0)
	x_rate = random.uniform(0.0,1.0)
	args["opt_d_weight"] = d_rate/(d_rate + t_rate + e_rate + x_rate)
	args["opt_t_weight"] = t_rate/(d_rate + t_rate + e_rate + x_rate)
	args["opt_e_weight"] = e_rate/(d_rate + t_rate + e_rate + x_rate)
	args["opt_x_weight"] = x_rate/(d_rate + t_rate + e_rate + x_rate)

	if random.uniform(0.0,1.0) < skill_rate:
		args["skillfile"] = "dummy" #スキル設定
		args["skillflag"] = 1
	else:
		args["skillfile"] = "None" #スキル非設定
		args["skillflag"] = 0

	timeout_flag = False
	if random.uniform(0.0,1.0) < timeout_rate:
		args["timeout"] = random.uniform(0.0,max_timeout) # 打ち切り時間指定
		timeout_flag = True
	else:
		args["timeout"] = TimeOut # フリーズ回避のための打ち切り時間

	if random.uniform(0.0,1.0) < rotation_rate:
		args["multitrip_flag"] = True #回転機能使用
	else:
		args["multitrip_flag"] = False #回転機能不使用

	args["multithread_num"] = random.randint(1,max_parallel_num) #並列数

	args["timePenaltyWeight"] = random.uniform(0.0,max_timePenaltyWeight) #時間違反量重み
	args["loadPenaltyWeight"] = random.uniform(0.0,max_loadPenaltyWeight) #積載量違反量重み
	args["load2PenaltyWeight"] = random.uniform(0.0,max_load2PenaltyWeight) #第2積載量違反量重み
	args["timeLevelingPenaltyWeight"] = random.uniform(0.0,max_timeLevelingPenaltyWeight) #時間ばらつき違反量重み
	args["visitsLevelingPenaltyWeight"] = random.uniform(0.0,max_visitsLevelingPenaltyWeight) #件数ばらつき違反量重み
	args["distLevelingPenaltyWeight"] = random.uniform(0.0,max_distLevelingPenaltyWeight) #距離ばらつき違反量重み
	args["sparePenaltyWeight"] = random.uniform(0.0,max_sparePenaltyWeight) #最大未割り当て荷物最適化違反重み

	if random.uniform(0.0,1.0) < timePenaltyType_hard_rate:
		args["timePenaltyType"] = "hard" #時間違反量ハード制約
	else:
		args["timePenaltyType"] = "soft"
	if random.uniform(0.0,1.0) < loadPenaltyType_hard_rate:
		args["loadPenaltyType"] = "hard" #積載量違反量ハード制約
	else:
		args["loadPenaltyType"] = "soft"
	if random.uniform(0.0,1.0) < load2PenaltyType_hard_rate:
		args["load2PenaltyType"] = "hard" #第2積載量違反量ハード制約
	else:
		args["load2PenaltyType"] = "soft"
	if random.uniform(0.0,1.0) < timeLevelingPenaltyType_hard_rate:
		args["timeLevelingPenaltyType"] = "hard" #時間ばらつき違反量ハード制約
	else:
		args["timeLevelingPenaltyType"] = "soft"
	if random.uniform(0.0,1.0) < visitsLevelingPenaltyType_hard_rate:
		args["visitsLevelingPenaltyType"] = "hard" #件数ばらつき違反量ハード制約
	else:
		args["visitsLevelingPenaltyType"] = "soft"
	if random.uniform(0.0,1.0) < distLevelingPenaltyType_hard_rate:
		args["distLevelingPenaltyType"] = "hard" #距離ばらつき違反量ハード制約
	else:
		args["distLevelingPenaltyType"] = "soft"

	if random.uniform(0.0,1.0) < delaystart_rate:
		args["delaystart_flag"] = True #出発遅延モード使用
	else:
		args["delaystart_flag"] = False #出発遅延モード不使用

	if random.uniform(0.0,1.0) < delaystart2_rate:
		args["delaystart2_flag"] = True #遅延最適化モード使用
	else:
		args["delaystart2_flag"] = False #遅延最適化モード不使用

	if random.uniform(0.0,1.0) < init_changecust_rate:
		args["init_changecust"] = "additive" #条件2
	else:
		args["init_changecust"] = "free" #条件1

	#メンバ変数用リスト(vehicle)
	vehno_list = random.sample(range(max_vehno+1),vnum)
	cap_list = [random.uniform(0.0,max_cap) for i in range(vnum)]
	cap2_list = [random.uniform(0.0,max_cap) for i in range(vnum)]
	vtype_list = random.sample([str(type) for type in range(max_vtype+1)] + ["common"],vnum)
	etype_list = random.sample([str(type) for type in range(max_etype+1)] + ["common"],vnum)
	visit_list = random.sample(range(max_visit),vnum)
	maxrotate_list = random.sample(range(max_rotate+1),vnum)
	veh_depotservt_list = [random.uniform(0.0,max_veh_depotservt) for i in range(vnum)]
	rmpriority_list = [random.randint(0,2) for i in range(vnum)]
	if random.uniform(0.0,1.0) < max_takebreak_rate:
		#takebreakなし
		bnum_list = []
		bnum = 0
		b_addtw_num_list = []
		b_addtw_num = 0
	else:
		#takebreak指定有り
		bnum_list = [random.randint(0,max_takebreak_num) for v in range(vnum)]
		bnum = sum(bnum_list)
		b_addtw_num_list = [random.randint(0,max_addtw_num) if random.uniform(0.0,1.0) < addtw_rate else 0 for i in range(bnum)]
		b_addtw_num = sum(b_addtw_num_list)
	reserved_v_flag_list = [True if random.uniform(0.0,1.0) > max_reserved_v_flag_rate else False for i in range(cnum + vnum)]

	# evihicle_dict
	ecapacity_list = [random.uniform(min_ecapacity,max_ecapacity) for i in range(vnum)]
	einitial_list = [random.uniform(min_einitial,ecapacity_list[i]) for i in range(vnum)]
	ecost_list = [random.uniform(min_ecost,max_ecost) for i in range(vnum)]
	emargin_list = [random.uniform(min_emargin,min(ecapacity_list[i],einitial_list[i])) for i in range(vnum)]
	# ecapacity_list = [100.0 for i in range(vnum)]
	# emargin_list = [1.0 for i in range(vnum)]
	# einitial_list = [10.0 for i in range(vnum)]
	# ecost_list = [random.uniform(min_ecost,max_ecost) for i in range(vnum)]


	startop_list,endop_list = [],[]
	for i in range(vnum):
		h1,h2 = str(random.randint(0,23)),str(random.randint(0,23))
		if int(h1) > int(h2):
			h1,h2 = h2,h1
		m1,m2 = str(random.randint(0,59)),str(random.randint(0,59))
		if int(h1) == int(h2) and int(m1) > int(m2):
			m1,m2 = m2,m1
		s1,s2 = str(random.randint(0,59)),str(random.randint(0,59))
		if int(h1) == int(h2) and int(m1) == int(m2) and int(s1) > int(s2):
			s1,s2 = s2,s1

		if int(h1)+8<24:
			startop_list.append("2000/1/1 " + str(int(h1)+8) + ":"+str(m1) + ":" + str(s1))
		else:
			startop_list.append("2000/1/2 " + str(int(h1)+8-24) + ":"+str(m1) + ":" + str(s1))
		if int(h2)+8<24:
			endop_list.append("2000/1/1 " + str(int(h2)+8) + ":"+str(m2) + ":" + str(s2))
		else:
			endop_list.append("2000/1/2 " + str(int(h2)+8-24) + ":"+str(m2) + ":" + str(s2))

	drskill_list = [random.uniform(0.0,max_drskill) for i in range(vnum)]
	opskill_list = [random.uniform(0.0,max_opskill) for i in range(vnum)]

	#メンバ変数用リスト(takebreak 休憩spot)
	takebreak_list = []
	b_spotid_list = []
	b_addtw_spotid_list = []
	if bnum > 0:
		for bn in range(bnum):
			if random.uniform(0.0,1.0) < max_takebreak_spotno_rate:
				b_spotid_list.append("")
			else:
				b_spotid_list.append(str(random.randint(1,max_spotid)))

		for bn in range(b_addtw_num):
			if random.uniform(0.0,1.0) < max_takebreak_spotno_rate:
				b_addtw_spotid_list.append("")
			else:
				b_addtw_spotid_list.append(str(random.randint(1,max_spotid)))
		#dem_list = [random.uniform(0.0,max_dem) for i in range(cnum + vnum)]
		#dem2_list = [random.uniform(0.0,max_dem2) for i in range(cnum + vnum)]
		#requestType_list = ["DELIVERY" if random.uniform(0.0,1.0) > dem_rate else "PICKUP" for i in range(cnum + vnum)]
		b_ready_list,b_due_list = [],[]
		for i in range(bnum):
			h1,h2 = str(random.randint(0,23)),str(random.randint(0,23))
			if int(h1) > int(h2):
				h1,h2 = h2,h1
			m1,m2 = str(random.randint(0,59)),str(random.randint(0,59))
			if int(h1) == int(h2) and int(m1) > int(m2):
				m1,m2 = m2,m1
			s1,s2 = str(random.randint(0,59)),str(random.randint(0,59))
			if int(h1) == int(h2) and int(m1) == int(m2) and int(s1) > int(s2):
				s1,s2 = s2,s1

			if int(h1)+8<24:
				b_ready_list.append("2000/1/1 " + str(int(h1)+8) + ":"+str(m1) + ":" + str(s1))
			else:
				b_ready_list.append("2000/1/2 " + str(int(h1)+8-24) + ":"+str(m1) + ":" + str(s1))
			if int(h2)+8<24:
				b_due_list.append("2000/1/1 " + str(int(h2)+8) + ":"+str(m2) + ":" + str(s2))
			else:
				b_due_list.append("2000/1/2 " + str(int(h2)+8-24) + ":"+str(m2) + ":" + str(s2))

		b_servt_list = [random.uniform(0.0,max_servt) for i in range(bnum)]
		b_reject_ve_list = [[vehno_list[j] for j in range(len(vehno_list)) if random.uniform(0.0,1.0) < reject_ve_rate] for i in range(bnum)]
		#b_parkt_open_list = [random.uniform(0.0,max_parkt_open) for i in range(bnum)]
		#b_parkt_arrive_list = [random.uniform(0.0,max_parkt_arrive) for i in range(bnum)]
		#b_depotservt_list = [random.uniform(0.0,max_cust_depotservt) for i in range(bnum)]

		br_cnt = 0
		ac_cnt = 0
		for vehno, bn in enumerate(bnum_list):
			_takebreak = []
			for i in range(bn):
				#メンバ変数に代入(takebreak)
				tb = copy.copy(c)
				b_custno = "b" + str(vehno+1)+str(i+1)
				b_spotid = b_spotid_list[br_cnt]
				b_dem = 0
				b_dem2 = 0
				b_ready = b_ready_list[br_cnt]
				b_due = b_due_list[br_cnt]
				b_servt = b_servt_list[br_cnt]
				b_reject_ve = b_reject_ve_list[br_cnt]
				b_parkt_open = 0
				b_parkt_arrive = 0
				b_depotservt = 0
				b_loaddivideunit = 1
				b_requestType = "DELIVERY"
				b_deny_unass = False

				tb["custno"] = str(b_custno)
				tb["spotid"] = str(b_spotid)
				tb["dem"] = float(b_dem)
				tb["dem2"] = float(b_dem2)
				tb["ready"] = b_ready
				tb["due"] = b_due
				tb["servt"] = float(b_servt)
				tb["reje_ve"] = b_reject_ve
				tb["parkt_open"] = float(b_parkt_open)
				tb["parkt_arrive"] = float(b_parkt_arrive)
				tb["depotservt"] = b_depotservt
				tb["demdivideunit"] = b_loaddivideunit
				tb["requestType"] = b_requestType
				tb["deny_unass"] = b_deny_unass
				b_additionalcust = []
				for j in range(b_addtw_num_list[br_cnt]):
					a_tb_custno = "b"+str(10001+ac_cnt)
					a_tb_spotid = b_addtw_spotid_list[ac_cnt]
					a_tb = []
					a_tb.append(a_tb_custno)
					a_tb.append(b_ready)
					a_tb.append(b_due)
					a_tb.append(str(a_tb_spotid))
					a_tb.append("0")
					a_tb.append("0")
					a_tb.append("0")
					b_additionalcust.append(a_tb)
					ac_cnt += 1
				tb["additionalcust"] = b_additionalcust

				_takebreak.append(tb)
				br_cnt += 1

			takebreak_list.append(_takebreak)
	else:
		for i in range(vnum):
			takebreak_list.append([])

	#メンバ変数用リスト(customer)
	custno_list = random.sample(range(1,max_custno+1),cnum + vnum + addtw_num)
	spotid_list = [random.randint(1,max_spotid) for i in range(cnum)] + [random.randint(0,max_spotid) for i in range(vnum)] + [random.randint(1,max_spotid) for i in range(addtw_num)]
	dem_list = [random.uniform(0.0,max_dem) for i in range(cnum + vnum)]
	dem2_list = [random.uniform(0.0,max_dem2) for i in range(cnum + vnum)]
	requestType_list = ["DELIVERY" if random.uniform(0.0,1.0) > dem_rate else "PICKUP" for i in range(cnum + vnum)]
	deny_unass_list = [True if random.uniform(0.0,1.0) > max_deny_unass_rate else False for i in range(cnum + vnum)]

	ready_list,due_list = [],[]
	for i in range(cnum + vnum + addtw_num):
		h1,h2 = str(random.randint(0,23)),str(random.randint(0,23))
		if int(h1) > int(h2):
			h1,h2 = h2,h1
		m1,m2 = str(random.randint(0,59)),str(random.randint(0,59))
		if int(h1) == int(h2) and int(m1) > int(m2):
			m1,m2 = m2,m1
		s1,s2 = str(random.randint(0,59)),str(random.randint(0,59))
		if int(h1) == int(h2) and int(m1) == int(m2) and int(s1) > int(s2):
			s1,s2 = s2,s1

		if int(h1)+8<24:
			ready_list.append("2000/1/1 " + str(int(h1)+8) + ":"+str(m1) + ":" + str(s1))
		else:
			ready_list.append("2000/1/2 " + str(int(h1)+8-24) + ":"+str(m1) + ":" + str(s1))
		if int(h2)+8<24:
			due_list.append("2000/1/1 " + str(int(h2)+8) + ":"+str(m2) + ":" + str(s2))
		else:
			due_list.append("2000/1/2 " + str(int(h2)+8-24) + ":"+str(m2) + ":" + str(s2))

	servt_list = [random.uniform(0.0,max_servt) for i in range(cnum + vnum)]
	reject_ve_list = [[vehno_list[j] for j in range(len(vehno_list)) if random.uniform(0.0,1.0) < reject_ve_rate] for i in range(cnum + vnum)]
	parkt_open_list = [random.uniform(0.0,max_parkt_open) for i in range(cnum + vnum + addtw_num)]
	parkt_arrive_list = [random.uniform(0.0,max_parkt_arrive) for i in range(cnum + vnum + addtw_num)]
	cust_deposervt_list= [random.uniform(0.0,max_cust_depotservt) for i in range(cnum + vnum)]

	# 大量輸送モード
	if random.uniform(0.0,1.0) < bulkshipping_rate:
		args["bulkShipping_flag"] = True #大量輸送モード
		if random.uniform(0.0,1.0) < bulkshipping_generate_rate:
			same_id_list = []
			tied_customer = random.randint(1,max_TiedCustomer)
			# TiedCustomerとなるspotid
			for i in range(tied_customer):
				id = random.choice(spotid_list[:-vnum-addtw_num])
				same_id_list.append(id)
			same_id_list = list(set(same_id_list))
			# 同じspotidの配送先を生成
			for i in same_id_list:
				index = [j for j, x in enumerate(spotid_list[:-vnum-addtw_num]) if x == i]
				same_ids = random.randint(1,max_same_id_customers_bulk)
				loop_cnt = 0
				if len(index) > same_ids:
					while len(index) > same_ids:
						del index[random.randint(0,len(index)-1)]
						loop_cnt += 1
						if loop_cnt > 100:
							break
				elif len(index) < same_ids:
					while len(list(set(index))) < same_ids:
						index.append(random.randint(0,len(spotid_list[:-vnum-addtw_num])-1))
						index = list(set(index))
						loop_cnt += 1
						if loop_cnt > 100:
							break
				# spotidと配送時間枠を更新
				for j in index:
					spotid_list[j] = spotid_list[index[0]]
					ready_list[j] = ready_list[index[0]]
					due_list[j] = due_list[index[0]]
	else:
		args["bulkShipping_flag"] = False #大量輸送モードなし

	# 直送便
	direct_flag = False
	generate_flag = False
	if random.uniform(0.0,1.0) < direct_rate:
		direct_flag = True
		init_read = False #初期解生成

		if random.uniform(0.0,1.0) < direct_generate_rate:
			generate_flag = True
			# 直送便になりやすい制約
			for j in range(len(spotid_list)-addtw_num):
				servt_list[j] = 0
				dem_list[j] = 0
				dem2_list[j] = 0

				ready_list[j] = "1000/1/1 0:00:00"
				due_list[j] = "3000/1/1 0:00:00"

		same_id_list = []
		direct_customer = random.randint(2,max_direct_customers)
		# 直送便となるspotid
		for i in range(direct_customer):
			id = random.choice(spotid_list[:-vnum-addtw_num])
			same_id_list.append(id)
		same_id_list = list(set(same_id_list))
		# 同じspotidの配送先を生成
		index_list = []
		direct_veh_list = []
		for i in same_id_list:
			index = [j for j, x in enumerate(spotid_list[:-vnum-addtw_num]) if x == i]
			if not index:
				continue
			same_ids = random.randint(2,max_same_id_customers_direct)
			loop_cnt = 0
			if len(index) > same_ids:
				while len(index) > same_ids:
					del index[random.randint(0,len(index)-1)]
					loop_cnt += 1
					if loop_cnt > 100:
						break
			elif len(index) < same_ids:
				# while len(list(set(index))) < same_ids:
				while len(index) < same_ids:
					index.append(random.randint(0,len(spotid_list[:-vnum-addtw_num])-1))
					# index = list(set(index))
					loop_cnt += 1
					if loop_cnt > 100:
						break
			# spotidと配送時間枠を更新
			index_not_included = []
			for j in index:
				if j not in index_list:
					index_list.append(j)
					index_not_included.append(j)
					spotid_list[j] = spotid_list[index[0]]
					ready_list[j] = ready_list[index[0]]
					due_list[j] = due_list[index[0]]
			index = index_not_included
			if not index:
				continue
			# 直送便となる車両
			direct_veh = random.randint(0,vnum-1)
			if direct_veh in direct_veh_list:
				continue
			direct_veh_list.append(direct_veh)
			startop_list[direct_veh] = ready_list[index[0]]
			endop_list[direct_veh] = due_list[index[0]]
			# 直送便に使用する配送先数
			cust_num_direct = random.randint(2,direct_customer)
			index = index[:cust_num_direct]
			# 第1積載か第2積載か選択
			dem_select = random.randint(0,1)
			if dem_select == 0:
				# 直送便の積載量
				cap_list[direct_veh] = int(cap_list[direct_veh])
				direct_cap = cap_list[direct_veh]
				dem_divide_list = []
				# 積載量を配送先の荷物量に分割
				for j in index:
					dem_divide_list.append(random.randint(1,10))
				# 荷物量を更新
				for j in range(len(index[:-1])):
					dem_direct = int(cap_list[direct_veh] * dem_divide_list[j] / sum(dem_divide_list))
					direct_cap -= dem_direct
					dem_list[index[j]] = dem_direct
				dem_list[index[-1]] = direct_cap
				# 第2積載の符号を一致させる
				for j in index:
					if dem_list[j] > 0:
						dem2_list[j] = abs(dem2_list[j])
					elif dem_list[j] < 0:
						dem2_list[j] = - abs(dem2_list[j])
				# 直送便になりやすい制約
				# if generate_flag:
				# 	for j in index:
				# 		dem2_list[j] = 0
				# 集荷
				# if random.uniform(0.0,1.0) < dem_rate:
				# 	for j in index:
				# 		if dem_list[j] > 0:
				# 			dem_list[j] = -dem_list[j]
			else:
				# 直送便の積載量
				cap2_list[direct_veh] = int(cap2_list[direct_veh])
				direct_cap2 = cap2_list[direct_veh]
				dem2_divide_list = []
				# 積載量を配送先の荷物量に分割
				for j in index:
					dem2_divide_list.append(random.randint(1,10))
				# 荷物量を更新
				for j in range(len(index[:-1])):
					dem2_direct = int(cap2_list[direct_veh] * dem2_divide_list[j] / sum(dem2_divide_list))
					direct_cap2 -= dem2_direct
					dem2_list[index[j]] = dem2_direct
				dem2_list[index[-1]] = direct_cap2
				# 第1積載の符号を一致させる
				for j in index:
					if dem2_list[j] > 0:
						dem_list[j] = abs(dem_list[j])
					elif dem2_list[j] < 0:
						dem_list[j] = - abs(dem_list[j])
				# 直送便になりやすい制約
				# if generate_flag:
				# 	for j in index:
				# 		dem_list[j] = 0
				# 集荷
				# if random.uniform(0.0,1.0) < dem_rate:
				# 	for j in index:
				# 		if dem2_list[j] > 0:
				# 			dem2_list[j] = -dem2_list[j]


	#メンバ変数用リスト(espot)
	espotno_list = random.sample(range(1,max_espot_num+1),enum)

	e_ready_list,e_due_list = [],[]
	for i in range(enum):
		h1,h2 = str(random.randint(0,23)),str(random.randint(0,23))
		if int(h1) > int(h2):
			h1,h2 = h2,h1
		m1,m2 = str(random.randint(0,59)),str(random.randint(0,59))
		if int(h1) == int(h2) and int(m1) > int(m2):
			m1,m2 = m2,m1
		s1,s2 = str(random.randint(0,59)),str(random.randint(0,59))
		if int(h1) == int(h2) and int(m1) == int(m2) and int(s1) > int(s2):
			s1,s2 = s2,s1

		if int(h1)+8<24:
			e_ready_list.append("2000/1/1 " + str(int(h1)+8) + ":"+str(m1) + ":" + str(s1))
		else:
			e_ready_list.append("2000/1/2 " + str(int(h1)+8-24) + ":"+str(m1) + ":" + str(s1))
		if int(h2)+8<24:
			e_due_list.append("2000/1/1 " + str(int(h2)+8) + ":"+str(m2) + ":" + str(s2))
		else:
			e_due_list.append("2000/1/2 " + str(int(h2)+8-24) + ":"+str(m2) + ":" + str(s2))

	e_servt_list = [random.uniform(0.0,max_servt) for i in range(enum)]
	e_reject_ve_list = [[vehno_list[j] for j in range(len(vehno_list)) if random.uniform(0.0,1.0) < reject_ve_rate] for i in range(enum)]
	e_rate_list = [random.uniform(min_erate,max_erate) for i in range(enum)]
	e_parkt_open_list = [random.uniform(0.0,max_parkt_open) for i in range(enum)]
	e_parkt_arrive_list = [random.uniform(0.0,max_parkt_arrive) for i in range(enum)]
	e_depotservt_list = [random.uniform(0.0,max_cust_depotservt) for i in range(enum)]
	# e_rate_list = [1e12 for i in range(enum)]

	#飛び地
	addtw_list = []
	for i in range(addtw_num):
		addtw_list.append([str(custno_list[cnum + vnum + i]), ready_list[cnum + vnum + i], due_list[cnum + vnum + i], str(spotid_list[cnum + vnum + i]), float(parkt_open_list[cnum + vnum + i]), float(parkt_arrive_list[cnum + vnum + i]), "false"])

	addtw_index = 0
	for i in range(cnum):
		#メンバ変数に代入(customer)
		customer = copy.copy(c)
		custno = custno_list[i]
		spotid = spotid_list[i]
		dem = dem_list[i]
		dem2 = dem2_list[i]
		ready = ready_list[i]
		due = due_list[i]
		servt = servt_list[i]
		reject_ve = reject_ve_list[i]
		parkt_open = parkt_open_list[i]
		parkt_arrive = parkt_arrive_list[i]
		for j in range(i+1): #同一SPOTIDには同じparktを設定するため
			if oRequestJSON['NormalCustomer'][j]["spotid"] == str(spotid):
				parkt_open = oRequestJSON['NormalCustomer'][j]["parkt_open"]
				parkt_arrive = oRequestJSON['NormalCustomer'][j]["parkt_arrive"]
				break
		depotservt = cust_deposervt_list[i]
		additionalcust = []
		for j in range(addtw_num_list[i]):
			additionalcust.append(addtw_list[addtw_index])
			addtw_index+=1
		loaddivideunit = loaddiivideunit_list[i]
		requestType = requestType_list[i]
		deny_unass = deny_unass_list[i]

		customer["custno"] = str(custno)
		customer["spotid"] = str(spotid)
		customer["dem"] = float(dem)
		customer["dem2"] = float(dem2)
		customer["ready"] = ready
		customer["due"] = due
		customer["servt"] = float(servt)
		customer["reje_ve"] = reject_ve
		customer["parkt_open"] = float(parkt_open)
		customer["parkt_arrive"] = float(parkt_arrive)
		customer["depotservt"] = depotservt
		customer["additionalcust"] = additionalcust
		customer["demdivideunit"] = loaddivideunit
		customer["requestType"] = requestType
		customer["deny_unass"] = deny_unass

		oRequestJSON['NormalCustomer'].append(customer)


	# 拠点配送先時間枠
	if direct_flag and generate_flag:
		oRequestJSON['NormalCustomer'][0]['ready'] = "1000/1/1 0:00:00"
		oRequestJSON['NormalCustomer'][0]['due'] = "3000/1/1 0:00:00"

	for i in range(enum):
		#メンバ変数に代入(espot)
		espot = copy.copy(e)
		custno = "e" + str(espotno_list[i])
		spotid = custno
		ready = e_ready_list[i]
		due = e_due_list[i]
		servt = e_servt_list[i]
		reject_ve = e_reject_ve_list[i]
		parkt_open = e_parkt_open_list[i]
		parkt_arrive = e_parkt_arrive_list[i]
		depotservt = e_depotservt_list[i]
		erate = e_rate_list[i]

		espot["custno"] = str(custno)
		espot["spotid"] = str(spotid)
		espot["ready"] = ready
		espot["due"] = due
		espot["servt"] = float(servt)
		espot["reje_ve"] = reject_ve
		espot["parkt_open"] = float(parkt_open)
		espot["parkt_arrive"] = float(parkt_arrive)
		espot["depotservt"] = depotservt
		espot["e_rate"] = erate

		oRequestJSON['ESpots'].append(espot)
	del oRequestJSON['ESpots'][0]


	oRequestJSON['Vehicles']=[]
	oRequestJSON['EndSpots']=[]

	for i in range(vnum):
		#メンバ変数に代入(vehicle)
		vehicle = copy.copy(v)
		vehno = vehno_list[i]
		cap = cap_list[i]
		cap2 = cap2_list[i]
		vtype = vtype_list[i]
		etype = etype_list[i]
		maxvisit = visit_list[i]
		# 直送便になりやすい制約
		if direct_flag and generate_flag:
			startoptime = "1000/1/1 0:00:00"
			endoptime = "3000/1/1 0:00:00"
		else:
			startoptime = startop_list[i]
			endoptime = endop_list[i]
		drskill = drskill_list[i]
		opskill = opskill_list[i]
		maxrotate = maxrotate_list[i]
		depotservt = veh_depotservt_list[i]
		rmpriority = rmpriority_list[i]
		takebreak = takebreak_list[i]
		reserved_v_flag = reserved_v_flag_list[i]

		ecapacity = ecapacity_list[i]
		einitial = einitial_list[i]
		ecost = ecost_list[i]
		emargin = emargin_list[i]
		vehicle["evehicle_dict"] = "{{'E-CAPACITY': {}, 'E-INITIAL': {}, 'E-COST': {}, 'E-MARGIN': {}}}".format(ecapacity, einitial, ecost, emargin)

		vehicle["vehno"] = vehno
		vehicle["cap"] = cap
		vehicle["cap2"] = cap2
		vehicle["vtype"] = vtype
		vehicle["etype"] = etype
		vehicle["maxvisit"] = maxvisit
		vehicle["startoptime"] = startoptime
		vehicle["endoptime"] = endoptime
		vehicle["drivingSkill"] = drskill
		vehicle["deliverySkill"] = opskill
		vehicle["maxrotate"] = maxrotate
		vehicle["depotservt"] = depotservt
		vehicle["rmpriority"] = rmpriority
		vehicle["takebreak"] = takebreak
		vehicle["reserved_v_flag"] = reserved_v_flag

		oRequestJSON['Vehicles'].append(vehicle)

		#メンバ変数に代入(endspot)
		endspot = copy.copy(es)

		custno = custno_list[cnum + i]
		spotid = spotid_list[cnum + i]
		dem = 0
		# 最終訪問先配送先時間枠
		if direct_flag and generate_flag:
			ready = "1000/1/1 0:00:00"
			due = "3000/1/1 0:00:00"
		else:
			ready = ready_list[cnum + i]
			due = due_list[cnum + i]
		servt = servt_list[cnum + i]
		reject_ve = reject_ve_list[cnum + i]
		parkt_open = parkt_open_list[cnum + i]
		parkt_arrive = parkt_arrive_list[cnum + i]
		for j in range(cnum): #同一SPOTIDには同じparktを設定するため
			if oRequestJSON['NormalCustomer'][j]["spotid"] == str(spotid):
				parkt_open = oRequestJSON['NormalCustomer'][j]["parkt_open"]
				parkt_arrive = oRequestJSON['NormalCustomer'][j]["parkt_arrive"]
				break
		for j in range(i): #同一SPOTIDには同じparktを設定するため
			if oRequestJSON['EndSpots'][j]["spotid"] == str(spotid):
				parkt_open = oRequestJSON['EndSpots'][j]["parkt_open"]
				parkt_arrive = oRequestJSON['EndSpots'][j]["parkt_arrive"]
				break
		depotservt = cust_deposervt_list[cnum + i]

		endspot["vehno"] = vehno
		endspot["custno"] = str(custno)
		endspot["spotid"] = str(spotid)
		endspot["dem"] = float(dem)
		endspot["ready"] = ready
		endspot["due"] = due
		endspot["servt"] = float(servt)
		endspot["reje_ve"] = []
		endspot["depotservt"] = depotservt

		oRequestJSON['EndSpots'].append(endspot)

	args["starttime"] = oRequestJSON['Vehicles'][0]["startoptime"]
	args["endtime"] = oRequestJSON['Vehicles'][0]["endoptime"]


	#テーブル作成
	oCostJSON['management_dtable'] = []
	oCostJSON['management_ttable'] = []

	dtable_num = random.randint(1,max_dtable_num)
	ttable_num = random.randint(1,max_ttable_num)
	table_type_list = [str(vtype) for vtype in range(0,max_vtype+1)] + ["common","time_common"]

	# 拠点と充電スポットを合わせたテーブルの大きさ
	_b_spotid_list = [spotid for spotid in b_spotid_list if spotid != ""]
	_b_addtw_spotid_list = [spotid for spotid in b_addtw_spotid_list if spotid != ""]
	table_size = len(list(set(spotid_list + b_spotid_list + b_addtw_spotid_list + [0]))) + len(list(set(espotno_list)))

	for i in range(dtable_num):
		dtable['vtype'] = random.choice(table_type_list)

		dtable['start_time'] = random.uniform(min_starttime,max_starttime)

		if generate_flag and i == 0:
			dtable['vtype'] = "common"
			dtable['start_time'] = 0.0
		elif generate_flag:
			dtable['vtype'] = random.choice(table_type_list[:-2])

		dtable['table'] = [[0.0 for j in range(table_size)] for k in range(table_size)]

		dist_list = [[random.uniform(0.0,max_dist) for j in range(table_size)] for k in range(table_size)]

		for k in range(table_size):
			for j in range(table_size):
				if k != j:
					dtable['table'][k][j] = dist_list[k][j]

					if generate_flag==False and random.uniform(0.0,1.0) < impassable_rate:
						dtable['table'][k][j] = -1 #一部を-1に変更

		oCostJSON['management_dtable'].append(dtable.copy())

	for i in range(ttable_num):
		ttable['vtype'] = random.choice(table_type_list)

		ttable['start_time'] = random.uniform(min_starttime,max_starttime)

		if generate_flag and i == 0:
			ttable['vtype'] = "common"
			ttable['start_time'] = 0.0
		elif generate_flag:
			ttable['vtype'] = random.choice(table_type_list[:-2])

		ttable['table'] = [[0.0 for j in range(table_size)] for k in range(table_size)]

		time_list = [[random.uniform(0.0,max_time) for j in range(table_size)] for k in range(table_size)]

		for k in range(table_size):
			for j in range(table_size):
				if k != j:
					ttable['table'][k][j] = time_list[k][j]

					if generate_flag==False and random.uniform(0.0,1.0) < impassable_rate:
						ttable['table'][k][j] = -1 #一部を-1に変更

		oCostJSON['management_ttable'].append(ttable.copy())
		

	if random.uniform(0.0,1.0) < etable_rate:
		# 電力テーブル追加
		oCostJSON['management_etable'] = copy.deepcopy(oCostJSON['management_dtable'])

		etable = oCostJSON['management_etable'][0]
		oCostJSON['management_etable'] = []

		etable_num = random.randint(1,max_etable_num)
		etable_type_list = [str(etype) for etype in range(0,max_etype+1)] + ["common","time_common"]

		for i in range(etable_num):
			etable['vtype'] = random.choice(table_type_list)
			etable['etype'] = random.choice(etable_type_list)

			etable['start_time'] = random.uniform(min_starttime,max_starttime)

			if generate_flag and i == 0:
				etable['vtype'] = "common"
				etable['etype'] = "common"
				etable['start_time'] = 0.0
			elif generate_flag:
				etable['vtype'] = random.choice(table_type_list[:-2])

			etable['table'] = [[0.0 for j in range(table_size)] for k in range(table_size)]

			e_list = [[random.uniform(0.0,max_e) for j in range(table_size)] for k in range(table_size)]

			for k in range(table_size):
				for j in range(table_size):
					if k != j:
						etable['table'][k][j] = e_list[k][j]

						if generate_flag==False and random.uniform(0.0,1.0) < impassable_rate:
							etable['table'][k][j] = -1 #一部を-1に変更

			oCostJSON['management_etable'].append(etable.copy())


	if random.uniform(0.0,1.0) < xtable_rate:
		# 経験コストテーブル追加
		oCostJSON['management_xtable'] = copy.deepcopy(oCostJSON['management_dtable'])

		xtable = oCostJSON['management_xtable'][0]
		oCostJSON['management_xtable'] = []

		xtable_num = random.randint(1,max_xtable_num)

		for i in range(xtable_num):
			xtable['vtype'] = random.choice(table_type_list)

			xtable['start_time'] = random.uniform(min_starttime,max_starttime)

			if generate_flag and i == 0:
				xtable['vtype'] = "common"
				xtable['start_time'] = 0.0
			elif generate_flag:
				xtable['vtype'] = random.choice(table_type_list[:-2])

			xtable['table'] = [[0.0 for j in range(table_size)] for k in range(table_size)]

			x_list = [[random.uniform(0.0,max_x) for j in range(table_size)] for k in range(table_size)]

			for k in range(table_size):
				for j in range(table_size):
					if k != j:
						xtable['table'][k][j] = x_list[k][j]

						if generate_flag==False and random.uniform(0.0,1.0) < impassable_rate:
							xtable['table'][k][j] = -1 #一部を-1に変更

			oCostJSON['management_xtable'].append(xtable.copy())


	#初期解作成
	if init_read == True:
		oRequestJSON['InitialRoutes'] = [[""] for i in range(vnum)]

		split_list = random.choices(range(1,len(oRequestJSON['NormalCustomer'])),k=vnum+1)

		split_list = sorted(split_list)

		for i in range(vnum):
			if split_list[i]!=split_list[i+1]:
				oRequestJSON['InitialRoutes'][i] +=["0"]
				oRequestJSON['InitialRoutes'][i] += [oRequestJSON['NormalCustomer'][j]["custno"] for j in range(split_list[i],split_list[i+1])]
				oRequestJSON['InitialRoutes'][i] +=["0"]
	else:
		del oRequestJSON['InitialRoutes']

	random_dir = 'test/apiTest/test_data/random/'
	os.makedirs(random_dir, exist_ok=True)
	with open(os.path.join(random_dir,request_file),"w") as f:
		json.dump(oRequestJSON,f,indent=4,separators=(",",":"))

	cost_dir = 'test/apiTest/test_data/random/cost/'
	os.makedirs(cost_dir, exist_ok=True)
	with open(os.path.join(cost_dir,cost_file),"w") as f:
		json.dump(oCostJSON,f,indent=4,separators=(",",":"))
		

	return timeout_flag


#-----------------------------------------------------------------
# 共通関数
#-----------------------------------------------------------------
def create_JSON(request_file, cost_file):
	with open(path + request_file, 'r', encoding='utf-8') as f:
		sRequestJson = f.read()
	with open(path + cost_dir + cost_file, 'r', encoding='utf-8') as f:
		sCostJson = f.read()
	oParamJSON = {"requestJSON": json.loads(sRequestJson),
				  "costJSON": json.loads(sCostJson)
				  }
	return oParamJSON


#-----------------------------------------------------------------
# メイン関数(ランダムテストパターン生成)
#-----------------------------------------------------------------
if __name__ == '__main__':
	for test_no, request_file, cost_file, logon_flag in test_args:
		Create_Random_Test(test_no, request_file, cost_file, logon_flag)
