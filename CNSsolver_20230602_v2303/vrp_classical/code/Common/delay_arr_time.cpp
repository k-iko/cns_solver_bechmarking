#include "delay_arr_time.hpp"

namespace{
    bool vector_finder(const vector<int> &vec, const int number);
}

/**
 * @brief 早着する配送先があれば出発時刻を遅延する関数
 * @param[in] customers 配送先リスト
 * @param[in] vehicles 車両リスト
 * @param[in] argvs グローバル変数格納用
 * @details 早着する配送先があれば出発時刻を遅延する関数。
 */
void delay_arr_time(const vector<Customer> &customers, vector<Vehicle> &vehicles, const Cppargs &argvs){
    if(argvs.lastc_flag==true){
		for(auto v : vehicles){
			vector<int> depo_idx;
            map<int, double> servt_dict;

            int route_size = (int) v.route.size();

            if(route_size < 2){
                continue;
            }

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

            for(int i = 0; i < route_size-1; i++){
                if(customers.at(v.route.at(i)).custno == "0"){
                    depo_idx.emplace_back(i); //routeにおけるデポのインデックス
                }
            }
            depo_idx.emplace_back(route_size-1); //最終訪問先を追加

            //荷作業時間を計算
            int depo_idx_size = (int) depo_idx.size();
            //配送時間
            for(int i = 0; i < depo_idx_size-1; i++){
                servt_dict.emplace(depo_idx.at(i), 0);
                double servt = 0;

                for(int j = depo_idx.at(i)+1; j < depo_idx.at(i+1); j++){
                    if(customers.at(v.route.at(j)).requestType == "DELIVERY" && customers.at(v.route.at(j)).custno != customers.at(v.route.at(j-1)).custno
                       && customers.at(v.route.at(j)).custno.at(0) == 'b'){
                        servt += customers.at(v.route.at(j)).depotservt;
                    }
                }

                if(customers.at(v.route.at(depo_idx.at(i)+1)).custno != "0"){
                    servt_dict.at(depo_idx.at(i)) += servt+v.depotservt; //デポが連続していない場合
                }
                else{
                    servt_dict.at(depo_idx.at(i)) += servt; //デポが連続している場合
                }
            }

            //集荷時間
            servt_dict.emplace(route_size-1, 0);
            for(int i = 1; i < depo_idx_size; i++){
                double servt = 0;

                for(int j = depo_idx.at(i-1)+1; j < depo_idx.at(i); j++){
                    if(customers.at(v.route.at(j)).requestType == "PICKUP" && customers.at(v.route.at(j)).custno != customers.at(v.route.at(j-1)).custno){
                        servt += customers.at(v.route.at(j)).depotservt;
                    }
                }

                servt_dict.at(depo_idx.at(i)) += servt;
            }

            vector<double> load(route_size, 0), load2(route_size, 0);

            for(int i=0; i < route_size-1; i++){
                if(vector_finder(depo_idx, i) == true){
                    for(int j = i+1; j < depo_idx.at(distance(depo_idx.begin(), find(depo_idx.begin(), depo_idx.end(), i))+1); j++){
                        if(customers.at(v.route.at(j)).requestType == "DELIVERY"){
                            load.at(i) += customers.at(v.route.at(j)).dem;
                        
                            load2.at(i) += customers.at(v.route.at(j)).dem2;
                        }
                    }
                }
                else{
                    if(customers.at(v.route.at(i)).requestType == "DELIVERY"){
                        load.at(i) = load.at(i-1)-customers.at(v.route.at(i)).dem;
                        load2.at(i) = load2.at(i-1)-customers.at(v.route.at(i)).dem2;
                    }
                    else{
                        load.at(i) = load.at(i-1)+customers.at(v.route.at(i)).dem;
                        load2.at(i) = load2.at(i-1)+customers.at(v.route.at(i)).dem2;
                    }
                }
            }

            for(int i = route_size-2; i >= 0; i--){
                if(vector_finder(depo_idx, i) == true && vector_finder(depo_idx, i+1) == true){
                    load.at(i) = load.at(i+1);
                    load2.at(i) = load2.at(i+1);
                }
            }

            //誤差対策
            for(int i = 0; i < route_size; i++){
                if(load.at(i) < 1.0e-9){
                    load.at(i) = 0.0;
                }
            }
            for(int i = 0; i < route_size; i++){
                if(load2.at(i) < 1.0e-9){
                    load2.at(i) = 0.0;
                }
            }
				
			vector<int> no_park_idx1;
            bool no_park_flag1 = true;

            for(int i = depo_idx.at(0)+1; i < depo_idx.at(1); i++){
                if(customers.at(v.route.at(i)).requestType == "DELIVERY"){
                    no_park_flag1 = false;
                    break;
                }
            }
            if(no_park_flag1 == true && load.at(0) == 0 && load2.at(0) == 0){
                no_park_idx1.emplace_back(0);
            }

            for(int i = 1; i < depo_idx_size-1; i++){
                if(load.at(depo_idx.at(i)-1) == 0 && load2.at(depo_idx.at(i)-1) == 0){
                    no_park_flag1 = true;

                    for(int j = depo_idx.at(i)+1; j < depo_idx.at(i+1); j++){
                        if(customers.at(v.route.at(j)).requestType == "DELIVERY"){
                            no_park_flag1 = false;
                            break;
                        }
                    }
                    if(no_park_flag1 == true && load.at(depo_idx.at(i)) == 0 && load2.at(depo_idx.at(i)) == 0){
                        no_park_idx1.emplace_back(depo_idx.at(i));
                    }
                }
            }

            if(load.at(depo_idx.at(depo_idx_size-1)-1) == 0 && load2.at(depo_idx.at(depo_idx_size-1)-1) == 0){
                no_park_idx1.emplace_back(depo_idx.at(depo_idx_size-1));
            }


			v.delay_arr = vector<double>(route_size, 0); 

            double lv_t;
            int start_idx;
            int last_idx;

            for(int i = route_size-1; i >= 0; i--){
				if(customers.at(v.route.at(i)).custno == "0" || i == route_size-1){
					last_idx = i;
				}
                else{
					break;
                }
            }

			
            if(route_size == 2 || customers.at(v.route.at(last_idx-1)).spotid != customers.at(v.route.at(last_idx-2)).spotid){
                v.delay_arr.at(last_idx-1) = max(customers.at(v.route.at(last_idx-1)).ready_fs-customers.at(v.route.at(last_idx-1)).parkt_arrive, v.arr.at(last_idx-1)); //一番最後の配送先の到着時刻
            }
            else{
                v.delay_arr.at(last_idx-1) = max(customers.at(v.route.at(last_idx-1)).ready_fs, v.arr.at(last_idx-1)); //一番最後の配送先の到着時刻
            }
            if(servt_dict.count(last_idx-1) == 1){
                lv_t = v.delay_arr.at(last_idx-1)+servt_dict.at(last_idx-1)*v.opskill;
            }
            else{
                lv_t = v.delay_arr.at(last_idx-1)+customers.at(v.route.at(last_idx-1)).servt*v.opskill;
            }
            if(route_size == 2 || customers.at(v.route.at(last_idx-1)).spotid != customers.at(v.route.at(last_idx-2)).spotid){
                lv_t += customers.at(v.route.at(last_idx-1)).parkt_open;
            }
            if(v.chg_batt_dict.count(v.route.at(last_idx-1)) == 1){
                lv_t += v.chg_batt_dict.at(v.route.at(last_idx-1))/customers.at(v.route.at(last_idx-1)).e_rate;
            }

            //通行不可なら遅延しない
            if(get_ttable(v.vtype, lv_t, argvs.management_ttable, customers.at(v.route.at(last_idx-1)).spotno, customers.at(v.route.at(last_idx)).spotno) == -1
            || get_dtable(v.vtype, lv_t, argvs.management_dtable, customers.at(v.route.at(last_idx-1)).spotno, customers.at(v.route.at(last_idx)).spotno) == -1
            || (!argvs.management_etable.empty() && get_etable(v.vtype, v.etype, lv_t, argvs.management_etable, customers.at(v.route.at(last_idx-1)).spotno, customers.at(v.route.at(last_idx)).spotno) == -1)
            || (!argvs.management_xtable.empty() && get_xtable(v.vtype, lv_t, argvs.management_xtable, customers.at(v.route.at(last_idx-1)).spotno, customers.at(v.route.at(last_idx)).spotno) == -1)){
                v.delay_arr = v.arr;
                v.update_totalcost_dtable(customers, argvs);
                continue;
            }

            v.delay_arr.at(last_idx) = lv_t+get_ttable(v.vtype, lv_t, argvs.management_ttable, customers.at(v.route.at(last_idx-1)).spotno, customers.at(v.route.at(last_idx)).spotno)*v.drskill;

            if(last_idx != route_size-1){
                lv_t = v.delay_arr.at(last_idx)+servt_dict.at(last_idx)*v.opskill;
                if(route_size == 2 || customers.at(v.route.at(last_idx)).spotid != customers.at(v.route.at(last_idx-1)).spotid){
                    lv_t += customers.at(v.route.at(last_idx)).parkt_open+customers.at(v.route.at(last_idx)).parkt_arrive;
                }

                //通行不可なら遅延しない
                if(get_ttable(v.vtype, lv_t, argvs.management_ttable, customers.at(v.route.at(last_idx)).spotno, customers.at(v.route.at(route_size-1)).spotno) == -1
                || get_dtable(v.vtype, lv_t, argvs.management_dtable, customers.at(v.route.at(last_idx)).spotno, customers.at(v.route.at(route_size-1)).spotno) == -1
                || (!argvs.management_etable.empty() && get_etable(v.vtype, v.etype, lv_t, argvs.management_etable, customers.at(v.route.at(last_idx)).spotno, customers.at(v.route.at(route_size-1)).spotno) == -1)
                || (!argvs.management_xtable.empty() && get_xtable(v.vtype, lv_t, argvs.management_xtable, customers.at(v.route.at(last_idx)).spotno, customers.at(v.route.at(route_size-1)).spotno) == -1)){
                    v.delay_arr = v.arr;
                    v.update_totalcost_dtable(customers, argvs);
                    continue;
                }

                v.delay_arr.at(route_size-1) = lv_t+get_ttable(v.vtype, lv_t, argvs.management_ttable, customers.at(v.route.at(last_idx)).spotno, customers.at(v.route.at(route_size-1)).spotno)*v.drskill;
            }

            start_idx = last_idx-2;
            for(int i = 0; i < route_size-1; i++){
                if((argvs.timePenaltyType == "hard" || argvs.timePenaltyWeight > 0 || customers.at(v.route.at(i)).custno.substr(0, 1) != "e")
                && compare_time(v.arr.at(i), customers.at(v.route.at(i)).due_fs) > 0){
                    start_idx = i-1;
                    for(int j = i; j < route_size; j++){
                        v.delay_arr.at(i) = v.arr.at(i);
                    }

                    break;
                }
            }
            

			for(int i = start_idx; i >= 0; i--){
                if(customers.at(v.route.at(i)).custno == customers.at(v.route.at(i+1)).custno && customers.at(v.route.at(i)).custno != "0"){
					v.delay_arr.at(i) = v.delay_arr.at(i+1);
					continue;
                }

                //通行不可なら遅延しない
				if(get_ttable(v.vtype, v.delay_arr.at(i+1), argvs.management_ttable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1
				|| get_dtable(v.vtype, v.delay_arr.at(i+1), argvs.management_dtable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1
				|| (!argvs.management_etable.empty() && get_etable(v.vtype, v.etype, v.delay_arr.at(i+1), argvs.management_etable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1)
				|| (!argvs.management_xtable.empty() && get_xtable(v.vtype, v.delay_arr.at(i+1), argvs.management_xtable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1)){
					v.delay_arr = v.arr;
                    v.update_totalcost_dtable(customers, argvs);
					break;
                }

				double tr_time_temp = get_ttable(v.vtype, v.delay_arr.at(i+1), argvs.management_ttable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno)*v.drskill; //仮の移動時間
				lv_t = v.delay_arr.at(i+1)-tr_time_temp; //仮の出発時刻

                //通行不可なら遅延しない
				if(get_ttable(v.vtype, lv_t, argvs.management_ttable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1
				|| get_dtable(v.vtype, lv_t, argvs.management_dtable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1
				|| (!argvs.management_etable.empty() && get_etable(v.vtype, v.etype, lv_t, argvs.management_etable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1)
				|| (!argvs.management_xtable.empty() && get_xtable(v.vtype, lv_t, argvs.management_xtable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1)){
					v.delay_arr = v.arr;
                    v.update_totalcost_dtable(customers, argvs);
					break;
                }

				double tr_time = get_ttable(v.vtype, lv_t, argvs.management_ttable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno)*v.drskill; //移動時間を再計算

                vector<vector<double>> ttable_temp;
                //table検索(配送時刻がある場合)
                for(auto table = argvs.management_ttable.rbegin(); table != argvs.management_ttable.rend(); table++){
                    if(get<0>(table->first) != "common" && get<0>(table->first) == v.vtype && compare_time(get<1>(table->first), lv_t) <= 0){
                        ttable_temp = table->second;
                    }
                }

                for(auto table = argvs.management_ttable.rbegin(); table != argvs.management_ttable.rend(); table++){
                    if(get<0>(table->first) == "time_common" && compare_time(get<1>(table->first), lv_t) <= 0){
                        ttable_temp = table->second;
                    }
                }

                //table検索(配送時刻がない場合)
                for(auto table = argvs.management_ttable.rbegin(); table != argvs.management_ttable.rend(); table++){
                    if(get<0>(table->first) == "common"){
                        ttable_temp = table->second;
                    }
                }

				if(compare_time(tr_time, tr_time_temp) != 0){
					bool impassable = true; 
                    
					while(true){
                        bool common_table_flag = false;
						for(auto table = argvs.management_ttable.begin(); table != argvs.management_ttable.end(); table++){
                            if(get<0>(table->first) == "common"){
								if(table->second == ttable_temp){
									v.delay_arr = v.arr;
									common_table_flag = true;
									break;
                                }
                            }
                        }

						if(common_table_flag == true){
							break;
                        }

                        int index = 0;
                        double start_time;
                        bool table_found = false;
                        
						for(auto table = argvs.management_ttable.begin(); table != argvs.management_ttable.end(); table++){
                            auto ttable_before = argvs.management_ttable.begin();
                            int index = distance(argvs.management_ttable.begin(), argvs.management_ttable.find(table->first));
                            for(int j = 0; j < index-1; j++){
                                ttable_before++;
                            }
							if(table->second == ttable_temp && index > 0 && get<0>(table->first) == get<0>(ttable_before->first)){
								lv_t = get<1>(ttable_before->first); //次に遅い時刻を採用
                                ttable_temp = table->second;
                                table_found = true;
								break;
                            }
							else if(table->second == ttable_temp){
								start_time = get<1>(ttable_before->first);
                                index++;
                            }
						}
						if(!table_found){
                            bool table_found2 = false;
                            bool same_table_flag = false;
                            for(auto table = argvs.management_ttable.rbegin(); table != argvs.management_ttable.rend(); table++){
                                if(get<0>(table->first) == "time_common" && compare_time(start_time, get<1>(table->first)) > 0){
                                    if(table->second == ttable_temp){
                                        same_table_flag = true;
                                        continue;
                                    }
                                    lv_t = get<1>(table->first); //次に遅い時刻を採用
                                    ttable_temp = table->second;
                                    table_found2 = true;
                                    break;
                                }
                            }
							if(!table_found2){
                                bool table_found3 = false;
                                for(auto table = argvs.management_ttable.begin(); table != argvs.management_ttable.end(); table++){
                                    if(get<0>(table->first) == "common"){
                                        if(same_table_flag == true || table->second == ttable_temp){
											continue;
                                        }
                                        lv_t = get<1>(table->first); //次に遅い時刻を採用
                                        ttable_temp = table->second;
                                        table_found3 = true;
                                        break;
                                    }
                                }
								if(!table_found3){ //繰り返す対象のテーブルがなくなっても出発時刻が確定しなければ遅延なし
									v.delay_arr=v.arr;
									break;
                                }
                            }
                        }
                        
                        //通行不可なら遅延しない
                        if(get_ttable(v.vtype, lv_t, argvs.management_ttable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1
                        || get_dtable(v.vtype, lv_t, argvs.management_dtable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1
                        || (!argvs.management_etable.empty() && get_etable(v.vtype, v.etype, lv_t, argvs.management_etable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1)
                        || (!argvs.management_xtable.empty() && get_xtable(v.vtype, lv_t, argvs.management_xtable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1)){
                            v.delay_arr = v.arr;
                            v.update_totalcost_dtable(customers, argvs);
                            impassable = false;
                            break;
                        }

						tr_time=get_ttable(v.vtype, lv_t, argvs.management_ttable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno)*v.drskill; //移動時間を再計算

						if(compare_time(lv_t, v.delay_arr.at(i+1)-tr_time) <= 0 && compare_time(v.lv.at(i),v.delay_arr.at(i+1)-tr_time) <= 0){ //出発時刻確定
							lv_t = v.delay_arr.at(i+1)-tr_time;
							if(servt_dict.count(i) == 1){ 
								v.delay_arr.at(i) = lv_t-servt_dict.at(i)*v.opskill;
                            }
							else{
								v.delay_arr.at(i) = lv_t-customers.at(v.route.at(i)).servt*v.opskill;
                            }
							if((i == 0 || customers.at(v.route.at(i)).spotid != customers.at(v.route.at(i-1)).spotid) && customers.at(v.route.at(i)).custno.substr(0, 1) != "e" && vector_finder(no_park_idx1, i) == false){
								v.delay_arr.at(i) -= customers.at(v.route.at(i)).parkt_open+customers.at(v.route.at(i)).parkt_arrive;
                            }
							if(v.chg_batt_dict.count(v.route.at(i)) == 1){
								v.delay_arr.at(i) -= v.chg_batt_dict.at(v.route.at(i))/customers.at(v.route.at(i)).e_rate;
                            }
							break;
                        }
                    }

					if(!impassable){
                        break;
                    }
                }

				if(v.chg_batt_dict.count(v.route.at(i)) == 1){
					double lv = customers.at(v.route.at(i)).due_fs+customers.at(v.route.at(i)).servt*v.opskill+v.chg_batt_dict.at(v.route.at(i))/customers.at(v.route.at(i)).e_rate;

					if((argvs.timePenaltyType == "hard" || argvs.timePenaltyWeight > 0) && compare_time(lv_t, lv) > 0){
						if(servt_dict.count(i) == 1){ 
							lv_t = customers.at(v.route.at(i)).due_fs+servt_dict.at(i)*v.opskill+v.chg_batt_dict.at(v.route.at(i))/customers.at(v.route.at(i)).e_rate;
							
							v.delay_arr.at(i) = lv_t-servt_dict.at(i)*v.opskill-v.chg_batt_dict.at(v.route.at(i))/customers.at(v.route.at(i)).e_rate;
                        }
						else{
							lv_t = customers.at(v.route.at(i)).due_fs+customers.at(v.route.at(i)).servt*v.opskill+v.chg_batt_dict.at(v.route.at(i))/customers.at(v.route.at(i)).e_rate;
							
							v.delay_arr.at(i) = lv_t-customers.at(v.route.at(i)).servt*v.opskill-v.chg_batt_dict.at(v.route.at(i))/customers.at(v.route.at(i)).e_rate;
                        }

						bool impassable = true;
						for(int j = i+1; j < route_size; j++){ //その後の到着時刻を再計算
                            if(customers.at(v.route.at(j)).custno == customers.at(v.route.at(j-1)).custno && customers.at(v.route.at(j)).custno != "0"){
                                v.delay_arr.at(j) = v.delay_arr.at(j-1);
                                continue;
                            }
                        
                            //通行不可なら遅延しない
                            if(get_ttable(v.vtype, lv_t, argvs.management_ttable, customers.at(v.route.at(j-1)).spotno, customers.at(v.route.at(j)).spotno) == -1
                            || get_dtable(v.vtype, lv_t, argvs.management_dtable, customers.at(v.route.at(j-1)).spotno, customers.at(v.route.at(j)).spotno) == -1
                            || (!argvs.management_etable.empty() && get_etable(v.vtype, v.etype, lv_t, argvs.management_etable, customers.at(v.route.at(j-1)).spotno, customers.at(v.route.at(j)).spotno) == -1)
                            || (!argvs.management_xtable.empty() && get_xtable(v.vtype, lv_t, argvs.management_xtable, customers.at(v.route.at(j-1)).spotno, customers.at(v.route.at(j)).spotno) == -1)){
                                v.delay_arr = v.arr;
                                v.update_totalcost_dtable(customers, argvs);
                                impassable = false;
                                break;
                            }

							tr_time = get_ttable(v.vtype, lv_t, argvs.management_ttable, customers.at(v.route.at(j-1)).spotno, customers.at(v.route.at(j)).spotno)*v.drskill;
							v.delay_arr.at(j) = lv_t+tr_time;
							if(servt_dict.count(j) == 1){ 
                                if((j == 0 || customers.at(v.route.at(j)).spotid != customers.at(v.route.at(j-1)).spotid) && customers.at(v.route.at(j)).custno.substr(0, 1) != "e" && vector_finder(no_park_idx1, j) == false){
								    lv_t = max(v.delay_arr.at(j)+customers.at(v.route.at(j)).parkt_arrive, customers.at(v.route.at(j)).ready_fs)+servt_dict.at(j)*v.opskill;
                                }
                                else{
                                    lv_t = max(v.delay_arr.at(j), customers.at(v.route.at(j)).ready_fs)+servt_dict.at(j)*v.opskill;
                                }
                            }
							else{
                                if((j == 0 || customers.at(v.route.at(j)).spotid != customers.at(v.route.at(j-1)).spotid) && customers.at(v.route.at(j)).custno.substr(0, 1) != "e" && vector_finder(no_park_idx1, j) == false){
								    lv_t = max(v.delay_arr.at(j)+customers.at(v.route.at(j)).parkt_arrive, customers.at(v.route.at(j)).ready_fs)+customers.at(v.route.at(j)).servt*v.opskill;
                                }
                                else{
                                    lv_t = max(v.delay_arr.at(j), customers.at(v.route.at(j)).ready_fs)+customers.at(v.route.at(j)).servt*v.opskill;
                                }
                            }
							if((j == 0 || customers.at(v.route.at(j)).spotid != customers.at(v.route.at(j-1)).spotid) && customers.at(v.route.at(j)).custno.substr(0, 1) != "e" && vector_finder(no_park_idx1, j) == false){
								lv_t += customers.at(v.route.at(j)).parkt_open;
                            }
							if(v.chg_batt_dict.count(v.route.at(j)) == 1){
								lv_t += v.chg_batt_dict.at(v.route.at(j))/customers.at(v.route.at(j)).e_rate;
                            }
                        }

						if(!impassable){
							break;
						}
                    }
					else{
						if(servt_dict.count(i) == 1){ 
							v.delay_arr.at(i) = lv_t-servt_dict.at(i)*v.opskill-v.chg_batt_dict.at(v.route.at(i))/customers.at(v.route.at(i)).e_rate;
						}
						else{
							v.delay_arr.at(i) = lv_t-customers.at(v.route.at(i)).servt*v.opskill-v.chg_batt_dict.at(v.route.at(i))/customers.at(v.route.at(i)).e_rate;
						}
					}
                }
				else{
					double lv = customers.at(v.route.at(i)).due_fs+customers.at(v.route.at(i)).servt*v.opskill;
					if((i == 0 || customers.at(v.route.at(i)).spotid != customers.at(v.route.at(i-1)).spotid) && vector_finder(no_park_idx1, i) == false){
						lv += customers.at(v.route.at(i)).parkt_open;
					}

					if((argvs.timePenaltyType == "hard" || argvs.timePenaltyWeight > 0) && compare_time(lv_t, lv) > 0){
						if(servt_dict.count(i) == 1){ 
							lv_t = customers.at(v.route.at(i)).due_fs+servt_dict.at(i)*v.opskill;
							if((i == 0 || customers.at(v.route.at(i)).spotid != customers.at(v.route.at(i-1)).spotid) && vector_finder(no_park_idx1, i) == false){
								lv_t += customers.at(v.route.at(i)).parkt_open;
							}
							
							v.delay_arr.at(i) = lv_t-servt_dict.at(i)*v.opskill;
							if((i == 0 || customers.at(v.route.at(i)).spotid != customers.at(v.route.at(i-1)).spotid) && vector_finder(no_park_idx1, i) == false){
								v.delay_arr.at(i) -= customers.at(v.route.at(i)).parkt_open+customers.at(v.route.at(i)).parkt_arrive;
							}
                        }
						else{
							lv_t = customers.at(v.route.at(i)).due_fs+customers.at(v.route.at(i)).servt*v.opskill;
							if((i == 0 || customers.at(v.route.at(i)).spotid != customers.at(v.route.at(i-1)).spotid) && vector_finder(no_park_idx1, i) == false){
								lv_t += customers.at(v.route.at(i)).parkt_open;
							}
							
							v.delay_arr.at(i) = lv_t-customers.at(v.route.at(i)).servt*v.opskill;
							if((i == 0 || customers.at(v.route.at(i)).spotid != customers.at(v.route.at(i-1)).spotid) && vector_finder(no_park_idx1, i) == false){
								v.delay_arr.at(i) -= customers.at(v.route.at(i)).parkt_open+customers.at(v.route.at(i)).parkt_arrive;
							}
                        }

						bool impassable = true;
						for(int j = i+1; j < route_size; j++){ //その後の到着時刻を再計算
                            if(customers.at(v.route.at(j)).custno == customers.at(v.route.at(j-1)).custno && customers.at(v.route.at(j)).custno != "0"){
                                v.delay_arr.at(j) = v.delay_arr.at(j-1);
                                continue;
                            }
                            //通行不可なら遅延しない
                            if(get_ttable(v.vtype, lv_t, argvs.management_ttable, customers.at(v.route.at(j-1)).spotno, customers.at(v.route.at(j)).spotno) == -1
                            || get_dtable(v.vtype, lv_t, argvs.management_dtable, customers.at(v.route.at(j-1)).spotno, customers.at(v.route.at(j)).spotno) == -1
                            || (!argvs.management_etable.empty() && get_etable(v.vtype, v.etype, lv_t, argvs.management_etable, customers.at(v.route.at(j-1)).spotno, customers.at(v.route.at(j)).spotno) == -1)
                            || (!argvs.management_xtable.empty() && get_xtable(v.vtype, lv_t, argvs.management_xtable, customers.at(v.route.at(j-1)).spotno, customers.at(v.route.at(j)).spotno) == -1)){
                                v.delay_arr = v.arr;
                                v.update_totalcost_dtable(customers, argvs);
                                impassable = false;
                                break;
                            }

							tr_time = get_ttable(v.vtype, lv_t, argvs.management_ttable, customers.at(v.route.at(j-1)).spotno, customers.at(v.route.at(j)).spotno)*v.drskill;
							v.delay_arr.at(j) = lv_t+tr_time;
							if(servt_dict.count(j) == 1){ 
                                if((j == 0 || customers.at(v.route.at(j)).spotid != customers.at(v.route.at(j-1)).spotid) && customers.at(v.route.at(j)).custno.substr(0, 1) != "e" && vector_finder(no_park_idx1, j) == false){
								    lv_t = max(v.delay_arr.at(j)+customers.at(v.route.at(j)).parkt_arrive, customers.at(v.route.at(j)).ready_fs)+servt_dict.at(j)*v.opskill;
                                }
                                else{
                                    lv_t = max(v.delay_arr.at(j), customers.at(v.route.at(j)).ready_fs)+servt_dict.at(j)*v.opskill;
                                }
                            }
							else{
                                if((j == 0 || customers.at(v.route.at(j)).spotid != customers.at(v.route.at(j-1)).spotid) && customers.at(v.route.at(j)).custno.substr(0, 1) != "e" && vector_finder(no_park_idx1, j) == false){
								    lv_t = max(v.delay_arr.at(j)+customers.at(v.route.at(j)).parkt_arrive, customers.at(v.route.at(j)).ready_fs)+customers.at(v.route.at(j)).servt*v.opskill;
                                }
                                else{
                                    lv_t = max(v.delay_arr.at(j), customers.at(v.route.at(j)).ready_fs)+customers.at(v.route.at(j)).servt*v.opskill;
                                }
                            }
							if((j == 0 || customers.at(v.route.at(j)).spotid != customers.at(v.route.at(j-1)).spotid) && customers.at(v.route.at(j)).custno.substr(0, 1) != "e" && vector_finder(no_park_idx1, j) == false){
								lv_t += customers.at(v.route.at(j)).parkt_open;
                            }
							if(v.chg_batt_dict.count(v.route.at(j)) == 1){
								lv_t += v.chg_batt_dict.at(v.route.at(j))/customers.at(v.route.at(j)).e_rate;
                            }
                        }

						if(!impassable){
							break;
						}
                    }
					else{
						if(servt_dict.count(i) == 1){ 
							v.delay_arr.at(i) = lv_t-servt_dict.at(i)*v.opskill;
						}
						else{
							v.delay_arr.at(i) = lv_t-customers.at(v.route.at(i)).servt*v.opskill;
						}
						if((i == 0 || customers.at(v.route.at(i)).spotid != customers.at(v.route.at(i-1)).spotid) && vector_finder(no_park_idx1, i) == false){
							v.delay_arr.at(i) -= customers.at(v.route.at(i)).parkt_open+customers.at(v.route.at(i)).parkt_arrive;
						}
					}
				}
			}
				
		
			v.update_totalcost_dtable(customers, argvs);

			for(int i = 0; i < route_size; i++){
				if(v.rem_batt.at(i)-v.chg_batt.at(i) >= v.e_margin && v.delay_rem_batt.at(i)-v.delay_chg_batt.at(i) < v.e_margin){ //遅延することで電欠となるなら遅延しない
					v.delay_arr=v.arr;
					break;
				}
			}

			if((argvs.timePenaltyType == "hard" || argvs.timePenaltyWeight > 0) && v.arr.at(0) != v.delay_arr.at(0) && argvs.lowertime > 0){
                if(compare_time(v.delay_lv.at(last_idx-1)-argvs.lowertime, v.delay_arr.at(0)) < 0 && compare_time(v.arr.at(0), v.delay_lv.at(last_idx-1)-argvs.lowertime) <= 0){
				    v.delay_arr.at(0) = v.delay_lv.at(last_idx-1)-argvs.lowertime; //ltimeを下回らないように遅延する
                }
			}

			v.update_totalcost_dtable(customers, argvs);

            for(int i = 0; i < route_size-1; i++){
                //通行不可なら遅延しない
				if(get_ttable(v.vtype, v.delay_lv.at(i), argvs.management_ttable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1
				|| get_dtable(v.vtype, v.delay_lv.at(i), argvs.management_dtable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1
				|| (!argvs.management_etable.empty() && get_etable(v.vtype, v.etype, v.delay_lv.at(i), argvs.management_etable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1)
				|| (!argvs.management_xtable.empty() && get_xtable(v.vtype, v.delay_lv.at(i), argvs.management_xtable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1)){
					v.delay_arr = v.arr;
                    v.update_totalcost_dtable(customers, argvs);
					break;
                }
            }
        }
    }
	else{
		for(auto v : vehicles){
			vector<int> depo_idx;
            map<int, double> servt_dict;

            int route_size = (int) v.route.size();

            if(route_size < 2){
                continue;
            }

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

            for(int i = 0; i < route_size-1; i++){
                if(customers.at(v.route.at(i)).custno == "0"){
                    depo_idx.emplace_back(i); //routeにおけるデポのインデックス
                }
            }
            depo_idx.emplace_back(route_size-1); //最終訪問先を追加

            //荷作業時間を計算
            int depo_idx_size = (int) depo_idx.size();
            //配送時間
            for(int i = 0; i < depo_idx_size-1; i++){
                servt_dict.emplace(depo_idx.at(i), 0);
                double servt = 0;

                for(int j = depo_idx.at(i)+1; j < depo_idx.at(i+1); j++){
                    if(customers.at(v.route.at(j)).requestType == "DELIVERY" && customers.at(v.route.at(j)).custno != customers.at(v.route.at(j-1)).custno){
                        servt += customers.at(v.route.at(j)).depotservt;
                    }
                }

                if(customers.at(v.route.at(depo_idx.at(i)+1)).custno != "0"){
                    servt_dict.at(depo_idx.at(i)) += servt+v.depotservt; //デポが連続していない場合
                }
                else{
                    servt_dict.at(depo_idx.at(i)) += servt; //デポが連続している場合
                }
            }

            //集荷時間
            servt_dict.emplace(route_size-1, 0);
            for(int i = 1; i < depo_idx_size; i++){
                double servt = 0;

                for(int j = depo_idx.at(i-1)+1; j < depo_idx.at(i); j++){
                    if(customers.at(v.route.at(j)).requestType == "PICKUP" && customers.at(v.route.at(j)).custno != customers.at(v.route.at(j-1)).custno){
                        servt += customers.at(v.route.at(j)).depotservt;
                    }
                }

                servt_dict.at(depo_idx.at(i)) += servt;
            }

            vector<double> load(route_size, 0), load2(route_size, 0);

            for(int i=0; i < route_size-1; i++){
                if(vector_finder(depo_idx, i) == true){
                    for(int j = i+1; j < depo_idx.at(distance(depo_idx.begin(), find(depo_idx.begin(), depo_idx.end(), i))+1); j++){
                        if(customers.at(v.route.at(j)).requestType == "DELIVERY"){
                            load.at(i) += customers.at(v.route.at(j)).dem;
                        
                            load2.at(i) += customers.at(v.route.at(j)).dem2;
                        }
                    }
                }
                else{
                    if(customers.at(v.route.at(i)).requestType == "DELIVERY"){
                        load.at(i) = load.at(i-1)-customers.at(v.route.at(i)).dem;
                        load2.at(i) = load2.at(i-1)-customers.at(v.route.at(i)).dem2;
                    }
                    else{
                        load.at(i) = load.at(i-1)+customers.at(v.route.at(i)).dem;
                        load2.at(i) = load2.at(i-1)+customers.at(v.route.at(i)).dem2;
                    }
                }
            }

            for(int i = route_size-2; i >= 0; i--){
                if(vector_finder(depo_idx, i) == true && vector_finder(depo_idx, i+1) == true){
                    load.at(i) = load.at(i+1);
                    load2.at(i) = load2.at(i+1);
                }
            }

            //誤差対策
            for(int i = 0; i < route_size; i++){
                if(load.at(i) < 1.0e-9){
                    load.at(i) = 0.0;
                }
            }
            for(int i = 0; i < route_size; i++){
                if(load2.at(i) < 1.0e-9){
                    load2.at(i) = 0.0;
                }
            }
				
			vector<int> no_park_idx1;
            bool no_park_flag1 = true;

            for(int i = depo_idx.at(0)+1; i < depo_idx.at(1); i++){
                if(customers.at(v.route.at(i)).requestType == "DELIVERY"){
                    no_park_flag1 = false;
                    break;
                }
            }
            if(no_park_flag1 == true && load.at(0) == 0 && load2.at(0) == 0){
                no_park_idx1.emplace_back(0);
            }

            for(int i = 1; i < depo_idx_size-1; i++){
                if(load.at(depo_idx.at(i)-1) == 0 && load2.at(depo_idx.at(i)-1) == 0){
                    no_park_flag1 = true;

                    for(int j = depo_idx.at(i)+1; j < depo_idx.at(i+1); j++){
                        if(customers.at(v.route.at(j)).requestType == "DELIVERY"){
                            no_park_flag1 = false;
                            break;
                        }
                    }
                    if(no_park_flag1 == true && load.at(depo_idx.at(i)) == 0 && load2.at(depo_idx.at(i)) == 0){
                        no_park_idx1.emplace_back(depo_idx.at(i));
                    }
                }
            }

            if(load.at(depo_idx.at(depo_idx_size-1)-1) == 0 && load2.at(depo_idx.at(depo_idx_size-1)-1) == 0){
                no_park_idx1.emplace_back(depo_idx.at(depo_idx_size-1));
            }


			v.delay_arr = vector<double>(route_size, 0); 

			v.delay_arr.at(route_size-1) = max(customers.at(v.route.at(route_size-1)).ready_fs, v.arr.at(route_size-1));

            double lv_t;
            int start_idx = route_size-2;
			for(int i = 0; i < route_size; i++){
				if((argvs.timePenaltyType == "hard" || argvs.timePenaltyWeight > 0 || customers.at(v.route.at(i)).custno.substr(0, 1) != "e")
				&& compare_time(v.arr.at(i), customers.at(v.route.at(i)).due_fs) > 0){
					start_idx = i-1;
					for(int j = i; j < route_size; j++){
						v.delay_arr.at(i) = v.arr.at(i);
					}

					break;
				}
			}

			for(int i = start_idx; i >= 0; i--){
                if(customers.at(v.route.at(i)).custno == customers.at(v.route.at(i+1)).custno && customers.at(v.route.at(i)).custno != "0"){
					v.delay_arr.at(i) = v.delay_arr.at(i+1);
					continue;
                }
                //通行不可なら遅延しない
				if(get_ttable(v.vtype, v.delay_arr.at(i+1), argvs.management_ttable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1
				|| get_dtable(v.vtype, v.delay_arr.at(i+1), argvs.management_dtable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1
				|| (!argvs.management_etable.empty() && get_etable(v.vtype, v.etype, v.delay_arr.at(i+1), argvs.management_etable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1)
				|| (!argvs.management_xtable.empty() && get_xtable(v.vtype, v.delay_arr.at(i+1), argvs.management_xtable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1)){
					v.delay_arr = v.arr;
                    v.update_totalcost_dtable(customers, argvs);
					break;
                }

				double tr_time_temp = get_ttable(v.vtype, v.delay_arr.at(i+1), argvs.management_ttable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno)*v.drskill; //仮の移動時間
				lv_t = v.delay_arr.at(i+1)-tr_time_temp; //仮の出発時刻

                //通行不可なら遅延しない
				if(get_ttable(v.vtype, lv_t, argvs.management_ttable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1
				|| get_dtable(v.vtype, lv_t, argvs.management_dtable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1
				|| (!argvs.management_etable.empty() && get_etable(v.vtype, v.etype, lv_t, argvs.management_etable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1)
				|| (!argvs.management_xtable.empty() && get_xtable(v.vtype, lv_t, argvs.management_xtable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1)){
					v.delay_arr = v.arr;
                    v.update_totalcost_dtable(customers, argvs);
					break;
                }

				double tr_time = get_ttable(v.vtype, lv_t, argvs.management_ttable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno)*v.drskill; //移動時間を再計算

                vector<vector<double>> ttable_temp;
                //table検索(配送時刻がある場合)
                for(auto table = argvs.management_ttable.rbegin(); table != argvs.management_ttable.rend(); table++){
                    if(get<0>(table->first) != "common" && get<0>(table->first) == v.vtype && compare_time(get<1>(table->first), lv_t) <= 0){
                        ttable_temp = table->second;
                    }
                }

                for(auto table = argvs.management_ttable.rbegin(); table != argvs.management_ttable.rend(); table++){
                    if(get<0>(table->first) == "time_common" && compare_time(get<1>(table->first), lv_t) <= 0){
                        ttable_temp = table->second;
                    }
                }

                //table検索(配送時刻がない場合)
                for(auto table = argvs.management_ttable.rbegin(); table != argvs.management_ttable.rend(); table++){
                    if(get<0>(table->first) == "common"){
                        ttable_temp = table->second;
                    }
                }

				if(compare_time(tr_time, tr_time_temp) != 0){
					bool impassable = true; 
                    
					while(true){
                        bool common_table_flag = false;
						for(auto table = argvs.management_ttable.begin(); table != argvs.management_ttable.end(); table++){
                            if(get<0>(table->first) == "common"){
								if(table->second == ttable_temp){
									v.delay_arr = v.arr;
									common_table_flag = true;
									break;
                                }
                            }
                        }

						if(common_table_flag == true){
							break;
                        }
                        
                        int index = 0;
                        double start_time;
                        bool table_found = false;
                        
						for(auto table = argvs.management_ttable.begin(); table != argvs.management_ttable.end(); table++){
                            auto ttable_before = argvs.management_ttable.begin();
                            int index = distance(argvs.management_ttable.begin(), argvs.management_ttable.find(table->first));
                            for(int j = 0; j < index-1; j++){
                                ttable_before++;
                            }
							if(table->second == ttable_temp && index > 0 && get<0>(table->first) == get<0>(ttable_before->first)){
								lv_t = get<1>(ttable_before->first); //次に遅い時刻を採用
                                ttable_temp = table->second;
                                table_found = true;
								break;
                            }
							else if(table->second == ttable_temp){
								start_time = get<1>(ttable_before->first);
                                index++;
                            }
						}
						if(!table_found){
                            bool table_found2 = false;
                            bool same_table_flag = false;
                            for(auto table = argvs.management_ttable.rbegin(); table != argvs.management_ttable.rend(); table++){
                                if(get<0>(table->first) == "time_common" && compare_time(start_time, get<1>(table->first)) > 0){
                                    if(table->second == ttable_temp){
                                        same_table_flag = true;
                                        continue;
                                    }
                                    lv_t = get<1>(table->first); //次に遅い時刻を採用
                                    ttable_temp = table->second;
                                    table_found2 = true;
                                    break;
                                }
                            }
							if(!table_found2){
                                bool table_found3 = false;
                                for(auto table = argvs.management_ttable.begin(); table != argvs.management_ttable.end(); table++){
                                    if(get<0>(table->first) == "common"){
                                        if(same_table_flag == true || table->second == ttable_temp){
											continue;
                                        }
                                        lv_t = get<1>(table->first); //次に遅い時刻を採用
                                        ttable_temp = table->second;
                                        table_found3 = true;
                                        break;
                                    }
                                }
								if(!table_found3){ //繰り返す対象のテーブルがなくなっても出発時刻が確定しなければ遅延なし
									v.delay_arr=v.arr;
									break;
                                }
                            }
                        }
                        
                        //通行不可なら遅延しない
                        if(get_ttable(v.vtype, lv_t, argvs.management_ttable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1
                        || get_dtable(v.vtype, lv_t, argvs.management_dtable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1
                        || (!argvs.management_etable.empty() && get_etable(v.vtype, v.etype, lv_t, argvs.management_etable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1)
                        || (!argvs.management_xtable.empty() && get_xtable(v.vtype, lv_t, argvs.management_xtable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1)){
                            v.delay_arr = v.arr;
                            v.update_totalcost_dtable(customers, argvs);
                            impassable = false;
                            break;
                        }

						tr_time=get_ttable(v.vtype, lv_t, argvs.management_ttable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno)*v.drskill; //移動時間を再計算

						if(compare_time(lv_t, v.delay_arr.at(i+1)-tr_time) <= 0 && compare_time(v.lv.at(i),v.delay_arr.at(i+1)-tr_time) <= 0){ //出発時刻確定
							lv_t = v.delay_arr.at(i+1)-tr_time;
							if(servt_dict.count(i) == 1){ 
								v.delay_arr.at(i) = lv_t-servt_dict.at(i)*v.opskill;
                            }
							else{
								v.delay_arr.at(i) = lv_t-customers.at(v.route.at(i)).servt*v.opskill;
                            }
							if((i == 0 || customers.at(v.route.at(i)).spotid != customers.at(v.route.at(i-1)).spotid) && customers.at(v.route.at(i)).custno.substr(0, 1) != "e" && vector_finder(no_park_idx1, i) == false){
								v.delay_arr.at(i) -= customers.at(v.route.at(i)).parkt_open+customers.at(v.route.at(i)).parkt_arrive;
                            }
							if(v.chg_batt_dict.count(v.route.at(i)) == 1){
								v.delay_arr.at(i) -= v.chg_batt_dict.at(v.route.at(i))/customers.at(v.route.at(i)).e_rate;
                            }
							break;
                        }
                    }

					if(!impassable){
                        break;
                    }
                }

				if(v.chg_batt_dict.count(v.route.at(i)) == 1){
					double lv = customers.at(v.route.at(i)).due_fs+customers.at(v.route.at(i)).servt*v.opskill+v.chg_batt_dict.at(v.route.at(i))/customers.at(v.route.at(i)).e_rate;

					if((argvs.timePenaltyType == "hard" || argvs.timePenaltyWeight > 0) && compare_time(lv_t, lv) > 0){
						if(servt_dict.count(i) == 1){ 
							lv_t = customers.at(v.route.at(i)).due_fs+servt_dict.at(i)*v.opskill+v.chg_batt_dict.at(v.route.at(i))/customers.at(v.route.at(i)).e_rate;
							
							v.delay_arr.at(i) = lv_t-servt_dict.at(i)*v.opskill-v.chg_batt_dict.at(v.route.at(i))/customers.at(v.route.at(i)).e_rate;
                        }
						else{
							lv_t = customers.at(v.route.at(i)).due_fs+customers.at(v.route.at(i)).servt*v.opskill+v.chg_batt_dict.at(v.route.at(i))/customers.at(v.route.at(i)).e_rate;
							
							v.delay_arr.at(i) = lv_t-customers.at(v.route.at(i)).servt*v.opskill-v.chg_batt_dict.at(v.route.at(i))/customers.at(v.route.at(i)).e_rate;
                        }

						bool impassable = true;
						for(int j = i+1; j < route_size; j++){ //その後の到着時刻を再計算
                            if(customers.at(v.route.at(j)).custno == customers.at(v.route.at(j-1)).custno && customers.at(v.route.at(j)).custno != "0"){
                                v.delay_arr.at(j) = v.delay_arr.at(j-1);
                                continue;
                            }
                            //通行不可なら遅延しない
                            if(get_ttable(v.vtype, lv_t, argvs.management_ttable, customers.at(v.route.at(j-1)).spotno, customers.at(v.route.at(j)).spotno) == -1
                            || get_dtable(v.vtype, lv_t, argvs.management_dtable, customers.at(v.route.at(j-1)).spotno, customers.at(v.route.at(j)).spotno) == -1
                            || (!argvs.management_etable.empty() && get_etable(v.vtype, v.etype, lv_t, argvs.management_etable, customers.at(v.route.at(j-1)).spotno, customers.at(v.route.at(j)).spotno) == -1)
                            || (!argvs.management_xtable.empty() && get_xtable(v.vtype, lv_t, argvs.management_xtable, customers.at(v.route.at(j-1)).spotno, customers.at(v.route.at(j)).spotno) == -1)){
                                v.delay_arr = v.arr;
                                v.update_totalcost_dtable(customers, argvs);
                                impassable = false;
                                break;
                            }

							tr_time = get_ttable(v.vtype, lv_t, argvs.management_ttable, customers.at(v.route.at(j-1)).spotno, customers.at(v.route.at(j)).spotno)*v.drskill;
							v.delay_arr.at(j) = lv_t+tr_time;
							if(servt_dict.count(j) == 1){ 
                                if((j == 0 || customers.at(v.route.at(j)).spotid != customers.at(v.route.at(j-1)).spotid) && customers.at(v.route.at(j)).custno.substr(0, 1) != "e" && vector_finder(no_park_idx1, j) == false){
								    lv_t = max(v.delay_arr.at(j)+customers.at(v.route.at(j)).parkt_arrive, customers.at(v.route.at(j)).ready_fs)+servt_dict.at(j)*v.opskill;
                                }
                                else{
                                    lv_t = max(v.delay_arr.at(j), customers.at(v.route.at(j)).ready_fs)+servt_dict.at(j)*v.opskill;
                                }
                            }
							else{
                                if((j == 0 || customers.at(v.route.at(j)).spotid != customers.at(v.route.at(j-1)).spotid) && customers.at(v.route.at(j)).custno.substr(0, 1) != "e" && vector_finder(no_park_idx1, j) == false){
								    lv_t = max(v.delay_arr.at(j)+customers.at(v.route.at(j)).parkt_arrive, customers.at(v.route.at(j)).ready_fs)+customers.at(v.route.at(j)).servt*v.opskill;
                                }
                                else{
                                    lv_t = max(v.delay_arr.at(j), customers.at(v.route.at(j)).ready_fs)+customers.at(v.route.at(j)).servt*v.opskill;
                                }
                            }
							if((j == 0 || customers.at(v.route.at(j)).spotid != customers.at(v.route.at(j-1)).spotid) && customers.at(v.route.at(j)).custno.substr(0, 1) != "e" && vector_finder(no_park_idx1, j) == false){
								lv_t += customers.at(v.route.at(j)).parkt_open;
                            }
							if(v.chg_batt_dict.count(v.route.at(j)) == 1){
								lv_t += v.chg_batt_dict.at(v.route.at(j))/customers.at(v.route.at(j)).e_rate;
                            }
                        }

						if(!impassable){
							break;
						}
                    }
					else{
						if(servt_dict.count(i) == 1){ 
							v.delay_arr.at(i) = lv_t-servt_dict.at(i)*v.opskill-v.chg_batt_dict.at(v.route.at(i))/customers.at(v.route.at(i)).e_rate;
						}
						else{
							v.delay_arr.at(i) = lv_t-customers.at(v.route.at(i)).servt*v.opskill-v.chg_batt_dict.at(v.route.at(i))/customers.at(v.route.at(i)).e_rate;
						}
					}
                }
				else{
					double lv = customers.at(v.route.at(i)).due_fs+customers.at(v.route.at(i)).servt*v.opskill;
					if((i == 0 || customers.at(v.route.at(i)).spotid != customers.at(v.route.at(i-1)).spotid) && vector_finder(no_park_idx1, i) == false){
						lv += customers.at(v.route.at(i)).parkt_open;
					}

					if((argvs.timePenaltyType == "hard" || argvs.timePenaltyWeight > 0) && compare_time(lv_t, lv) > 0){
						if(servt_dict.count(i) == 1){ 
							lv_t = customers.at(v.route.at(i)).due_fs+servt_dict.at(i)*v.opskill;
							if((i == 0 || customers.at(v.route.at(i)).spotid != customers.at(v.route.at(i-1)).spotid) && vector_finder(no_park_idx1, i) == false){
								lv_t += customers.at(v.route.at(i)).parkt_open;
							}
							
							v.delay_arr.at(i) = lv_t-servt_dict.at(i)*v.opskill;
							if((i == 0 || customers.at(v.route.at(i)).spotid != customers.at(v.route.at(i-1)).spotid) && vector_finder(no_park_idx1, i) == false){
								v.delay_arr.at(i) -= customers.at(v.route.at(i)).parkt_open+customers.at(v.route.at(i)).parkt_arrive;
							}
                        }
						else{
							lv_t = customers.at(v.route.at(i)).due_fs+customers.at(v.route.at(i)).servt*v.opskill;
							if((i == 0 || customers.at(v.route.at(i)).spotid != customers.at(v.route.at(i-1)).spotid) && vector_finder(no_park_idx1, i) == false){
								lv_t += customers.at(v.route.at(i)).parkt_open;
							}
							
							v.delay_arr.at(i) = lv_t-customers.at(v.route.at(i)).servt*v.opskill;
							if((i == 0 || customers.at(v.route.at(i)).spotid != customers.at(v.route.at(i-1)).spotid) && vector_finder(no_park_idx1, i) == false){
								v.delay_arr.at(i) -= customers.at(v.route.at(i)).parkt_open+customers.at(v.route.at(i)).parkt_arrive;
							}
                        }

						bool impassable = true;
						for(int j = i+1; j < route_size; j++){ //その後の到着時刻を再計算
                            if(customers.at(v.route.at(j)).custno == customers.at(v.route.at(j-1)).custno && customers.at(v.route.at(j)).custno != "0"){
                                v.delay_arr.at(j) = v.delay_arr.at(j-1);
                                continue;
                            }
                            //通行不可なら遅延しない
                            if(get_ttable(v.vtype, lv_t, argvs.management_ttable, customers.at(v.route.at(j-1)).spotno, customers.at(v.route.at(j)).spotno) == -1
                            || get_dtable(v.vtype, lv_t, argvs.management_dtable, customers.at(v.route.at(j-1)).spotno, customers.at(v.route.at(j)).spotno) == -1
                            || (!argvs.management_etable.empty() && get_etable(v.vtype, v.etype, lv_t, argvs.management_etable, customers.at(v.route.at(j-1)).spotno, customers.at(v.route.at(j)).spotno) == -1)
                            || (!argvs.management_xtable.empty() && get_xtable(v.vtype, lv_t, argvs.management_xtable, customers.at(v.route.at(j-1)).spotno, customers.at(v.route.at(j)).spotno) == -1)){
                                v.delay_arr = v.arr;
                                v.update_totalcost_dtable(customers, argvs);
                                impassable = false;
                                break;
                            }

							tr_time = get_ttable(v.vtype, lv_t, argvs.management_ttable, customers.at(v.route.at(j-1)).spotno, customers.at(v.route.at(j)).spotno)*v.drskill;
							v.delay_arr.at(j) = lv_t+tr_time;
							if(servt_dict.count(j) == 1){ 
                                if((j == 0 || customers.at(v.route.at(j)).spotid != customers.at(v.route.at(j-1)).spotid) && customers.at(v.route.at(j)).custno.substr(0, 1) != "e" && vector_finder(no_park_idx1, j) == false){
								    lv_t = max(v.delay_arr.at(j)+customers.at(v.route.at(j)).parkt_arrive, customers.at(v.route.at(j)).ready_fs)+servt_dict.at(j)*v.opskill;
                                }
                                else{
                                    lv_t = max(v.delay_arr.at(j), customers.at(v.route.at(j)).ready_fs)+servt_dict.at(j)*v.opskill;
                                }
                            }
							else{
                                if((j == 0 || customers.at(v.route.at(j)).spotid != customers.at(v.route.at(j-1)).spotid) && customers.at(v.route.at(j)).custno.substr(0, 1) != "e" && vector_finder(no_park_idx1, j) == false){
								    lv_t = max(v.delay_arr.at(j)+customers.at(v.route.at(j)).parkt_arrive, customers.at(v.route.at(j)).ready_fs)+customers.at(v.route.at(j)).servt*v.opskill;
                                }
                                else{
                                    lv_t = max(v.delay_arr.at(j), customers.at(v.route.at(j)).ready_fs)+customers.at(v.route.at(j)).servt*v.opskill;
                                }
                            }
							if((j == 0 || customers.at(v.route.at(j)).spotid != customers.at(v.route.at(j-1)).spotid) && customers.at(v.route.at(j)).custno.substr(0, 1) != "e" && vector_finder(no_park_idx1, j) == false){
								lv_t += customers.at(v.route.at(j)).parkt_open;
                            }
							if(v.chg_batt_dict.count(v.route.at(j)) == 1){
								lv_t += v.chg_batt_dict.at(v.route.at(j))/customers.at(v.route.at(j)).e_rate;
                            }
                        }

						if(!impassable){
							break;
						}
                    }
					else{
						if(servt_dict.count(i) == 1){ 
							v.delay_arr.at(i) = lv_t-servt_dict.at(i)*v.opskill;
						}
						else{
							v.delay_arr.at(i) = lv_t-customers.at(v.route.at(i)).servt*v.opskill;
						}
						if((i == 0 || customers.at(v.route.at(i)).spotid != customers.at(v.route.at(i-1)).spotid) && vector_finder(no_park_idx1, i) == false){
							v.delay_arr.at(i) -= customers.at(v.route.at(i)).parkt_open+customers.at(v.route.at(i)).parkt_arrive;
						}
					}
				}
			}
				
		
			v.update_totalcost_dtable(customers, argvs);

			for(int i = 0; i < route_size; i++){
				if(v.rem_batt.at(i)-v.chg_batt.at(i) >= v.e_margin && v.delay_rem_batt.at(i)-v.delay_chg_batt.at(i) < v.e_margin){ //遅延することで電欠となるなら遅延しない
					v.delay_arr=v.arr;
					break;
				}
			}

			if((argvs.timePenaltyType == "hard" || argvs.timePenaltyWeight > 0) && v.arr.at(0) != v.delay_arr.at(0) && argvs.lowertime > 0){
                if(compare_time(v.delay_lv.at(route_size-1)-argvs.lowertime, v.delay_arr.at(0)) < 0 && compare_time(v.arr.at(0), v.delay_lv.at(route_size-1)-argvs.lowertime) <= 0){
				    v.delay_arr.at(0) = v.delay_lv.at(route_size-1)-argvs.lowertime; //ltimeを下回らないように遅延する
                }
			}

			v.update_totalcost_dtable(customers, argvs);

            for(int i = 0; i < route_size-1; i++){
                //通行不可なら遅延しない
				if(get_ttable(v.vtype, v.delay_lv.at(i), argvs.management_ttable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1
				|| get_dtable(v.vtype, v.delay_lv.at(i), argvs.management_dtable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1
				|| (!argvs.management_etable.empty() && get_etable(v.vtype, v.etype, v.delay_lv.at(i), argvs.management_etable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1)
				|| (!argvs.management_xtable.empty() && get_xtable(v.vtype, v.delay_lv.at(i), argvs.management_xtable, customers.at(v.route.at(i)).spotno, customers.at(v.route.at(i+1)).spotno) == -1)){
					v.delay_arr = v.arr;
                    v.update_totalcost_dtable(customers, argvs);
					break;
                }
            }
		}
	}
}

namespace{
    //vecにnumberが含まれているか調べる
    bool vector_finder(const vector<int> &vec, const int number){
        auto itr = find(vec.begin(), vec.end(), number);
        size_t index = distance(vec.begin(), itr);

        if(index != vec.size()){
            return true; //発見できたとき
        }
        else{
            return false; //発見できなかったとき
        }
    }
}