#include "get_dtable.hpp"

/**
 * @brief 対応する距離を返す関数
 * @param[in] vtype 車両タイプ
 * @param[in] start_time テーブル適用開始時刻
 * @param[in] management_dtable テーブル一覧
 * @param[in] from_c 出発地点のspotno
 * @param[in] to_c 到着地点のspotno
 * @return double from_cからto_cまでの距離
 * @details vtypeとstart_timeに対応するfrom_cからto_cまでの距離を返す関数。
 */
double get_dtable(const string vtype, const double start_time, const map<tuple<string, double>, vector<vector<double>>> &management_dtable, const int from_c, const int to_c){
    //table検索(配送時刻がある場合)
    for(auto table = management_dtable.rbegin(); table != management_dtable.rend(); table++){
        if(get<0>(table->first) != "common" && get<0>(table->first) == vtype && compare_time(get<1>(table->first), start_time) <= 0){
            return table->second.at(from_c).at(to_c);
        }
    }

    for(auto table = management_dtable.rbegin(); table != management_dtable.rend(); table++){
        if(get<0>(table->first) == "time_common" && compare_time(get<1>(table->first), start_time) <= 0){
            return table->second.at(from_c).at(to_c);
        }
    }

    //table検索(配送時刻がない場合)
    for(auto table = management_dtable.rbegin(); table != management_dtable.rend(); table++){
        if(get<0>(table->first) == "common"){
            return table->second.at(from_c).at(to_c);
        }
    }

    string sErrMes = "distance table (aka dtable) does not exist (vtype = "+vtype+", start_time = "+to_string(start_time)+")";
    throw sErrMes;
}