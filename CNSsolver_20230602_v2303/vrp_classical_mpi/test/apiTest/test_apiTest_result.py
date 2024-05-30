# -*- coding: utf-8 -*-
# !/usr/bin/env python3

# ---------------------------------------------------------------------
# ---------------------------------------------------------------------
#
# APIテスト環境 結果確認
#

import sys
import os
import pprint

sys.path.append(os.path.join(os.path.dirname(__file__), '../../code'))

import main_tw
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
import csv
import filecmp
import pickle
import shutil
import glob
import math
from itertools import combinations

log_dir = './test/apiTest/log/'
out_dir = './test/apiTest/out/'
exp_dir = './test/apiTest/exp/'

vlog_onece = False

# ---------------------------------------------------------------------
# ---------------------------------------------------------------------
#
# パブリック用関数
#

# ---------------------------------------------------------------------
# ログ作成
# ---------------------------------------------------------------------
def log(test_no, oVehicles, oCustomer, oArgvs):

	# フォルダ作成
	os.makedirs(log_dir, exist_ok=True)

	log_filename = log_dir+oArgvs.outputfile.split('.csv')[0] + '_'+str(test_no)+'.csv'
	log_detail_filename = log_dir+oArgvs.outputfile.split('.csv')[0]+'_'+str(test_no)+'.detail.csv'

	if oVehicles != None:
		#ログファイル生成
		main_tw.generate_routefile(log_filename,oVehicles,oArgvs.lastc_flag,oArgvs)
		#ログ詳細ファイル生成
		main_tw.generate_resultfile(log_detail_filename,oCustomer,oVehicles,oArgvs.inputfile,oArgvs.lastc_flag,oArgvs)
		if oArgvs.evplan:
			main_tw.write_chargetime(log_detail_filename,oVehicles,oArgvs.lastc_flag,oArgvs)

		# ログ作成成功
		return True

	else:
		with open(log_filename, 'w') as f:
			f.write("oVehicles == None")
		with open(log_detail_filename, 'w') as f:
			f.write("oVehicles == None")
		# ログ作成失敗
		return False

# ---------------------------------------------------------------------
# Vehicleログ出力先指定
# ---------------------------------------------------------------------
def vehicle_log_init(caplog, test_no, oVehicles, oArgvs):
	global vlog_onece

	# フォルダ作成
	os.makedirs(log_dir, exist_ok=True)

	if caplog == None:
		vlog_filename = log_dir+oArgvs.outputfile.split('.csv')[0] + '_vlog_result.txt'
		if not vlog_onece:
			if os.path.exists(vlog_filename):
				os.remove(vlog_filename)
			vlog_onece = True
			formatter="%(asctime)s %(levelname)-8s %(pathname)s:%(funcName)s:%(lineno)d %(message)s"
			logging.basicConfig(filename=vlog_filename, format=formatter, level=logging.DEBUG)
		logging.log(logging.DEBUG, '\n\n <<<<<<' + str(test_no) + '>>>>>>')
	else:
		vlog_filename = log_dir+oArgvs.outputfile.split('.csv')[0] + '_vlog_'+str(test_no)+'.txt'
		if os.path.exists(vlog_filename):
			os.remove(vlog_filename)
		caplog.set_level(logging.DEBUG)

	return vlog_filename

# ---------------------------------------------------------------------
# コマンドライン実行 ログ出力先指定
# ---------------------------------------------------------------------
def command_line_log_init(caplog, test_no):
	global vlog_onece

	# フォルダ作成
	os.makedirs(log_dir, exist_ok=True)

	if caplog == None:
		vlog_filename = log_dir + 'output_vlog_result.txt'
		if not vlog_onece:
			if os.path.exists(vlog_filename):
				os.remove(vlog_filename)
			vlog_onece = True
			formatter="%(asctime)s %(levelname)-8s %(pathname)s:%(funcName)s:%(lineno)d %(message)s"
			logging.basicConfig(filename=vlog_filename, format=formatter, level=logging.DEBUG)
		logging.log(logging.DEBUG, '\n\n <<<<<<' + str(test_no) + '>>>>>>')
	else:
		vlog_filename = log_dir + 'output_vlog_'+str(test_no)+'.txt'
		if os.path.exists(vlog_filename):
			os.remove(vlog_filename)
		caplog.set_level(logging.DEBUG)

	return vlog_filename

# ---------------------------------------------------------------------
# Vehicleログ出力
# ---------------------------------------------------------------------
def vehicle_log_output(test_no, oVehicles, oNormalCusts, oArgvs):
	logging.log(logging.DEBUG, '\n----------------vehicle log output----------------')
	sLogBuf = "\n"
	divided=False
	for c in oNormalCusts:
		if c.demdivideunit!=1:
			divided=True
			break
	used_customers=[]
	for v in oVehicles:
		sLogBuf = v.output(oArgvs, sLogBuf,used_customers,divided)
		sLogBuf += "\n"
	logging.log(logging.INFO, sLogBuf)
	if oArgvs.delaystart_flag:
		logging.log(logging.DEBUG, '\n----------------delay----------------')
		sLogBuf = "\n"
		used_customers=[]
		for v in oVehicles:
			sLogBuf = v.output(oArgvs, sLogBuf, used_customers, divided, True)
			sLogBuf += "\n"
		logging.log(logging.INFO, sLogBuf)
	return

# ---------------------------------------------------------------------
# Vehicleログダンプ
# ---------------------------------------------------------------------
def vehicle_log_dump(caplog, filename):
	if caplog != None:
		with open(filename,"w") as f:
			for row in caplog.record_tuples:
				f.write(row[2])
	return

# ---------------------------------------------------------------------
# ハード制約
# 制約：充電スポット営業時間枠制約
# 内容：充電スポットの訪問時間違反
# ---------------------------------------------------------------------
def HardCheck_ChargingSpotTimeFrame(test_no, oVehicles, oArgvs):
	if oVehicles == None or oArgvs == None:
		return
	if oArgvs.evplan == False:
		return

	for v in oVehicles:
		arr_cnt = 0
		for r in v.route:
			custno = r.custno
			if custno.startswith('e'):
				# 充電スポットの営業終了時間
				due = r.due_fs
				# 到着時刻
				arr = v.arr[arr_cnt]
				assert arr <= due

				# 出発遅延
				if oArgvs.delaystart_flag:
					# 到着時刻 (遅延後)
					delay_arr = v.delay_arr[arr_cnt]
					assert main_tw.compare_time(delay_arr, due) <= 0
			arr_cnt += 1

	return

# ---------------------------------------------------------------------
# ハード制約
# 制約：電動車両充電量制約
# 内容：電動車両の充電上限違反
# ---------------------------------------------------------------------
def HardCheck_EVChargeLimit(test_no, oVehicles, oArgvs):
	if oVehicles==None or oArgvs==None:
		return
	if oArgvs.evplan == False:
		return

	for v in oVehicles:
		for rb in v.rem_batt:
			assert rb <= v.e_capacity

	return

# ---------------------------------------------------------------------
# ハード制約
# 制約：通行不可制約
# 内容：移動経路無し
# (※注)
# get_dtableは単体テスト実施済みのため、本APIテスト内で利用する
# get_ttableは単体テスト実施済みのため、本APIテスト内で利用する
# get_xtableは単体テスト実施済みのため、本APIテスト内で利用する
# get_etableは単体テスト実施済みのため、本APIテスト内で利用する
# ---------------------------------------------------------------------
def HardCheck_Impassable(test_no, oVehicles, oArgvs):
	if oVehicles==None or oArgvs==None:
		return

	# 距離テーブルが-1となるカスタマー間は通行不可
	# 配送経路から上記違反がないのかを確認する
	for v in oVehicles:
		for cust_from, cust_to, lv_from in zip(v.route[:-1], v.route[1:], v.lv[:-1]):
			if cust_to.custno[0]=="b" and cust_to.spotid=="":
				# 休憩スポットでspotidが""の時のチェック
				i = v.route.index(cust_to)
				for l in reversed(range(0, i)):
					if v.route[l].custno[0]!="b":
						break
				else:
					assert False

				for m in (range(i, len(v.route))):
					if v.route[m].custno[0]!="b":
						break
				else:
					assert False

				# from
				lv_from = v.lv[l]
				table = main_tw.get_dtable(v.vtype, lv_from, oArgvs.management_dtable)
				ret = table[v.route[l].spotno][v.route[m].spotno] 
				assert ret != -1

				# to
				lv_to = v.lv[m]
				table = main_tw.get_dtable(v.vtype, lv_to, oArgvs.management_dtable)
				ret = table[v.route[l].spotno][v.route[m].spotno] 
				assert ret != -1
			else:
				table = main_tw.get_dtable(v.vtype, lv_from, oArgvs.management_dtable)
				ret = table[cust_from.spotno][cust_to.spotno] 
				assert ret != -1

	# 時間テーブルが-1となるカスタマー間は通行不可
	# 配送経路から上記違反がないのかを確認する
	for v in oVehicles:
		for cust_from, cust_to, lv_from in zip(v.route[:-1], v.route[1:], v.lv[:-1]):
			if cust_to.custno[0]=="b" and cust_to.spotid=="":
				# 休憩スポットでspotidが""の時のチェック
				i = v.route.index(cust_to)
				for l in reversed(range(0, i)):
					if v.route[l].custno[0]!="b":
						break
				else:
					assert False

				for m in (range(i, len(v.route))):
					if v.route[m].custno[0]!="b":
						break
				else:
					assert False

				# from
				lv_from = v.lv[l]
				table = main_tw.get_ttable(v.vtype, lv_from, oArgvs.management_ttable)
				ret = table[v.route[l].spotno][v.route[m].spotno] 
				assert ret != -1

				# to
				lv_to = v.lv[m]
				table = main_tw.get_ttable(v.vtype, lv_to, oArgvs.management_ttable)
				ret = table[v.route[l].spotno][v.route[m].spotno] 
				assert ret != -1
			else:
				table = main_tw.get_ttable(v.vtype, lv_from, oArgvs.management_ttable)
				ret = table[cust_from.spotno][cust_to.spotno] 
				assert ret != -1

	# 電費テーブルが-1となるカスタマー間は通行不可
	# 配送経路から上記違反がないのかを確認する
	if oArgvs.evplan and oArgvs.management_etable:
		for v in oVehicles:
			for cust_from, cust_to, lv_from in zip(v.route[:-1], v.route[1:], v.lv[:-1]):
				if cust_to.custno[0]=="b" and cust_to.spotid=="":
					# 休憩スポットでspotidが""の時のチェック
					i = v.route.index(cust_to)
					for l in reversed(range(0, i)):
						if v.route[l].custno[0]!="b":
							break
					else:
						assert False

					for m in (range(i, len(v.route))):
						if v.route[m].custno[0]!="b":
							break
					else:
						assert False

					# from
					lv_from = v.lv[l]
					table = main_tw.get_etable(v.vtype, v.etype, lv_from, oArgvs.management_etable)
					ret = table[v.route[l].spotno][v.route[m].spotno] 
					assert ret != -1

					# to
					lv_to = v.lv[m]
					table = main_tw.get_etable(v.vtype, v.etype, lv_to, oArgvs.management_etable)
					ret = table[v.route[l].spotno][v.route[m].spotno] 
					assert ret != -1
				else:
					table = main_tw.get_etable(v.vtype, v.etype, lv_from, oArgvs.management_etable)
					ret = table[cust_from.spotno][cust_to.spotno] 
					assert ret != -1

	# 経験コストテーブルが-1となるカスタマー間は通行不可
	# 配送経路から上記違反がないのかを確認する
	if oArgvs.management_xtable:
		for v in oVehicles:
			for cust_from, cust_to, lv_from in zip(v.route[:-1], v.route[1:], v.lv[:-1]):
				if cust_to.custno[0]=="b" and cust_to.spotid=="":
					# 休憩スポットでspotidが""の時のチェック
					i = v.route.index(cust_to)
					for l in reversed(range(0, i)):
						if v.route[l].custno[0]!="b":
							break
					else:
						assert False

					for m in (range(i, len(v.route))):
						if v.route[m].custno[0]!="b":
							break
					else:
						assert False

					# from
					lv_from = v.lv[l]
					table = main_tw.get_xtable(v.vtype, lv_from, oArgvs.management_xtable)
					ret = table[v.route[l].spotno][v.route[m].spotno] 
					assert ret != -1

					# to
					lv_to = v.lv[m]
					table = main_tw.get_xtable(v.vtype, lv_to, oArgvs.management_xtable)
					ret = table[v.route[l].spotno][v.route[m].spotno] 
					assert ret != -1
				else:
					table = main_tw.get_xtable(v.vtype, lv_from, oArgvs.management_xtable)
					ret = table[cust_from.spotno][cust_to.spotno] 
					assert ret != -1

	return


# ---------------------------------------------------------------------
# ハード制約
# 制約：車両立寄不可制約
# 内容：配送先へ訪問不可
# ---------------------------------------------------------------------
def HardCheck_VehicleStopOff(test_no, oVehicles, oArgvs):
	if oVehicles==None or oArgvs==None:
		return

	# Vehiclルートの中に、配送できないカスタマー番号(reje_ve)が存在するかを確認
	# 存在する場合エラーとする
	for v in oVehicles:
		for cust in v.route:
			for ve in cust.reje_ve:
				assert ve != v.vehno

		# 休憩機能用takebreakの中もチェックする
		for cust in v.takebreak:
			for ve in cust.reje_ve:
				assert ve != v.vehno

	return

# ---------------------------------------------------------------------
# ハード制約
# 制約：車両訪問数上限 
# 内容：車両1台の訪問先件数の制限
# メモ：訪問先件数はシステム内で0初期化されているため未設定の場合、maxvisit==0として考える
# ---------------------------------------------------------------------
def HardCheck_MaxNumVehicleVisits(test_no, oArgvs, oVehicles):
	if oVehicles==None or oArgvs==None:
		return

	# 訪問数算出
	def calc_spot_len(vehicle):
		v = vehicle
		spot_list=[]
		#olist+=["route"+str(v.vehno)]
		before_spotid = None
		for r in v.route[1:-1]:
			spotid = r.spotid
			# 訪問先カウント。下記はカウントしない
			# 充電スポット('e'を含む)、拠点('0')、前回と同じスポット(before_spotno == spotno)
			if not 'e' in spotid and spotid != '0' and before_spotid != spotid:
				spot_list.append(spotid)
				before_spotid = spotid

		return len(spot_list)

	log_filename = log_dir + oArgvs.outputfile.split('.csv')[0] + '_'+str(test_no)+'.csv'
	csv_reader=csv.reader(open(log_filename,"r"),delimiter=",",quotechar='"')

	for v in oVehicles:
		route_cnt = 0 #出力結果(output_csvの何番目のrouteと比較するかの値)
		vehicle_no = v.vehno

		if v.maxvisit != 0: #maxvisitは訪問先の上限がないため判定を行わない
			for row in csv_reader:
				if not "echarge" in row[0] and  not "eremarr" in row[0] and not row[1]=="Id":
					if int(vehicle_no) == route_cnt:
						# visit_len = calc_spot_len(v)
						visit_len = main_tw.calc_visitnum(v.route)
						assert visit_len <= v.maxvisit
	return

# ---------------------------------------------------------------------
# ハード制約
# 制約：回転数上限 
# 内容：車両1台の回転数の制限
# メモ：回転数はシステム内で0初期化されているため未設定の場合、maxrotate==0として考える
# ---------------------------------------------------------------------
def HardCheck_MaxNumVehicleRotates(test_no, oArgvs, oVehicles):
	if oVehicles==None or oArgvs==None:
		return

	# 回転数算出
	def calc_rotate_num(vehicle):
		v = vehicle
		num=1
		
		pickup_opt = True
		for r in reversed(v.route[1:-2]):
			if r.custno=="0" and pickup_opt==False:
				num+=1
			if r.custno[0]!="b":
				pickup_opt = False

		return num

	log_filename = log_dir + oArgvs.outputfile.split('.csv')[0] + '_'+str(test_no)+'.csv'
	csv_reader=csv.reader(open(log_filename,"r"),delimiter=",",quotechar='"')

	for v in oVehicles:
		if v.maxrotate==0:
			continue

		assert calc_rotate_num(v) <= v.maxrotate
	return

# ---------------------------------------------------------------------
# ハード制約
# 制約：時間枠制約
# 内容：時間ペナルティハード制約時の時間枠違反
# ---------------------------------------------------------------------
def HardCheck_TimeFrame(test_no, oVehicles, oArgvs):
	if oVehicles == None or oArgvs == None:
		return
	if oArgvs.timePenaltyType != "hard":
		return

	for v in oVehicles:
		arr_cnt = 0

		if oArgvs.lastc_flag:
			for i in reversed(range(0,len(v.route))):
				if v.route[i].custno=="0" or i==len(v.route)-1:
					last_idx=i
				else:
					break
		else:
			last_idx=len(v.route)

		for r in v.route[1:last_idx]:
			custno = r.custno
			
			due = r.due_fs
			# 到着時刻
			arr = v.arr[arr_cnt]
			assert main_tw.compare_time(arr, due) <= 0

			# 出発遅延
			if oArgvs.delaystart_flag:
				# 到着時刻 (遅延後)
				delay_arr = v.delay_arr[arr_cnt]
				assert main_tw.compare_time(delay_arr, due) <= 0
			arr_cnt += 1

		#営業時間枠
		if oArgvs.lastc_flag:
			assert main_tw.compare_time(v.starthour, v.arr[0]) <= 0
			if v.route[-2].custno == "0":
				assert main_tw.compare_time(v.lv[-3], v.endhour) <= 0
			else:
				assert main_tw.compare_time(v.lv[-2], v.endhour) <= 0
		else:
			assert main_tw.compare_time(v.starthour, v.arr[0]) <= 0
			assert main_tw.compare_time(v.lv[-1], v.endhour) <= 0

	return

# ---------------------------------------------------------------------
# ハード制約
# 制約：積載量制約
# 内容：積載量ペナルティハード制約時の積載量違反
# ---------------------------------------------------------------------
def HardCheck_LoadLimit(test_no, oVehicles, oArgvs):
	if oVehicles == None or oArgvs == None:
		return
	if oArgvs.loadPenaltyType != "hard":
		return

	for v in oVehicles:
		depo_idx = [i for i in range(len(v.route)-1) if v.route[i].custno == "0"]+[len(v.route)-1]

		for i in range(len(depo_idx)-1):
			dem = sum([c.dem for c in v.route[depo_idx[i]+1:depo_idx[i+1]] if c.requestType=="DELIVERY" and c.custno[0]!="b" and c.custno[0]!="e"])
			assert dem <= v.cap

			for j in range(depo_idx[i]+1,depo_idx[i+1]):
				if v.route[j].custno[0] != "b" and v.route[j].custno[0] != "e":
					if v.route[j].requestType=="DELIVERY":
						dem -= v.route[j].dem
					else:
						dem += v.route[j].dem
				assert dem <= v.cap

	return

# ---------------------------------------------------------------------
# ハード制約
# 制約：第2積載量制約
# 内容：第2積載量ペナルティハード制約時の第2積載量違反
# ---------------------------------------------------------------------
def HardCheck_Load2Limit(test_no, oVehicles, oArgvs):
	if oVehicles == None or oArgvs == None:
		return
	if oArgvs.load2PenaltyType != "hard":
		return

	for v in oVehicles:
		depo_idx = [i for i in range(len(v.route)-1) if v.route[i].custno == "0"]+[len(v.route)-1]

		for i in range(len(depo_idx)-1):
			dem2 = sum([c.dem2 for c in v.route[depo_idx[i]+1:depo_idx[i+1]] if c.requestType=="DELIVERY" and c.custno[0]!="b" and c.custno[0]!="e"])
			assert dem2 <= v.cap2

			for j in range(depo_idx[i]+1,depo_idx[i+1]):
				if v.route[j].custno[0] != "b" and v.route[j].custno[0] != "e":
					if v.route[j].requestType=="DELIVERY":
						dem2 -= v.route[j].dem2
					else:
						dem2 += v.route[j].dem2
				assert dem2 <= v.cap2

	return

# ---------------------------------------------------------------------
# ハード制約
# 制約：時間ばらつき制約
# 内容：時間ばらつきペナルティハード制約時の時間ばらつき違反
# ---------------------------------------------------------------------
def HardCheck_avetime(test_no, oVehicles, oArgvs):
	epsilon = 0

	if oVehicles == None or oArgvs == None:
		return
	if oArgvs.timeLevelingPenaltyType != "hard":
		return

	if oArgvs.opt_unassigned_flag==True: # 未割り当て荷物最適化の場合は予約車両を除外する
		oVehicles = [v for v in oVehicles if v.reserved_v_flag == False]

	if oArgvs.avetime_ape_flag:
		avetime_ape = SoftCheck_avetime_APE(test_no, oVehicles, oArgvs, oArgvs.avetime_ape, epsilon)
	elif oArgvs.avetime_ae_flag:
		avetime_ae = SoftCheck_avetime_AE(test_no, oVehicles, oArgvs, oArgvs.avetime_ae, epsilon)

	return

# ---------------------------------------------------------------------
# ハード制約
# 制約：件数ばらつき制約
# 内容：件数ばらつきペナルティハード制約時の件数ばらつき違反
# ---------------------------------------------------------------------
def HardCheck_avevisits(test_no, oVehicles, oArgvs):
	epsilon = 0

	if oVehicles == None or oArgvs == None:
		return
	if oArgvs.visitsLevelingPenaltyType != "hard":
		return

	if oArgvs.opt_unassigned_flag==True: # 未割り当て荷物最適化の場合は予約車両を除外する
		oVehicles = [v for v in oVehicles if v.reserved_v_flag == False]

	if oArgvs.avevisits_ape_flag:
		avevisits_ape = SoftCheck_avevisits_APE(test_no, oVehicles, oArgvs, oArgvs.avevisits_ape, epsilon)
	elif oArgvs.avevisits_ae_flag:
		avevisits_ae = SoftCheck_avevisits_AE(test_no, oVehicles, oArgvs, oArgvs.avevisits_ae, epsilon)

	return

# ---------------------------------------------------------------------
# ハード制約
# 制約：距離ばらつき制約
# 内容：距離ばらつきペナルティハード制約時の距離ばらつき違反
# ---------------------------------------------------------------------
def HardCheck_avedist(test_no, oVehicles, oArgvs):
	epsilon = 0

	if oVehicles == None or oArgvs == None:
		return
	if oArgvs.distLevelingPenaltyType != "hard":
		return

	if oArgvs.opt_unassigned_flag==True: # 未割り当て荷物最適化の場合は予約車両を除外する
		oVehicles = [v for v in oVehicles if v.reserved_v_flag == False]

	if oArgvs.avedist_ape_flag:
		avedist_ape = SoftCheck_avedist_APE(test_no, oVehicles, oArgvs, oArgvs.avedist_ape, epsilon)
	elif oArgvs.avedist_ae_flag:
		avedist_ae = SoftCheck_avedist_AE(test_no, oVehicles, oArgvs, oArgvs.avedist_ae, epsilon)

	return

# ---------------------------------------------------------------------
# ハード制約
# 制約：割当済荷物変更
# 内容：割当済荷物変更可否
# ---------------------------------------------------------------------
def HardCheck_changecust(test_no, oVehicles, oArgvs, oInitRoutes):
	if oArgvs.init_changecust!="additive":
		return

	routes=[route[2:-1] for route in oInitRoutes]
	if len(routes)==0:
		return

	if oArgvs.decr_vnum==True and len(routes)!=len(oVehicles):
		return

	if oArgvs.opt_unassigned_flag==True: # 未割り当て荷物最適化の場合
		reserved_cust = [c for v in oVehicles for c in v.route if v.reserved_v_flag == True]
		nonreserved_cust = [c for v in oVehicles for c in v.route if v.reserved_v_flag == False]

	if oArgvs.bulkShipping_flag==True:
		for i in range(len(routes)):
			ridx=1
			for j in range(len(routes[i])):
				if routes[i][j][0]!="b":
					continue
				for k in range(ridx,len(oVehicles[i].route)-1):
					for c in oVehicles[i].route[k].customers:
						if routes[i][j]==c.custno:
							ridx=k
							break
						elif oArgvs.opt_unassigned_flag==True:
							if oVehicles[i].reserved_v_flag == True:
								if c in nonreserved_cust:
									#通常車両から予約車両に移動した荷物なら許容する
									ridx=k
									break
							else:
								if c in reserved_cust:
									#予約車両から通常車両に移動した荷物なら許容する
									ridx=k
									break
							continue
						else:
							continue
					break
				else:
					assert False
	else:
		for i in range(len(routes)):
			ridx=1
			for j in range(len(routes[i])):
				if routes[i][j][0]!="b":
					continue
				for k in range(ridx,len(oVehicles[i].route)-1):
					if routes[i][j]==oVehicles[i].route[k].custno:
						ridx=k+1
						break
					elif oArgvs.opt_unassigned_flag==True:
						if oVehicles[i].reserved_v_flag == True:
							if routes[i][j] in nonreserved_cust:
								#通常車両から予約車両に移動した荷物なら許容する
								ridx=k+1
								break
						else:
							if routes[i][j] in reserved_cust:
								#予約車両から通常車両に移動した荷物なら許容する
								ridx=k+1
								break
				else:
					assert False

	if oArgvs.multitrip_flag==True:
		rotate_idx={}
		
		for j in range(len(oVehicles)):
			if oArgvs.opt_unassigned_flag==True and len(routes)<=j:
				break
			v=oVehicles[j]
			idx=1
			for i in range(len(routes[j])):
				if i!=len(routes[j])-1 and routes[j][i]=="0" and routes[j][i+1]!="0":
					idx+=1
				else:
					rotate_idx[routes[j][i]]=idx

			idx=0
			for i in range(len(v.route)-1):
				if v.route[i].custno=="0" and v.route[i+1].custno!="0":
					idx+=1
				elif v.route[i].changecust=="additive" and v.route[i].custno[0]!="b":
					if oArgvs.bulkShipping_flag==True:
						if rotate_idx[v.route[i].customers[0].custno]!=idx:
							if oArgvs.opt_unassigned_flag==True:
								if v.reserved_v_flag == True:
									continue
							assert False
					else:
						if rotate_idx[v.route[i].custno]!=idx:
							if oArgvs.opt_unassigned_flag==True:
								continue
							assert False

	return


# ---------------------------------------------------------------------
# ソフト制約
# 制約：車両営業時間枠制約
# 内容：車両の営業時間違反
# メモ：
# ---------------------------------------------------------------------
def SoftCheck_VehicleBusinessHours(test_no, oArgvs, oVehicles):
	return

# ---------------------------------------------------------------------
# ソフト制約
# 制約：配送先受入れ時間枠制約
# 内容：配送先の受入れ時間違反
# メモ：
# ---------------------------------------------------------------------
def SoftCheck_CustomerAcceptanceTime(test_no, oArgvs, oVehicles):
	return

# ---------------------------------------------------------------------
# ソフト制約
# 制約：APE制約
# 内容：配送業務時間のばらつき度合
# メモ：APE(absolute percentage error)とは車両の配送時間のバラつきであり、
#       以下の式で表され、0以上1以下の範囲の値を取る。
#             APE = |t_i - μ|/μ
#             t_i : 各車両の配送時間(iは車両番号)
#             μ  : 車両の配送時間の平均
#             (APE-epsilon)が指定範囲かどうかを確認。epsilonは判定マージン[%]
# ---------------------------------------------------------------------
def SoftCheck_avetime_APE(test_no, oVehicles, oArgvs, threshold,epsilon):
	APE = 0
	# 各車両の配送時間(t_i)算出
	t_i_list = []

	if oArgvs.opt_unassigned_flag==True: # 未割り当て荷物最適化の場合は予約車両を除外する
		oVehicles = [v for v in oVehicles if v.reserved_v_flag == False]

	for v in oVehicles:
		last_idx = main_tw.get_last_idx(v, oArgvs)
		if not v.direct_flag:
			if oArgvs.delaystart_flag and v.delay_arr!=[]:
				t_i = v.delay_lv[last_idx]-v.delay_arr[0]
			else:
				t_i = v.lv[last_idx]-v.arr[0]

			if len(v.route) > 2: #長さ2より大きなルートのみカウント(配送先のないルートは未成立のためAPE算出に用いない)
				t_i_list.append([v.vehno, t_i])

	# APE算出
	mu = np.mean([val[1] for val in t_i_list])
	for val in (t_i_list):
		t_i = val[1]
		APE = np.abs(t_i-mu)/mu
		# APE = |t_i - μ|/μ
		# APEが指定誤差内であることを確認
		print(test_no, "vehno:", val[0], ": APE:",APE)
		if test_no != '4_1_7' and test_no != '4_1_8' and test_no != '25_2_1' and test_no != '25_2_2' and test_no != '25_2_3' and test_no != '25_2_5' and test_no != '25_2_6':
			assert APE <= threshold + epsilon

	return APE

def SoftCheck_avetime_AE(test_no, oVehicles, oArgvs, threshold, epsilon):
	AE = 0
	time_veh = []

	if oArgvs.opt_unassigned_flag==True: # 未割り当て荷物最適化の場合は予約車両を除外する
		oVehicles = [v for v in oVehicles if v.reserved_v_flag == False]

	if oArgvs.lastc_flag:
		# 対象車両以外の走行時間
		for v in oVehicles:
			last_idx = main_tw.get_last_idx(v, oArgvs)
			if len(v.route) >= 3 and v.direct_flag==False:
				if oArgvs.delaystart_flag and v.delay_arr!=[]:
					time_veh.append(v.delay_lv[last_idx]-v.delay_arr[0])
				else:
					time_veh.append(v.lv[last_idx]-v.arr[0])
	else:
		# 対象車両以外の走行時間
		for v in oVehicles:
			if len(v.route) >= 3 and v.direct_flag==False:
				if oArgvs.delaystart_flag and v.delay_arr!=[]:
					time_veh.append(v.delay_lv[-1]-v.delay_arr[0])
				else:
					time_veh.append(v.lv[-1]-v.arr[0])

	# AE算出
	for custs in combinations(time_veh, 2):
		diff = math.fabs(custs[0] - custs[1])
		AE += max(diff-oArgvs.avetime_ae, 0)	#avetime_aeに収まっている場合は0とする

	assert AE <= threshold + epsilon

	return AE

def SoftCheck_avevisits_APE(test_no, oVehicles, oArgvs, threshold, epsilon):
	APE = 0
	if oArgvs.decr_vnum:
		if len([v for v in oVehicles if len([c for c in v.route[1:-1] if c.custno!="0" and c.custno[0]!="e" and c.custno[0]!="b"]) >= 1 and v.direct_flag==False])-1==0:
			return
	else:
		if len([v for v in oVehicles if len([c for c in v.route[1:-1] if c.custno!="0" and c.custno[0]!="e" and c.custno[0]!="b"]) >= 1 and v.direct_flag==False])==0:
			return

	if oArgvs.opt_unassigned_flag==True: # 未割り当て荷物最適化の場合は予約車両を除外する
		oVehicles = [v for v in oVehicles if v.reserved_v_flag == False]

	if oArgvs.bulkShipping_flag:
		mu=sum([len(r.customers) for v in oVehicles for r in v.route[1:-1] if len([c for c in v.route[1:-1] if c.custno!="0" and c.custno[0]!="e" and c.custno[0]!="b"]) >= 1 and v.direct_flag==False and not r.spotid.startswith("e") and r.custno!="0" and not r.spotid.startswith("b") ])	#vehiclesからmvehiclesを除いた訪問数の合計を算出
	else:
		mu=sum([len(v.route)-2 for v in oVehicles if len([c for c in v.route[1:-1] if c.custno!="0" and c.custno[0]!="b"]) >= 1 and v.direct_flag==False])	#vehiclesからmvehiclesを除いた訪問数の合計を算出
		e_cnt = len([r for v in oVehicles for r in v.route[1:-1] if len([c for c in v.route[1:-1] if c.custno!="0"]) >= 1 and r.spotid.startswith("e") and r.custno=="0"])
		mu -= e_cnt
	mu/=float(len([v for v in oVehicles if len([c for c in v.route[1:-1] if c.custno!="0" and c.custno[0]!="e" and c.custno[0]!="b"]) >= 1 and v.direct_flag==False]))	#車両台数で割る(muとする)

	# APE算出
	for v in oVehicles:
		if len([c for c in v.route[1:-1] if c.custno!="0" and c.custno[0]!="e" and c.custno[0]!="b"]) >= 1 and v.direct_flag==False:
			if main_tw.compare_time(mu)==0:
				sErrMes = "APE margin of visitsleveling(avevisits_ape) can't calculate due to delivery number=0"
				raise ValueError(sErrMes)

			if oArgvs.bulkShipping_flag:
				visit_num = 0
				for r in v.route[1:-1]:
					visit_num += len(r.customers)
				e_cnt = len([r for r in v.route[1:-1] for c in r.customers if len([c for c in v.route[1:-1] if c.custno!="0"]) >= 1 and c.spotid.startswith("e") and c.custno=="0"])
				diff = math.fabs(mu-visit_num-e_cnt)/mu
				APE = max(diff-oArgvs.avevisits_ape,0)	#avevisits_apeに収まっている場合は0とする
			else:
				e_cnt = len([r for r in v.route[1:-1] if len([c for c in v.route[1:-1] if c.custno!="0"]) >= 1 and r.spotid.startswith("e") and r.custno=="0"])
				diff = math.fabs(mu-(len(v.route)-e_cnt-2))/mu
				APE = max(diff-oArgvs.avevisits_ape,0)	#avevisits_apeに収まっている場合は0とする

			print(test_no, "vehno:", v.vehno, ": APE:",APE)
			if test_no != '36_1_8' and test_no != '36_1_9':
				assert APE <= threshold + epsilon

	return APE

def SoftCheck_avevisits_AE(test_no, oVehicles, oArgvs, threshold, epsilon):
	AE=0
	visits_veh = []

	if oArgvs.opt_unassigned_flag==True: # 未割り当て荷物最適化の場合は予約車両を除外する
		oVehicles = [v for v in oVehicles if v.reserved_v_flag == False]

	if oArgvs.bulkShipping_flag:
		# 対象車両以外の訪問数
		for v in oVehicles:
			if len([c for c in v.route[1:-1] if c.custno!="0" and c.custno[0]!="e" and c.custno[0]!="b"]) >= 1 and v.direct_flag==False:
				visits = 0
				for r in v.route[1:-1]:
					# 充電スポットを除いた訪問数
					visits += len([c for c in r.customers if not c.spotid.startswith("e") and c.custno!="0" and not c.spotid.startswith("b")])
				visits_veh.append(visits)
	else:
		# 対象車両以外の訪問数
		for v in oVehicles:
			if len([c for c in v.route[1:-1] if c.custno!="0" and c.custno[0]!="e" and c.custno[0]!="b"]) >= 1 and v.direct_flag==False:
				# 充電スポットを除いた訪問数
				visits = len([r for r in v.route[1:-1] if not r.spotid.startswith("e") and r.custno!="0" and not r.spotid.startswith("b")])
				visits_veh.append(visits)

	# AE算出
	for custs in combinations(visits_veh, 2):
		diff = math.fabs(custs[0] - custs[1])
		AE += max(diff-oArgvs.avevisits_ae, 0)	#avevisits_aeに収まっている場合は0とする

	if test_no != '36_1_21' and test_no != '36_1_22':
		assert AE <= threshold + epsilon

	return AE

def SoftCheck_avedist_APE(test_no, oVehicles, oArgvs, threshold,epsilon):
	APE = 0
	# 各車両の移動距離(d_i)算出
	d_i_list = []

	if oArgvs.opt_unassigned_flag==True: # 未割り当て荷物最適化の場合は予約車両を除外する
		oVehicles = [v for v in oVehicles if v.reserved_v_flag == False]

	for v in oVehicles:
		if not v.direct_flag:
			if oArgvs.delaystart_flag and v.delay_arr!=[]:
				if oArgvs.lastc_flag == True:
					d_i = v.delay_totalcost_wolast
				else:
					d_i = v.delay_totalcost
			else:
				if oArgvs.lastc_flag == True:
					d_i = v.totalcost_wolast
				else:
					d_i = v.totalcost

			if len(v.route) > 2: #長さ2より大きなルートのみカウント(配送先のないルートは未成立のためAPE算出に用いない)
				d_i_list.append([v.vehno, d_i])

	# APE算出
	mu = np.mean([val[1] for val in d_i_list])
	for val in (d_i_list):
		t_i = val[1]
		APE = np.abs(d_i-mu)/mu
		# APE = |d_i - μ|/μ
		# APEが指定誤差内であることを確認
		print(test_no, "vehno:", val[0], ": APE:",APE)
		assert APE <= threshold + epsilon

	return APE

def SoftCheck_avedist_AE(test_no, oVehicles, oArgvs, threshold, epsilon):
	AE = 0
	dist_veh = []

	if oArgvs.opt_unassigned_flag==True: # 未割り当て荷物最適化の場合は予約車両を除外する
		oVehicles = [v for v in oVehicles if v.reserved_v_flag == False]

	if oArgvs.lastc_flag:
		# 対象車両以外の移動距離
		for v in oVehicles:
			if len(v.route) >= 3 and v.direct_flag==False:
				if oArgvs.delaystart_flag and v.delay_arr!=[]:
					dist_veh.append(v.delay_totalcost_wolast)
				else:
					dist_veh.append(v.totalcost_wolast)
	else:
		# 対象車両以外の走行時間
		for v in oVehicles:
			if len(v.route) >= 3 and v.direct_flag==False:
				if oArgvs.delaystart_flag and v.delay_arr!=[]:
					dist_veh.append(v.delay_totalcost)
				else:
					dist_veh.append(v.totalcost)

	# AE算出
	for custs in combinations(dist_veh, 2):
		diff = math.fabs(custs[0] - custs[1])
		AE += max(diff-oArgvs.avedist_ae, 0)	#avedist_aeに収まっている場合は0とする

	assert AE <= threshold + epsilon

	return AE

# ---------------------------------------------------------------------
# ソフト制約
# 制約：積載量制約(2種類)
# 内容：車両積載量違反
# メモ：
# ---------------------------------------------------------------------
def SoftCheck_LoadCapacity(test_no, oArgvs, oVehicles):
	return


# ---------------------------------------------------------------------
# 配送計画出力要件
# 要件：拠点が先頭に必ず現れる
# ---------------------------------------------------------------------
def PlacCheck_TopBase(test_no, oArgvs):
	if oArgvs==None:
		return

	log_filename = log_dir + oArgvs.outputfile.split('.csv')[0] + '_'+str(test_no)+'.csv'
	# 1番目と最後の配送先のみが0となっていることを確認
	csv_reader=csv.reader(open(log_filename,"r"),delimiter=",",quotechar='"')
	for row in csv_reader:
		if not "echarge" in row[0] and  not "eremarr" in row[0] and not row[1]=="Id":
			assert row[1]=="0" #拠点が先頭に必ず現れる

	print(test_no + ":要件：拠点が先頭に必ず現れる")

	return

# ---------------------------------------------------------------------
# 配送計画出力要件
# 要件：拠点が先頭と最後以外に現れない
# ---------------------------------------------------------------------
def PlacCheck_TopEndExceptBase(test_no, oVehicles, oArgvs):
	if oArgvs==None or oArgvs.multitrip_flag==True: #回転機能の場合はチェックしない
		return

	# if test_no=="29_1_19" or \
	# 	test_no=="43_1_1" or test_no=="43_1_5" or test_no=="43_1_6" or \
	# 	test_no=="43_1_9" or test_no=="43_1_10" or test_no=="43_1_11" or \
	# 	test_no=="43_1_12" or test_no=="43_1_14" or test_no=="43_1_15" or \
	# 	test_no=="43_1_16" or test_no=="43_1_17" or test_no=="43_1_18" or \
	# 	test_no=="43_1_25" or test_no=="43_1_26" or test_no=="43_1_27": #集荷最適化の場合はチェックしない
	# 	return

	log_filename = log_dir + oArgvs.outputfile.split('.csv')[0] + '_'+str(test_no)+'.csv'
	# 1番目と最後の配送先のみが0となっていることを確認
	csv_reader=csv.reader(open(log_filename,"r"),delimiter=",",quotechar='"')
	vidx=0
	for row in csv_reader:
		if not "echarge" in row[0] and  not "eremarr" in row[0] and not row[1]=="Id":
			last_idx = main_tw.get_last_idx(oVehicles[vidx], oArgvs)
			assert not "0" in row[2:last_idx] #拠点が先頭と最後以外に現れない
			vidx += 1
	print(test_no + ":要件：拠点が先頭と最後以外に現れない")

	return

# ---------------------------------------------------------------------
# 配送計画出力要件
# 要件：各車両の最後は、最終訪問先が指定されている場合、該当最終訪問先となる
# ---------------------------------------------------------------------
def PlacCheck_EndVehicleSpecificationEndSpot(test_no, oEndSpots, oArgvs ):
	if oEndSpots==None or oArgvs==None:
		return
	
	# 最終訪問先が指定されていない場合は判定を行わない
	# 最終訪問先が無い場合、oEndSpotsのカスタマー番号は0が指定されている
	# ★★今回(2021/5E向け)のテストでは必ず最終訪問先へ0を指定するため、下記判定は行わない
	#for cust in oEndSpots:
	#		if cust['custno'] != '0':
	#		break
	#else:
	#		return

	# 最終訪問先(oEndSpotsが持つビークル番号(vehno)のカスタマー番号)がoutput.csvの最終訪問先と一致することを確認
	# 例) oEndSpotsが持つビークル番号が3、カスタマー番号が156の場合
	#     output.csvの中のroute3の最終地点のカスタマー番号が156となることを確認する

	log_filename = log_dir + oArgvs.outputfile.split('.csv')[0] + '_'+str(test_no)+'.csv'

	#車両削減が有効の場合
	#車両削減により全てのEndSpotへ到達するとは限らない
	#EndSpot候補を集計し、ルートの最終拠点がこの候補に含まれるのかを確認する。
	EndSpots_list = []
	for end_spot in oEndSpots:
		EndSpots_list.append(end_spot['custno'])

	for end_spot in oEndSpots:
		end_spot_vehno = end_spot['vehno']

		csv_reader=csv.reader(open(log_filename,"r"),delimiter=",",quotechar='"')
		for row in csv_reader:
			if not "echarge" in row[0] and  not "eremarr" in row[0] and not row[1]=="Id":
				route_vehno = row[0].split('route')[1] #ログファイルからビークル番号取得
				if end_spot_vehno == route_vehno:
					if oArgvs.decr_vnum == False:
						assert end_spot['custno'] == row[-1]
					else:
						assert row[-1] in EndSpots_list

	print(test_no + ":要件：各車両の最後は、最終訪問先が指定されている場合、該当最終訪問先となる")

	return

# ---------------------------------------------------------------------
# 配送計画出力要件
# 要件：各車両の最後は、最終訪問先が指定されていない場合、拠点となる
# ---------------------------------------------------------------------
def PlacCheck_EndVehicleEndSpot(test_no, oEndSpots, oArgvs ):
	if oEndSpots==None or oArgvs==None:
		return

	# 最終訪問先が指定されている場合判定を行わない
	if len(oEndSpots) > 0:
		return

	log_filename = log_dir + oArgvs.outputfile.split('.csv')[0] + '_'+str(test_no)+'.csv'

	# 1番目と最後の配送先のみが0となっていることを確認
	csv_reader=csv.reader(open(log_filename,"r"),delimiter=",",quotechar='"')
	for row in csv_reader:
		if not "echarge" in row[0] and  not "eremarr" in row[0] and not row[1]=="Id":
				assert row[-1]=="0" #各車両の最後は、最終訪問先が指定されていない場合、拠点となる

	return

# ---------------------------------------------------------------------
# 配送計画出力要件
# 要件：充電解作成の場合、同じ充電スポットが連続して現れない
# ---------------------------------------------------------------------
def PlacCheck_NonContinuous(test_no, oVehicles, oArgvs):
	if oVehicles==None or oArgvs==None:
		return
	if oArgvs.evplan == False:
		return

	log_filename = log_dir + oArgvs.outputfile.split('.csv')[0] + '_'+str(test_no)+'.csv'

	# 同じ充電スポットが連続して現れないことを確認
	csv_reader=csv.reader(open(log_filename,"r"),delimiter=",",quotechar='"')
	for row in csv_reader:
		if not "echarge" in row[0] and  not "eremarr" in row[0] and not row[1]=="Id":
			espot = ""
			for spot in row[1:]:
				# 充電スポットの場合、直前の配送先が同じ充電スポットでないことを確認
				if spot.startswith("e"):
					assert spot != espot
					espot = spot
				else:
					espot = ""

	return

# ---------------------------------------------------------------------
# 配送計画出力要件
# 充電解作成の場合、充電スポットが2回以上現れない
# ---------------------------------------------------------------------
def PlacCheck_ChargeSpotOne(test_no, oVehicles, oArgvs):
	if oVehicles==None or oArgvs==None:
		return
	if oArgvs.evplan == False:
		return

	log_filename = log_dir + oArgvs.outputfile.split('.csv')[0] + '_'+str(test_no)+'.csv'

	# 充電スポットが2回以上現れないことを確認
	csv_reader=csv.reader(open(log_filename,"r"),delimiter=",",quotechar='"')
	for row in csv_reader:
		if not "echarge" in row[0] and  not "eremarr" in row[0] and not row[1]=="Id":
			espot_cnt = 0
			for spot in row[1:]:
				if spot.startswith("e"):
					espot_cnt += 1
				# 充電スポットが一度現れた場合、他に充電スポットが現れないことを確認
				assert espot_cnt < 2

	return

# ---------------------------------------------------------------------
# 配送計画出力要件
# 要件：配送先全てが先頭と最後以外に1回のみ過不足なく現れる
# (拠点、充電スポット、最終訪問先はチェック対象外)
# ---------------------------------------------------------------------
def PlacCheck_AllDeliveryOnce(test_no, oNormalCustomer, oEndSpots, oVehicles, oArgvs):
	if oNormalCustomer==None or oArgvs==None:
		return

	spots=[]
	log_filename = log_dir + oArgvs.outputfile.split('.csv')[0] + '_'+str(test_no)+'.csv'

	for cust in oNormalCustomer:
		if not cust['custno']=="0":
			spots.append(cust['custno'])

	for v in oVehicles:
		for t in v.takebreak:
			spots.append(t.custno)

	csv_reader=csv.reader(open(log_filename,"r"),delimiter=",",quotechar='"')
	for row in csv_reader:
		if not row[1]=="Id":
			for i in range(1,len(row)-1):
				if not "echarge" in row[0] and  not "eremarr" in row[0] and not row[1]=="Id":
					if row[i]=="0" or "e" in row[i] or "b" in row[i]:
						continue
					for cust in oNormalCustomer:
						if row[i] in spots:
							spots.remove(row[i])
							break

						for addcust in cust['additionalcust']:
							if row[i]==addcust[0]:
								if cust['custno'] in spots:
									spots.remove(cust['custno'])
								break
						else:
							continue
						break
					else:
						if test_no.startswith("46_1_") or test_no=="50_1_90": #荷物自動分割の場合はチェックしない
							continue
						for c in oNormalCustomer:
							if c['custno']==row[i] and c['demdivideunit']!=1: #荷物自動分割の場合はチェックしない
								break
						else:
							raise ValueError(row[i]+" is not included in the input file")
	if test_no=="49_1_28" or test_no=="50_1_90":
		return

	if oArgvs.opt_unassigned_flag == True: #荷物最適化ならば荷物が除外されることが有る
		return

	spots = [s for s in spots if s[0]!="b"]
	assert not spots

	print(test_no + ":要件：配送先全てが先頭と最後以外に1回のみ過不足なく現れる")

	return


# ---------------------------------------------------------------------
# 入力パラメータ整合性テスト(Argvs)
# テスト後のVehicle配列数とArgvs.ivnum(台数)が一致することを確認
# ※routeサイズが3未満は空ルートのためカウントしない
# ---------------------------------------------------------------------
def ParamCheck_Argvs_ivnum(test_no, oVehicles, oNormalCusts, oArgvs, ivnum):
	if oVehicles==None or oArgvs==None:
		return

	if oArgvs.opt_unassigned_flag==True: # 未割り当て荷物最適化の場合はテストしない
		return

	# 車両数を設定
	# カスタマー数が車両数より少ない場合、車両全てにカスタマーを配置することはできない
	# その場合、in_ivnumはカスタマーを乗せることができる車両数の台数として扱う
	cost_num = len(oNormalCusts) - 1
	in_ivnum = min(ivnum, cost_num)

	out_ivnum = 0
	for v in oVehicles:

		if oArgvs.decr_vnum or oArgvs.opt_unassigned_flag:
			return
			# if len(v.route) >= 2: #長さ2未満のルートはルートとして未成立(拠点のみ)となるためカウントしない(台数削減モードONの場合は車両なし(routeが拠点のみの２箇所)もルートとしてカウントする)
				# out_ivnum += 1
		else:
			if len(v.route) >= 3: #長さ3未満のルートはルートとして未成立(拠点のみ)となるためカウントしない
				out_ivnum += 1


	assert in_ivnum == out_ivnum

	return

# ---------------------------------------------------------------------
# 入力パラメータ整合性テスト(Argvs)
# 1台の配送時間の下限の時間がlowertimeを下回らないことを確認
# 拠点に戻らない場合：TOTALTIME_TO_LAST
# 拠点に戻る場合：TOTALTIME_TO_DEPO
# ---------------------------------------------------------------------
def ParamCheck_Argvs_lowertime(test_no, oVehicles, oArgvs):
	if oVehicles==None or oArgvs==None:
		return

	if oArgvs.lowertime > 0: # lowertimeが有効な値(0以外)の場合のみ判定する

		if oArgvs.opt_unassigned_flag==True: # 未割り当て荷物最適化の場合は予約車両を除外する
			oVehicles = [v for v in oVehicles if v.reserved_v_flag == False]

		for v in oVehicles:
			last_idx = main_tw.get_last_idx(v, oArgvs)
			if oArgvs.delaystart_flag and v.delay_arr!=[]:
				time_wolast = v.delay_lv[last_idx]-v.delay_arr[0]
				time = v.delay_lv[-1]-v.delay_arr[0]
			else:
				time_wolast = v.lv[last_idx]-v.arr[0]
				time = v.lv[-1]-v.arr[0]

			if oArgvs.lastc_flag == True:
				#拠点を出発し最後の配送先まで
				assert time_wolast >= oArgvs.lowertime
			else:
				#拠点を出発し最後の配送先での配送を終えて拠点に戻るまで
				assert time >= oArgvs.lowertime

	return


# ---------------------------------------------------------------------
# 入力パラメータ整合性テスト(Argvs)
# 1台の配送時間の上限の時間がuppertimeを上回らないことを確認
# 拠点に戻らない場合：TOTALTIME_TO_LAST
# 拠点に戻る場合：TOTALTIME_TO_DEPO
# ---------------------------------------------------------------------
def ParamCheck_Argvs_uppertime(test_no, oVehicles, oArgvs):
	if oVehicles==None or oArgvs==None:
		return

	if oArgvs.uppertime > 0: # uppertimeが有効な値(0以外)の場合のみ判定する

		if oArgvs.opt_unassigned_flag==True: # 未割り当て荷物最適化の場合は予約車両を除外する
			oVehicles = [v for v in oVehicles if v.reserved_v_flag == False]

		for v in oVehicles:
			last_idx = main_tw.get_last_idx(v, oArgvs)
			if oArgvs.delaystart_flag and v.delay_arr!=[]:
				time_wolast = v.delay_lv[last_idx]-v.delay_arr[0]
				time = v.delay_lv[-1]-v.delay_arr[0]
			else:
				time_wolast = v.lv[last_idx]-v.arr[0]
				time = v.lv[-1]-v.arr[0]

			if oArgvs.lastc_flag == True:
				#拠点を出発し最後の配送先まで
				assert time_wolast <= oArgvs.uppertime
			else:
				#拠点を出発し最後の配送先での配送を終えて拠点に戻るまで
				assert time <= oArgvs.uppertime

	return

# ---------------------------------------------------------------------
# 入力パラメータ整合性テスト(Argvs)
# 集荷有無の判定フラグの確認
# pickup_flag  True:集荷あり False:集荷なし
# ---------------------------------------------------------------------
def ParamCheck_Argvs_pickup_flag(test_no, oNormalCustomer, oArgvs):
	if oNormalCustomer==None or oArgvs==None:
		return

	pickup_flag = False

	for c in oNormalCustomer[1:]:
		if c['requestType'] == "PICKUP":
			pickup_flag = True

	assert pickup_flag == oArgvs.pickup_flag

	return

# ---------------------------------------------------------------------
# 入力パラメータ整合性テスト(Vehicle)
# 入力車両番号(vehno)が実行結果のルートに存在することを確認
# ---------------------------------------------------------------------
def ParamCheck_Vehicle_vehno(test_no, oVehicles, oInVehicles, oNormalCusts, oArgvs, ivnum):
	if oVehicles==None or oInVehicles==None:
		return

	# 本要件を見たなさいテスト番号は常に正常終了する
	if test_no == '4_1_12' or test_no == '16_1_1' or test_no == '16_1_6'\
			or test_no == '16_1_7' or test_no == '16_1_8' or test_no == '16_1_9'\
			or test_no == '16_1_10'	or test_no == '19_1_23' or test_no == '19_1_24'\
			or test_no == '21_1_20'	or test_no == '23_1_6' or test_no == '25_2_4'\
			or test_no == '27_1_32' or test_no == '31_1_65' or test_no == '33_1_19'\
			or test_no == '36_1_10' or test_no == '36_1_23' or test_no == '4_1_27'\
			or test_no == '16_1_11' or test_no == '38_1_12' or test_no == '38_1_26'\
			or test_no == '15_1_20' or test_no == '15_1_21' or test_no == '41_1_13'\
			or test_no == '45_1_16' or test_no == '45_1_24' or test_no == '46_1_23'\
			or test_no == '47_1_17' or test_no == '47_1_27' or test_no == '48_1_3'\
			or test_no == '48_1_4' or test_no == '48_1_5' or test_no == '48_1_6'\
			or test_no == '48_1_7' or test_no == '48_1_11' or test_no == '48_1_14'\
			or test_no == '49_1_18' or test_no == '17_1_17' or test_no == '44_1_27'\
			or test_no == '50_1_52'	: 
		return

	# 本要件を見たなさいテスト番号は常に正常終了する（ランダムテスト）
	if 'random' in test_no and oArgvs.decr_vnum or oArgvs.opt_unassigned_flag:
		return

	# 空車両数を算出
	# カスタマー数が車両数より少ない場合、車両全てにカスタマーを配置することはできない
	# その場合、空となる車両数を許容するため空車両数を算出
	if oArgvs.bulkShipping_flag:
		cost_num = 0
		for custs in oNormalCusts:
			cost_num += len(custs.customers)
		cost_num -= 1
	else:
		cost_num = len(oNormalCusts) - 1
	if ivnum > cost_num:
		emp_vnum = ivnum - cost_num
	else:
		emp_vnum = 0

	#入力指定された車両番号を取得
	vno_list = [v["vehno"] for v in oInVehicles]

	if oArgvs.opt_unassigned_flag==True: # 未割り当て荷物最適化の場合は予約車両を除外する
		oVehicles = [v for v in oVehicles if v.reserved_v_flag == False]

	for v in oVehicles:
		if oArgvs.decr_vnum:
			if len(v.route) >= 2 and v.rmpriority!=0: #長さ3未満のルートはルートとして未成立(拠点のみ)となるためカウントしない(台数削減モードONの場合は車両なし(routeが拠点のみの２箇所)もルートとしてカウントする)
				if v.vehno in vno_list:
					vno_list.remove(v.vehno)

		if len(v.route) >= 3: #長さ3未満のルートはルートとして未成立(拠点のみ)となるためカウントしない
			if v.vehno in vno_list:
				vno_list.remove(v.vehno)

	assert len(vno_list) <= emp_vnum #未割当の車両を確認。カスタマー数が車両より少ない場合、未割当の車両(len(vno_list)が空車両数(emp_vnum)より少ない場合エラーとする

	return


# ---------------------------------------------------------------------
# 入力パラメータ整合性テスト(経過時間)
# ルート内の下記経過時間が正しく算出されていることを確認
#  - 配送先の受け入れ開始時刻 ready_fs
#  - 配送先の受け入れ終了時刻 due_fs
# ---------------------------------------------------------------------
def ParamCheck_ready_due_fs(test_no, oVehicles, oArgvs):

	if oVehicles==None or oArgvs==None:
		return

	base_start = oArgvs.starttime
	base_end = oArgvs.endtime

	# 経過時間算出(Argv指定時刻ベース)
	#開始時刻文字列をdatetime型に変換
	if len(base_start) < 6:
		base_st=base_start.split(":")
		base_st=datetime.datetime(2000,1,1,int(base_st[0]),int(base_st[1]),00)

	else:
		base_st=datetime.datetime.strptime(base_start, '%Y/%m/%d %H:%M:%S')
	#終了時刻文字列をdatetime型に変換
	if len(base_end) < 6:
		base_ed=base_end.split(":")
		base_ed=datetime.datetime(2000,1,1,int(base_ed[0]),int(base_ed[1]),00)
	else:
		base_ed=datetime.datetime.strptime(base_end, '%Y/%m/%d %H:%M:%S')

	if test_no != '16_1_7' and test_no != '15_1_21' and test_no != '45_1_24':
		assert oVehicles[0].startoptime == oArgvs.starttime
		assert oVehicles[0].endoptime == oArgvs.endtime

	for v in oVehicles:
		# 配送時刻の一致を確認
		# VehicleとArgvで配送時刻が不一致の入力は未保証としassertとする

		for c in v.route:
			if oArgvs.bulkShipping_flag:
				for custs in c.__dict__["customers"]:
					start = custs.ready
					end = custs.due

					# 経過時間算出
					#開始時刻文字列をdatetime型に変換
					if len(start) < 6:
						st=start.split(":")
						st=datetime.datetime(2000,1,1,int(st[0]),int(st[1]),00)

					else:
						st=datetime.datetime.strptime(start, '%Y/%m/%d %H:%M:%S')
					#終了時刻文字列をdatetime型に変換
					if len(end) < 6:
						ed=end.split(":")
						ed=datetime.datetime(2000,1,1,int(ed[0]),int(ed[1]),00)
					else:
						ed=datetime.datetime.strptime(end, '%Y/%m/%d %H:%M:%S')

					#経過時間算出
					ready_fs = st - base_st
					ready_fs = (ready_fs.total_seconds())/3600.0

					due_fs = ed - base_st
					due_fs = (due_fs.total_seconds())/3600.0

					# 実数換算値妥当性比較
					assert ready_fs == c.ready_fs
					assert due_fs == c.due_fs
			else:
				start = c.ready
				end = c.due

				# 経過時間算出
				#開始時刻文字列をdatetime型に変換
				if len(start) < 6:
					st=start.split(":")
					st=datetime.datetime(2000,1,1,int(st[0]),int(st[1]),00)

				else:
					st=datetime.datetime.strptime(start, '%Y/%m/%d %H:%M:%S')
				#終了時刻文字列をdatetime型に変換
				if len(end) < 6:
					ed=end.split(":")
					ed=datetime.datetime(2000,1,1,int(ed[0]),int(ed[1]),00)
				else:
					ed=datetime.datetime.strptime(end, '%Y/%m/%d %H:%M:%S')

				#経過時間算出
				ready_fs = st - base_st
				ready_fs = (ready_fs.total_seconds())/3600.0

				due_fs = ed - base_st
				due_fs = (due_fs.total_seconds())/3600.0

				# 実数換算値妥当性比較
				assert ready_fs == c.ready_fs
				assert due_fs == c.due_fs

	return

# ---------------------------------------------------------------------
# ハード制約（コマンドライン実行）
# 制約：車両訪問数上限 
# 内容：車両1台の訪問先件数の制限
# メモ：訪問先件数はシステム内で0初期化されているため未設定の場合、maxvisit==0として考える
# ---------------------------------------------------------------------
def CommandLine_HardCheck_MaxNumVehicleVisits(test_no, args, path):
	if not "-maxvisit" in args: 
		return

	# 訪問数算出
	def calc_spot_len(route):
		spot_list=[]
		before_spotid = None
		for spotid in route[1:-1]:
			# 訪問先カウント。下記はカウントしない
			# 充電スポット('e'を含む)、拠点('0')、前回と同じスポット(before_spotno == spotno)
			if not 'e' in spotid and spotid != '0' and before_spotid != spotid:
				spot_list.append(spotid)
				before_spotid = spotid

		return len(spot_list)

	csv_reader=csv.reader(open(args["-maxvisit"][args["-maxvisit"].find("d"):],"r"),delimiter=",",quotechar='"')
	maxvisit = {}
	for row in csv_reader:
		if not "VEHICLE" in row[0]:
			maxvisit[row[0]] = int(row[1])

	csv_reader=csv.reader(open(os.path.join(path,"output_"+test_no+".csv"),"r"),delimiter=",",quotechar='"')
	for row in csv_reader:
		if not "echarge" in row[0] and  not "eremarr" in row[0] and not row[1]=="Id":
			vehno = row[0].split('route')[1] #ログファイルからビークル番号取得
			visit_len = calc_spot_len(row)
			if vehno in maxvisit and maxvisit[vehno]>0:
				assert visit_len <= maxvisit[vehno]

	return

# ---------------------------------------------------------------------
# ハード制約（コマンドライン実行）
# 制約：回転数上限 
# 内容：車両1台の回転数の制限
# メモ：回転数はシステム内で0初期化されているため未設定の場合、maxrotate==0として考える
# ---------------------------------------------------------------------
def CommandLine_HardCheck_MaxNumVehicleRotates(test_no, args, path):
	if not "-mtv" in args: 
		return

	# 回転数算出
	def calc_rotate_num(route):
		num=0
		
		for r in route[1:-2]:
			if r=="0":
				num+=1

		return num

	csv_reader=csv.reader(open(args["-mtv"][args["-mtv"].find("d"):],"r"),delimiter=",",quotechar='"')
	maxrotate= {}
	for row in csv_reader:
		if not "VEHICLE" in row[0]:
			maxrotate[row[0]] = int(row[1])

	csv_reader=csv.reader(open(os.path.join(path,"output_"+test_no+".csv"),"r"),delimiter=",",quotechar='"')
	for row in csv_reader:
		if not "echarge" in row[0] and  not "eremarr" in row[0] and not row[1]=="Id":
			vehno = row[0].split('route')[1] #ログファイルからビークル番号取得
			rotate_num = calc_rotate_num(row)
			if vehno in maxrotate and maxrotate[vehno]>0:
				assert rotate_num <= maxrotate[vehno]

	return

# ---------------------------------------------------------------------
# ハード制約（コマンドライン実行）
# 制約：通行不可制約
# 内容：移動経路無し
# (※注)
# get_dtableは単体テスト実施済みのため、本APIテスト内で利用する
# get_ttableは単体テスト実施済みのため、本APIテスト内で利用する
# get_xtableは単体テスト実施済みのため、本APIテスト内で利用する
# get_etableは単体テスト実施済みのため、本APIテスト内で利用する
# ---------------------------------------------------------------------
def CommandLine_HardCheck_Impassable(test_no, args, path):
	# spotidを求める
	def get_spotid(custno):
		id_csv=csv.reader(open(args["-si"][args["-si"].find("d"):],"r"),delimiter=",",quotechar='"')
		for row in id_csv:
			if row[0] == custno:
				return row[1]

		if "-addtwspot" in args:
			id_csv=csv.reader(open(args["-addtwspot"][args["-addtwspot"].find("d"):],"r"),delimiter=",",quotechar='"')
			for row in id_csv:
				if row[1] == custno:
					return row[4]

		if "-br" in args:
			id_csv=csv.reader(open(args["-br"][args["-br"].find("d"):],"r"),delimiter=",",quotechar='"')
			for row in id_csv:
				if row[1] == custno:
					return row[4]

		if "-addbrspot" in args:
			id_csv=csv.reader(open(args["-addbrspot"][args["-addbrspot"].find("d"):],"r"),delimiter=",",quotechar='"')
			for row in id_csv:
				if row[1] == custno:
					return row[4]

		raise ValueError("Cust({}) does not have spotid.".format(custno))
	
	# 時刻をUNIX時間に変換
	def get_time(t):
		return datetime.datetime.strptime(t, '%Y/%m/%d %H:%M:%S').timestamp()
	
	# 通行不可チェック
	def impassable_check(args, table_type):
		table = []
		management_table = []
		start = get_time(args["-s"])
		table.append([start, args["-"+table_type]])
		management_table.append([start, args["-"+table_type]])
		if "-"+table_type+"s" in args: 
			args["-"+table_type+"s"] = path + "/" + table_type+ "s_" + test_no + ".csv"
			csv_reader=csv.reader(open(args["-"+table_type+"s"],"r"),delimiter=",",quotechar='"')
			for row in csv_reader:
				if not "START_TIME" in row[0]:
					table.append([get_time(row[0]), row[1]])
			management_table=[table for table in sorted(table,key=lambda x:(x[0]))]
		elif table_type == "ec" and "-"+table_type+"sv" in args:
			table[0] = ["", "", start, args["-"+table_type]]
			management_table[0] = ["", "", start, args["-"+table_type]]
			args["-"+table_type+"sv"] = path + "/" + table_type + "sv_" + test_no + ".csv"
			csv_reader=csv.reader(open(args["-"+table_type+"sv"],"r"),delimiter=",",quotechar='"')
			for row in csv_reader:
				if not "VEHICLE_TYPE" in row[0]:
					table.append([row[0], row[1], get_time(row[2]), row[3]])
			management_table=[table for table in sorted(table,key=lambda x:(x[0], x[1], x[2]))]
		elif "-"+table_type+"sv" in args: 
			table[0] = ["", start, args["-"+table_type]]
			management_table[0] = ["", start, args["-"+table_type]]
			args["-"+table_type+"sv"] = path + "/" + table_type + "sv_" + test_no + ".csv"
			csv_reader=csv.reader(open(args["-"+table_type+"sv"],"r"),delimiter=",",quotechar='"')
			for row in csv_reader:
				if not "VEHICLE_TYPE" in row[0]:
					table.append([row[0], get_time(row[1]), row[2]])
			management_table=[table for table in sorted(table,key=lambda x:(x[0], x[1]))]

		# 時間別
		if "-"+table_type in args and not "-"+table_type+"sv" in args: 
			csv_reader=csv.reader(open(os.path.join(path,"output_"+test_no+".csv.detail.csv"),"r"),delimiter=",",quotechar='"')
			lv_csv=csv.reader(open(args["-i"][args["-i"].find("d"):],"r"),delimiter=",",quotechar='"')
			veh_flag = False
			for row in csv_reader:
				if "VEHICLE" in row[0] and row:
					veh_flag = True
					route = row
				if "ARRIVAL" in row[0] and veh_flag:
					for i in range(4, len(row)-1):
						servt = 0
						for lv_row in lv_csv:
							if route[i] == lv_row[0]:
								servt = lv_row[6]
								break
						# 配送先出発時刻
						lv = get_time(args["-s"]) + float(row[i]) * 3600 + float(servt) * 60
						for table_time in reversed(management_table):
							assert_flag = False
							# start_timeからテーブル選択（start_time<=出発時刻 or common）
							if table_time[0] <= lv or table_time == management_table[0]:
								id0 = get_spotid(route[i])
								id1 = get_spotid(route[i+1])
								if id0 == id1:
									assert_flag = True
									break
								csv_table = csv.reader(open(table_time[1][table_time[1].find("d"):],"r"),delimiter=",",quotechar='"')
								# 配送区間の時間
								for table in csv_table:
									# 配送区間の距離
									if table[0] == id0 and table[1] == id1:
										assert float(table[2]) != -1
										assert_flag = True
										break
							if assert_flag:
								break
						if not assert_flag:
							raise ValueError("[{}, {}] is not included in the {}table.".format(route[i], route[i+1], table_type))
					veh_flag = False
					assert_flag = False
		# 時間別車両別
		elif "-"+table_type+"sv" in args: 
			csv_reader=csv.reader(open(os.path.join(path,"output_"+test_no+".csv.detail.csv"),"r"),delimiter=",",quotechar='"')
			lv_csv=csv.reader(open(args["-i"][args["-i"].find("d"):],"r"),delimiter=",",quotechar='"')
			veh_flag = False
			for row in csv_reader:
				if "VEHICLE" in row[0] and row:
					vehno = row[1]
					vtype = ""
					etype = ""
					if "-vs" in args: 
						vs_csv=csv.reader(open(args["-vs"][args["-vs"].find("d"):],"r"),delimiter=",",quotechar='"')
						for v in vs_csv:
							if not "VEHICLE NO." in v[0] and v[0] == vehno:
								vtype = v[6]
								etype = v[7]
					veh_flag = True
					route = row
				if "ARRIVAL" in row[0] and veh_flag:
					for i in range(4, len(row)-1):
						servt = 0
						for lv_row in lv_csv:
							if route[i] == lv_row[0]:
								servt = lv_row[6]
								break
						# 配送先出発時刻
						lv = get_time(args["-s"]) + float(row[i]) * 3600 + float(servt) * 60
						for table_time in reversed(management_table):
							assert_flag = False
							if table_type == "ec":
								# start_timeからテーブル選択（（vtype==車両番号 etype==車両番号 and start_time<=出発時刻） or common）
								if table_time[0] == vtype and table_time[1] == etype and table_time[2] <= lv or table_time == management_table[0]:
									id0 = get_spotid(route[i])
									id1 = get_spotid(route[i+1])
									if id0 == id1:
										assert_flag = True
										break
									csv_table = csv.reader(open(table_time[3][table_time[3].find("d"):],"r"),delimiter=",",quotechar='"')
									# 配送区間の時間
									for table in csv_table:
										# 配送区間の距離
										if table[0] == id0 and table[1] == id1:
											assert float(table[2]) != -1
											assert_flag = True
											break
							else:
								# start_timeからテーブル選択（（vtype==車両番号 and start_time<=出発時刻） or common）
								if table_time[0] == vtype and table_time[1] <= lv or table_time == management_table[0]:
									id0 = get_spotid(route[i])
									id1 = get_spotid(route[i+1])
									if id0 == id1:
										assert_flag = True
										break
									csv_table = csv.reader(open(table_time[2][table_time[2].find("d"):],"r"),delimiter=",",quotechar='"')
									# 配送区間の時間
									for table in csv_table:
										# 配送区間の距離
										if table[0] == id0 and table[1] == id1:
											assert float(table[2]) != -1
											assert_flag = True
											break
							if assert_flag:
								break
						if not assert_flag:
							raise ValueError("[{}, {}] is not included in the {}table.".format(route[i], route[i+1], table_type))
					veh_flag = False
					assert_flag = False

	# 距離テーブルが-1となるカスタマー間は通行不可
	# 配送経路から上記違反がないのかを確認する
	impassable_check(args, "d")

	# 時間テーブルが-1となるカスタマー間は通行不可
	# 配送経路から上記違反がないのかを確認する
	impassable_check(args, "t")

	# 電費テーブルが-1となるカスタマー間は通行不可
	# 配送経路から上記違反がないのかを確認する
	if "-ec" in args: 
		impassable_check(args, "ec")

	# 経験コストテーブルが-1となるカスタマー間は通行不可
	# 配送経路から上記違反がないのかを確認する
	if "-x" in args: 
		impassable_check(args, "x")
	return

# ---------------------------------------------------------------------
# ハード制約（コマンドライン実行）
# 制約：車両立寄不可制約
# 内容：配送先へ訪問不可
# ---------------------------------------------------------------------
def CommandLine_HardCheck_VehicleStopOff(test_no, args, path):
	if not "-rej" in args: 
		return

	csv_reader=csv.reader(open(args["-rej"][args["-rej"].find("d"):],"r"),delimiter=",",quotechar='"')
	reject_custs = []
	reject_vehicles = []
	for row in csv_reader:
		if not "CUST NO." in row[0]:
			reject_custs.append(row[0])
			reject_vehicles.append(row[1:])

	# Vehiclルートの中に、配送できないカスタマー番号(reje_ve)が存在するかを確認
	# 存在する場合エラーとする
	csv_reader=csv.reader(open(os.path.join(path,"output_"+test_no+".csv"),"r"),delimiter=",",quotechar='"')
	for row in csv_reader:
		if not "echarge" in row[0] and  not "eremarr" in row[0] and not row[1]=="Id":
			vehno = row[0].split('route')[1] #ログファイルからビークル番号取得
			for cust in row[1:]:
				for i in range(len(reject_custs)):
					if reject_custs[i] == cust: # 立ち寄り制限のある配送先
						assert not vehno in reject_vehicles[i] # 車両番号が一致するか確認

	return

# ---------------------------------------------------------------------
# ハード制約（コマンドライン実行）
# 制約：充電スポット営業時間枠制約
# 内容：充電スポットの訪問時間違反
# ---------------------------------------------------------------------
def CommandLine_HardCheck_ChargingSpotTimeFrame(test_no, args, path):
	if not "-evplan" in args: 
		return

	csv_reader=csv.reader(open(args["-es"][args["-es"].find("d"):],"r"),delimiter=",",quotechar='"')
	e_spots = []
	e_due = []
	for row in csv_reader:
		if not "E-SPOT" in row[0]:
			e_spots.append(row[0])
			e_due.append(datetime.datetime.strptime(row[5], '%Y/%m/%d %H:%M:%S').timestamp())

	csv_reader=csv.reader(open(os.path.join(path,"output_"+test_no+".csv.detail.csv"),"r"),delimiter=",",quotechar='"')
	e_flag = False
	e_index = 0
	due = 0
	for row in csv_reader:
		if "VEHICLE" in row[0] and row:
			for i in range(len(row)):
				if row[i].startswith('e'):
					e_flag = True
					e_index = i
					for j in range(len(e_spots)):
						if e_spots[j] == row[i]:
							due = e_due[j]
		if "ARRIVAL" in row[0] and e_flag:
			# UNIX時間で比較
			arr = datetime.datetime.strptime(args["-s"], '%Y/%m/%d %H:%M:%S').timestamp() + float(row[e_index]) * 3600
			assert main_tw.compare_time(arr, due) <= 0
			e_flag = False

	return

# ---------------------------------------------------------------------
# ハード制約（コマンドライン実行）
# 制約：電動車両充電量制約
# 内容：電動車両の充電上限違反
# ---------------------------------------------------------------------
def CommandLine_HardCheck_EVChargeLimit(test_no, args, path):
	if not "-evplan" in args: 
		return

	csv_reader=csv.reader(open(args["-ev"][args["-ev"].find("d"):],"r"),delimiter=",",quotechar='"')
	e_capacity = []
	for row in csv_reader:
		if not "E-CAPACITY" in row[0]:
			for i in range(int(args["-v"])):
				e_capacity.append(int(row[0]))

	if "-vs" in args:
		csv_reader=csv.reader(open(args["-vs"][args["-vs"].find("d"):],"r"),delimiter=",",quotechar='"')
		for row in csv_reader:
			if not "VEHICLE" in row[0]:
				e_capacity[int(row[0])] = int(row[2])

	csv_reader=csv.reader(open(os.path.join(path,"output_"+test_no+".csv"),"r"),delimiter=",",quotechar='"')
	e_charge = []
	e_flag = False
	vehno = 0
	for row in csv_reader:
		if "echarge" in row[0]:
			e_charge = row
			e_flag = True
		if "eremarr" in row[0] and e_flag:
			for i in range(1, len(e_charge)):
				assert float(e_charge[i]) + float(row[i]) <= e_capacity[vehno]
			e_flag = False
			vehno += 1

	return

# ---------------------------------------------------------------------
# 配送計画出力要件（コマンドライン実行）
# 要件：拠点が先頭に必ず現れる
# ---------------------------------------------------------------------
def CommandLine_PlacCheck_TopBase(test_no, path):
	# 1番目の配送先が0となっていることを確認
	csv_reader=csv.reader(open(os.path.join(path,"output_"+test_no+".csv"),"r"),delimiter=",",quotechar='"')
	for row in csv_reader:
		if not "echarge" in row[0] and  not "eremarr" in row[0] and not row[1]=="Id":
			assert row[1] == "0" #拠点が先頭に必ず現れる

	print(test_no + ":要件：拠点が先頭に必ず現れる")

	return

# ---------------------------------------------------------------------
# 配送計画出力要件（コマンドライン実行）
# 要件：拠点が先頭と最後以外に現れない
# ---------------------------------------------------------------------
def CommandLine_PlacCheck_TopEndExceptBase(test_no, path):
	if test_no=="37_1_44" or test_no=="37_1_45" or test_no=="37_1_46": #回転機能の場合はチェックしない
		return

	if test_no=="37_1_13": #集荷最適化の場合はチェックしない
		return

	# 拠点が先頭と最後以外に現れないことを確認
	csv_reader=csv.reader(open(os.path.join(path,"output_"+test_no+".csv"),"r"),delimiter=",",quotechar='"')
	for row in csv_reader:
		if not "echarge" in row[0] and  not "eremarr" in row[0] and not row[1]=="Id":
			assert not "0" in row[2:-1] #拠点が先頭と最後以外に現れない

	print(test_no + ":要件：拠点が先頭と最後以外に現れない")

	return

# ---------------------------------------------------------------------
# 配送計画出力要件（コマンドライン実行）
# 要件：各車両の最後は、最終訪問先が指定されている場合、該当最終訪問先となる
# ---------------------------------------------------------------------
def CommandLine_PlacCheck_EndVehicleSpecificationEndSpot(test_no, args, path):
	if not "-vend" in args: 
		return

	csv_reader=csv.reader(open(args["-vend"][args["-vend"].find("d"):],"r"),delimiter=",",quotechar='"')
	EndSpots_list = {}
	for row in csv_reader:
		if not "VEHICLE" in row[0]:
			EndSpots_list[row[0]] = row[1]

	csv_reader=csv.reader(open(os.path.join(path,"output_"+test_no+".csv"),"r"),delimiter=",",quotechar='"')
	for row in csv_reader:
		if not "echarge" in row[0] and  not "eremarr" in row[0] and not row[1]=="Id":
			vehno = row[0].split('route')[1] #ログファイルからビークル番号取得			assert EndSpots_list[vehno] == row[-1]

	print(test_no + ":要件：各車両の最後は、最終訪問先が指定されている場合、該当最終訪問先となる")

	return

# ---------------------------------------------------------------------
# 配送計画出力要件（コマンドライン実行）
# 要件：各車両の最後は、最終訪問先が指定されていない場合、拠点となる
# ---------------------------------------------------------------------
def CommandLine_PlacCheck_EndVehicleEndSpot(test_no, args, path):
	if "-vend" in args: 
		return

	# 1番目と最後の配送先のみが0となっていることを確認
	csv_reader=csv.reader(open(os.path.join(path,"output_"+test_no+".csv"),"r"),delimiter=",",quotechar='"')
	for row in csv_reader:
		if not "echarge" in row[0] and  not "eremarr" in row[0] and not row[1]=="Id":
			assert row[-1] == "0" #各車両の最後は、最終訪問先が指定されていない場合、拠点となる

	return

# ---------------------------------------------------------------------
# 配送計画出力要件（コマンドライン実行）
# 要件：配送先全てが先頭と最後以外に1回のみ過不足なく現れる
# (拠点、充電スポット、最終訪問先はチェック対象外)
# ---------------------------------------------------------------------
def CommandLine_PlacCheck_AllDeliveryOnce(test_no, args, path):
	csv_reader=csv.reader(open(args["-i"][args["-i"].find("d"):],"r"),delimiter=",",quotechar='"')
	spots=[]
	for cust in csv_reader:
		if cust[0] != "CUST NO." and cust[0] != "0":
			spots.append(cust[0])

	if "-br" in args:
		csv_reader=csv.reader(open(args["-br"][args["-br"].find("d"):],"r"),delimiter=",",quotechar='"')
		for cust in csv_reader:
			if cust[0] != "VEHICLE NO.":
				spots.append(cust[1])

	csv_reader=csv.reader(open(os.path.join(path,"output_"+test_no+".csv"),"r"),delimiter=",",quotechar='"')
	for row in csv_reader:
		if not row[1]=="Id":
			for i in range(1,len(row)-1):
				if not "echarge" in row[0] and  not "eremarr" in row[0] and not row[1]=="Id":
					if row[i]=="0" or "e" in row[i]:
						continue
					if row[i] in spots:
						spots.remove(row[i])
					else:
						if test_no=="37_1_63" or test_no=="37_1_66": #荷物自動分割の場合はチェックしない
							continue

						if "-addtwspot" in args:
							addtwspot=csv.reader(open(args["-addtwspot"][args["-addtwspot"].find("d"):],"r"),delimiter=",",quotechar='"')
							for row2 in addtwspot:
								if row[i]==row2[1]:
									spots.remove(row2[0])
									break
							else:
								raise ValueError(row[i]+" is not included in the input file")
						else:
							raise ValueError(row[i]+" is not included in the input file")

	spots = [s for s in spots if s[0]!="b"]
	assert not spots

	print(test_no + ":要件：配送先全てが先頭と最後以外に1回のみ過不足なく現れる")

	return

# ---------------------------------------------------------------------
# 配送計画出力要件（コマンドライン実行）
# 要件：充電解作成の場合、同じ充電スポットが連続して現れない
# ---------------------------------------------------------------------
def CommandLine_PlacCheck_NonContinuous(test_no, args, path):
	if not "-evplan" in args: 
		return

	# 同じ充電スポットが連続して現れないことを確認
	csv_reader=csv.reader(open(os.path.join(path,"output_"+test_no+".csv"),"r"),delimiter=",",quotechar='"')
	for row in csv_reader:
		if not "echarge" in row[0] and  not "eremarr" in row[0] and not row[1]=="Id":
			espot = ""
			for spot in row[1:]:
				# 充電スポットの場合、直前の配送先が同じ充電スポットでないことを確認
				if spot.startswith("e"):
					assert spot != espot
					espot = spot
				else:
					espot = ""

	return

# ---------------------------------------------------------------------
# 配送計画出力要件（コマンドライン実行）
# 充電解作成の場合、充電スポットが2回以上現れない
# ---------------------------------------------------------------------
def CommandLine_PlacCheck_ChargeSpotOne(test_no, args, path):
	if not "-evplan" in args: 
		return

	# 充電スポットが2回以上現れないことを確認
	csv_reader=csv.reader(open(os.path.join(path,"output_"+test_no+".csv"),"r"),delimiter=",",quotechar='"')
	for row in csv_reader:
		if not "echarge" in row[0] and  not "eremarr" in row[0] and not row[1]=="Id":
			espot_cnt = 0
			for spot in row[1:]:
				if spot.startswith("e"):
					espot_cnt += 1
				# 充電スポットが一度現れた場合、他に充電スポットが現れないことを確認
				assert espot_cnt < 2

	return

# ---------------------------------------------------------------------
# ルート改善確認
# 内容：vlogから指定(sProc)の改善処理が行われたかどうかをチェックする
# ---------------------------------------------------------------------
def ImprovementCheck(test_no, oArgvs, sProc_list):
	if oArgvs==None:
		return

	Improvement_dict = {'relocate': 0, 'inswap': 0, 'exchange': 0, 'merge': 0, 'cross': 0} #各処理毎の改善実行回数
	Improvement_Order = []

	vlog_filename = log_dir+oArgvs.outputfile.split('.csv')[0] + '_vlog_'+str(test_no)+'.txt'
	with open(vlog_filename) as f:
		l_strip = [s.strip() for s in f.readlines()]
	for l in l_strip:
		if 'relocate' in l:
			l_list = l.split(' ')
			Improvement_dict['relocate']	+= int(l_list[1])  #relocate
			Improvement_dict['inswap']		+= int(l_list[3])  #inswap
			Improvement_dict['exchange']	+= int(l_list[6])  #exchange
			Improvement_dict['merge']		+= int(l_list[9])  #merge
			Improvement_dict['cross']		+= int(l_list[12]) #cross

			for _ in range(int(l_list[1])):
				Improvement_Order.append('relocate')
			for _ in range(int(l_list[3])):
				Improvement_Order.append('inswap')
			for _ in range(int(l_list[6])):
				Improvement_Order.append('exchange')
			for _ in range(int(l_list[9])):
				Improvement_Order.append('merge')
			for _ in range(int(l_list[12])):
				Improvement_Order.append('cross')

	assert Improvement_Order == sProc_list
	
	return


# ---------------------------------------------------------------------
# システム実行結果(Vehicles)をテキスト形式で保存
# ---------------------------------------------------------------------
#Vehicleh配列のテキストファイル作成
def DumpVehicle(filename, oVehicle):

	with open(filename, 'w') as f:
		out_dict = {}
		cnt = 0
		for v in oVehicle:
			f.write("----------------------------------------------------------------\n")
			for key, val in v.__dict__.items():
				if "route" in key:
					f.write("route [\n")
					#f.write(str(val)+"\n") カスタマー アドレス表示
					for c in val:
						if "customers" in c.__dict__.keys():
							f.write("\tTiedCustomer\n")
						f.write("\t[")
						for c_key, c_val in c.__dict__.items():
							if c_key == "customers":
								f.write(c_key+":")
								for custs in c_val:
									f.write(str(custs.__dict__["custno"])+":")
								f.write("\t")
							elif c_key == "additionalcust":
								f.write(c_key+":")
								f.write("[")
								for custs in c_val:
									if c_val.index(custs)!=len(c_val)-1:
										f.write(str(custs.__dict__["custno"])+", ")
									else:
										f.write(str(custs.__dict__["custno"]))
								f.write("]:")
								f.write("\t")
							elif c_key == "deny_unass":
								f.write(c_key+":")
								if c_val == True or c_val != 0:
									f.write("True")
								else:
									f.write("False")
								f.write("\t")
							else:
								f.write(c_key+":"+str(c_val)+":"+"\t")
						f.write("]\n")
						if "customers" in c.__dict__.keys():
							f.write("\t\tCustomer\n")
							for custs in c.__dict__["customers"]:
								f.write("\t\t[")
								for c_key, c_val in custs.__dict__.items():
									if c_key == "additionalcust":
										f.write(c_key+":")
										f.write("[")
										for custs in c_val:
											if c_val.index(custs)!=len(c_val)-1:
												f.write(str(custs.__dict__["custno"])+", ")
											else:
												f.write(str(custs.__dict__["custno"]))
										f.write("]:")
										f.write("\t")
									else:
										f.write(c_key+":"+str(c_val)+":"+"\t")
								f.write("]\n")
					f.write("]\n")
				elif "enddepot" in key:
					f.write("enddepot [\n")
					#f.write(str(val)+"\n") # enddepot アドレス表示
					for c_key, c_val in val.__dict__.items():
						if c_key == "additionalcust":
							f.write(c_key+":")
							f.write("[")
							for custs in c_val:
								if c_val.index(custs)!=len(c_val)-1:
									f.write(str(custs.__dict__["custno"])+", ")
								else:
									f.write(str(custs.__dict__["custno"]))
							f.write("]:")
							f.write("\t")
						else:
							f.write(c_key+":"+str(c_val)+":"+"\t")
					f.write("]\n")
					f.write("]\n")
				elif key == "chg_batt_dict":
					f.write(key + ":{")
					chg_cnt = 0
					for c in val:
						f.write("\n[")
						for c_key, c_val in c.__dict__.items():
							
							if c_key == "additionalcust" or c_key == "customers":
								f.write(c_key+":")
								f.write("[")
								for custs in c_val:
									if c_val.index(custs)!=len(c_val)-1:
										f.write(str(custs.__dict__["custno"])+", ")
									else:
										f.write(str(custs.__dict__["custno"]))
								f.write("]:")
								f.write("\t")
							else:
								f.write(c_key+":"+str(c_val)+":"+"\t")
						f.write("]: " + str(list(val.values())[chg_cnt]) + "\n")
						cnt += 1
					f.write("}\n")
				elif key == "delay_chg_batt_dict":
					f.write(key + ":{")
					chg_cnt = 0
					for c in val:
						f.write("\n[")
						for c_key, c_val in c.__dict__.items():
							if c_key == "additionalcust" or c_key == "customers":
								f.write(c_key+":")
								f.write("[")
								for custs in c_val:
									if c_val.index(custs)!=len(c_val)-1:
										f.write(str(custs.__dict__["custno"])+", ")
									else:
										f.write(str(custs.__dict__["custno"]))
								f.write("]:")
								f.write("\t")
							else:
								f.write(c_key+":"+str(c_val)+":"+"\t")
						f.write("]: " + str(list(val.values())[chg_cnt]) + "\n")
						cnt += 1
					f.write("}\n")
				elif key == "takebreak":
					f.write("takebreak:[")
					#f.write(str(val)+"\n") カスタマー アドレス表示
					for c in val:
						if val.index(c)!=len(val)-1:
							f.write(str(c.__dict__["custno"])+", ")
						else:
							f.write(str(c.__dict__["custno"]))
					f.write("]\n")
				else:
					f.write(key+":"+str(val)+"\n")
			f.write("\n")

	return

# Errorのテキストファイル作成
def DumpError(filename, oError):
	with open(filename, 'w') as f:
		for e in oError:
			f.write(e)
	return

#Vehicleh配列のファイルの出力と期待値比較
def CmpVehics(test_no, oVehicles):
	# フォルダ作成
	os.makedirs(out_dir, exist_ok=True)
	os.makedirs(exp_dir, exist_ok=True)

	# Vehicleh配列出力
	out_filename = out_dir + 'out_'+str(test_no)+'.txt'

	if oVehicles==None:
		open(out_filename, 'w') #空ファイルを作成
		return True

	DumpVehicle(out_filename, oVehicles)

	exp_filename = exp_dir + 'out_'+str(test_no)+'.txt'

	if os.path.exists(exp_filename):
		# 期待値比較実施
		ret = filecmp.cmp(out_filename, exp_filename)

		# テスト18-1-2~4 期待値と一致しないことを確認 (18-1-1以外)
		if test_no[0:4] == '18_1' and test_no[5:6] != '1':
			return ret == False
		# テスト31-1-67 期待値と一致しないことを確認
		if test_no == '31_1_67':
			return ret == False

		# テスト40-1-5 期待値と一致しないことを確認
		if test_no == '40_1_5':
			return ret == False

		return ret == True
	else:
		# 期待値比較を実施できない
		print(exp_filename + " not found ---------------")
		return False

	return

# エラー出力と期待値比較
def CmpError(test_no, oErrors):
	# フォルダ作成
	os.makedirs(out_dir, exist_ok=True)
	os.makedirs(exp_dir, exist_ok=True)

	# Error情報出力
	out_filename = out_dir + 'err_'+str(test_no)+'.txt'

	if oErrors==[]:
		open(out_filename, 'w') #空ファイルを作成
		return True

	DumpError(out_filename, oErrors)

	exp_filename = exp_dir + 'err_'+str(test_no)+'.txt'

	if os.path.exists(exp_filename):
		# 期待値比較実施
		ret = filecmp.cmp(out_filename, exp_filename)
		return ret == True
	else:
		# 期待値比較を実施できない
		print(exp_filename + " not found ---------------")
		return False

	return

#コマンドライン実行のファイルの出力と期待値比較
def CmpCommand(test_no, args):
	# フォルダ作成
	os.makedirs(out_dir, exist_ok=True)
	os.makedirs(exp_dir, exist_ok=True)

	# 入力データ名変更
	input_filename = log_dir + args["-i"][args["-i"].find("D"):-4] + "-best0.txt"
	log_filename = log_dir + 'output_' + str(test_no) + '.csv.detail.csv'
	shutil.copy(input_filename, log_dir + 'input_' + str(test_no)+'.txt')

	log_filename = log_dir + 'output_' + str(test_no) + '.csv.detail.csv'
	out_filename = out_dir + 'out_' + str(test_no) + '.csv'
	shutil.copy(log_filename, out_filename)

	exp_filename = exp_dir + 'out_' + str(test_no) + '.csv'

	if os.path.exists(exp_filename):
		# 期待値比較実施
		ret = filecmp.cmp(out_filename, exp_filename)
		return ret == True
	else:
		# 期待値比較を実施できない
		print(exp_filename + " not found ---------------")
		return False

# ---------------------------------------------------------------------
# タイムアウトチェック
# 内容：タイムアウト発生の有無を確認
# ---------------------------------------------------------------------
def TimeOutCheck(test_no, oArgvs, margin=0.95):
	endtime=time.time()
	if oArgvs.timeout > 0 and (endtime - oArgvs.ls_d_starttime) > oArgvs.timeout * margin:
		print("Time-out occurred")
		assert False

	return

# ---------------------------------------------------------------------
# タイムアウト精度チェック
# 内容：タイムアウト発生の精度を確認
# ---------------------------------------------------------------------
def TimeOut_Accuracy_Check(test_no, oArgvs, margin=0.95):
	endtime=time.time()
	if oArgvs.timeout == 0:
		oArgvs.timeout = 1e12
	print('total time: {}'.format(endtime - oArgvs.ls_d_starttime))
	if (endtime - oArgvs.ls_d_starttime - oArgvs.timeout) / oArgvs.timeout > 1 - margin:
		print("Accuracy of Time-Out is low.")
		assert False

	return

#-----------------------------------------------------------------
# 追加確認(なし)
#-----------------------------------------------------------------
def AdditionalCheck_None(test_no, oVehicles, oCustomer, oArgvs, timeout_flag, calc_time_no_parallel=-1, calc_time_parallel=-1):
	print("AdditionalCheck_None")
	return

#-----------------------------------------------------------------
# 追加確認(4-1)
#-----------------------------------------------------------------
def AdditionalCheckFunc_4(test_no, oVehicles, oCustomer, oArgvs, timeout_flag, calc_time_no_parallel=-1, calc_time_parallel=-1):
	epsilon = 0 #APE判定マージン

	if '4_1_22' in test_no or '4_1_23' in test_no:
		return

	
	if oArgvs.avetime_ape_flag:
		avetime_ape = SoftCheck_avetime_APE(test_no, oVehicles, oArgvs, oArgvs.avetime_ape, epsilon)
	elif oArgvs.avetime_ae_flag:
		avetime_ae = SoftCheck_avetime_AE(test_no, oVehicles, oArgvs, oArgvs.avetime_ae, epsilon)
	return

#-----------------------------------------------------------------
# 追加確認(18-1)
#-----------------------------------------------------------------
def AdditionalCheckFunc_18(test_no, oVehicles, oCustomer, oArgvs, timeout_flag, calc_time_no_parallel=-1, calc_time_parallel=-1):
	if '18_1_2' in test_no:
		TimeOut_Accuracy_Check(test_no, oArgvs, 0.8)
	if '18_1_3' in test_no:
		TimeOut_Accuracy_Check(test_no, oArgvs, 0.8)
	if '18_1_5' in test_no:
		TimeOut_Accuracy_Check(test_no, oArgvs, 0.4)

#-----------------------------------------------------------------
# 追加確認(20-1)
#-----------------------------------------------------------------
def AdditionalCheckFunc_20(test_no, oVehicles, oCustomer, oArgvs, timeout_flag, calc_time_no_parallel=-1, calc_time_parallel=-1):
	epsilon = 0 #APE判定マージン
	if '20_1_5' in test_no:
		APE = SoftCheck_avetime_APE(test_no, oVehicles, oArgvs, oArgvs.avetime_ape, epsilon)
	return

#-----------------------------------------------------------------
# 追加確認(21-1)
#-----------------------------------------------------------------
def AdditionalCheckFunc_21(test_no, oVehicles, oCustomer, oArgvs, timeout_flag, calc_time_no_parallel=-1, calc_time_parallel=-1):
	epsilon = 0 #APE判定マージン
	if '21_1_13' in test_no:
		APE = SoftCheck_avetime_APE(test_no, oVehicles, oArgvs, oArgvs.avetime_ape, epsilon)
	return

#-----------------------------------------------------------------
# 追加確認(23-1)
#-----------------------------------------------------------------
def AdditionalCheckFunc_23(test_no, oVehicles, oCustomer, oArgvs, timeout_flag, calc_time_no_parallel=-1, calc_time_parallel=-1):
	if '23_1_1' == test_no:
		ImprovementCheck(test_no, oArgvs, ['cross'])
	if '23_1_2' == test_no:
		ImprovementCheck(test_no, oArgvs, ['merge'])
	if '23_1_3' == test_no:
		ImprovementCheck(test_no, oArgvs, ['merge'])
	if '23_1_4' == test_no:
		ImprovementCheck(test_no, oArgvs, ['merge'])
	if '23_1_5' == test_no:
		ImprovementCheck(test_no, oArgvs, ['cross'])
	if '23_1_6' == test_no:
		ImprovementCheck(test_no, oArgvs, ['merge', 'inswap'])
	if '23_1_7' == test_no:
		ImprovementCheck(test_no, oArgvs, ['inswap'])
	if '23_1_8' == test_no:
		ImprovementCheck(test_no, oArgvs, ['merge'])
	if '23_1_9' == test_no:
		ImprovementCheck(test_no, oArgvs, ['merge'])
	if '23_1_10' == test_no:
		ImprovementCheck(test_no, oArgvs, ['merge'])
	if '23_1_11' == test_no:
		ImprovementCheck(test_no, oArgvs, ['inswap'])

#-----------------------------------------------------------------
# 追加確認(25-2)
#-----------------------------------------------------------------
def AdditionalCheckFunc_25_2(test_no, oVehicles, oCustomer, oArgvs, timeout_flag, calc_time_no_parallel=-1, calc_time_parallel=-1):
	epsilon = 0 #APE判定マージン
	APE = SoftCheck_avetime_APE(test_no, oVehicles, oArgvs, oArgvs.avetime_ape, epsilon)


#-----------------------------------------------------------------
# 追加確認(27-1)
#-----------------------------------------------------------------
def AdditionalCheckFunc_27(test_no, oVehicles, oCustomer, oArgvs, timeout_flag, calc_time_no_parallel=-1, calc_time_parallel=-1):
	epsilon = 0 #APE判定マージン
	if '27_1_25' == test_no:
		APE = SoftCheck_avetime_APE(test_no, oVehicles, oArgvs, oArgvs.avetime_ape, epsilon)
	if '27_1_41' == test_no:
		ImprovementCheck(test_no, oArgvs, ['merge'])

#-----------------------------------------------------------------
# 追加確認(29-1)
#-----------------------------------------------------------------
def AdditionalCheckFunc_29(test_no, oVehicles, oCustomer, oArgvs, timeout_flag, calc_time_no_parallel=-1, calc_time_parallel=-1):
	if '29_1_31' == test_no:
		ImprovementCheck(test_no, oArgvs, ['merge'])

#-----------------------------------------------------------------
# 追加確認(31-1)
#-----------------------------------------------------------------
def AdditionalCheckFunc_31(test_no, oVehicles, oCustomer, oArgvs, timeout_flag, calc_time_no_parallel=-1, calc_time_parallel=-1):
	epsilon = 0 #APE判定マージン
	if '31_1_57' == test_no:
		APE = SoftCheck_avetime_APE(test_no, oVehicles, oArgvs, oArgvs.avetime_ape, epsilon)
	if '31_1_67' == test_no:
		TimeOut_Accuracy_Check(test_no, oArgvs, 0.6)

#-----------------------------------------------------------------
# 追加確認(35-1)
#-----------------------------------------------------------------
def AdditionalCheckFunc_35(test_no, oVehicles, oCustomer, oArgvs, timeout_flag, calc_time_no_parallel=-1, calc_time_parallel=-1):
	epsilon = 0 #APE判定マージン
	if '35_1_3' == test_no or '35_1_4' == test_no:
		APE = SoftCheck_avetime_APE(test_no, oVehicles, oArgvs, oArgvs.avetime_ape, epsilon)

#-----------------------------------------------------------------
# 追加確認(36-1)
#-----------------------------------------------------------------
def AdditionalCheckFunc_36(test_no, oVehicles, oCustomer, oArgvs, timeout_flag, calc_time_no_parallel=-1, calc_time_parallel=-1):
	epsilon=0
	if '36_1_15' == test_no:
		ImprovementCheck(test_no, oArgvs, ['merge'])
	if '36_1_28' == test_no:
		ImprovementCheck(test_no, oArgvs, ['merge'])

	if oArgvs.avevisits_ape_flag:
		avevisits_ape = SoftCheck_avevisits_APE(test_no, oVehicles, oArgvs, oArgvs.avevisits_ape, epsilon)
	elif oArgvs.avevisits_ae_flag:
		avevisits_ae = SoftCheck_avevisits_AE(test_no, oVehicles, oArgvs, oArgvs.avevisits_ae, epsilon)
	return

#-----------------------------------------------------------------
# 追加確認(40-1)
#-----------------------------------------------------------------
def AdditionalCheckFunc_40(test_no, oVehicles, oCustomer, oArgvs, timeout_flag, calc_time_no_parallel=-1, calc_time_parallel=-1):
	epsilon=0.8
	if '40_1_2' == test_no or '40_1_4' == test_no:
		assert calc_time_no_parallel*epsilon>calc_time_parallel
	if '40_1_3' == test_no:
		assert calc_time_no_parallel>calc_time_parallel
	if '40_1_5' in test_no:
		TimeOut_Accuracy_Check(test_no, oArgvs, 0.9)

	return

#-----------------------------------------------------------------
# 追加確認(47-1)
#-----------------------------------------------------------------
def AdditionalCheckFunc_47(test_no, oVehicles, oCustomer, oArgvs, timeout_flag, calc_time_no_parallel=-1, calc_time_parallel=-1):
	epsilon = 0 #APE判定マージン
	
	if '47_1_14' == test_no or '47_1_15' == test_no or '47_1_24' == test_no or '47_1_25' == test_no or '47_1_32' == test_no:
		return

	if oArgvs.avedist_ape_flag:
		avedist_ape = SoftCheck_avedist_APE(test_no, oVehicles, oArgvs, oArgvs.avedist_ape, epsilon)
	elif oArgvs.avedist_ae_flag:
		avedist_ae = SoftCheck_avedist_AE(test_no, oVehicles, oArgvs, oArgvs.avedist_ae, epsilon)
	return

#-----------------------------------------------------------------
# 追加確認(ランダム)
#-----------------------------------------------------------------
def AdditionalCheckFunc_random(test_no, oVehicles, oCustomer, oArgvs, timeout_flag, calc_time_no_parallel=-1, calc_time_parallel=-1):
	if not timeout_flag:
		TimeOutCheck(test_no, oArgvs)

	totalcost_check(test_no, oVehicles, oArgvs)

def totalcost_check(test_no, oVehicles, oArgvs):
	record_totalcost(test_no, oVehicles, oArgvs, 'a')

	total = []
	with open(log_dir + 'output_' + test_no + '_totalcost.csv', 'r') as f:
		lines = csv.reader(f)
		for line in lines:
			if len(line) > 1:
				total.append(float(line[1]))
	cnt = 0
	for i in range(int(len(total)/2)):
		if total[i] < total[int(len(total)/2)+i]:
			cnt += 1
	if cnt == int(len(total)/2):
		print('Total penalty become worse.')
		assert False

def record_totalcost(test_no, oVehicles, oArgvs, csv_option):
	with open(log_dir + 'output_' + test_no + '_totalcost.csv', csv_option, newline='') as f:
		writer = csv.writer(f)
		if csv_option == 'w':
			writer.writerow(['初期解'])
		else:
			writer.writerow([])
			writer.writerow(['改善解'])
		totalcost_wolast=0
		totalcost=0
		for v in oVehicles:
			if v.delay_arr==[]:
				totalcost_wolast+=v.totalcost_wolast
				totalcost+=v.totalcost
			else:
				totalcost_wolast+=v.delay_totalcost_wolast
				totalcost+=v.delay_totalcost
		if oArgvs.lastc_flag==True: 
			olist=[]
			olist+=["TOTALCOST"]
			olist+=[totalcost_wolast]
			writer.writerow(olist)
		else:
			olist=[]
			olist+=["TOTALCOST"]
			olist+=[totalcost]
			writer.writerow(olist)
		olist=[]
		olist+=["TOTALCOST_TO_LAST"]
		olist+=[totalcost_wolast]
		writer.writerow(olist)
		olist=[]
		olist+=["TOTALCOST_TO_DEPO"]
		olist+=[totalcost]
		writer.writerow(olist)

		totaltime_wolast,totaltime=0,0
		for v in oVehicles:
			if v.reserved_v_flag==True and len(v.route) < 2:
				continue
			if oArgvs.delaystart_flag and v.delay_arr!=[]:
				if len(v.route)>2 and v.route[-2].custno=="0":
					totaltime_wolast+=v.delay_arr[-3]-v.delay_arr[0]
				else:
					totaltime_wolast+=v.delay_arr[-2]-v.delay_arr[0]
				totaltime+=v.delay_arr[-1]-v.delay_arr[0]
			else:
				if len(v.route)>2 and v.route[-2].custno=="0":
					totaltime_wolast+=v.arr[-3]-v.arr[0]
				else:
					totaltime_wolast+=v.arr[-2]-v.arr[0]
				totaltime+=v.arr[-1]-v.arr[0]

		olist=[]
		olist+=["TOTALTIME_TO_LAST"]
		olist+=[totaltime_wolast]
		writer.writerow(olist)
		olist=[]
		olist+=["TOTALTIME_TO_DEPO"]
		olist+=[totaltime]
		writer.writerow(olist)

		if oArgvs.evplan:
			if oArgvs.management_etable:
				totalerec_wolast,totalerec=0,0
				for v in oVehicles:
					if v.delay_arr==[]:
						for i in range(len(v.route)-2):
							etable1=main_tw.get_etable(v.vtype,v.etype,v.lv[i],oArgvs.management_etable)
							totalerec+=etable1[v.route[i].spotno][v.route[i+1].spotno]

							if i+1==len(v.route)-2 and v.route[i+1].custno=="0":
								continue

							totalerec_wolast+=etable1[v.route[i].spotno][v.route[i+1].spotno]
						
						etable2=main_tw.get_etable(v.vtype,v.etype,v.lv[-2],oArgvs.management_etable)
						totalerec+=etable2[v.route[-2].spotno][v.route[-1].spotno]
					else:
						for i in range(len(v.route)-2):
							etable1=main_tw.get_etable(v.vtype,v.etype,v.delay_lv[i],oArgvs.management_etable)
							totalerec+=etable1[v.route[i].spotno][v.route[i+1].spotno]

							if i+1==len(v.route)-2 and v.route[i+1].custno=="0":
								continue

							totalerec_wolast+=etable1[v.route[i].spotno][v.route[i+1].spotno]
						
						etable2=main_tw.get_etable(v.vtype,v.etype,v.delay_lv[-2],oArgvs.management_etable)
						totalerec+=etable2[v.route[-2].spotno][v.route[-1].spotno]
			else:
				totalerec_wolast,totalerec=0,0
				for v in oVehicles:
					if v.delay_arr==[]:
						for i in range(len(v.route)-2):
							dtable1=main_tw.get_dtable(v.vtype,v.lv[i],oArgvs.management_dtable)
							totalerec+=(dtable1[v.route[i].spotno][v.route[i+1].spotno]/1000)/v.e_cost

							if i+1==len(v.route)-2 and v.route[i+1].custno=="0":
								continue
							
							totalerec_wolast+=(dtable1[v.route[i].spotno][v.route[i+1].spotno]/1000)/v.e_cost
						
						dtable2=main_tw.get_dtable(v.vtype,v.lv[-2],oArgvs.management_dtable)
						totalerec+=(dtable2[v.route[-2].spotno][v.route[-1].spotno]/1000)/v.e_cost
					else:
						for i in range(len(v.route)-2):
							dtable1=main_tw.get_dtable(v.vtype,v.delay_lv[i],oArgvs.management_dtable)
							totalerec+=(dtable1[v.route[i].spotno][v.route[i+1].spotno]/1000)/v.e_cost

							if i+1==len(v.route)-2 and v.route[i+1].custno=="0":
								continue
							totalerec_wolast+=(dtable1[v.route[i].spotno][v.route[i+1].spotno]/1000)/v.e_cost
						
						dtable2=main_tw.get_dtable(v.vtype,v.delay_lv[-2],oArgvs.management_dtable)
						totalerec+=(dtable2[v.route[-2].spotno][v.route[-1].spotno]/1000)/v.e_cost

			olist=[]
			olist+=["TOTALEREC_TO_LAST"]
			olist+=[totalerec_wolast]
			writer.writerow(olist)
			olist=[]
			olist+=["TOTALEREC_TO_DEPO"]
			olist+=[totalerec]
			writer.writerow(olist)

		if oArgvs.management_xtable:
			totalxcost_wolast,totalxcost=0,0
			for v in oVehicles:
				if v.delay_arr==[]:
					for i in range(len(v.route)-2):
						xtable1=main_tw.get_xtable(v.vtype,v.lv[i],oArgvs.management_xtable)
						totalxcost+=main_tw.exp_xtable(v.route[i],v.route[i+1],xtable1)

						if i+1==len(v.route)-2 and v.route[i+1].custno=="0":
							continue

						totalxcost_wolast+=main_tw.exp_xtable(v.route[i],v.route[i+1],xtable1)
					
					xtable2=main_tw.get_xtable(v.vtype,v.lv[-2],oArgvs.management_xtable)
					totalxcost+=main_tw.exp_xtable(v.route[-2],v.route[-1],xtable2)
				else:
					for i in range(len(v.route)-2):
						if i+1==len(v.route)-2 and v.route[i+1].custno=="0":
							continue
						xtable1=main_tw.get_xtable(v.vtype,v.delay_lv[i],oArgvs.management_xtable)
						totalxcost+=main_tw.exp_xtable(v.route[i],v.route[i+1],xtable1)

						totalxcost_wolast+=main_tw.exp_xtable(v.route[i],v.route[i+1],xtable1)						
					
					xtable2=main_tw.get_xtable(v.vtype,v.delay_lv[-2],oArgvs.management_xtable)
					totalxcost+=main_tw.exp_xtable(v.route[-2],v.route[-1],xtable2)

			olist=[]
			olist+=["TOTALXCOST_TO_LAST"]
			olist+=[totalxcost_wolast]
			writer.writerow(olist)
			olist=[]
			olist+=["TOTALXCOST_TO_DEPO"]
			olist+=[totalxcost]
			writer.writerow(olist)

# ---------------------------------------------------------------------
# ---------------------------------------------------------------------
#
# プライベート用関数
#

