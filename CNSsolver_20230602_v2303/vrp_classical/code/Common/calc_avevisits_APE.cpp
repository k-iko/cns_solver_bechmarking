#include "calc_avevisits_APE.hpp"

/**
 * @brief 配送件数平準化ペナルティ(パーセンテージ)を計算する関数
 * @param[in] vehicles 車両リスト
 * @param[in] customers 配送先リスト
 * @param[in] mvehicles 改善のあった車両の配送件数
 * @param[in] avevisits_ape 許容誤差
 * @param[in] bulkShipping_flag 荷物tie-up用フラグ(true : 荷物をtie-upする) 
 * @param[in] decr_vnum 改善によって削除された車両のインデックス
 * @return double 配送件数平準化ペナルティ
 * @details 各車両の配送件数と全車両の平均のずれの割合が許容誤差を超えている分のペナルティを計算する関数。
 */
double calc_avevisits_APE(const vector<Vehicle> &vehicles, const vector<Customer> &customers, const map<int, double> &mvehicles, const double avevisits_ape, const bool bulkShipping_flag, const bool break_flag, const int decr_veh){
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

    if(bulkShipping_flag == true){
        for(int i = 0; i < vnum; i++){
            if(vehicles.at(i).direct_flag == true || non_depo_num.at(i) == 0 || mvehicles.count(i) == 1){
                continue; //直送便か空車両か改善車両ならスキップ 
            }  

            int visits_num = 0;
            for(int j = 1; j < route_size.at(i)-1; j++){
                int tieup_num = customers.at(vehicles.at(i).route.at(j)).tieup_customer.size();
                for(int k = 0; k < tieup_num; k++){
                    if(customers.at(customers.at(vehicles.at(i).route.at(j)).tieup_customer.at(k)).custno.at(0) == 'e' || customers.at(customers.at(vehicles.at(i).route.at(j)).tieup_customer.at(k)).custno.at(0) == 'b' || customers.at(customers.at(vehicles.at(i).route.at(j)).tieup_customer.at(k)).custno == "0"){
                        continue; //充電スポットかデポの場合はスキップ
                    }

                    visits_num++;
                }
            }

            mu += visits_num;
        }
    }
    else{
        for(int i = 0; i < vnum; i++){
            if(vehicles.at(i).direct_flag == true || non_depo_num.at(i) == 0 || mvehicles.count(i) == 1){
                continue; //直送便か空車両か改善車両ならスキップ 
            }  

            int visits_num = 0;
            for(int j = 1; j < route_size.at(i)-1; j++){
                if(customers.at(vehicles.at(i).route.at(j)).custno.at(0) == 'e' || customers.at(vehicles.at(i).route.at(j)).custno.at(0) == 'b' || customers.at(vehicles.at(i).route.at(j)).custno == "0"){
                    continue; //充電スポットかデポの場合はスキップ
                }

                visits_num++;
            }

            mu += visits_num;
        }
    }

    for(auto visits_num : mvehicles){
        mu += visits_num.second;
    }

    mu /= valid_vnum; //平均値算出

    if(compare_time(mu) == 0){
        string sErrMes = "failed to calculate APE margin of time-leveling (aka avetime_ape) due to delivery time = 0";
        throw sErrMes;
    }


    //誤差計算
    double ape_p = 0;
    double ape_m = 0;
    double ape = 0;
    
    for(int i = 0; i < vnum; i++){
        double diff;
        if(vehicles.at(i).direct_flag == true || non_depo_num.at(i) == 0 || mvehicles.count(i) == 1){
            continue; //直送便か空車両か改善車両ならスキップ 
        }  

        int visits_num = 0;
        if(bulkShipping_flag == true){
            for(int j = 1; j < route_size.at(i)-1; j++){
                int tieup_num = customers.at(vehicles.at(i).route.at(j)).tieup_customer.size();
                for(int k = 0; k < tieup_num; k++){
                    if(customers.at(customers.at(vehicles.at(i).route.at(j)).tieup_customer.at(k)).custno.at(0) == 'e' || customers.at(customers.at(vehicles.at(i).route.at(j)).tieup_customer.at(k)).custno.at(0) == 'b' || customers.at(customers.at(vehicles.at(i).route.at(j)).tieup_customer.at(k)).custno == "0"){
                        continue; //充電スポットかデポの場合はスキップ
                    }

                    visits_num++;
                }
            }

            diff = abs(mu-visits_num)/mu;
        }
        else{
            for(int j = 1; j < route_size.at(i)-1; j++){
                if(customers.at(vehicles.at(i).route.at(j)).custno.at(0) == 'e' || customers.at(vehicles.at(i).route.at(j)).custno.at(0) == 'b' ||customers.at(vehicles.at(i).route.at(j)).custno == "0" || customers.at(vehicles.at(i).route.at(j)).custno == customers.at(vehicles.at(i).route.at(j-1)).custno){
                    continue; //充電スポットかデポの場合または同一CUSTNOが連続する場合はスキップ
                }

                visits_num++;
            }

            diff = abs(mu-visits_num)/mu;
        }

        if(mu <= visits_num){
            ape_p = max(max(diff-avevisits_ape,0.0), ape_p);
        }
        else{
            ape_m = max(max(diff-avevisits_ape,0.0), ape_m);
        }
    }

    for(auto visits_num : mvehicles){
        double diff;
        if(visits_num.first != decr_veh){
            diff = abs(mu-visits_num.second)/mu;
            if(mu <= visits_num.second){
                ape_p = max(max(diff-avevisits_ape,0.0), ape_p);
            }
            else{
                ape_m = max(max(diff-avevisits_ape,0.0), ape_m);
            }
        }
    }

    ape = (ape_p+ape_m) * valid_vnum / 2;

    return ape;
}

vector<double> calc_avevisits_APE_list(const vector<Vehicle> &vehicles, const vector<Customer> &customers, const double avevisits_ape, const bool bulkShipping_flag){
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

    if(bulkShipping_flag == true){
        for(int i = 0; i < vnum; i++){
            if(vehicles.at(i).direct_flag == true || non_depo_num.at(i) == 0){
                continue; //直送便か空車両か改善車両ならスキップ 
            }  

            int visits_num = 0;
            for(int j = 1; j < route_size.at(i)-1; j++){
                int tieup_num = customers.at(vehicles.at(i).route.at(j)).tieup_customer.size();
                for(int k = 0; k < tieup_num; k++){
                    if(customers.at(customers.at(vehicles.at(i).route.at(j)).tieup_customer.at(k)).spotid.substr(0, 1) == "e" || customers.at(customers.at(vehicles.at(i).route.at(j)).tieup_customer.at(k)).custno == "0"){
                        continue; //充電スポットかデポの場合はスキップ
                    }

                    visits_num++;
                }
            }

            mu += visits_num;
        }
    }
    else{
        for(int i = 0; i < vnum; i++){
            if(vehicles.at(i).direct_flag == true || non_depo_num.at(i) == 0){
                continue; //直送便か空車両か改善車両ならスキップ 
            }  

            int visits_num = 0;
            for(int j = 1; j < route_size.at(i)-1; j++){
                if(customers.at(vehicles.at(i).route.at(j)).spotid.substr(0, 1) == "e" || customers.at(vehicles.at(i).route.at(j)).custno == "0"){
                    continue; //充電スポットかデポの場合はスキップ
                }

                visits_num++;
            }

            mu += visits_num;
        }
    }

    mu /= valid_vnum; //平均値算出

    if(compare_time(mu) == 0){
        string sErrMes = "failed to calculate APE margin of time-leveling (aka avetime_ape) due to delivery time = 0";
        throw sErrMes;
    }


    //誤差計算
    
    for(int i = 0; i < vnum; i++){
        double diff;
        if(vehicles.at(i).direct_flag == true || non_depo_num.at(i) == 0){
            continue; //直送便か空車両か改善車両ならスキップ 
        }  

        if(bulkShipping_flag == true){
            int visits_num = 0;
            for(int j = 1; j < route_size.at(i)-1; j++){
                int tieup_num = customers.at(vehicles.at(i).route.at(j)).tieup_customer.size();
                for(int k = 0; k < tieup_num; k++){
                    if(customers.at(customers.at(vehicles.at(i).route.at(j)).tieup_customer.at(k)).spotid.substr(0, 1) == "e" || customers.at(customers.at(vehicles.at(i).route.at(j)).tieup_customer.at(k)).custno == "0"){
                        continue; //充電スポットかデポの場合はスキップ
                    }

                    visits_num++;
                }
            }

            diff = abs(mu-visits_num)/mu;
        }
        else{
            int visits_num = 0;
            for(int j = 1; j < route_size.at(i)-1; j++){
                if(customers.at(vehicles.at(i).route.at(j)).spotid.substr(0, 1) == "e" || customers.at(vehicles.at(i).route.at(j)).custno == "0" || customers.at(vehicles.at(i).route.at(j)).custno == customers.at(vehicles.at(i).route.at(j-1)).custno){
                    continue; //充電スポットかデポの場合または同一CUSTNOが連続する場合はスキップ
                }

                visits_num++;
            }

            diff = abs(mu-visits_num)/mu;
        }

        ape.push_back(max(diff-avevisits_ape, 0.0));
    }

    return ape;
}