#include "delay_arr_time2.hpp"

/**
 * @brief 早着する配送先があれば出発時刻を遅延する( 最適化考慮)関数
 * @param[in] customers 配送先リスト
 * @param[in] vehicles 車両リスト
 * @param[in] argvs グローバル変数格納用
 * @details 早着する配送先があれば出発時刻を遅延する(最適化考慮)関数。
 */
void delay_arr_time2(const vector<Customer> &customers, vector<Vehicle> &vehicles, const Cppargs &argvs){
    delay_arr_time(customers, vehicles, argvs);

    double adist = 0, bdist = 0, atime = 0, btime = 0, apower = 0, bpower = 0, aexp = 0, bexp = 0, atpen = 0, btpen = 0;

    map<int, double> bmvehicles, amvehicles;

    int vcnt = -1;
	for(auto v : vehicles){
        vcnt++;
        int route_size = (int) v.route.size();

        // 未割り当て荷物最適化 予約車両は除外する
        if(argvs.break_flag==true || v.reserved_v_flag==true){
            v.delay_arr = v.arr;
            v.delay_st = v.st;
            v.delay_lv = v.lv;
            v.delay_totalcost = v.totalcost;
            v.delay_totalcost_wolast = v.totalcost_wolast;
            v.delay_rem_batt = v.rem_batt;
            v.delay_chg_batt = v.chg_batt;
            v.delay_slack_t = v.slack_t;
            v.delay_chg_batt_dict = v.chg_batt_dict;
            // v.update_totalcost_dtable(customers, argvs);
            continue;
        }

        bool all_depo = true;
        for(int i = 1; i < route_size-1; i++){
            if((customers.at(v.route.at(i)).custno != "0")&&(customers.at(v.route.at(i)).custno.at(0) != 'b')){
                all_depo = false;
                break;
            }
        }
		if(all_depo){
			continue;
        }

		int last_idx = route_size-1;
		if(argvs.lastc_flag == true){
			for(int i = route_size-1; i >= 0; i--){
				if(customers.at(v.route.at(i)).custno == "0" || i == route_size-1){
					last_idx = i;
                }
				else{
                    last_idx--;
					break;
                }
            }
        }

        if(compare_time(argvs.lowertime) > 0 || compare_time(argvs.uppertime) > 0){
			return_lutime_penalty(v.delay_lv.at(last_idx)-v.delay_arr.at(0), argvs.lowertime, argvs.uppertime, v.lv.at(last_idx)-v.arr.at(0), atpen, btpen);
        }

        bmvehicles[vcnt] = v.lv.at(last_idx)-v.arr.at(0);
		amvehicles[vcnt] = v.delay_lv.at(last_idx)-v.delay_arr.at(0);

		for(int i = 0; i < last_idx; i++){            
			bdist+=get_dtable(v.vtype, v.lv.at(i), argvs.management_dtable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno);
			adist+=get_dtable(v.vtype, v.delay_lv.at(i), argvs.management_dtable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno);

			if(!argvs.management_etable.empty()){
				bpower+=get_etable(v.vtype, v.etype, v.lv.at(i), argvs.management_etable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno);
				apower+=get_etable(v.vtype, v.etype, v.delay_lv.at(i), argvs.management_etable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno);
            }
			if(!argvs.management_xtable.empty()){
				bexp+=get_xtable(v.vtype, v.lv.at(i), argvs.management_xtable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno);
				aexp+=get_xtable(v.vtype, v.delay_lv.at(i), argvs.management_xtable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno);
            }
        }

		btime += v.lv.at(last_idx)-v.arr.at(0);
		atime += v.delay_lv.at(last_idx)-v.delay_arr.at(0);
    }

    double btime_error, atime_error, bdist_error, adist_error;
    if(argvs.avetime_ape_flag == true){
		btime_error = calc_avetime_APE(vehicles, customers, bmvehicles, argvs.avetime_ape, argvs.lastc_flag, argvs.break_flag);
		atime_error = calc_avetime_APE(vehicles, customers, amvehicles, argvs.avetime_ape, argvs.lastc_flag, argvs.break_flag);
    }
	else if(argvs.avetime_ae_flag == true){
		btime_error = calc_avetime_AE(vehicles, customers, bmvehicles, argvs.avetime_ae, argvs.lastc_flag);
		atime_error = calc_avetime_AE(vehicles, customers, amvehicles, argvs.avetime_ae, argvs.lastc_flag);
    }
    else{
		btime_error = 0.0;
		atime_error = 0.0;
    }

    if(argvs.avedist_ape_flag == true || argvs.avedist_ae_flag == true){
        vcnt = 0;
        for(auto v : vehicles){
            vcnt++;
            int route_size = (int) v.route.size();

            bool all_depo = true;
            for(int i = 1; i < route_size-1; i++){
                if((customers.at(v.route.at(i)).custno != "0")&&(customers.at(v.route.at(i)).custno.at(0) != 'b')){
                    all_depo = false;
                    break;
                }
            }
            if(all_depo){
                continue;
            }

            if(argvs.lastc_flag == true){
                bmvehicles[vcnt] = v.totalcost_wolast;
                amvehicles[vcnt] = v.delay_totalcost_wolast;
            }
            else{
                bmvehicles[vcnt] = v.totalcost;
                amvehicles[vcnt] = v.delay_totalcost;
            }
        }
    }

    if(argvs.avedist_ape_flag == true){
		bdist_error = calc_avedist_APE(vehicles, customers, bmvehicles, argvs.avedist_ape, argvs.lastc_flag, argvs.break_flag);
		adist_error = calc_avedist_APE(vehicles, customers, amvehicles, argvs.avedist_ape, argvs.lastc_flag, argvs.break_flag);
    }
	else if(argvs.avedist_ae_flag == true){
		bdist_error = calc_avedist_AE(vehicles, customers, bmvehicles, argvs.avedist_ae, argvs.lastc_flag);
		adist_error = calc_avedist_AE(vehicles, customers, amvehicles, argvs.avedist_ae, argvs.lastc_flag);
    }
    else{
		bdist_error = 0.0;
		adist_error = 0.0;
    }

    double cdiff;
	bool ires = improved_or_not(atpen, btpen, adist, bdist, 0, 0, 0, 0, atime_error, btime_error, 0, 0, adist_error, bdist_error, atime, btime, apower, bpower, aexp, bexp, 0, 0, cdiff, argvs);

	if(cdiff <= 0){
		for(auto v : vehicles){
            int route_size = (int) v.route.size();
            bool all_depo = true;
            for(int i = 1; i < route_size-1; i++){
                if((customers.at(v.route.at(i)).custno != "0")&&(customers.at(v.route.at(i)).custno.at(0) != 'b')){
                    all_depo = false;
                    break;
                }
            }
            if(all_depo){
                continue;
            }

            if(compare_time(v.arr.at(0), v.delay_arr.at(0)) != 0){
                v.arr.at(0) = v.delay_arr.at(0);
                v.update_totalcost_dtable(customers, argvs);
            }
        }
    }
	else{
		for(auto v : vehicles){
            int route_size = (int) v.route.size();
            bool all_depo = true;
            for(int i = 1; i < route_size-1; i++){
                if((customers.at(v.route.at(i)).custno != "0")&&(customers.at(v.route.at(i)).custno.at(0) != 'b')){
                    all_depo = false;
                    break;
                }
            }
            if(all_depo){
                continue;
            }
            
			v.delay_arr.at(0) = v.arr.at(0);
			v.update_totalcost_dtable(customers, argvs);
        }
    }
}
