#include "find_power_to_charge.hpp"

/**
 * @brief 拠点に戻るために充電しなければいけない電力量および充電後の電力量を計算する関数
 * @param[in] customers 配送先リスト
 * @param[in] start_idx 開始インデックス
 * @param[in] argvs グローバル変数格納用
 * @return tuple<double, double> 拠点に戻るために充電しなければいけない電力量および充電後の電力量
 * @details start_idxから拠点に戻るために充電しなければいけない電力量および充電後の電力量を計算する関数。
 */
tuple<double, double> Vehicle::find_power_to_charge(const vector<Customer> &customers, const int start_idx, const Cppargs &argvs){
    double dist_to_depot = 0, q_to_depot = 0;

    double cur_rem_batt = rem_batt.at(start_idx);

    if(argvs.management_etable.empty()){ //電力テーブルがない場合
        dist_to_depot += max_dist_from_table(vtype, lv.at(start_idx), customers.at(route.at(start_idx)), customers.at(route.at(start_idx+1)), argvs.management_dtable);

        int route_size = (int) route.size();
        for(int i = start_idx+1; i < route_size-1; i++){
            dist_to_depot += get_dtable(vtype, lv.at(i), argvs.management_dtable, customers.at(route.at(i)).spotno, customers.at(route.at(i+1)).spotno);
        }

        q_to_depot = (dist_to_depot/1000)/e_cost;
    }
    else{ //電力テーブルがある場合
        q_to_depot += max_e_from_table(vtype, etype, lv.at(start_idx), customers.at(route.at(start_idx)), customers.at(route.at(start_idx+1)), argvs.management_etable);

        int route_size = (int) route.size();
        for(int i = start_idx+1; i < route_size-1; i++){
            q_to_depot += get_etable(vtype, etype, lv.at(i), argvs.management_etable, customers.at(route.at(i)).spotno, customers.at(route.at(i+1)).spotno);
        }
    }

    q_to_depot += e_margin; //マージンを加算

    double charge_to_depot = q_to_depot-cur_rem_batt; //拠点に戻るために充電しなければいけない電力

    if(!delay_rem_batt.empty()){
        dist_to_depot = 0, q_to_depot = 0;

        cur_rem_batt = delay_rem_batt.at(start_idx);

        if(argvs.management_etable.empty()){ //電力テーブルがない場合
            dist_to_depot += max_dist_from_table(vtype, delay_lv.at(start_idx), customers.at(route.at(start_idx)), customers.at(route.at(start_idx+1)), argvs.management_dtable);

            int route_size = (int) route.size();
            for(int i = start_idx+1; i < route_size-1; i++){
                dist_to_depot += get_dtable(vtype, delay_lv.at(i), argvs.management_dtable, customers.at(route.at(i)).spotno, customers.at(route.at(i+1)).spotno);
            }

            q_to_depot = (dist_to_depot/1000)/e_cost;
        }
        else{ //電力テーブルがある場合
            q_to_depot += max_e_from_table(vtype, etype, delay_lv.at(start_idx), customers.at(route.at(start_idx)), customers.at(route.at(start_idx+1)), argvs.management_etable);

            int route_size = (int) route.size();
            for(int i = start_idx+1; i < route_size-1; i++){
                q_to_depot += get_etable(vtype, etype, delay_lv.at(i), argvs.management_etable, customers.at(route.at(i)).spotno, customers.at(route.at(i+1)).spotno);
            }
        }

        q_to_depot += e_margin; //マージンを加算

        charge_to_depot = q_to_depot-cur_rem_batt; //拠点に戻るために充電しなければいけない電力
    }

    return make_tuple(charge_to_depot, q_to_depot);
}