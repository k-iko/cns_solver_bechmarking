#include "calc_avedist_APE.hpp"

/**
 * @brief 距離平準化ペナルティ(パーセンテージ)を計算する関数
 * @param[in] vehicles 車両リスト
 * @param[in] customers 配送先リスト
 * @param[in] mvehicles 改善のあった車両の距離
 * @param[in] avedist_ape 許容誤差
 * @param[in] lastc_flag 最終訪問先をペナルティ計算に含めるかどうかのフラグ(true : 含めない)
 * @param[in] decr_vnum 改善によって削除された車両のインデックス
 * @return double 距離平準化ペナルティ
 * @details 各車両の距離と全車両の平均のずれの割合が許容誤差を超えている分のペナルティを計算する関数。
 */
double calc_avedist_APE(const vector<Vehicle> &vehicles, const vector<Customer> &customers, const map<int, double> &mvehicles, const double avedist_ape, const bool lastc_flag, const bool break_flag, const int decr_veh){
    int valid_vnum = (int) 0; //有効車両台数
    int vnum = (int) vehicles.size(); //車両台数
    vector<int> route_size;
    vector<int> non_depo_num;
    double mu = 0;

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
    
    if(decr_veh != -1 && break_flag==false){
        valid_vnum -=1;
    }

    if(valid_vnum <= 0){
        return 0; //直送便以外で対象車両が存在しない場合は0を返す
    }

    if(lastc_flag == true){
        for(int i = 0; i < vnum; i++){
            if(vehicles.at(i).direct_flag == true || non_depo_num.at(i) == 0 || mvehicles.count(i) == 1){
                continue; //直送便か空車両か改善車両ならスキップ 
            }  

            mu += vehicles.at(i).totalcost_wolast;
        }
    }
    else{
        for(int i = 0; i < vnum; i++){
            if(vehicles.at(i).direct_flag == true || non_depo_num.at(i) == 0 || mvehicles.count(i) == 1){
                continue; //直送便か空車両か改善車両ならスキップ 
            }  

            mu += vehicles.at(i).totalcost;
        }
    }

    for(auto dist : mvehicles){
        mu += dist.second;
    }

    mu /= valid_vnum; //平均値算出

    if(mu == 0){
        string sErrMes = "failed to calculate APE margin of distance-leveling (aka avedist_ape) due to distance = 0";
        throw sErrMes;
    }


    //誤差計算
    double ape_p = 0;
    double ape_m = 0;
    double ape;
    
    for(int i = 0; i < vnum; i++){
        double diff;
        if(vehicles.at(i).direct_flag == true || non_depo_num.at(i) == 0 || mvehicles.count(i) == 1){
            continue; //直送便か空車両か改善車両ならスキップ 
        }  

        if(lastc_flag == true){
            diff = abs(mu-vehicles.at(i).totalcost_wolast)/mu;
            if(mu <= vehicles.at(i).totalcost_wolast){
                ape_p = max(max(diff-avedist_ape,0.0), ape_p);
            }
            else{
                ape_m = max(max(diff-avedist_ape,0.0), ape_m);
            }
        }
        else{
            diff = abs(mu-vehicles.at(i).totalcost)/mu;
            if(mu <= vehicles.at(i).totalcost){
                ape_p = max(max(diff-avedist_ape,0.0), ape_p);
            }
            else{
                ape_m = max(max(diff-avedist_ape,0.0), ape_m);
            }
        }
    }

    for(auto dist : mvehicles){
        double diff;
        if(dist.first != decr_veh){
            diff = abs(mu-dist.second)/mu;
            if(mu <= dist.second){
                ape_p = max(max(diff-avedist_ape,0.0), ape_p);
            }
            else{
                ape_m = max(max(diff-avedist_ape,0.0), ape_m);
            }
        }
    }

    ape = (ape_p+ape_m) * valid_vnum / 2;

    return ape;
}

vector<double> calc_avedist_APE_list(const vector<Vehicle> &vehicles, const vector<Customer> &customers, const double avedist_ape, const bool lastc_flag){
    int valid_vnum = (int) 0; //有効車両台数
    int vnum = (int) vehicles.size(); //車両台数
    vector<int> route_size;
    vector<int> non_depo_num;
    double mu = 0;
    vector<double> ape;

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
    
    if(valid_vnum <= 0){
        return ape; //直送便以外で対象車両が存在しない場合は0を返す
    }

    if(lastc_flag == true){
        for(int i = 0; i < vnum; i++){
            if(vehicles.at(i).direct_flag == true || non_depo_num.at(i) == 0){
                continue; //直送便か空車両か改善車両ならスキップ 
            }  

            mu += vehicles.at(i).totalcost_wolast;
        }
    }
    else{
        for(int i = 0; i < vnum; i++){
            if(vehicles.at(i).direct_flag == true || non_depo_num.at(i) == 0){
                continue; //直送便か空車両か改善車両ならスキップ 
            }  

            mu += vehicles.at(i).totalcost;
        }
    }

    mu /= valid_vnum; //平均値算出

    if(mu == 0){
        string sErrMes = "failed to calculate APE margin of distance-leveling (aka avedist_ape) due to distance = 0";
        throw sErrMes;
    }


    //誤差計算
    
    for(int i = 0; i < vnum; i++){
        double diff;
        if(vehicles.at(i).direct_flag == true || non_depo_num.at(i) == 0){
            continue; //直送便か空車両か改善車両ならスキップ 
        }  

        if(lastc_flag == true){
            diff = abs(mu-vehicles.at(i).totalcost_wolast)/mu;
        }
        else{
            diff = abs(mu-vehicles.at(i).totalcost)/mu;
        }

        ape.push_back(max(diff-avedist_ape, 0.0));
    }

    return ape;
}