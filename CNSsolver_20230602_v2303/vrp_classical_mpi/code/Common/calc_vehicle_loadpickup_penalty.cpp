#include "calc_vehicle_loadpickup_penalty.hpp"

/**
 * @brief 車両の第1,第2積載量に対する集荷違反量を計算する関数
 * @param[in] vehicle 車両オブジェクト
 * @param[in] customers 配送先リスト
 * @param[in] route 計算対象ルート
 * @return tuple<flaot, flaot> 第1,第2積載量に対する集荷違反量
 * @details 車両の第1,第2積載量に対する集荷違反量を計算する関数。
 */
tuple<double, double> calc_vehicle_loadpickup_penalty(const Vehicle &vehicle, const vector<Customer> &customers, const vector<int> &route){
    double max_load_pen = 0, max_load_pen2 = 0;
    vector<int> depo_idx;
    vector<double> load_onbrd, load_onbrd2;

    int route_size = (int) route.size();
    for(int i = 0; i < route_size-1; i++){
        if(customers.at(route.at(i)).custno == "0"){
            depo_idx.emplace_back(i); //routeにおけるデポのインデックス
        }
    }
    depo_idx.emplace_back(route_size-1); //最終訪問先を追加

    int depo_idx_size = (int) depo_idx.size();
    for(int i = 0; i < depo_idx_size-1; i++){
        double dem_sum = 0, dem2_sum = 0;
        for(int j = depo_idx.at(i)+1; j < depo_idx.at(i+1); j++){
            if(customers.at(route.at(j)).requestType == "DELIVERY"){
                dem_sum += customers.at(route.at(j)).dem;
            
                dem2_sum += customers.at(route.at(j)).dem2;
            }
        }

        load_onbrd.emplace_back(dem_sum); //1回転分の荷物量
        load_onbrd2.emplace_back(dem2_sum); //1回転分の第2荷物量
    }

    if(route_size >= 2){
        for(int i = 0; i < depo_idx_size-1; i++){
            vector<double> load_pens, load_pens2;
            for(int j = depo_idx.at(i)+1; j < depo_idx.at(i+1); j++){
                //j番目の配送先出発時荷物量を計算
                if(customers.at(route.at(j)).requestType == "DELIVERY"){
                    load_onbrd.at(i) -= customers.at(route.at(j)).dem;
                    load_onbrd2.at(i) -= customers.at(route.at(j)).dem2;
                }
                else{
                    load_onbrd.at(i) += customers.at(route.at(j)).dem;
                    load_onbrd2.at(i) += customers.at(route.at(j)).dem2;
                }

                if(customers.at(route.at(j)).requestType == "PICKUP"){
                    //その地点での荷物量と車両の積載量を比較して、積載量を超えている値を違反値としてリストに格納
                    double load_pen = load_onbrd.at(i)-vehicle.cap;
                    if(load_pen > 0){
                        load_pens.emplace_back(load_pen);
                    }
                
                    //その地点での荷物量と車両の積載量を比較して、積載量を超えている値を違反値としてリストに格納
                    double load_pen2 = load_onbrd2.at(i)-vehicle.cap2;
                    if(load_pen2 > 0){
                        load_pens2.emplace_back(load_pen2);
                    }
                }
            }

            if(!load_pens.empty()){
                max_load_pen += *max_element(load_pens.begin(), load_pens.end());
            }

            if(!load_pens2.empty()){
                max_load_pen2 += *max_element(load_pens2.begin(), load_pens2.end());
            }
        }
    }

    return make_tuple(max_load_pen, max_load_pen2);
}