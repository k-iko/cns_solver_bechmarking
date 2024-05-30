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
import math
import copy
import csv
import subprocess

unique_no = 0 # テスト固有番号(テストの乱数シードを変更したい場合この値を任意の整数へ変更)

sample_num = 1 #テスト数

# NormalCustomer
customer_num = 50 #配送先数
max_x = 50
min_x = -50
max_y = 50
min_y = -50

# Vehicles
vehicle_num = 4 #車両数

#-----------------------------------------------------------------
# ランダムテストパターン作成
#-----------------------------------------------------------------
def Create_Random_Data():
	os.makedirs("plot", exist_ok=True)
	random.seed(sample_num * unique_no)

	for i in range(sample_num):
		coordinate=[[0,0]]
		#inputファイル作成
		with open("plot/input_"+str(i)+".csv","w", newline="") as f:
			csv_writer = csv.writer(f)
			csv_writer.writerow(["CUST NO.","LATITUDE","LONGITUDE","DEMAND","FROM TIME","TO TIME","SERVICE TIME"])

			csv_writer.writerow([0,0,0,0,"2000/1/1 9:00:00","2000/1/1 17:00:00",0])
			for j in range(1,customer_num):
				x = random.uniform(min_x,max_x)
				y = random.uniform(min_y,max_y)
				csv_writer.writerow([j,x,y,0,"2000/1/1 9:00:00","2000/1/1 17:00:00",0])
				coordinate.append([x,y])

		#distファイル作成
		with open("plot/dist_"+str(i)+".csv","w", newline="") as f:
			csv_writer = csv.writer(f)
			
			for j in range(customer_num):
				for k in range(customer_num):
					dist = math.sqrt((coordinate[j][0]-coordinate[k][0])*(coordinate[j][0]-coordinate[k][0])+(coordinate[j][1]-coordinate[k][1])*(coordinate[j][1]-coordinate[k][1]))
					csv_writer.writerow([j,k,dist])

		#timeファイル作成
		with open("plot/time_"+str(i)+".csv","w", newline="") as f:
			csv_writer = csv.writer(f)
			
			for j in range(customer_num):
				for k in range(customer_num):
					dist = math.sqrt((coordinate[j][0]-coordinate[k][0])*(coordinate[j][0]-coordinate[k][0])+(coordinate[j][1]-coordinate[k][1])*(coordinate[j][1]-coordinate[k][1]))
					time = dist/max(max_x-min_x,max_y-min_y)/10
					csv_writer.writerow([j,k,time])
					
def Run_Solver():
	for i in range(sample_num):
		with open("plot/result_"+str(i)+".txt","w") as f:
			subprocess.run(["python","./code/main_tw.py",
							"-i","plot/input_"+str(i)+".csv",
							"-d","plot/dist_"+str(i)+".csv",
							"-t","plot/time_"+str(i)+".csv",
							"-o","plot/output_"+str(i)+".csv",
							"-v",str(vehicle_num),
							"-s","2000/1/1 9:00:00",
							"-e","2000/1/1 17:00:00",
							"-avennum","1"],
							stdout=f)

def Plot_Route():
	for i in range(sample_num):
		subprocess.run(["python","./plot_result.py",
						"plot/output_"+str(i)+".csv",
						"plot/input_"+str(i)+".csv",
						"plot/out_"+str(i)+".html"])

#-----------------------------------------------------------------
# メイン関数(ランダムテストパターン生成)
#-----------------------------------------------------------------
if __name__ == '__main__':
	Create_Random_Data()

	Run_Solver()

	Plot_Route()
