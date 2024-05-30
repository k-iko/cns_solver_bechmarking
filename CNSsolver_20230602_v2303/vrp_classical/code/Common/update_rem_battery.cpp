#include "update_rem_battery.hpp"

/**
 * @brief 各配送先での残電力及び充電量を計算する関数
 * @param[in] customers 配送先リスト
 * @param[in] argvs グローバル変数格納用
 * @param[in] after_idx 計算開始インデックス
 * @details 各配送先での残電力及び充電量を計算し、更新する関数。
 */
void Vehicle::update_rem_battery(const vector<Customer> &customers, const Cppargs &argvs, const int after_idx){
    int route_size = (int) route.size();

    if(route_size < 2){
        rem_batt = vector<double>(2, 0);
        chg_batt = vector<double>(2, 0);

        if(!delay_arr.empty() && argvs.break_flag==false){
            delay_rem_batt = vector<double>(2, 0);
            delay_chg_batt = vector<double>(2, 0);
        }

        return;
    }

    if(after_idx == 0){
        rem_batt = vector<double>(route_size, 0);
        chg_batt = vector<double>(route_size, 0);
        
        rem_batt.at(0) = e_initial;

        for(int i = 1; i < route_size; i++){
            Customer from_c = customers.at(route.at(i-1)), to_c = customers.at(route.at(i));

            if(argvs.management_etable.empty()){ //電力テーブルがない場合
                double tdist = get_dtable(vtype, lv.at(i-1), argvs.management_dtable, from_c.spotno, to_c.spotno)/1000;
                rem_batt.at(i) = rem_batt.at(i-1)-tdist/e_cost;
            }
            else{ //電力テーブルがある場合
                rem_batt.at(i) = rem_batt.at(i-1)-get_etable(vtype, etype, lv.at(i-1), argvs.management_etable, from_c.spotno, to_c.spotno);
            }

            bool is_espot = false;

            if(to_c.custno.substr(0, 1) == "e"){
                is_espot = true;
            }

            if(is_espot){ //充電スポットなら
                double q_to_charge, q_to_depot;

                tie(q_to_charge, q_to_depot) = find_power_to_charge(customers, i, argvs);

                if(q_to_depot <= rem_batt.at(i)){
                    chg_batt_dict[route.at(i)] = 0; //充電が足りる場合は0にする
                }
                if(q_to_depot > rem_batt.at(i)){
                    if(e_capacity >= q_to_depot){ //充電が足りず容量を超えない場合
                        rem_batt.at(i) = q_to_depot;
                        chg_batt_dict[route.at(i)] = q_to_charge;
                    }
                    else{ //充電が足りず容量を超える場合
                        chg_batt_dict[route.at(i)] = e_capacity-rem_batt.at(i);
                        rem_batt.at(i) = e_capacity;
                    }
                }

                chg_batt.at(i) = chg_batt_dict.at(route.at(i));
            }
        }
    }
    else{
        for(int i = after_idx; i < route_size-1; i++){
            Customer from_c = customers.at(route.at(i)), to_c = customers.at(route.at(i+1));

            if(argvs.management_etable.empty()){ //電力テーブルがない場合
                double tdist = get_dtable(vtype, lv.at(i), argvs.management_dtable, from_c.spotno, to_c.spotno)/1000;
                rem_batt.at(i+1) = rem_batt.at(i)-tdist/e_cost;
            }
            else{ //電力テーブルがある場合
                rem_batt.at(i+1) = rem_batt.at(i)-get_etable(vtype, etype, lv.at(i), argvs.management_etable, from_c.spotno, to_c.spotno);
            }

            if(chg_batt_dict.count(route.at(i+1)) == 1){
                rem_batt.at(i) += chg_batt_dict.at(route.at(i+1));
            }
        }
    }

    if(!delay_arr.empty() && argvs.break_flag==false){
        if(after_idx == 0){
            delay_rem_batt = vector<double>(route_size, 0);
            delay_chg_batt = vector<double>(route_size, 0);
            
            delay_rem_batt.at(0) = e_initial;

            for(int i = 1; i < route_size; i++){
                Customer from_c = customers.at(route.at(i-1)), to_c = customers.at(route.at(i));

                if(argvs.management_etable.empty()){ //電力テーブルがない場合
                    double tdist = get_dtable(vtype, delay_lv.at(i-1), argvs.management_dtable, from_c.spotno, to_c.spotno)/1000;
                    delay_rem_batt.at(i) = delay_rem_batt.at(i-1)-tdist/e_cost;
                }
                else{ //電力テーブルがある場合
                    delay_rem_batt.at(i) = delay_rem_batt.at(i-1)-get_etable(vtype, etype, delay_lv.at(i-1), argvs.management_etable, from_c.spotno, to_c.spotno);
                }

                bool is_espot = false;

                if(to_c.custno.substr(0, 1) == "e"){
                    is_espot = true;
                }

                if(is_espot){ //充電スポットなら
                    double q_to_charge, q_to_depot;

                    tie(q_to_charge, q_to_depot) = find_power_to_charge(customers, i, argvs);

                    if(q_to_depot <= rem_batt.at(i)){
                        delay_chg_batt_dict[route.at(i)] = 0; //充電が足りる場合は0にする
                    }
                    if(q_to_depot > delay_rem_batt.at(i)){
                        if(e_capacity >= q_to_depot){ //充電が足りず容量を超えない場合
                            delay_rem_batt.at(i) = q_to_depot;
                            delay_chg_batt_dict[route.at(i)] = q_to_charge;
                        }
                        else{ //充電が足りず容量を超える場合
                            delay_chg_batt_dict[route.at(i)] = e_capacity-delay_rem_batt.at(i);
                            delay_rem_batt.at(i) = e_capacity;
                        }
                    }

                    delay_chg_batt.at(i) = delay_chg_batt_dict.at(route.at(i));
                }
            }
        }
        else{
            for(int i = after_idx; i < route_size-1; i++){
                Customer from_c = customers.at(route.at(i)), to_c = customers.at(route.at(i+1));

                if(argvs.management_etable.empty()){ //電力テーブルがない場合
                    double tdist = get_dtable(vtype, delay_lv.at(i), argvs.management_dtable, from_c.spotno, to_c.spotno)/1000;
                    delay_rem_batt.at(i+1) = delay_rem_batt.at(i)-tdist/e_cost;
                }
                else{ //電力テーブルがある場合
                    delay_rem_batt.at(i+1) = delay_rem_batt.at(i)-get_etable(vtype, etype, delay_lv.at(i), argvs.management_etable, from_c.spotno, to_c.spotno);
                }

                if(delay_chg_batt_dict.count(route.at(i+1)) == 1){
                    delay_rem_batt.at(i) += delay_chg_batt_dict.at(route.at(i+1));
                }
            }
        }
    }
}