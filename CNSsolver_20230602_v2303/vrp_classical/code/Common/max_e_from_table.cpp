#include "max_e_from_table.hpp"

/**
 * @brief 充電時間を考慮したテーブルでの消費電力を返す関数
 * @param[in] vtype 車両タイプ
 * @param[in] etype 電動車両タイプ
 * @param[in] start_time テーブル適用開始時刻
 * @param[in] from_c 出発地点の配送先
 * @param[in] to_c 到着地点の配送先
 * @param[in] management_etable テーブル一覧
 * @return double 充電時間を考慮したテーブルでの消費電力
 * @details start_time以降のテーブルでの最大消費電力を返す関数。
 */
double max_e_from_table(const string vtype, const string etype, const double start_time, const Customer &from_c, const Customer &to_c, const map<tuple<string, string, double>, vector<vector<double>>> &management_etable){
    bool table_found = false;
    tuple<double, vector<vector<double>>> etable_last;
    vector<vector<double>> temp;
    etable_last = make_tuple(-1e9, temp);
    double max_e = -1e9;
    
    //table検索
    for(auto table = management_etable.rbegin(); table != management_etable.rend(); table++){
        if(get<0>(table->first) == vtype && get<1>(table->first) == etype){
            if(compare_time(get<0>(etable_last), get<2>(table->first)) < 0){
                get<1>(etable_last) = table->second; //時刻最大のテーブルを保存
            }

            vector<vector<double>> etable = table->second;
            table_found = true;

            if(compare_time(get<2>(table->first), start_time) >= 0){
                max_e = max(etable.at(from_c.spotno).at(to_c.spotno), max_e); //start_time以降のテーブルの最大値を保存
            }
        }

        if(get<0>(table->first) == "time_common" && get<1>(table->first) == "time_common"){
            if(compare_time(get<0>(etable_last), get<2>(table->first)) < 0){
                get<1>(etable_last) = table->second; //時刻最大のテーブルを保存
            }

            vector<vector<double>> etable = table->second;
            table_found = true;

            if(compare_time(get<2>(table->first), start_time) >= 0){
                max_e = max(etable.at(from_c.spotno).at(to_c.spotno), max_e); //start_time以降のテーブルの最大値を保存
            }
        }
    
        if(get<0>(table->first) == "common" && get<1>(table->first) == "common"){
            if(compare_time(get<0>(etable_last), get<2>(table->first)) < 0){
                get<1>(etable_last) = table->second; //時刻最大のテーブルを保存
            }

            vector<vector<double>> etable = table->second;
            table_found = true;

            if(compare_time(get<2>(table->first), start_time) >= 0){
                max_e = max(etable.at(from_c.spotno).at(to_c.spotno), max_e); //start_time以降のテーブルの最大値を保存
            }
        }
    }

    //該当の車両識別番号のtableが見つかった場合
    if(table_found){
        if(max_e != -1e9){
            return max_e;
        }
        else{
            return get<1>(etable_last).at(from_c.spotno).at(to_c.spotno); //start_time以降のテーブルがない場合は時刻最大のテーブルの値を返す
        }
    }

    if(max_e == -1e9){
        string sErrMes = "epower table (aka etable) does not exist (vtype = "+vtype+", etype = "+etype+", start_time = "+to_string(start_time)+")";
        throw sErrMes;
    }
}