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
import logging
from io import StringIO
import re
import numpy as np
import copy

def CreateArgs(args):
	instance = main_tw.Argvs([])
	instance.avetime_ape_flag					= args["avetime_ape_flag"]
	instance.avetime_ape						= float(args["avetime_ape"])
	instance.avetime_ae_flag					= args["avetime_ae_flag"]
	instance.avetime_ae							= float(args["avetime_ae"])
	instance.time_error							= float(args["time_error"])
	instance.avevisits_ape_flag					= args["avevisits_ape_flag"]
	instance.avevisits_ape						= float(args["avevisits_ape"])
	instance.avevisits_ae_flag					= args["avevisits_ae_flag"]
	instance.avevisits_ae						= int(args["avevisits_ae"])
	instance.visits_error						= float(args["visits_error"])
	instance.avedist_ape_flag					= args["avedist_ape_flag"]
	instance.avedist_ape						= float(args["avedist_ape"])
	instance.avedist_ae_flag					= args["avedist_ae_flag"]
	instance.avedist_ae							= float(args["avedist_ae"])
	instance.dist_error							= float(args["dist_error"])
	instance.aload								= float(args["aload"])
	instance.atpen								= float(args["atpen"])
	instance.bulkShipping_flag					= args["bulkShipping_flag"]
	instance.decr_vnum							= args["decr_vnum"]
	instance.delaystart_flag 					= args["delaystart_flag"]
	instance.delaystart2_flag 					= args["delaystart2_flag"]
	instance.demand								= int(args["demand"])
	instance.demand2 							= int(args["demand2"])
	instance.distfile							= args["distfile"] if args["distfile"]!="None" else None
	instance.ds_file 							= args["ds_file"] if args["ds_file"]!="None" else None
	instance.dsv_file							= args["dsv_file"] if args["dsv_file"]!="None" else None
	instance.e_cost_file						= args["e_cost_file"] if args["e_cost_file"]!="None" else None
	instance.e_spot_file						= args["e_spot_file"] if args["e_spot_file"]!="None" else None
	instance.e_vehicle_file						= args["e_vehicle_file"] if args["e_vehicle_file"]!="None" else None
	instance.endtime 							= str(args["endtime"])
	instance.es_file 							= args["es_file"] if args["es_file"]!="None" else None
	instance.esv_file							= args["esv_file"] if args["esv_file"]!="None" else None
	instance.evplan								= args["evplan"]
	instance.hetero_file						= args["hetero_file"] if args["hetero_file"]!="None" else None
	instance.initfile							= args["initfile"] if args["initfile"]!="None" else None
	instance.inputfile							= args["inputfile"] if args["inputfile"]!="None" else None
	instance.ivnum								= args["ivnum"]
	instance.lastc2_flag						= args["lastc2_flag"]
	instance.lastc_flag							= args["lastc_flag"]
	instance.lowertime							= float(args["lowertime"])
	#instance.ls_d_starttime					= float(args["ls_d_starttime"])
	instance.management_dtable					= []	# args["management_dtable"] API内部で設定するためここでは設定不要
	instance.management_etable					= []	# args["management_etable"] API内部で設定するためここでは設定不要
	instance.management_ttable					= []	# args["management_ttable"] API内部で設定するためここでは設定不要
	instance.management_xtable					= []	# args["management_xtable"] API内部で設定するためここでは設定不要
	instance.maxvisit_file						= args["maxvisit_file"] if args["maxvisit_file"]!="None" else None
	instance.multi_p 				= int(args["multi_p"])
	instance.non_violation_vehicles_route		= {} #args["non_violation_vehicles_route"]
	instance.num_addv		  					= int(args["num_addv"])
	instance.opt_d_weight						= float(args["opt_d_weight"])
	instance.opt_e_weight						= float(args["opt_e_weight"])
	instance.opt_t_weight						= float(args["opt_t_weight"])
	instance.opt_x_weight						= float(args["opt_x_weight"])
	instance.outputfile							= args["outputfile"] if args["outputfile"]!="None" else None
	instance.outputs_count						= float(args["outputs_count"])
	instance.outputstime						= float(args["outputstime"])
	instance.pickup_flag						= args["pickup_flag"]
	instance.rejectve_file						= args["rejectve_file"] if args["rejectve_file"]!="None" else None
	instance.skillfile			  				= args["skillfile"] if args["skillfile"]!="None" else None
	instance.skillflag		 	 				= int(args["skillflag"])
	instance.skillshuffle_flag					= args["skillshuffle_flag"]
	instance.spot_input_file 					= args["spot_input_file"] if args["spot_input_file"]!="None" else None
	instance.spotid_list						= []	# args["spotid_list"]
	instance.st_top			  					= args["st_top"] if args["st_top"]!="None" else None
	instance.starttime		  					= args["starttime"] if args["starttime"]!="None" else None
	instance.timefile		 	 				= args["timefile"] if args["timefile"]!="None" else None
	instance.timeout 							= float(args["timeout"])
	instance.ts_file 							= args["ts_file"] if args["ts_file"]!="None" else None
	instance.tsv_file							= args["tsv_file"] if args["tsv_file"]!="None" else None
	instance.uppertime			  				= float(args["uppertime"])
	instance.vendfile							= args["vendfile"] if args["vendfile"]!="None" else None
	instance.vtimefile		  					= args["vtimefile"] if args["vtimefile"]!="None" else None
	instance.x_cost_file						= args["x_cost_file"] if args["x_cost_file"]!="None" else None
	instance.xs_file 							= args["xs_file"] if args["xs_file"]!="None" else None
	instance.xsv_file			  				= args["xsv_file"] if args["xsv_file"]!="None" else None
	instance.multitrip_flag						= args["multitrip_flag"]
	instance.multitripv_file					= args["multitripv_file"] if args["multitripv_file"]!="None" else None
	instance.multitripc_file					= args["multitripc_file"] if args["multitripc_file"]!="None" else None
	instance.multithread_num					= int(args["multithread_num"])
	instance.timePenaltyWeight					= float(args["timePenaltyWeight"])
	instance.loadPenaltyWeight					= float(args["loadPenaltyWeight"])
	instance.load2PenaltyWeight					= float(args["load2PenaltyWeight"])
	instance.timeLevelingPenaltyWeight			= float(args["timeLevelingPenaltyWeight"])
	instance.visitsLevelingPenaltyWeight		= float(args["visitsLevelingPenaltyWeight"])
	instance.distLevelingPenaltyWeight			= float(args["distLevelingPenaltyWeight"])
	instance.timePenaltyType					= args["timePenaltyType"]
	instance.loadPenaltyType					= args["loadPenaltyType"]
	instance.load2PenaltyType					= args["load2PenaltyType"]
	instance.timeLevelingPenaltyType			= args["timeLevelingPenaltyType"]
	instance.visitsLevelingPenaltyType			= args["visitsLevelingPenaltyType"]
	instance.distLevelingPenaltyType			= args["distLevelingPenaltyType"]
	instance.addtwspot_file						= args["addtwspot_file"]
	instance.demdivideunit_file					= args["demdivideunit_file"]
	instance.rmpriority_file					= args["rmpriority_file"]
	instance.init_changecust					= args["init_changecust"]
	instance.break_interval						= args["break_interval"]
	instance.break_time							= args["break_time"]
	instance.break_interval						= args["break_interval"]
	instance.opt_unassigned_flag				= args["opt_unassigned_flag"]
	instance.sparePenaltyWeight					= args["sparePenaltyWeight"]



	return instance


def CreateCustomer(param, oArgvs):
	custs = []
	for cust in param:
		additionalcust=[main_tw.Customer(
			ac[0],				# custno    : 顧客ID
			ac[3],				# spotid    : スポットID
			cust["y"],					# y         : 緯度
			cust["x"],					# x         : 経度
			cust["dem"],            	# dem       : 荷物重量
			cust["dem2"],				# dem2      : 荷物重量2
			ac[1],				# ready     : 拠点の営業開始日時 例:"2000/1/1 0:00:00"
			ac[2],				# due       : 拠点の営業終了日時 例:"2000/1/1 0:00:00"
			cust["servt"],				# servt     : 荷作業時間
			oArgvs.starttime,			# starttime : ソルバ基準日時，ここでは未確定のため仮の値を設定しておく(後で設定)
			cust["reje_ve"],			# reje_ve   : 立寄不可車両群
			ac[5],				# parkt_arrive		: 駐車時間
			ac[4],				# parkt_open		: 駐車時間
			cust["depotservt"],			# depotservt: 拠点作業時間
			[],			# addtwspot : 追加時間枠場所
			cust["demdivideunit"],		# demdivideunit: 分割単位
			cust["requestType"],		# requestType: 荷物の種類
			ac[6],			# deny_unass: 未割当て禁止フラグ
			cust["e_rate"]) for ac in cust["additionalcust"]]
		# 配送先の追加
		custs.append(main_tw.Customer(
			cust["custno"],				# custno    : 顧客ID
			cust["spotid"],				# spotid    : スポットID
			cust["y"],					# y         : 緯度
			cust["x"],					# x         : 経度
			cust["dem"],            	# dem       : 荷物重量
			cust["dem2"],				# dem2      : 荷物重量2
			cust["ready"],				# ready     : 拠点の営業開始日時 例:"2000/1/1 0:00:00"
			cust["due"],				# due       : 拠点の営業終了日時 例:"2000/1/1 0:00:00"
			cust["servt"],				# servt     : 荷作業時間
			oArgvs.starttime,			# starttime : ソルバ基準日時，ここでは未確定のため仮の値を設定しておく(後で設定)
			cust["reje_ve"],			# reje_ve   : 立寄不可車両群
			cust["parkt_arrive"],				# parkt_arrive		: 駐車時間
			cust["parkt_open"],				# parkt_open		: 駐車時間
			cust["depotservt"],			# depotservt: 拠点作業時間
			additionalcust,			# addtwspot : 追加時間枠場所
			cust["demdivideunit"],		# demdivideunit: 分割単位
			cust["requestType"],		# requestType: 荷物の種類
			cust["deny_unass"],			# deny_unass: 未割当て禁止フラグ
			cust["e_rate"]))			# e_rate    : 充電レート
	return custs

def CreateVehicle(vehs_param, espot_param, oArgvs):
	vehs = []
	for veh, cust in zip(vehs_param, espot_param):
		additionalcust=[main_tw.Customer(
			ac[0],				# custno    : 顧客ID
			ac[3],				# spotid    : スポットID
			cust["y"],					# y         : 緯度
			cust["x"],					# x         : 経度
			cust["dem"],            	# dem       : 荷物重量
			cust["dem2"],				# dem2      : 荷物重量2
			ac[1],				# ready     : 拠点の営業開始日時 例:"2000/1/1 0:00:00"
			ac[2],				# due       : 拠点の営業終了日時 例:"2000/1/1 0:00:00"
			cust["servt"],				# servt     : 荷作業時間
			oArgvs.starttime,			# starttime : ソルバ基準日時，ここでは未確定のため仮の値を設定しておく(後で設定)
			cust["reje_ve"],			# reje_ve   : 立寄不可車両群
			ac[5],				# parkt_arrive		: 駐車時間
			ac[4],				# parkt_open		: 駐車時間
			cust["depotservt"],			# depotservt: 拠点作業時間
			[],			# addtwspot : 追加時間枠場所
			cust["demdivideunit"],		# demdivideunit: 分割単位
			cust["requestType"],		# requestType: 荷物の種類
			ac[6],		# deny_unass: 未割当て禁止フラグ
			cust["e_rate"]) for ac in cust["additionalcust"]]

		# EndSpotカスタマーインスタンス生成
		endspot = main_tw.Customer(
				cust["custno"],				# custno    : 顧客ID
				cust["spotid"],				# spotid    : スポットID
				cust["y"],					# y         : 緯度
				cust["x"],					# x         : 経度
				cust["dem"],            	# dem       : 荷物重量
				cust["dem2"],				# dem2      : 荷物重量2
				cust["ready"],				# ready     : 拠点の営業開始日時 例:"2000/1/1 0:00:00"
				cust["due"],				# due       : 拠点の営業終了日時 例:"2000/1/1 0:00:00"
				cust["servt"],				# servt     : 荷作業時間
				oArgvs.starttime,			# starttime : ソルバ基準日時，ここでは未確定のため仮の値を設定しておく(後で設定)
				cust["reje_ve"],			# reje_ve   : 立寄不可車両群
				cust["parkt_arrive"],				# parkt_arrive		: 駐車時間
				cust["parkt_open"],				# parkt_open		: 駐車時間
				cust["depotservt"],			# depotservt: 拠点作業時間
				additionalcust,			# addtwspot : 追加時間枠場所
				cust["demdivideunit"],		# demdivideunit: 分割単位
				cust["requestType"],		# requestType: 荷物の種類
				cust["deny_unass"],		# deny_unass: 未割当て禁止フラグ
				cust["e_rate"])			# e_rate    : 充電レート

		takebreak = []
		for t in veh['takebreak']:
			additionalcust=[main_tw.Customer(
				ac[0],						# custno    : 顧客ID
				ac[3],						# spotid    : スポットID
				t["y"],						# y         : 緯度
				t["x"],						# x         : 経度
				t["dem"],            		# dem       : 荷物重量
				t["dem2"],					# dem2      : 荷物重量2
				ac[1],						# ready     : 拠点の営業開始日時 例:"2000/1/1 0:00:00"
				ac[2],						# due       : 拠点の営業終了日時 例:"2000/1/1 0:00:00"
				ac[6],						# servt     : 荷作業時間
				oArgvs.starttime,			# starttime : ソルバ基準日時，ここでは未確定のため仮の値を設定しておく(後で設定)
				t["reje_ve"],				# reje_ve   : 立寄不可車両群
				ac[4],						# parkt_arrive		: 駐車時間
				ac[5],						# parkt_open		: 駐車時間
				t["depotservt"],			# depotservt: 拠点作業時間
				[],							# addtwspot : 追加時間枠場所
				t["demdivideunit"],			# demdivideunit: 分割単位
				t["requestType"],			# requestType: 荷物の種類
				t["deny_unass"],		# deny_unass: 未割当て禁止フラグ
				t["e_rate"]) for ac in t['additionalcust']]

			takebreak.append(main_tw.Customer(
				t["custno"],				# custno    : 顧客ID
				t["spotid"],				# spotid    : スポットID
				t["y"],						# y         : 緯度
				t["x"],						# x         : 経度
				t["dem"],            		# dem       : 荷物重量
				t["dem2"],					# dem2      : 荷物重量2
				t["ready"],					# ready     : 拠点の営業開始日時 例:"2000/1/1 0:00:00"
				t["due"],					# due       : 拠点の営業終了日時 例:"2000/1/1 0:00:00"
				t["servt"],					# servt     : 荷作業時間
				oArgvs.starttime,			# starttime : ソルバ基準日時，ここでは未確定のため仮の値を設定しておく(後で設定)
				t["reje_ve"],				# reje_ve   : 立寄不可車両群
				t["parkt_arrive"],			# parkt_arrive		: 駐車時間
				t["parkt_open"],			# parkt_open		: 駐車時間
				t["depotservt"],			# depotservt: 拠点作業時間
				additionalcust,				# 
				t["demdivideunit"],			# demdivideunit: 分割単位
				t["requestType"],			# requestType: 荷物の種類
				t["deny_unass"],		# deny_unass: 未割当て禁止フラグ
				t["e_rate"]))

		# Vehicleインスタンス生成
		evehicle_dict = json.loads(veh["evehicle_dict"].replace("'", '"'))
		vehs.append(main_tw.Vehicle(
			veh["vehno"],			# _vehno
			veh["cap"],				# _cap
			veh["cap2"],			# _cap2
			evehicle_dict,			# _evehicle_dict
			veh["vtype"],			# _vtype
			veh["etype"],			# _etype
			endspot,				# _enddepot
			veh["maxvisit"],		# _maxvisit
			veh["startoptime"],		# _startoptime
			veh["endoptime"],		# _endoptime
			veh["maxrotate"],		# _maxrotate
			veh["depotservt"],		# depotservt
			veh["rmpriority"],		# rmpriority
			takebreak,				# takebreak
			veh["reserved_v_flag"], # reserved_v_flag
			oArgvs))				# oArgvs

	return vehs
