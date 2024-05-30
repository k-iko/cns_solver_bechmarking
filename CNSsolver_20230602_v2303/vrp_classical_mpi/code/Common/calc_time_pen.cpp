#include "calc_time_pen.hpp"

/**
 * @brief 出発時刻を更新し、到着地点での遅刻ペナルティを計算する関数
 * @param[in] vehicle 車両オブジェクト
 * @param[in] from_c 出発地点の配送先
 * @param[in] to_c 到着地点の配送先
 * @param[in] lv_t 出発時刻
 * @param[in] proc_time 作業時間
 * @param[in] management_ttable 時刻テーブル一覧
 * @param[in] no_park to_cの駐車時間を計算するかのフラグ(true:計算しない)
 * @return double 遅刻ペナルティ
 * @details 出発時刻を出発地点のものから到着地点のものに更新し、到着地点にどれだけ遅刻したかを計算する関数。
 */
double calc_time_pen(const Vehicle &vehicle, const Customer &from_c, const Customer &to_c, double &lv_t, const double proc_time, const map<tuple<string, double>, vector<vector<double>>> &management_ttable, bool no_park){
    double arr_t = lv_t+get_ttable(vehicle.vtype, lv_t, management_ttable, from_c.spotno, to_c.spotno)*vehicle.drskill; //to_cに到着する時刻
    double st_t;

    if(to_c.spotid != from_c.spotid && !no_park){
        lv_t = max(arr_t+to_c.parkt_arrive, to_c.ready_fs)+proc_time*vehicle.opskill; //to_cを出発する時刻
        st_t = max(arr_t+to_c.parkt_arrive, to_c.ready_fs)+to_c.parkt_open; //to_cを出発する時刻
    }
    else{
        lv_t = max(arr_t, to_c.ready_fs)+proc_time*vehicle.opskill; //to_cを出発する時刻
        st_t = max(arr_t, to_c.ready_fs);
    }
    if(to_c.spotid != from_c.spotid && !no_park){
        lv_t += to_c.parkt_open; //同一スポットでなければ駐車時間を加算
    }

    double time_pen = max(0.0, st_t-to_c.due_fs); //受け入れ終了時刻に遅刻した場合はペナルティとする

    return time_pen;
}