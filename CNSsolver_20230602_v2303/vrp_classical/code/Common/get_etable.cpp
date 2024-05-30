#include "get_etable.hpp"

/**
 * @brief 対応する消費電力を返す関数
 * @param[in] vtype 車両タイプ
 * @param[in] etype 電動車両タイプ
 * @param[in] start_time テーブル適用開始時刻
 * @param[in] management_etable テーブル一覧
 * @param[in] from_c 出発地点のspotno
 * @param[in] to_c 到着地点のspotno
 * @return double from_cからto_cまでの消費電力
 * @details vtypeとetypeとstart_timeに対応するfrom_cからto_cまでの消費電力を返す関数。
 */
double get_etable(const string vtype, const string etype, const double start_time, const map<tuple<string, string, double>, vector<vector<double>>> &management_etable, const int from_c, const int to_c){
    //table検索(配送時刻がある場合)
    for(auto table = management_etable.rbegin(); table != management_etable.rend(); table++){
        if(get<0>(table->first) != "common" && get<1>(table->first) != "common" && get<0>(table->first) == vtype && get<1>(table->first) == etype && compare_time(get<2>(table->first), start_time) <= 0){
            return table->second.at(from_c).at(to_c);
        }
    }

    for(auto table = management_etable.rbegin(); table != management_etable.rend(); table++){
        if(get<0>(table->first) == "time_common" && get<1>(table->first) == "time_common" && compare_time(get<2>(table->first), start_time) <= 0){
            return table->second.at(from_c).at(to_c);
        }
    }

    //table検索(配送時刻がない場合)
    for(auto table = management_etable.rbegin(); table != management_etable.rend(); table++){
        if(get<0>(table->first) == "common" && get<1>(table->first) == "common"){
            return table->second.at(from_c).at(to_c);
        }
    }

    string sErrMes = "epower table (aka etable) does not exist (vtype = "+vtype+", etype = "+etype+", start_time = "+to_string(start_time)+")";
    throw sErrMes;
}