#include "max_dist_from_table.hpp"

/**
 * @brief 充電時間を考慮したテーブルでの距離を返す関数
 * @param[in] vtype 車両タイプ
 * @param[in] start_time テーブル適用開始時刻
 * @param[in] from_c 出発地点の配送先
 * @param[in] to_c 到着地点の配送先
 * @param[in] management_dtable テーブル一覧
 * @return double 充電時間を考慮したテーブルでの距離
 * @details start_time以降のテーブルでの最大距離を返す関数。
 */
double max_dist_from_table(const string vtype, const double start_time, const Customer &from_c, const Customer &to_c, const map<tuple<string, double>, vector<vector<double>>> &management_dtable){
    bool table_found = false;
    tuple<double, vector<vector<double>>> dtable_last;
    vector<vector<double>> temp;
    dtable_last = make_tuple(-1e9, temp);
    double max_dist = -1e9;
    
    //table検索
    for(auto table = management_dtable.rbegin(); table != management_dtable.rend(); table++){
        if(get<0>(table->first) == vtype){
            if(compare_time(get<0>(dtable_last), get<1>(table->first)) < 0){
                get<1>(dtable_last) = table->second; //時刻最大のテーブルを保存
            }

            vector<vector<double>> dtable = table->second;
            table_found = true;

            if(compare_time(get<1>(table->first), start_time) >= 0){
                max_dist = max(dtable.at(from_c.spotno).at(to_c.spotno), max_dist); //start_time以降のテーブルの最大値を保存
            }
        }

        if(get<0>(table->first) == "time_common"){
            if(compare_time(get<0>(dtable_last), get<1>(table->first)) < 0){
                get<1>(dtable_last) = table->second; //時刻最大のテーブルを保存
            }

            vector<vector<double>> dtable = table->second;
            table_found = true;

            if(compare_time(get<1>(table->first), start_time) >= 0){
                max_dist = max(dtable.at(from_c.spotno).at(to_c.spotno), max_dist); //start_time以降のテーブルの最大値を保存
            }
        }
    
        if(get<0>(table->first) == "common"){
            if(compare_time(get<0>(dtable_last), get<1>(table->first)) < 0){
                get<1>(dtable_last) = table->second; //時刻最大のテーブルを保存
            }

            vector<vector<double>> dtable = table->second;
            table_found = true;

            if(compare_time(get<1>(table->first), start_time) >= 0){
                max_dist = max(dtable.at(from_c.spotno).at(to_c.spotno), max_dist); //start_time以降のテーブルの最大値を保存
            }
        }
    }

    //該当の車両識別番号のtableが見つかった場合
    if(table_found){
        if(max_dist != -1e9){
            return max_dist;
        }
        else{
            return get<1>(dtable_last).at(from_c.spotno).at(to_c.spotno); //start_time以降のテーブルがない場合は時刻最大のテーブルの値を返す
        }
    }

    if(max_dist == -1e9){
        string sErrMes = "distance table (aka dtable) does not exist (vtype = "+vtype+", start_time = "+to_string(start_time)+")";
        throw sErrMes;
    }
}