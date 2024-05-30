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

sys.path.append(os.path.join(os.path.dirname(__file__), '../../code'))

import main_tw
import json
import traceback
import random
import time
import datetime
from io import StringIO
import re
import numpy as np
import copy
import logging
import csv

import test_apiTest_result as result
import test_apiTest_create as create

"""
プログラム変数は以下ルールで記述している:
sから始まる: 文字列(str型)
nから始まる: 数字(int型 or float型)
bから始まる: ブール(boolean型)
eから始まる: 列挙型(enum型)
oから始まる: それ以外

ただしx・y・i等，明らかに数字と分かる変数名については，例外的に上記ル
ールを適用しないで，単にx・y・i等と書く(読みにくくなるため)．

例えばCNSソルバではcustnoは"0"や"1"が入る変数．これに数字の0や1を入れ
るとバグになる．このように最低限度の型の注意喚起を知らせるために，この
ような命名規則を導入している．

ただしPythonでは一般的ではない記述なので，PSDCD殿が実際にプログラムを
作成する際は，このルールを守っても良いし，守らなくても良い．
//"""


# ---------------------------------------------------------------------
# ---------------------------------------------------------------------
#
# パブリック用関数
#

def driver(test_no, sParamJSON, logon_flag, caplog, addCheckFunc=None, continuity_test_flag=False, timeout_flag=False, calc_time_no_parallel=-1):
	# (浜田が小さいサンプルで試した時に起きた問題への対処処理)
	# (最新のJNA-cython-python接続方式では不要かも)
	#
	# JNAから呼ぶときに限った現象(Cでは起こらない現象)として，
	# PyInit_cnssolvergw(関数初期化の関数)を呼ぶと，plan関数本体が勝enrouteCharging手
	# に実行されてしまう．その場合，plan("","")で呼ばれることが分かっ
	# ているので，そういう呼ばれ方をした場合には，処理を即終了させる．
	if len(sParamJSON) == 0: return ""

	# 内部変数初期化
	oArgvs = None
	oNormalCusts = None
	oVehicles = None
	oInitRoutes = None
	oVRP = None
	oError = []
	start_time=time.time()

	try:
		#-----------------------------------------------------------------
		# JSON解析
		#-----------------------------------------------------------------
		oParamJSON = json.loads(sParamJSON)
		oRequestJSON = oParamJSON["requestJSON"]
		oCostJSON = oParamJSON["costJSON"]

		#-----------------------------------------------------------------
		# Argvsインスタンス生成
		#-----------------------------------------------------------------
		oArgvs = create.CreateArgs(oRequestJSON["Args"])

		#-----------------------------------------------------------------
		#Customerインスタンスリスト生成
		#-----------------------------------------------------------------
		oNormalCusts = create.CreateCustomer(oRequestJSON["NormalCustomer"], oArgvs) #通常スポット(拠点・配送先)を示す顧客オブジェクト
		oCSCusts = []
		if "ESpots" in oRequestJSON.keys():
			oCSCusts = create.CreateCustomer(oRequestJSON["ESpots"], oArgvs) # 充電スポットを示す顧客オブジェクト

		#-----------------------------------------------------------------
		#Vehicleインスタンスリスト生成
		#-----------------------------------------------------------------
		oVehicles = create.CreateVehicle(oRequestJSON["Vehicles"], oRequestJSON["EndSpots"], oArgvs)
		ivnum = len(oVehicles)

		#-----------------------------------------------------------------
		# 初期解オブジェクト(構造体)の作成
		#-----------------------------------------------------------------
		oInitRoutes = []
		if 'InitialRoutes' in oRequestJSON:
			if len(oRequestJSON["InitialRoutes"]) != 0:
				for route in oRequestJSON["InitialRoutes"]:
					oInitRoutes.append([str(r) for r in route])
			
		#-----------------------------------------------------------------
		# コスト表群の作成
		#-----------------------------------------------------------------

		# コストJSONを元に，以下のようなオブジェクトを作成する．
		# start_timeは，各表の開始日時 - ソルバ基準日時(nBaseDateTime)で
		# 計算する．
		#
		# ((注意すべきこと1))
		#
		# 各コスト表(listのlist)の縦軸・横軸の要素の並び順
		# は，以下のルールに従う．
		#
		# 以下に含まれるすべてのspotIDを重複なしで辞書順に並べたもの．
		# * 計画に使うcustomers
		# * 計画に使うe_spots
		# * 各車両に含まれる_enddepot
		# 例)
		# * 計画に使うcustomers       : "0" "1" "2" "15"
		# * 計画に使うe_spots         : "e1" "e2"
		# * 各車両に含まれる_enddepot : "0" "2" "3"
		# 結果: "0" "1" "15" "2" "3" "e1" "e2"
		#
		# 一方パナ配送計画のコスト表JSONのコスト表の縦軸・横軸の並び順
		# は，コスト表作成リクエストJSONに書かれたspots下の順序に従う．
		#
		# したがってその並べ替えを行う必要がある(なお以下ではその並べ
		# 替え処理についてはプログラム例を示していない)．
		#
		# ((注意すべきこと2))
		#
		# 「通行不可(経路無効)」に相当するマジックナンバー「-1」につい
		# ては，気にせずそのまま渡してよい．
		#

		# 距離表
		oArgvs.management_dtable = []
		for oTable in oCostJSON["management_dtable"]:
			oArgvs.management_dtable.append({
				"vtype"      : oTable["vtype"],  # 車両タイプ
				"start_time" : oTable["start_time"],  # 開始時刻(基準日時からの相対時間，単位hour)
				"table": oTable["table"]
			})

		# 時間表
		oArgvs.management_ttable = []
		for oTable in oCostJSON["management_ttable"]:
			oArgvs.management_ttable.append({
				"vtype"      : oTable["vtype"],  # 車両タイプ
				"start_time" : oTable["start_time"],  # 開始時刻(基準日時からの相対時間，単位hour)
				"table": oTable["table"]
			})

		# 消費電力表
		oArgvs.management_etable = []
		if "management_etable" in oCostJSON.keys():
			for oTable in oCostJSON["management_etable"]:
				oArgvs.management_etable.append({
					"vtype"      : oTable["vtype"],  # 車両タイプ
					"etype"      : oTable["etype"],  # 電動車両タイプ
					"start_time" : oTable["start_time"],  # 開始時刻(基準日時からの相対時間，単位hour)
					"table": oTable["table"]
				})

		# 経験コスト表
		oArgvs.management_xtable = []
		if "management_xtable" in oCostJSON.keys():
			for oTable in oCostJSON["management_xtable"]:
				oArgvs.management_xtable.append({
					"vtype"      : oTable["vtype"],  # 車両タイプ
					"start_time" : oTable["start_time"],  # 開始時刻(基準日時からの相対時間，単位hour)
					"table": oTable["table"]
				})

		#-----------------------------------------------------------------
		# スキルオブジェクト(構造体)の作成
		#-----------------------------------------------------------------
		oSkills = None
		if oArgvs.skillflag != 0:
			oSkills = []
			for oVehicleJSON in oRequestJSON["Vehicles"]:
				# 車両名(※1),荷作業スキル係数(※2),運転スキル係数(※3)
				oSkills.append(['route' + str(oVehicleJSON["vehno"]), oVehicleJSON['deliverySkill'], oVehicleJSON['drivingSkill']])

		#-----------------------------------------------------------------
		# Vehicleログファイル設定
		#-----------------------------------------------------------------
		vlog_filename = result.vehicle_log_init(caplog, test_no, oVehicles, oArgvs)

		#-----------------------------------------------------------------
		# 初期解の処理
		#-----------------------------------------------------------------
		# bLastC    = oArgvs.lastc_flag                          # argvsのlastc_flagメンバーと同じ値を設定
		# bDecrVNum = oArgvs.decr_vnum                           # argvsのdecr_vnumメンバーと同じ値を設定
		# bAve      = oArgvs.aveflag                             # argvsのaveflagメンバーと同じ値を設定

		#if len(oInitRoutes) > 0:
		if 'InitialRoutes' in oRequestJSON:
			#
			# 初期解インポートモード
			# set_initroutes2(_custs,e_spots,_vehs,_nbflag,_lastc_flag,iniroutes,skills,argvs)
			#
			(_, oNormalCusts) = main_tw.set_initroutes2(
			oNormalCusts,                                  # _custs      : 拠点＋配送先
			oCSCusts,                                      # e_spots     : 充電場
			oVehicles,                                     # _vehs       : 車両群
			main_tw.NbFlag(True, True, True, True, False), # _nbflag     : アルゴoption
			# bLastC,                                        # _lastc_flag : 非巡回かどうか
			oInitRoutes,                                   # iniroutes   : 初期解
			oSkills,                                       # skills      : スキル設定
			oArgvs)                                        # argvs       : CNS solver Basic settings
		else:
			#
			# 初期解作成モード
			# adjust_nveh_construction_I2(customers,e_spots,vehicles,ivnum,_nbflag,decr_vnum,aveflag,lastc_flag,skills,argvs)
			#

			(_, oNormalCusts) = main_tw.adjust_nveh_construction_I2(
			oNormalCusts,                                  # _custs      : 拠点＋配送先
			oCSCusts,                                      # e_spots     : 充電場
			oVehicles,                                     # _vehs       : 車両群
			ivnum,                                         # ivnum       : 車両数(oVehicles要素数と一致しないといけない)
			main_tw.NbFlag(True, True, True, True, False), # _nbflag     : アルゴoption
			# bDecrVNum,                                     # decr_vnum   : 台数削減モード
			# bAve,                                          # aveflag     : タスク平準化モードのONOFF
			# bLastC,                                        # _lastc_flag : 非巡回かどうか
			oSkills,                                       # skills      : スキル設定
			oArgvs)                                        # argvs       : CNS solver Basic settings

		# 初期解を保存する
		# oVehiclesInit = copy.deepcopy(oVehicles)

		#-----------------------------------------------------------------
		#
		# 最終解(電欠関連の処理以外)の作成
		# localsearch_dtable(customers,vehicles,nbflag,ivnum,lflag,decr_vnum,argvs)
		#

		# ランダムテスト、フィルタ
		if 'random' in test_no:
			result.record_totalcost(test_no, oVehicles, oArgvs, 'w')

		oVehicles, _ = main_tw.localsearch_dtable(
		oNormalCusts,                                 # customers   : 拠点＋配送先
		oVehicles,                                    # vehicles    : 車両群
		main_tw.NbFlag(True, True, True, True, True), # _nbflag     : アルゴoption
		ivnum,                                        # ivnum       : 車両数(oVehicles要素数と一致しないといけない)
		# bLastC,                                       # _lastc_flag : 非巡回かどうか
		# bDecrVNum,                                    # decr_vnum   : 台数削減モード
		oSkills,                                      # skills      : スキル設定
		oArgvs)                                       # argvs       : CNS solver Basic settings
		# 注)oVRPの実態はoVehicles

		# 途中充電付き解に電力違反の可能があるため、最終解を保持する
		#oVRP = copy.deepcopy(oVRP)

		#-----------------------------------------------------------------
		#
		# 途中充電付き計画の作成
		# localsearch_ev(custs3, vehs, e_spots, argvs)
		#
		oEVRP               = None   # EV計画
		bEVRPIsCreated      = False  # EV計画が作られたか
		bEVRPCreateErr      = False  # EV計画が作ろうとして失敗した
		oEVRPEVViolations   = []     # 車両番号と違反messageの辞書

		#
		# 処理失敗(=例外発生)しても落ちないようにするためtry-exceptガードを入れる．
		# (失敗した場合は通常配送計画の結果を返すようにするため)
		#
		if oArgvs.evplan:
			(_, oVehicles) = main_tw.localsearch_ev(
			oNormalCusts,   # custs3  : 拠点＋配送先
			oVehicles,      # vehs    : 車両群
			oCSCusts,       # e_spots : 充電スポット
			oArgvs)         # argvs   : CNS solver Basic settings
			# 注)oEVRPの実態はoVehicles
			bEVRPIsCreated = True

			# 電力違反チェック
			# oEVRPViolations   = 違反車両と違反messageを入れる(例:{1:"overload", 3:"out of timewindow"})
			# oEVRPEVViolations = checkEVPlan(oEVRP, oRequestJSON["config"]["marginalEpower"])

		end_time=time.time()
		calc_time_parallel=end_time-start_time

	except Exception as e:
		# 例外取得
		if len(e.args) == 0:
			#エラーメッセージが存在しない場合
			oError.append("Error Mesg None")
		else:
			#エラーメッセージが存在する場合
			oError.append(str(e.args[0]))

	if oError == []:
		# ログ出力(デバッグ用)
		if logon_flag:
			result.log(test_no, oVehicles, oNormalCusts, oArgvs)

		# Vehicles期待値比較
		cmp_ret = result.CmpVehics(test_no, oVehicles)
		# Error期待値比較できない場合は正常動作とする(Vehicles側のみ比較とするため)
		err_ret = True

		# 制約条件(ハード)確認
		result.HardCheck_MaxNumVehicleVisits(test_no, oArgvs, oVehicles)
		result.HardCheck_MaxNumVehicleRotates(test_no, oArgvs, oVehicles)
		result.HardCheck_Impassable(test_no, oVehicles, oArgvs)
		result.HardCheck_VehicleStopOff(test_no, oVehicles, oArgvs)
		result.HardCheck_ChargingSpotTimeFrame(test_no, oVehicles, oArgvs)
		result.HardCheck_EVChargeLimit(test_no, oVehicles, oArgvs)
		result.HardCheck_TimeFrame(test_no, oVehicles, oArgvs)
		result.HardCheck_LoadLimit(test_no, oVehicles, oArgvs)
		result.HardCheck_Load2Limit(test_no, oVehicles, oArgvs)
		result.HardCheck_avetime(test_no, oVehicles, oArgvs)
		result.HardCheck_avevisits(test_no, oVehicles, oArgvs)
		result.HardCheck_avedist(test_no, oVehicles, oArgvs)
		result.HardCheck_changecust(test_no, oVehicles, oArgvs, oInitRoutes)

		# 配送計画出力要件確認
		result.PlacCheck_TopBase(test_no, oArgvs)
		result.PlacCheck_TopEndExceptBase(test_no, oVehicles, oArgvs)
		result.PlacCheck_EndVehicleSpecificationEndSpot(test_no, oRequestJSON["EndSpots"], oArgvs)
		result.PlacCheck_EndVehicleEndSpot(test_no, oRequestJSON["EndSpots"], oArgvs)
		result.PlacCheck_AllDeliveryOnce(test_no, oRequestJSON["NormalCustomer"], oRequestJSON["EndSpots"], oVehicles, oArgvs)
		result.PlacCheck_NonContinuous(test_no, oVehicles, oArgvs)
		result.PlacCheck_ChargeSpotOne(test_no, oVehicles, oArgvs)

		# 入力パラメータ整合性テスト
		result.ParamCheck_Argvs_ivnum(test_no, oVehicles, oNormalCusts, oArgvs, ivnum)
		# result.ParamCheck_Argvs_lowertime(test_no, oVehicles, oArgvs)
		# result.ParamCheck_Argvs_uppertime(test_no, oVehicles, oArgvs)
		result.ParamCheck_Argvs_pickup_flag(test_no, oRequestJSON["NormalCustomer"], oArgvs)
		result.ParamCheck_Vehicle_vehno(test_no, oVehicles, oRequestJSON["Vehicles"], oNormalCusts, oArgvs, ivnum)
		result.ParamCheck_ready_due_fs(test_no, oVehicles, oArgvs)

		#Vehicle結果をcaplogへ格納
		result.vehicle_log_output(test_no, oVehicles, oNormalCusts, oArgvs)

		#Vehicleログ掃き出し
		result.vehicle_log_dump(caplog, vlog_filename)


	else:
		# Vehicles期待値比較できない場合は正常動作とする(Error側のみ比較とするため)
		cmp_ret = True
		# Error期待値比較
		err_ret = result.CmpError(test_no, oError)


	if oError == []:
		# テスト項目毎の追加確認
		if addCheckFunc != None:
			addCheckFunc(test_no, oVehicles, oNormalCusts, oArgvs, timeout_flag, calc_time_no_parallel, calc_time_parallel)

	# 期待値比較結果確認
	# (Vehicles期待値比較とErrorの期待値ファイルを両方出力してから内容を確認)

	print('TOTAL TIME: {}'.format(time.time() - oArgvs.ls_d_starttime))

	# 導通テストの場合エラーチェックを行わない(フリーズの有無のみを確認)
	if not continuity_test_flag:
		assert cmp_ret
		assert err_ret

	return

#
# 車両オブジェクト群からレスポンスJSONを作成する関数
#
def createResponseJSON(oVRP, oRequestJSON, oCustMap, oVehicleIDMapC2P, oVehicleIDMapP2C, oCustIDMapC2P,
					   oSpotIDMapC2P, oCNSSpotIDSort, oEVRPEVViolations, bEVRPCreateErr=False):

	#
	#レスポンスJSON作成
	#
	sResponseJSON = {
		"result": {
			"status": "OK",
			"msg": ""
		},
		"spots": oRequestJSON["spots"],
		"vehicles": oRequestJSON["vehicles"]
	}

	# 配送計画結果マップ
	oVehicleResults = {}
	for oVehicle in oVRP:
		oVehicleResults[oVehicleIDMapC2P[oVehicle.vehno]] = oVehicle

	#
	# 車両のTripを作成
	#
	for oVehicleJSON in sResponseJSON["vehicles"]:
		oVehicleJSON["trip"] = None
		oVehicleJSON["result"] = None
		if oVehicleJSON['id'] in oVehicleResults:
			oVehicleResult = oVehicleResults[oVehicleJSON['id']]
			sStartDateTime = oVehicleJSON['startDateTime']
			oTripList = []
			# 荷積みアクション
			for oCust in oVehicleResult.route:
				oCustomers = [oCust]
				if oRequestJSON["config"]["bulkShipping"]:
					oCustomers = oCust.customers
				for oCustomer in oCustomers:
					sPanaCustID = oCustIDMapC2P.get(oCustomer.custno)
					if sPanaCustID in oCustMap:
						if oCustMap[sPanaCustID]["requestType"] == 'DELIVERY':  # 判断是否是配达的时间
							oLoad = {
								'startDateTime': sStartDateTime,
								'startSpot': 'DEPOT',
								'action': 'LOAD',
								'arg': sPanaCustID
							}
							if oRequestJSON["config"]["enrouteCharging"]:
								oLoad['endEpower'] = round(oVehicleJSON['initEpower'], 4)
							oTripList.append(oLoad)
							# 拠点での荷積時間は0として扱うため、CNSソルバの計画はそのまま使うという仕様になっております（QA #102762）
							# 次のアクションの開始時間
							# sStartDateTime = addTime(sStartDateTime, oCustMap[sPanaCustID]["loadDuration"])

			# 移動アクション
			for i in range(1, len(oVehicleResult.route)):
				# 移動時間は1秒が超える場合
				if (oVehicleResult.arr[i] - oVehicleResult.arr[i - 1] - oVehicleResult.route[
					i - 1].servt) * 3600 > 1:
					#sPrePanaSpotID = oSpotIDMapC2P[oCNSSpotIDSort[oVehicleResult.route[i - 1].spotid]]
					sPrePanaSpotID = oSpotIDMapC2P[oCNSSpotIDSort[int(oVehicleResult.route[i - 1].spotid)]]
					oMove = {
						'startDateTime': addTime(sStartDateTime, oVehicleResult.lv[i - 1] * 3600),
						'startSpot': sPrePanaSpotID,
						'action': 'MOVE',
						'arg': ''
					}
					if oRequestJSON["config"]["enrouteCharging"]:
						# 出発する際の残電力量-充電量
						oMove['endEpower'] = round(oVehicleResult.rem_batt[i] - oVehicleResult.chg_batt[i], 4)
					oTripList.append(oMove)

				oCustomers = [oVehicleResult.route[i]]
				if oRequestJSON["config"]["bulkShipping"]:
					oCustomers = oVehicleResult.route[i].customers
				for oCustomer in oCustomers:
					sPanaCustID = oCustIDMapC2P.get(oCustomer.custno)
					# 荷下ろしアクション
					if sPanaCustID is not None and sPanaCustID != 'DEPOT':
						if sPanaCustID[:3] == "CS_":
							oCharge = {
								'startDateTime': addTime(sStartDateTime, oVehicleResult.arr[i] * 3600),
								'startSpot': sPanaCustID,
								'action': 'CHARGE',
								'arg': sPanaCustID
							}
							if oRequestJSON["config"]["enrouteCharging"]:
								oCharge['endEpower'] = round(oVehicleResult.rem_batt[i], 4)  # 电量
							oTripList.append(oCharge)

						else:
							# 荷下ろしの開始時間(車両出発時間＋到着時刻)
							sUnloadStartDateTime = addTime(sStartDateTime, oVehicleResult.arr[i] * 3600)
							# 荷下ろしを開始できる最も早い時刻 > 実際の到着時刻
							if compareToTime(oCustomer.ready, sUnloadStartDateTime) > 0:
								sUnloadStartDateTime = oCustomer.ready
							oUnload = {
								'startDateTime': sUnloadStartDateTime,
								'startSpot': oCustMap[sPanaCustID]["spot"],
								'action': 'UNLOAD',
								'arg': sPanaCustID
							}
							if oCustMap[sPanaCustID]['requestType'] == 'PICKUP':
								oUnload['action'] = 'LOAD'  # 装货
							if oRequestJSON["config"]["enrouteCharging"]:
								oUnload['endEpower'] = round(oVehicleResult.rem_batt[i], 4)  # 电量
							oTripList.append(oUnload)
			# 帰還スポット追加
			if len(oTripList) != 0:
				# 帰還スポットID
				#sEndSpotID = oSpotIDMapC2P[oCNSSpotIDSort[oVehicleResult.route[-1].spotid]]
				sEndSpotID = oSpotIDMapC2P[oCNSSpotIDSort[int(oVehicleResult.route[-1].spotid)]]
				# 帰還スポットへ戻る開始時間
				sBackStartDateTime = addTime(sStartDateTime, oVehicleResult.arr[-1] * 3600)
				# 集荷の荷下ろし時間（QA #102762）
				# nUnloadDuration = 0
				# 拠点へ戻る場合
				if sEndSpotID == 'DEPOT':
					for i in range(1, len(oVehicleResult.route) - 1):
						sPanaCustID = oCustIDMapC2P[oVehicleResult.route[i].custno]
						# 集荷あり
						if sPanaCustID in oCustMap and oCustMap[sPanaCustID]['requestType'] == 'PICKUP':
							oUnload = {
								'startDateTime': sBackStartDateTime, # addTime(sBackStartDateTime, nUnloadDuration)（QA #102762）
								'startSpot': 'DEPOT',
								'action': 'UNLOAD',
								'arg': sPanaCustID
							}
							# 拠点での荷積時間は0として扱うため、CNSソルバの計画はそのまま使うという仕様になっております（QA #102762）
							# nUnloadDuration += oCustMap[sPanaCustID]["unloadDuration"]
							if oRequestJSON["config"]["enrouteCharging"]:
								oUnload['endEpower'] = round(oVehicleResult.rem_batt[-1], 4)
							oTripList.append(oUnload)
				# Finish
				oFinish = {
					'startDateTime': sBackStartDateTime, # addTime(sBackStartDateTime, nUnloadDuration)（QA #102762）
					'startSpot': sEndSpotID,
					'action': 'FINISH',
					'arg': ''
				}
				if oRequestJSON["config"]["enrouteCharging"]:
					oFinish['endEpower'] = round(oVehicleResult.rem_batt[-1], 4)
				oTripList.append(oFinish)
				oVehicleJSON["trip"] = oTripList
				oVehicleJSON["result"] = {"status": "OK", "msg": ''}

	#
	# 配送できない荷物を設定
	#
	oUnassignedRequestIds = []
	for oCustJSON in oRequestJSON['requests']:
		nFlag = 0
		for sPanaVehicleID in oVehicleResults:
			for oCustomer in oVehicleResults[sPanaVehicleID].route:
				if oCustJSON['id'] == oCustIDMapC2P.get(oCustomer.custno):
					nFlag = 1
					break
			if nFlag == 1:
				break
		if nFlag == 0:
			oUnassignedRequestIds.append(oCustJSON['id'])
	sResponseJSON["unassignedRequestIds"] = oUnassignedRequestIds

	#
	# 制約チェック
	#
	bViolationErr = False
	for sVehicleJSON in sResponseJSON['vehicles']:
		if sVehicleJSON['trip'] is not None:
			oViolations = []
			# 電力違反
			if oVehicleIDMapP2C[sVehicleJSON["id"]] in oEVRPEVViolations:
				oViolations.append('ev plan with violation')

			# 営業時間枠制約
			if compareToTime(sVehicleJSON['trip'][-1]['startDateTime'], sVehicleJSON['endDateTime']) > 0:
				oViolations.append('normal plan with violation')

			# 積載量上限制約
			else:
				nLoadWeight = 0
				for oTrip in sVehicleJSON['trip']:
					# 積載量上限制約
					if oTrip['action'] == 'LOAD':
						nLoadWeight += oCustMap[oTrip['arg']]['weight']
					elif oTrip['action'] == 'UNLOAD':
						nLoadWeight -= oCustMap[oTrip['arg']]['weight']
					if nLoadWeight > sVehicleJSON['maxLoadWeight']:
						oViolations.append('normal plan with violation')
						break
					# 顧客時間枠制約
					if oTrip["startSpot"] == 'DEPOT':  # oRequestMap['spot_荷物4']['endDateTime']='2000/01/01 00:01:00'
						continue
					elif oTrip['action'] == 'LOAD' and oCustMap[oTrip["arg"]]['requestType'] == 'PICKUP':
						if compareToTime(oTrip['startDateTime'], oCustMap[oTrip["arg"]]['endDateTime']) > 0:
							oViolations.append('normal plan with violation')
							break
					elif oTrip['action'] == 'UNLOAD' and oCustMap[oTrip["arg"]]['requestType'] == 'DELIVERY':
						if compareToTime(oTrip['startDateTime'], oCustMap[oTrip["arg"]]['endDateTime']) > 0:
							oViolations.append('normal plan with violation')
							break

			if len(oViolations) > 0:
				sVehicleJSON["result"]["status"] = "Warn"
				sVehicleJSON["result"]["msg"] = "; ".join(oViolations)
				bViolationErr = True


	# 警告メッセージ
	oMsgs = []
	if bViolationErr: oMsgs.append("plan has violations")
	if bEVRPCreateErr: oMsgs.append("failed to create EV plan")
	if len(oMsgs) > 0:
		sResponseJSON["result"]["status"] = "Warn"
		sResponseJSON["result"]["msg"] = "; ".join(oMsgs)

	return sResponseJSON


#
# 立寄不可の車両ID群取得
#
def getUnvisitables(oSpotsJSON, sSpotID):
	oUnvisitableList = []
	for oSpotJSON in oSpotsJSON:
		if oSpotJSON["id"] == sSpotID:
			if "unvisitable" in oSpotJSON and oSpotJSON["unvisitable"] != "":
				oUnvisitableList = oSpotJSON["unvisitable"].split(",")
			break
	return oUnvisitableList


#
# 時間を加える
#
def addTime(sDateTime, nSec):
	return (datetime.datetime.strptime(sDateTime, "%Y/%m/%d %H:%M:%S") +
			datetime.timedelta(seconds=nSec)).strftime("%Y/%m/%d %H:%M:%S")


#
# 時間差
#
def minusDatetime(sDateTime1, sDateTime2):
	subTime = datetime.datetime.strptime(sDateTime1, '%Y/%m/%d %H:%M:%S') - \
			  datetime.datetime.strptime(sDateTime2, '%Y/%m/%d %H:%M:%S')
	return subTime.seconds / 3600.0


#
# 時間比較
#
def compareToTime(sStartDateTime, sEndDateTime):
	oStartDateTime = datetime.datetime.strptime(sStartDateTime, "%Y/%m/%d %H:%M:%S")
	oEndDateTime = datetime.datetime.strptime(sEndDateTime, "%Y/%m/%d %H:%M:%S")
	if oStartDateTime > oEndDateTime:
		return 1
	if oStartDateTime == oEndDateTime:
		return 0
	if oStartDateTime < oEndDateTime:
		return -1


#
# 表をソートする
#
def sortTable(oTable, oSortIdxs):
	oTable = np.array(oTable)
	nLen = len(oTable)
	oTableSort = np.zeros((nLen, nLen))
	for i in range(len(oSortIdxs)):
		oTableSort[:, i] = oTable[:, oSortIdxs[i]][oSortIdxs]
	return oTableSort.tolist()


#
# 電力制約違反があるかチェック
#
def checkEVPlan(oEVRP, nMarginalEpower):
	oVehicleIDList = []
	for oVehicle in oEVRP:
		bFlag = False
		for i in range(len(oVehicle.route)):
			if round(oVehicle.rem_batt[i], 2) < nMarginalEpower:
				bFlag = True
				break
		if bFlag: oVehicleIDList.append(oVehicle.vehno)
	return oVehicleIDList


#
# この文字列にencode("uft8")を実行する．そうしないとJava側で文字化けする．
#
def encode(sResponseJSON):
	# return json.dumps(sResponseJSON, ensure_ascii=False).encode(encoding='UTF-8')
	return json.dumps(sResponseJSON, ensure_ascii=False)


def driver_command_line(test_no, args, caplog, continuity_test_flag=False):
	oError = []
	try:
		argv = []
		path = os.path.join("test", "apiTest", "log")
		os.makedirs(path, exist_ok=True)
		vlog_filename = result.command_line_log_init(caplog, test_no)

		# 相対path変更
		def change_path(args, table_type):
			if "-"+table_type+"s" in args: 
				csv_reader=csv.reader(open(args["-"+table_type+"s"][args["-"+table_type+"s"].find("d"):],"r"),delimiter=",",quotechar='"')
				with open(path + "/" + table_type+ "s_" + test_no + ".csv", "w", newline="") as f:
					writer = csv.writer(f)
					for row in csv_reader:
						if not "START_TIME" in row[0]:
							row[1] = row[1][6:]
						writer.writerow(row)
				args["-"+table_type+"s"] = "./" + table_type + "s_" + test_no + ".csv"
			elif "-"+table_type+"sv" in args: 
				csv_reader=csv.reader(open(args["-"+table_type+"sv"][args["-"+table_type+"sv"].find("d"):],"r"),delimiter=",",quotechar='"')
				with open(path + "/" + table_type+ "sv_" + test_no + ".csv", "w", newline="") as f:
					writer = csv.writer(f)
					for row in csv_reader:
						if not "VEHICLE_TYPE" in row[0]:
							if table_type == "ec":
								row[3] = row[3][6:]
							else:
								row[2] = row[2][6:]
						writer.writerow(row)
				args["-"+table_type+"sv"] = "./" + table_type + "sv_" + test_no + ".csv"

			return args

		# ds, dsv中のpath変更
		args = change_path(args, "d")
		# ts, tsv中のpath変更
		args = change_path(args, "t")
		# ecs, ecsv中のpath変更
		if "-ec" in args: 
			args = change_path(args, "ec")
		# xs, xsv中のpath変更
		if "-x" in args: 
			args = change_path(args, "x")

		# main実行
		for key,value in args.items():
			if key=="main":
				argv.append(value)
			else:
				argv.append(key)
				argv.append(value)

		sys.argv = argv

		currentDir = os.getcwd()
		os.chdir(path)

		main_tw.main()

		os.chdir(currentDir)

	except Exception as e:
		# 例外取得
		if len(e.args) == 0:
			#エラーメッセージが存在しない場合
			oError.append("Error Mesg None")
		else:
			#エラーメッセージが存在する場合
			oError.append(str(e.args[0]))

	if oError == []:
		# コマンドライン実行期待値比較
		cmp_ret = result.CmpCommand(test_no, args)
		# Error期待値比較できない場合は正常動作とする
		err_ret = True

		# 制約条件(ハード)確認
		result.CommandLine_HardCheck_MaxNumVehicleVisits(test_no, args, path)
		result.CommandLine_HardCheck_MaxNumVehicleRotates(test_no, args, path)
		result.CommandLine_HardCheck_Impassable(test_no, args, path)
		result.CommandLine_HardCheck_VehicleStopOff(test_no, args, path)
		result.CommandLine_HardCheck_ChargingSpotTimeFrame(test_no, args, path)
		result.CommandLine_HardCheck_EVChargeLimit(test_no, args, path)

		# 配送計画出力要件確認
		result.CommandLine_PlacCheck_TopBase(test_no, path)
		result.CommandLine_PlacCheck_TopEndExceptBase(test_no, path)
		result.CommandLine_PlacCheck_EndVehicleSpecificationEndSpot(test_no, args, path)
		result.CommandLine_PlacCheck_EndVehicleEndSpot(test_no, args, path)
		result.CommandLine_PlacCheck_AllDeliveryOnce(test_no, args, path)
		result.CommandLine_PlacCheck_NonContinuous(test_no, args, path)
		result.CommandLine_PlacCheck_ChargeSpotOne(test_no, args, path)

		#Vehicleログ掃き出し
		result.vehicle_log_dump(caplog, vlog_filename)

	else:
		# Vehicles期待値比較できない場合は正常動作とする(Error側のみ比較とするため)
		cmp_ret = True
		# Error期待値比較
		os.chdir("../../../")
		raise ValueError(oError)
		err_ret = result.CmpError(test_no, oError)

	# 期待値比較結果確認
	# (Vehicles期待値比較とErrorの期待値ファイルを両方出力してから内容を確認)

	# 導通テストの場合エラーチェックを行わない(フリーズの有無のみを確認)
	if not continuity_test_flag:
		assert cmp_ret
		assert err_ret

	return


def generate_file_for_replanning(test_no, sParamJSON):
	try:
		#-----------------------------------------------------------------
		# JSON解析
		#-----------------------------------------------------------------
		oParamJSON = json.loads(sParamJSON)
		oRequestJSON = oParamJSON["requestJSON"]
		oCostJSON = oParamJSON["costJSON"]


		# Args
		# starttime更新
		path = os.path.join("test", "apiTest", "log")

		# 車両1のroute, arr取得
		csv_reader=csv.reader(open(os.path.join(path,"output_"+test_no+".detail.csv"),"r"),delimiter=",",quotechar='"')
		route_custno = []
		arr = []
		for row in csv_reader:
			if "VEHICLE" in row[0]:
				route_custno = row[4:4+int(row[3])]
			if "ARRIVAL" in row[0]:
				arr = row[4:4+int(row[3])]
				break

		# route, arrから充電スポット削除
		e_index = [i for i, c in enumerate(route_custno) if c.startswith("e")]
		for e in e_index:
			del route_custno[e]
			del arr[e]

		# 配送先1のservt取得
		for cust in oRequestJSON["NormalCustomer"]:
			if cust["custno"] == route_custno[1]:
				servt = cust["servt"]
				break

		# starttime計算
		lv = float(arr[1]) * 3600 + float(servt) * 60
		starttime = datetime.datetime.strptime(oRequestJSON["Args"]["starttime"], '%Y/%m/%d %H:%M:%S').timestamp()
		starttime_str = str(datetime.datetime.fromtimestamp(starttime+lv)).replace("-", "/")
		oRequestJSON["Args"]["starttime"] = starttime_str[:starttime_str.find(".")]

		# 35-1-5~7 パラメータ修正
		if test_no == "35_1_5":
			oRequestJSON["Args"]["starttime"] = "2000/1/1 15:00:00"
		if test_no == "35_1_6":
			oRequestJSON["Args"]["lastc_flag"] = False
		if test_no == "35_1_7":
			oRequestJSON["Args"]["opt_d_weight"] = 0.0
			oRequestJSON["Args"]["opt_t_weight"] = 1.0


		# NormalCustomer
		# 全spotid取得
		all_ids = []
		for cust in oRequestJSON["NormalCustomer"]:
			all_ids.append(cust["spotid"])
		for cust in oRequestJSON["EndSpots"]:
			all_ids.append(cust["spotid"])
		if "ESpots" in oRequestJSON.keys():
			for cust in oRequestJSON["ESpots"]:
				all_ids.append(cust["spotid"])
		all_ids = list(set(all_ids))
		all_ids.sort()

		# spotidのインデックス取得(コストテーブル)
		all_ids_index = copy.copy(all_ids)
		for i in range(len(all_ids_index)):
			if all_ids_index[i].startswith("e"):
				all_ids_index[i] = int(i)
			else:
				all_ids_index[i] = int(all_ids_index[i])


		# EndSpots
		# 1台目以外削除
		del oRequestJSON["EndSpots"][1:]


		# InitialRoutes
		# 1台目以外削除
		del oRequestJSON["InitialRoutes"][1:]
		route = []
		route.append(oRequestJSON["InitialRoutes"][0][0])
		del route_custno[0]
		for c in route_custno:
			route.append(int(c))
		route[1] = 0
		route[-1] = 0
		route_replanning = []
		route_replanning.append(route)

		# routeのspotid取得
		route_ids = []
		for custno in route_custno:
			for cust in oRequestJSON["NormalCustomer"]:
				if cust["custno"] == custno:
					route_ids.append(cust["spotid"])
					break
			for cust in oRequestJSON["EndSpots"]:
				if cust["custno"] == custno:
					route_ids.append(cust["spotid"])
					break
		route_ids = list(set(route_ids))
		route_ids.sort()


		# 拠点、不要配送先削除
		del_index = []
		cnt = 0
		for cust in oRequestJSON["NormalCustomer"]:
			if not cust["custno"] in route_custno or cust["spotid"] == "0":
				del_index.append(cnt)
			cnt += 1

		for index in reversed(del_index):
			del oRequestJSON["NormalCustomer"][index]

		# 拠点設定(spotidはそのまま)
		oRequestJSON["NormalCustomer"][0]["custno"] = "0"
		oRequestJSON["NormalCustomer"][0]["reje_ve"] = []


		# Vehicles
		# 1台目以外削除
		del oRequestJSON["Vehicles"][1:]

		# startoptime更新
		oRequestJSON["Vehicles"][0]["startoptime"] = oRequestJSON["Args"]["starttime"]

		# E-INITIAL更新
		with open(os.path.join("test","apiTest", "out", "out_"+test_no+".txt"), "r") as out_file:
			for row in out_file:
				if "rem_batt" in row:
					spaces = [i for i, s in enumerate(row) if s == " "]
					if test_no == "35_1_8":
						start_index = spaces[1] + 1
					else:
						start_index = spaces[0] + 1
					end_index = row[start_index:].find(",") + start_index
					e_initial = row[start_index:end_index]
					break
		evehicle_dict = oRequestJSON["Vehicles"][0]["evehicle_dict"]
		start_index = evehicle_dict.find("INITIAL") + len("INITIAL") + 3
		end_index = evehicle_dict[start_index:].find(",") + start_index
		oRequestJSON["Vehicles"][0]["evehicle_dict"] = evehicle_dict[:start_index] + e_initial + evehicle_dict[end_index:]


		# request作成
		request_replanning = {}
		request_replanning["Args"] = oRequestJSON["Args"]
		request_replanning["NormalCustomer"] = oRequestJSON["NormalCustomer"]
		if "ESpots" in oRequestJSON.keys():
			request_replanning["ESpots"] = oRequestJSON["ESpots"]
		request_replanning["Vehicles"] = oRequestJSON["Vehicles"]
		request_replanning["EndSpots"] = oRequestJSON["EndSpots"]
		request_replanning["InitialRoutes"] = route_replanning

		path = os.path.join("test", "apiTest", "test_data")
		test_no_replanning = test_no + "_replanning"
		request_replanning_name = test_no_replanning + "_request.json"
		with open(path + "/" + request_replanning_name, "w") as f:
			data = json.dumps(request_replanning, indent=4)
			f.write(data)


		# テーブル編集
		def del_table(table_data, route_ids, lv_time, e_flag=False):
			management_table = []
			for oTable in table_data:
				table = []
				size = len(oTable["table"])
				for y in range(size):
					row = []
					for x in range(size):
						if x in route_ids and y in route_ids:
							row.append(oTable["table"][y][x])
					if row:
						table.append(row)

				start_time = oTable["start_time"] - lv_time
				if e_flag:
					if oTable["vtype"] == "common" and oTable["etype"] == "common":
						start_time = 0.0
					management_table.append({
						"vtype"      : oTable["vtype"],  # 車両タイプ
						"etype"      : oTable["etype"],  # 電動車両タイプ
						"start_time" : start_time,  # 開始時刻(基準日時からの相対時間，単位hour)
						"table": table
					})
				else:
					if oTable["vtype"] == "common":
						start_time = 0.0
					management_table.append({
						"vtype"      : oTable["vtype"],  # 車両タイプ
						"start_time" : start_time,  # 開始時刻(基準日時からの相対時間，単位hour)
						"table": table
					})

			return management_table, len(table)


		# コストテーブル インデックス指定
		table_index = []
		for id in route_ids:
			for i in range(len(all_ids)):
				if id == all_ids[i]:
					table_index.append(all_ids_index[i])
					break
		if "ESpots" in oRequestJSON.keys():
			for cust in oRequestJSON["ESpots"]:
				for i in range(len(all_ids)):
					if cust["spotid"] == all_ids[i]:
						table_index.append(all_ids_index[i])
						break

		# cost作成
		cost_replanning = {}
		# 距離表
		management_dtable, table_size = del_table(oCostJSON["management_dtable"], table_index, lv/3600)
		cost_replanning["management_dtable"] = management_dtable
		# 時間表
		management_ttable, _ = del_table(oCostJSON["management_ttable"], table_index, lv/3600)
		cost_replanning["management_ttable"] = management_ttable
		# 消費電力表
		if "management_etable" in oCostJSON.keys():
			management_etable, _ = del_table(oCostJSON["management_etable"], table_index, lv/3600, True)
			cost_replanning["management_etable"] = management_etable
		# 経験コスト表
		if "management_xtable" in oCostJSON.keys():
			management_xtable, _ = del_table(oCostJSON["management_xtable"], table_index, lv/3600)
			cost_replanning["management_xtable"] = management_xtable

		cost_replanning_name = str(table_size) + "x" + str(table_size) + "_cost_" + test_no_replanning + ".json"
		with open(path + "/cost/" + cost_replanning_name, "w") as f:
			data = json.dumps(cost_replanning, indent=4)
			f.write(data)

		return test_no_replanning, request_replanning_name, cost_replanning_name

	except Exception as e:
		assert e
