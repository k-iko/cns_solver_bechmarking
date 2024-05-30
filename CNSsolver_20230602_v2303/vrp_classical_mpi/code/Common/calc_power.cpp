#include "calc_power.hpp"

/**
 * @brief 車両の区間内の総消費電力を計算する関数
 * @param[in] vehicle 車両オブジェクト
 * @param[in] customers 配送先リスト
 * @param[in] start_idx 開始インデックス
 * @param[in] end_idx 終了インデックス
 * @param[in] management_etable 距離テーブル一覧
 * @return double 車両の区間内の総消費電力
 * @details 車両のstart_idx～end_idxの区間内の総消費電力を計算する関数。
 */
double calc_power(const Vehicle &vehicle, const vector<Customer> &customers, const int start_idx, const int end_idx, const map<tuple<string, string, double>, vector<vector<double>>> &management_etable){
    double power = 0;
    
    for(int i = start_idx; i < end_idx; i++){
        power += get_etable(vehicle.vtype, vehicle.etype, vehicle.lv.at(i-1), management_etable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(i)).spotno);
    }

    return power;
}