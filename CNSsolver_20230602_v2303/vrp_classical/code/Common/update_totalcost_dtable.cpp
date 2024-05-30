#include "update_totalcost_dtable.hpp"

/**
 * @brief 車両情報を更新する関数
 * @param[in] customers 配送先リスト
 * @param[in] argvs グローバル変数格納用
 * @param[in] start_idx 開始インデックス
 * @details  車両情報を更新する関数。
 */
void Vehicle::update_totalcost_dtable(const vector<Customer> &customers, const Cppargs &argvs, const int start_idx, const bool delayst2_flag){
    int route_size = (int) route.size();
    int last_idx = route_size-2;

    update_arrtime_tw(customers, argvs.management_ttable, argvs.break_flag, delayst2_flag);

    //総走行距離を計算
    totalcost = 0;
    for(int i = 0; i < route_size-1; i++){
        totalcost += get_dtable(vtype, lv.at(i), argvs.management_dtable, customers.at(route.at(i)).spotno, customers.at(route.at(i+1)).spotno);
    }

    totalcost_wolast = 0;
    for(int i = route_size-2; i >= 1; i--){
        if((customers.at(route.at(i)).custno == "0")||(customers.at(route.at(i)).custno.at(0) == 'b')){
            last_idx = i-1;
        }
        else{
            break;
        }
    }
    for(int i = 0; i < last_idx; i++){
        totalcost_wolast += get_dtable(vtype, lv.at(i), argvs.management_dtable, customers.at(route.at(i)).spotno, customers.at(route.at(i+1)).spotno);
    }

    if(!delay_arr.empty() && argvs.break_flag==false){
        //総走行距離を計算
        delay_totalcost = 0;
        for(int i = 0; i < route_size-1; i++){
            delay_totalcost += get_dtable(vtype, delay_lv.at(i), argvs.management_dtable, customers.at(route.at(i)).spotno, customers.at(route.at(i+1)).spotno);
        }

        delay_totalcost_wolast = 0;
        for(int i = 0; i < last_idx; i++){
            delay_totalcost_wolast += get_dtable(vtype, delay_lv.at(i), argvs.management_dtable, customers.at(route.at(i)).spotno, customers.at(route.at(i+1)).spotno);
        }
    }

    update_load_onbrd(customers);
    update_rem_battery(customers, argvs, start_idx);
    update_arrtime_tw(customers, argvs.management_ttable, argvs.break_flag, delayst2_flag); //充電時間を反映させて再計算
    update_remdist(customers, argvs);
}