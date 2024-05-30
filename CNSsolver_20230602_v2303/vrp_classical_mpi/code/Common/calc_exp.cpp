#include "calc_exp.hpp"

/**
 * @brief 車両の区間内の総経験コストを計算する関数
 * @param[in] vehicle 車両オブジェクト
 * @param[in] customers 配送先リスト
 * @param[in] start_idx 開始インデックス
 * @param[in] end_idx 終了インデックス
 * @param[in] management_xtable 距離テーブル一覧
 * @return double 車両の区間内の総経験コスト
 * @details 車両のstart_idx～end_idxの区間内の総経験コストを計算する関数。
 */
double calc_exp(const Vehicle &vehicle, const vector<Customer> &customers, const int start_idx, const int end_idx, const map<tuple<string, double>, vector<vector<double>>> &management_xtable){
    double exp = 0;
    
    for(int i = start_idx; i < end_idx; i++){
        exp += get_xtable(vehicle.vtype, vehicle.lv.at(i-1), management_xtable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(i)).spotno);
    }

    return exp;
}