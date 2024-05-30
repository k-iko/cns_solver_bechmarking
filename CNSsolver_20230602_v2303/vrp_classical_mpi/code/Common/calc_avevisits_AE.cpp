#include "calc_avevisits_AE.hpp"

/**
 * @brief 配送件数平準化ペナルティ(絶対値)を計算する関数
 * @param[in] vehicles 車両リスト
 * @param[in] customers 配送先リスト
 * @param[in] mvehicles 改善のあった車両の配送件数
 * @param[in] avevisits_ae 許容誤差
 * @param[in] bulkShipping_flag 荷物tie-up用フラグ(true : 荷物をtie-upする) 
 * @param[in] decr_vnum 改善によって削除された車両のインデックス
 * @return double 配送件数平準化ペナルティ
 * @details 各車両の配送件数と全車両の平均のずれの絶対値が許容誤差を超えている分のペナルティを計算する関数。
 */
double calc_avevisits_AE(const vector<Vehicle> &vehicles, const vector<Customer> &customers, const map<int, double> &mvehicles, const double avevisits_ae, const bool bulkShipping_flag, const int decr_veh){
    int valid_vnum = (int) 0; //有効車両台数
    int vnum = (int) vehicles.size(); //車両台数
    vector<int> route_size;
    vector<int> non_depo_num;
    vector<int> visits_veh;

    for(int i = 0; i < vnum; i++){
        non_depo_num.emplace_back(0);
        route_size.emplace_back( vehicles.at(i).route.size());
        for(int j = 1; j < route_size.at(i)-1; j++){
            if((customers.at(vehicles.at(i).route.at(j)).custno != "0")&&(customers.at(vehicles.at(i).route.at(j)).custno.at(0) != 'b')){
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

            visits_veh.emplace_back(visits_num);
        }
    }
    else{
        for(int i = 0; i < vnum; i++){
            if(vehicles.at(i).direct_flag == true || non_depo_num.at(i) == 0 || mvehicles.count(i) == 1){
                continue; //直送便か空車両か改善車両ならスキップ 
            }  

            int visits_num = 0;
            for(int j = 1; j < route_size.at(i)-1; j++){
                if(customers.at(vehicles.at(i).route.at(j)).custno.at(0) == 'e' || customers.at(vehicles.at(i).route.at(j)).custno.at(0) == 'b' || customers.at(vehicles.at(i).route.at(j)).custno == "0" || customers.at(vehicles.at(i).route.at(j)).custno == customers.at(vehicles.at(i).route.at(j-1)).custno){
                    continue; //充電スポットかデポの場合または同一CUSTNOが連続する場合はスキップ
                }

                visits_num++;
            }

            visits_veh.emplace_back(visits_num);
        }
    }

    for(auto visits_num : mvehicles){
        visits_veh.emplace_back(visits_num.second);
    }


    //誤差計算
    double ae = 0;
    int visits_veh_size = (int) visits_veh.size();

    for(int i = 0; i < visits_veh_size; i++){
        for(int j = 0; j < i; j++){
            double diff = abs(visits_veh.at(i)-visits_veh.at(j));

            ae = max(max(diff-avevisits_ae, 0.0),ae);
        }
    }

    return ae;
}

vector<double> calc_avevisits_AE_list(const vector<Vehicle> &vehicles, const vector<Customer> &customers, const double avevisits_ae, const bool bulkShipping_flag){
    int valid_vnum = (int) 0; //有効車両台数
    int vnum = (int) vehicles.size(); //車両台数
    vector<int> route_size;
    vector<int> non_depo_num;
    vector<int> visits_veh;
    vector<double> ae;

    for(int i = 0; i < vnum; i++){
        non_depo_num.emplace_back(0);
        route_size.emplace_back( vehicles.at(i).route.size());
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

            visits_veh.emplace_back(visits_num);
        }
    }
    else{
        for(int i = 0; i < vnum; i++){
            if(vehicles.at(i).direct_flag == true || non_depo_num.at(i) == 0){
                continue; //直送便か空車両か改善車両ならスキップ 
            }  

            int visits_num = 0;
            for(int j = 1; j < route_size.at(i)-1; j++){
                if(customers.at(vehicles.at(i).route.at(j)).spotid.substr(0, 1) == "e" || customers.at(vehicles.at(i).route.at(j)).custno == "0" || customers.at(vehicles.at(i).route.at(j)).custno == customers.at(vehicles.at(i).route.at(j-1)).custno){
                    continue; //充電スポットかデポの場合または同一CUSTNOが連続する場合はスキップ
                }

                visits_num++;
            }

            visits_veh.emplace_back(visits_num);
        }
    }


    //誤差計算
    int visits_veh_size = (int) visits_veh.size();

    for(int i = 0; i < visits_veh_size; i++){
        for(int j = 0; j < i; j++){
            double diff = abs(visits_veh.at(i)-visits_veh.at(j));

            ae.push_back(max(diff-avevisits_ae, 0.0));
        }
    }

    return ae;
}