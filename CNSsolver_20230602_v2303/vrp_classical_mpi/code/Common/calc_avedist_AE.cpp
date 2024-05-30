#include "calc_avedist_AE.hpp"

/**
 * @brief 距離平準化ペナルティ(絶対値)を計算する関数
 * @param[in] vehicles 車両リスト
 * @param[in] customers 配送先リスト
 * @param[in] mvehicles 改善のあった車両の距離
 * @param[in] avedist_ae 許容誤差
 * @param[in] lastc_flag 最終訪問先をペナルティ計算に含めるかどうかのフラグ(true : 含めない)
 * @param[in] decr_vnum 改善によって削除された車両のインデックス
 * @return double 距離平準化ペナルティ
 * @details 各車両の距離と全車両の平均のずれの絶対値が許容誤差を超えている分のペナルティを計算する関数。
 */
double calc_avedist_AE(const vector<Vehicle> &vehicles, const vector<Customer> &customers, const map<int, double> &mvehicles, const double avedist_ae, const bool lastc_flag, const int decr_veh){
    int valid_vnum = (int) 0; //有効車両台数
    int vnum = (int) vehicles.size(); //車両台数
    vector<int> route_size;
    vector<int> non_depo_num;
    vector<double> dist_veh;

    for(int i = 0; i < vnum; i++){
        non_depo_num.emplace_back(0);
        route_size.emplace_back(vehicles.at(i).route.size());
        for(int j = 1; j < route_size.at(i)-1; j++){
            if((customers.at(vehicles.at(i).route.at(j)).custno != "0")&&(customers.at(vehicles.at(i).route.at(j)).custno.at(0) != 'b')&&(customers.at(vehicles.at(i).route.at(j)).custno.at(0) != 'e')){
                non_depo_num.at(i)++;
            }
        }

        if(vehicles.at(i).direct_flag ==true || non_depo_num.at(i) == 0){
            continue; //直送便か空車両ならスキップ 
        }  

        valid_vnum++;
    }

    if(decr_veh != -1){
        valid_vnum -=1;
    }

    if(lastc_flag == true){
        for(int i = 0; i < vnum; i++){
            if(vehicles.at(i).direct_flag == true || non_depo_num.at(i) == 0 || mvehicles.count(i) == 1){
                continue; //直送便か空車両か改善車両ならスキップ 
            } 

            dist_veh.emplace_back(vehicles.at(i).totalcost_wolast);
        }
    }
    else{
        for(int i = 0; i < vnum; i++){
            if(vehicles.at(i).direct_flag == true || non_depo_num.at(i) == 0 || mvehicles.count(i) == 1){
                continue; //直送便か空車両か改善車両ならスキップ 
            }  

            dist_veh.emplace_back(vehicles.at(i).totalcost);
        }
    }

    for(auto dist : mvehicles){
        dist_veh.emplace_back(dist.second);
    }


    //誤差計算
    double ae = 0;
    int dist_veh_size = (int) dist_veh.size();

    for(int i = 0; i < dist_veh_size; i++){
        for(int j = 0; j < i; j++){
            double diff = abs(dist_veh.at(i)-dist_veh.at(j));

            ae = max(max(diff-avedist_ae, 0.0), ae);
        }
    }

    return ae;
}

vector<double> calc_avedist_AE_list(const vector<Vehicle> &vehicles, const vector<Customer> &customers, const double avedist_ae, const bool lastc_flag){
    int valid_vnum = (int) 0; //有効車両台数
    int vnum = (int) vehicles.size(); //車両台数
    vector<int> route_size;
    vector<int> non_depo_num;
    vector<double> dist_veh;
    vector<double> ae;

    for(int i = 0; i < vnum; i++){
        non_depo_num.emplace_back(0);
        route_size.emplace_back(vehicles.at(i).route.size());
        for(int j = 1; j < route_size.at(i)-1; j++){
            if(customers.at(vehicles.at(i).route.at(j)).custno != "0"){
                non_depo_num.at(i)++;
            }
        }

        if(vehicles.at(i).direct_flag ==true || non_depo_num.at(i) == 0){
            continue; //直送便か空車両ならスキップ 
        }  

        valid_vnum++;
    }

    if(lastc_flag == true){
        for(int i = 0; i < vnum; i++){
            if(vehicles.at(i).direct_flag == true || non_depo_num.at(i) == 0){
                continue; //直送便か空車両か改善車両ならスキップ 
            } 

            dist_veh.emplace_back(vehicles.at(i).totalcost_wolast);
        }
    }
    else{
        for(int i = 0; i < vnum; i++){
            if(vehicles.at(i).direct_flag == true || non_depo_num.at(i) == 0){
                continue; //直送便か空車両か改善車両ならスキップ 
            }  

            dist_veh.emplace_back(vehicles.at(i).totalcost);
        }
    }

    //誤差計算
    int dist_veh_size = (int) dist_veh.size();

    for(int i = 0; i < dist_veh_size; i++){
        for(int j = 0; j < i; j++){
            double diff = abs(dist_veh.at(i)-dist_veh.at(j));

            ae.push_back(max(diff-avedist_ae, 0.0));
        }
    }

    return ae;
}