#include "cross_improved_lastc_dtable.hpp"

namespace{
    bool vector_finder(const vector<int> &vec, const int number);
    bool vector_finder(const vector<string> &vec, const string str);
    vector<vector<int>> combination_with_replacement(const int n, const int m);
}

/**
 * @brief cross処理後の改善有無を判定する関数
 * @param[in] vehicle1 車両オブジェクト
 * @param[in] vehicle2 車両オブジェクト
 * @param[in] customers 配送先リスト
 * @param[in] pos1 1台目の開始インデックス
 * @param[in] pos2 1台目の終了インデックス
 * @param[in] pos3 2台目の開始インデックス
 * @param[in] pos4 2台目の終了インデックス
 * @param[in] vehicles 車両リスト
 * @param[in] argvs グローバル変数格納用
 * @param[out] cdiff 改善量
 * @param[in] no_depo_parkt_flag 簡易アルゴリズム使用フラグ(true : 使用可能)
 * @param[in] one_table_flag 単一テーブル判定フラグ(true : 単一テーブル)
 * @return tuple<bool, vector<int>, vector<int>, bool, bool> 改善有無(true : 改善あり)、交換後の配送先の組み合わせ
 * @details cross処理後の最終配送先までの改善有無を判定する関数。
 */
tuple<bool, vector<int>, vector<int>, bool, bool, vector<int>, vector<int>, map<tuple<string, double>, vector<vector<double>>>, map<tuple<string, double>, vector<vector<double>>>, map<tuple<string, string, double>, vector<vector<double>>>, map<tuple<string, double>, vector<vector<double>>>, vector<vector<int>>, vector<Customer>> cross_improved_lastc_dtable(const Vehicle &vehicle1, const Vehicle &vehicle2, vector<Customer> &customers, const int pos1, const int pos2, const int pos3, const int pos4, vector<Vehicle> &vehicles, Cppargs &argvs, double &cdiff, bool no_depo_parkt_flag, bool one_table_flag){
    vector<int> custs1_best, custs2_best;
    vector<int> route1_best, route2_best;
    vector<Customer> customers_best;
    bool ires = false; //改善判定用
    bool pick_opt_flag1_best = false, pick_opt_flag2_best = false;
    int veh1_route_size = (int) vehicle1.route.size(), veh2_route_size = (int) vehicle2.route.size();
    double cdiff_best = 0;
    map<tuple<string, double>, vector<vector<double>>> ttable_best;
    map<tuple<string, double>, vector<vector<double>>> dtable_best;
    map<tuple<string, string, double>, vector<vector<double>>> etable_best;
    map<tuple<string, double>, vector<vector<double>>> xtable_best;
    map<tuple<string, double>, vector<vector<double>>> t_ttable;
    map<tuple<string, double>, vector<vector<double>>> t_dtable;
    map<tuple<string, string, double>, vector<vector<double>>> t_etable;
    map<tuple<string, double>, vector<vector<double>>> t_xtable;
    vector<vector<int>> t_route_best;
    vector<vector<int>> t_route;
    vector<Customer> t_customers;

    if(argvs.break_flag){
        ttable_best = argvs.management_ttable;
        dtable_best = argvs.management_dtable;
        etable_best = argvs.management_etable;
        xtable_best = argvs.management_xtable;
		t_route_best = r_backup(vehicles);
        customers_best = customers;
    }


    if(pos2 > veh1_route_size-1 || pos4 > veh2_route_size-1
    || pos1 > pos2-2 || pos3 > pos4-2
    || pos1 < 1 || pos3 < 1){
        return make_tuple(false, custs1_best, custs2_best, false, false, route1_best, route2_best, ttable_best, dtable_best, etable_best, xtable_best, t_route_best, customers_best); //条件外の場合
    }

    bool pick_opt_flag1 = false;
    bool pick_opt_flag2 = false;
    int pick_opt_idx1 = 0;
    int pick_opt_idx2 = 0;
    if(argvs.break_flag){
        pick_opt_flag1 = get_pick_opt_status(customers, vehicle1, argvs);
        pick_opt_flag2 = get_pick_opt_status(customers, vehicle2, argvs);
        pick_opt_idx1 = get_pick_opt_idx(customers, vehicle1, argvs);
        pick_opt_idx2 = get_pick_opt_idx(customers, vehicle2, argvs);

        if((pick_opt_flag1 == true && pos2 >= pick_opt_idx1) ||
        (pick_opt_flag2 == true && pos4 >= pick_opt_idx2)){
            return make_tuple(false, custs1_best, custs2_best, false, false, route1_best, route2_best, ttable_best, dtable_best, etable_best, xtable_best, t_route_best, customers_best); //条件外の場合
        }
    }
    else{
        if(argvs.pickup_flag == true){
            if(argvs.bulkShipping_flag == true){
                if(customers.at(customers.at(vehicle1.route.at(veh1_route_size-2)).tieup_customer.at(0)).custno == "0" &&
                customers.at(customers.at(vehicle1.route.at(veh1_route_size-1)).tieup_customer.at(0)).custno != "0"){
                    for(int i = veh1_route_size-3; i >= 1; i--){
                        if(customers.at(customers.at(vehicle1.route.at(i)).tieup_customer.at(0)).custno == "0" &&
                        customers.at(customers.at(vehicle1.route.at(i+1)).tieup_customer.at(0)).custno != "0"){
                            break;
                        }
                        if(customers.at(customers.at(vehicle1.route.at(i)).tieup_customer.at(0)).custno != "0" &&
                        customers.at(vehicle1.route.at(i)).requestType == "PICKUP"){
                            pick_opt_flag1 = true;
                            break;
                        }
                    }
                }
            }
            else{
                if(customers.at(vehicle1.route.at(veh1_route_size-2)).custno == "0" && 
                customers.at(vehicle1.route.at(veh1_route_size-1)).custno != "0"){
                    for(int i = veh1_route_size-3; i >=1; i--){
                        if(customers.at(vehicle1.route.at(i)).custno == "0" &&
                        customers.at(vehicle1.route.at(i+1)).custno != "0"){
                            break;
                        }
                        if(customers.at(vehicle1.route.at(i)).custno != "0" &&
                        customers.at(vehicle1.route.at(i)).requestType == "PICKUP"){
                            pick_opt_flag1 = true;
                            break;
                        }
                    }
                }
            }
        }

        if(argvs.pickup_flag == true){
            if(argvs.bulkShipping_flag == true){
                if(customers.at(customers.at(vehicle2.route.at(veh2_route_size-2)).tieup_customer.at(0)).custno == "0" &&
                customers.at(customers.at(vehicle2.route.at(veh2_route_size-1)).tieup_customer.at(0)).custno != "0"){
                    for(int i = veh2_route_size-3; i >= 1; i--){
                        if(customers.at(customers.at(vehicle2.route.at(i)).tieup_customer.at(0)).custno == "0" &&
                        customers.at(customers.at(vehicle2.route.at(i+1)).tieup_customer.at(0)).custno != "0"){
                            break;
                        }
                        if(customers.at(customers.at(vehicle2.route.at(i)).tieup_customer.at(0)).custno != "0" &&
                        customers.at(vehicle2.route.at(i)).requestType == "PICKUP"){
                            pick_opt_flag2 = true;
                            break;
                        }
                    }
                }
            }
            else{
                if(customers.at(vehicle2.route.at(veh2_route_size-2)).custno == "0" && 
                customers.at(vehicle2.route.at(veh2_route_size-1)).custno != "0"){
                    for(int i = veh2_route_size-3; i >=1; i--){
                        if(customers.at(vehicle2.route.at(i)).custno == "0" &&
                        customers.at(vehicle2.route.at(i+1)).custno != "0"){
                            break;
                        }
                        if(customers.at(vehicle2.route.at(i)).custno != "0" &&
                        customers.at(vehicle2.route.at(i)).requestType == "PICKUP"){
                            pick_opt_flag2 = true;
                            break;
                        }
                    }
                }
            }
        }

        if((pick_opt_flag1 == true && pos2 == veh1_route_size-1) ||
        (pick_opt_flag2 == true && pos4 == veh2_route_size-1)){
            return make_tuple(false, custs1_best, custs2_best, false, false, route1_best, route2_best, ttable_best, dtable_best, etable_best, xtable_best, t_route_best, customers_best);
        }
    }

    //集荷最適化(簡易アルゴリズム判定)
    bool pick_nondepo_flag = false;
    if(customers.at(vehicle1.route.at(veh1_route_size-1)).custno != "0" || customers.at(vehicle2.route.at(veh2_route_size-1)).custno != "0"){
        for(int i = 1; i < veh1_route_size-1; i++){
            if(customers.at(vehicle1.route.at(i)).custno != "0" && customers.at(vehicle1.route.at(i)).requestType == "PICKUP"){
                pick_nondepo_flag = true;
                break;
            }
        }
        for(int i = 1; i < veh2_route_size-1; i++){
            if(customers.at(vehicle2.route.at(i)).custno != "0" && customers.at(vehicle2.route.at(i)).requestType == "PICKUP"){
                pick_nondepo_flag = true;
                break;
            }
        }
    }

    //範囲内の配送先をデポとそれ以外で分ける
    vector<int> cust_list1, depo_list1, br_list1;
    for(int i = pos1; i < pos2; i++){
        if(customers.at(vehicle1.route.at(i)).custno == "0"){
            depo_list1.emplace_back(vehicle1.route.at(i)); //デポ
        }
        else if(customers.at(vehicle1.route.at(i)).custno.at(0) != 'b'){   // 休憩スポットを除外
            cust_list1.emplace_back(vehicle1.route.at(i)); //デポ以外
        }
        else{
            br_list1.emplace_back(vehicle1.route.at(i)); //デポ以外
        }
    }

    vector<int> cust_list2, depo_list2, br_list2;
    for(int i = pos3; i < pos4; i++){
        if(customers.at(vehicle2.route.at(i)).custno == "0"){
            depo_list2.emplace_back(vehicle2.route.at(i)); //デポ
        }
        else if(customers.at(vehicle2.route.at(i)).custno.at(0) != 'b'){   // 休憩スポットを除外
            cust_list2.emplace_back(vehicle2.route.at(i)); //デポ以外
        }
        else{
            br_list2.emplace_back(vehicle2.route.at(i)); //デポ以外
        }
    }


    if(cust_list1.empty() || cust_list2.empty()){
        return make_tuple(false, custs1_best, custs2_best, false, false, route1_best, route2_best, ttable_best, dtable_best, etable_best, xtable_best, t_route_best, customers_best); //条件外の場合
    }
    

    //変更後にvehnoがreje_veに含まれていたらfalseを返す
    for(int i = pos3; i < pos4; i++){
        if(vector_finder(customers.at(vehicle2.route.at(i)).reje_ve, vehicle1.vehno) == true){
            return make_tuple(false, custs1_best, custs2_best, false, false, route1_best, route2_best, ttable_best, dtable_best, etable_best, xtable_best, t_route_best, customers_best); //条件外の場合
        }
    }

    for(int i = pos1; i < pos2; i++){
        if(vector_finder(customers.at(vehicle1.route.at(i)).reje_ve, vehicle2.vehno) == true){
            return make_tuple(false, custs1_best, custs2_best, false, false, route1_best, route2_best, ttable_best, dtable_best, etable_best, xtable_best, t_route_best, customers_best); //条件外の場合
        }
    }

    //条件2が含まれるならスキップ
    if(argvs.init_changecust == "additive"){
        for(auto i : cust_list1){
            if(free_or_additive(customers.at(i), vehicle1, vehicles, customers, argvs) == "additive"){
                return make_tuple(false, custs1_best, custs2_best, false, false, route1_best, route2_best, ttable_best, dtable_best, etable_best, xtable_best, t_route_best, customers_best); //条件外の場合
            }
        }
        for(auto i : cust_list2){
            if(free_or_additive(customers.at(i), vehicle2, vehicles, customers, argvs) == "additive"){
                return make_tuple(false, custs1_best, custs2_best, false, false, route1_best, route2_best, ttable_best, dtable_best, etable_best, xtable_best, t_route_best, customers_best); //条件外の場合
            }
        }
    }

    //デポ挿入後の組み合わせ作成
    vector<vector<int>> insert_list1;
    vector<vector<int>> insert_set1;
    int depo_list1_size = (int) depo_list1.size();
    int cust_list2_size = (int) cust_list2.size();
    vector<vector<int>> comb_list1 = combination_with_replacement(depo_list1_size, cust_list2_size+1);

    if(comb_list1.empty()){
        insert_list1.emplace_back(cust_list2);
    }

    for(auto idx : comb_list1){
        vector<int> cust_list_temp = cust_list2;

        int idx_size = (int) idx.size();
        for(int i = 0; i < idx_size; i++){
            cust_list_temp.insert(cust_list_temp.begin()+i+idx.at(i), depo_list1.at(i));
        }

        vector<int> cust_set = cust_list_temp;
        cust_set.erase(unique(cust_set.begin(), cust_set.end()), cust_set.end());
        if(find(insert_set1.begin(), insert_set1.end(), cust_set) == insert_set1.end()){
            insert_list1.emplace_back(cust_list_temp);
            insert_set1.emplace_back(cust_set);
        }
    }

    vector<vector<int>> insert_list2;
    vector<vector<int>> insert_set2;
    int depo_list2_size = (int) depo_list2.size();
    int cust_list1_size = (int) cust_list1.size();
    vector<vector<int>> comb_list2 = combination_with_replacement(depo_list2_size, cust_list1_size+1);

    if(comb_list2.empty()){
        insert_list2.emplace_back(cust_list1);
    }

    for(auto idx : comb_list2){
        vector<int> cust_list_temp = cust_list1;

        int idx_size = (int) idx.size();
        for(int i = 0; i < idx_size; i++){
            cust_list_temp.insert(cust_list_temp.begin()+i+idx.at(i), depo_list2.at(i));
        }

        vector<int> cust_set = cust_list_temp;
        cust_set.erase(unique(cust_set.begin(), cust_set.end()), cust_set.end());
        if(find(insert_set2.begin(), insert_set2.end(), cust_set) == insert_set2.end()){
            insert_list2.emplace_back(cust_list_temp);
            insert_set2.emplace_back(cust_set);
        }
    }


    // 開始インデックスを計算
    vector<int> depo_idx1;
    for(int i = 0; i < veh1_route_size-1; i++){
        if(customers.at(vehicle1.route.at(i)).custno == "0"){
            depo_idx1.emplace_back(i); //routeにおけるデポのインデックス
        }
    }
    depo_idx1.emplace_back(veh1_route_size-1); //最終訪問先を追加

    int depo_idx1_size = depo_idx1.size();
    int start_idx1 = 0;
    // for(int i = 0; i < depo_idx1_size-1; i++){
    //     if(depo_idx1.at(i) < pos1 && pos1 <= depo_idx1.at(i+1)){
    //         start_idx1 = depo_idx1.at(i);
    //         break;
    //     }
    // }
    if(argvs.multitrip_flag == false && no_depo_parkt_flag == true && argvs.break_flag == false){
        start_idx1 = pos1-1;
    }

    vector<int> depo_idx2;
    for(int i = 0; i < veh2_route_size-1; i++){
        if(customers.at(vehicle2.route.at(i)).custno == "0"){
            depo_idx2.emplace_back(i); //routeにおけるデポのインデックス
        }
    }
    depo_idx2.emplace_back(veh2_route_size-1); //最終訪問先を追加

    int depo_idx2_size = depo_idx2.size();
    int start_idx2 = 0;
    // for(int i = 0; i < depo_idx2_size-1; i++){
    //     if(depo_idx2.at(i) < pos3 && pos3 <= depo_idx2.at(i+1)){
    //         start_idx2 = depo_idx2.at(i);
    //         break;
    //     }
    // }
    if(argvs.multitrip_flag == false && no_depo_parkt_flag == true && argvs.break_flag == false){
        start_idx2 = pos3-1;
    }


    //終了インデックスを計算
    int blast_idx1 = veh1_route_size-1;
    for(int i = veh1_route_size-2; i >= 0; i--){
        if((customers.at(vehicle1.route.at(i)).custno != "0")&&(customers.at(vehicle1.route.at(i)).custno[0] != 'b')&&(customers.at(vehicle1.route.at(i)).custno[0] != 'e')){
            break;
        }
        else{
            blast_idx1 = i;
        }
    }

    int blast_idx2 = veh2_route_size-1;
    for(int i = veh2_route_size-2; i >= 0; i--){
        if((customers.at(vehicle2.route.at(i)).custno != "0")&&(customers.at(vehicle2.route.at(i)).custno[0] != 'b')&&(customers.at(vehicle2.route.at(i)).custno[0] != 'e')){
            break;
        }
        else{
            blast_idx2 = i;
        }
    }


    double bdist = 0,
           btpen = 0,
           bpower = 0,
           bexp = 0,
           btime = 0,
           bload = 0,
           bload2 = 0,
           btime_error = 0,
           bvisits_error = 0,
           bdist_error = 0,
           bunass = 0;


    //改善前時間違反量を計算
    for(int i = start_idx1; i < blast_idx1; i++){
        if(i>0){
            if(customers.at(vehicle1.route.at(i-1)).custno != customers.at(vehicle1.route.at(i)).custno){
                btpen += max(0.0, vehicle1.st.at(i)-customers.at(vehicle1.route.at(i)).due_fs);
            }
        }
        else{
            btpen += max(0.0, vehicle1.st.at(i)-customers.at(vehicle1.route.at(i)).due_fs);
        }
    }
    btpen += max(0.0, vehicle1.lv.at(blast_idx1-1)-vehicle1.endhour);

    for(int i = start_idx2; i < blast_idx2; i++){
        if(i>0){
            if(customers.at(vehicle2.route.at(i-1)).custno != customers.at(vehicle2.route.at(i)).custno){
                btpen += max(0.0, vehicle2.st.at(i)-customers.at(vehicle2.route.at(i)).due_fs);
            }
        }
        else{
            btpen += max(0.0, vehicle2.st.at(i)-customers.at(vehicle2.route.at(i)).due_fs);
        }
    }
    btpen += max(0.0, vehicle2.lv.at(blast_idx2-1)-vehicle2.endhour);


    //改善前の作業終了時刻
    double bv1final_t = vehicle1.lv.at(blast_idx1-1), bv2final_t = vehicle2.lv.at(blast_idx2-1);


    //改善前のコスト計算
    double bdist1, bdist2;
    if(one_table_flag == false || argvs.multitrip_flag == true || pick_nondepo_flag == true || argvs.avedist_ae_flag == true || argvs.avedist_ape_flag == true || argvs.break_flag == true){
        bdist1 = calc_dist(vehicle1, customers, start_idx1+1, blast_idx1, argvs.management_dtable);
        bdist2 = calc_dist(vehicle2, customers, start_idx2+1, blast_idx2, argvs.management_dtable);
        bdist = bdist1+bdist2;
        
        if(!argvs.management_etable.empty()){
            bpower = calc_power(vehicle1, customers, start_idx1+1, blast_idx1, argvs.management_etable)
                    +calc_power(vehicle2, customers, start_idx2+1, blast_idx2, argvs.management_etable);
        }

        if(!argvs.management_xtable.empty()){
            bexp = calc_exp(vehicle1, customers, start_idx1+1, blast_idx1, argvs.management_xtable)
                +calc_exp(vehicle2, customers, start_idx2+1, blast_idx2, argvs.management_xtable);
        }
    }
    else{
        bdist1 = get_dtable(vehicle1.vtype, vehicle1.lv.at(pos1-1), argvs.management_dtable, customers.at(vehicle1.route.at(pos1-1)).spotno, customers.at(vehicle1.route.at(pos1)).spotno);
        bdist2 = get_dtable(vehicle2.vtype, vehicle2.lv.at(pos3-1), argvs.management_dtable, customers.at(vehicle2.route.at(pos3-1)).spotno, customers.at(vehicle2.route.at(pos3)).spotno);
        
        if(pos2 < blast_idx1){
            bdist1 += get_dtable(vehicle1.vtype, vehicle1.lv.at(pos2-1), argvs.management_dtable, customers.at(vehicle1.route.at(pos2-1)).spotno, customers.at(vehicle1.route.at(pos2)).spotno);
        }
        if(pos4 < blast_idx2){
            bdist2 += get_dtable(vehicle2.vtype, vehicle2.lv.at(pos4-1), argvs.management_dtable, customers.at(vehicle2.route.at(pos4-1)).spotno, customers.at(vehicle2.route.at(pos4)).spotno);
        }
        bdist = bdist1+bdist2;

        if(!argvs.management_etable.empty()){
            bpower = get_etable(vehicle1.vtype, vehicle1.etype, vehicle1.lv.at(pos1-1), argvs.management_etable, customers.at(vehicle1.route.at(pos1-1)).spotno, customers.at(vehicle1.route.at(pos1)).spotno)
                +get_etable(vehicle2.vtype, vehicle2.etype, vehicle2.lv.at(pos3-1), argvs.management_etable, customers.at(vehicle2.route.at(pos3-1)).spotno, customers.at(vehicle2.route.at(pos3)).spotno);
            if(pos2 < blast_idx1){
                bpower += get_etable(vehicle1.vtype, vehicle1.etype, vehicle1.lv.at(pos2-1), argvs.management_etable, customers.at(vehicle1.route.at(pos2-1)).spotno, customers.at(vehicle1.route.at(pos2)).spotno);
            }
            if(pos4 < blast_idx2){
                bpower += get_etable(vehicle2.vtype, vehicle2.etype, vehicle2.lv.at(pos4-1), argvs.management_etable, customers.at(vehicle2.route.at(pos4-1)).spotno, customers.at(vehicle2.route.at(pos4)).spotno);
            }
        }
        if(!argvs.management_xtable.empty()){
            bexp = get_xtable(vehicle1.vtype, vehicle1.lv.at(pos1-1), argvs.management_xtable, customers.at(vehicle1.route.at(pos1-1)).spotno, customers.at(vehicle1.route.at(pos1)).spotno)
                +get_xtable(vehicle2.vtype, vehicle2.lv.at(pos3-1), argvs.management_xtable, customers.at(vehicle2.route.at(pos3-1)).spotno, customers.at(vehicle2.route.at(pos3)).spotno);
            if(pos2 < blast_idx1){
                bexp += get_xtable(vehicle1.vtype, vehicle1.lv.at(pos2-1), argvs.management_xtable, customers.at(vehicle1.route.at(pos2-1)).spotno, customers.at(vehicle1.route.at(pos2)).spotno);
            }
            if(pos4 < blast_idx2){
                bexp += get_xtable(vehicle2.vtype, vehicle2.lv.at(pos4-1), argvs.management_xtable, customers.at(vehicle2.route.at(pos4-1)).spotno, customers.at(vehicle2.route.at(pos4)).spotno);
            }
        }
    }

    if(argvs.opt_t_weight != 0){
        btime = vehicle1.lv.at(blast_idx1-1)+vehicle2.lv.at(blast_idx2-1);
    }
    

    //改善前積載量違反量を計算
    bload = vehicle1.load_pen+vehicle2.load_pen;
    bload2 = vehicle1.load_pen2+vehicle2.load_pen2;


    //改善前平準化ペナルティを計算
    if(argvs.avetime_ape_flag == true || argvs.avetime_ae_flag == true){
        btime_error = argvs.time_error;
    }

    if(argvs.avevisits_ape_flag == true || argvs.avevisits_ae_flag == true){
        bvisits_error = argvs.visits_error;
    }

    if(argvs.avedist_ape_flag == true || argvs.avedist_ae_flag == true){
        bdist_error = argvs.dist_error;
    }

	//改善前未割り当て荷物最適化ペナルティの算出(重み付けはここではしない)
	if(argvs.opt_unassigned_flag==true){
		if(vehicle1.reserved_v_flag==true){
            for(int rc=0; rc<=blast_idx1; rc++){
                if(customers.at(vehicle1.route.at(rc)).custno != "0" && customers.at(vehicle1.route.at(rc)).custno.at(0) != 'b' && customers.at(vehicle1.route.at(rc)).custno.at(0) != 'e'){
                    bunass++;
                }
            }
        }
		if(vehicle2.reserved_v_flag==true){
            for(int rc=0; rc<=blast_idx2; rc++){
                if(customers.at(vehicle2.route.at(rc)).custno != "0" && customers.at(vehicle2.route.at(rc)).custno.at(0) != 'b' && customers.at(vehicle2.route.at(rc)).custno.at(0) != 'e'){
                    bunass++;
                }
            }
        }
    }

    int br_list1_size = 0;
    int br_list2_size = 0;

	if(argvs.break_flag){
		t_ttable = argvs.management_ttable;
		t_dtable = argvs.management_dtable;
		t_etable = argvs.management_etable;
		t_xtable = argvs.management_xtable;
		t_route = r_backup(vehicles);
        t_customers = customers;
        br_list1_size = br_list1.size();
        br_list2_size = br_list2.size();
    }


    //改善後ペナルティ計算
    for(auto custs1 : insert_list1){
        for(auto custs2 : insert_list2){
            int custs1_size = custs1.size(), custs2_size = custs2.size();

            double adist = 0,
                   atpen = 0,
                   apower = 0,
                   aexp = 0,
                   atime = 0,
                   aload = 0,
                   aload2 = 0,
                   atime_error = 0,
                   avisits_error = 0,
                   adist_error = 0,
                   aunass = 0;

            double btpen_temp = btpen, atpen_temp = 0;

            //ルート入れ替え
            vector<int> skv1_route, skv2_route;
            
            for (int i = 0; i < pos1; i++) {
                skv1_route.emplace_back(vehicle1.route.at(i));
            }
            for (int i = 0; i < custs1_size; i++) {
                skv1_route.emplace_back(custs1.at(i));
            }
            if(argvs.break_flag){
                for (int i = 0; i < br_list1_size; i++) {
                    skv1_route.emplace_back(br_list1.at(i));
                }
            }
            for (int i = pos2; i < veh1_route_size; i++) {
                skv1_route.emplace_back(vehicle1.route.at(i));
            }

            for (int i = 0; i < pos3; i++) {
                skv2_route.emplace_back(vehicle2.route.at(i));
            }
            for (int i = 0; i < custs2_size; i++) {
                skv2_route.emplace_back(custs2.at(i));
            }
            if(argvs.break_flag){
                for (int i = 0; i < br_list2_size; i++) {
                    skv2_route.emplace_back(br_list2.at(i));
                }
            }
            for (int i = pos4; i < veh2_route_size; i++) {
                skv2_route.emplace_back(vehicle2.route.at(i));
            }

            //変更後に訪問数上限を超えていたらcontinue
            int skv1_route_size = skv1_route.size();
            int visitnum1 = 0;
            if(argvs.break_flag){
                visitnum1 = calc_visitnum(skv1_route, customers);
            }
            else{
                for(int i =1; i < skv1_route_size-1; i++){
                    if(customers.at(skv1_route.at(i)).custno == "0" || customers.at(skv1_route.at(i)).spotid == customers.at(skv1_route.at(i-1)).spotid){
                        continue; //デポと連続する同一spotidの場合はスキップ
                    }

                    visitnum1++;
                }
            }

            if(vehicle1.maxvisit > 0 && visitnum1 > vehicle1.maxvisit){
                continue;
            }

            int skv2_route_size = skv2_route.size();
            int visitnum2 = 0;
            if(argvs.break_flag){
                visitnum2 = calc_visitnum(skv2_route, customers);
            }
            else{
                for(int i =1; i < skv2_route_size-1; i++){
                    if(customers.at(skv2_route.at(i)).custno == "0" || customers.at(skv2_route.at(i)).spotid == customers.at(skv2_route.at(i-1)).spotid){
                        continue; //デポと連続する同一spotidの場合はスキップ
                    }

                    visitnum2++;
                }
            }

            if(vehicle2.maxvisit > 0 && visitnum2 > vehicle2.maxvisit){
                continue;
            }

            //条件2が他の便に移るならスキップ
            if(argvs.multitrip_flag == true && argvs.init_changecust == "additive"){
                for(int i = 0; i < skv1_route_size-1; i++){
                    if(in_additive(customers.at(skv1_route.at(i)), vehicle1, skv1_route, customers, vehicles, argvs) == false){
    					continue;
                    }
                }

                for(int i = 0; i < skv2_route_size-1; i++){
                    if(in_additive(customers.at(skv2_route.at(i)), vehicle2, skv2_route, customers, vehicles, argvs) == false){
    					continue;
                    }
                }
            }

            //作業時間を計算
            vector<int> depo_idx1_temp;
            for(int i = 0; i < skv1_route_size-1; i++){
                if(customers.at(skv1_route.at(i)).custno == "0"){
                    depo_idx1_temp.emplace_back(i); //routeにおけるデポのインデックス
                }
            }
            depo_idx1_temp.emplace_back(skv1_route_size-1); //最終訪問先を追加

            vector<int> depo_idx1_temp2;
            for(int i = 0; i < veh1_route_size-1; i++){
                if(customers.at(vehicle1.route.at(i)).custno == "0"){
                    depo_idx1_temp2.emplace_back(i); //routeにおけるデポのインデックス
                }
            }
            depo_idx1_temp2.emplace_back(veh1_route_size-1); //最終訪問先を追加

            map<int, double> servt_dict1;
            int depo_idx1_temp_size = (int) depo_idx1_temp.size();
            //配送時間
            for(int i = 0; i < depo_idx1_temp_size-1; i++){
                servt_dict1.emplace(depo_idx1_temp2.at(i), 0);
                double servt = 0;

                for(int j = depo_idx1_temp.at(i)+1; j < depo_idx1_temp.at(i+1); j++){
                    if(customers.at(skv1_route.at(j)).requestType == "DELIVERY" && customers.at(skv1_route.at(j)).custno != customers.at(skv1_route.at(j-1)).custno
                      && customers.at(skv1_route.at(j)).custno.at(0) != 'b'){
                        servt += customers.at(skv1_route.at(j)).depotservt;
                    }
                }

                if(customers.at(vehicle1.route.at(depo_idx1_temp2.at(i)+1)).custno != "0"){
                    servt_dict1.at(depo_idx1_temp2.at(i)) += servt+vehicle1.depotservt; //デポが連続していない場合
                }
                else{
                    servt_dict1.at(depo_idx1_temp2.at(i)) += servt; //デポが連続している場合
                }
            }

            //集荷時間
            servt_dict1.emplace(distance(vehicle1.route.begin(), find(vehicle1.route.begin(), vehicle1.route.end(), skv1_route.at(skv1_route_size-1))), 0);
            for(int i = 1; i < depo_idx1_temp_size; i++){
                double servt = 0;

                for(int j = depo_idx1_temp.at(i-1)+1; j < depo_idx1_temp.at(i); j++){
                    if(customers.at(skv1_route.at(j)).requestType == "PICKUP" && customers.at(skv1_route.at(j)).custno != customers.at(skv1_route.at(j-1)).custno){
                        servt += customers.at(skv1_route.at(j)).depotservt;
                    }
                }

                servt_dict1.at(depo_idx1_temp2.at(i)) += servt;
            }

            vector<int> depo_idx2_temp;
            for(int i = 0; i < skv2_route_size-1; i++){
                if(customers.at(skv2_route.at(i)).custno == "0"){
                    depo_idx2_temp.emplace_back(i); //routeにおけるデポのインデックス
                }
            }
            depo_idx2_temp.emplace_back(skv2_route_size-1); //最終訪問先を追加

            vector<int> depo_idx2_temp2;
            for(int i = 0; i < veh2_route_size-1; i++){
                if(customers.at(vehicle2.route.at(i)).custno == "0"){
                    depo_idx2_temp2.emplace_back(i); //routeにおけるデポのインデックス
                }
            }
            depo_idx2_temp2.emplace_back(veh2_route_size-1); //最終訪問先を追加

            map<int, double> servt_dict2;
            int depo_idx2_temp_size = (int) depo_idx2_temp.size();
            //配送時間
            for(int i = 0; i < depo_idx2_temp_size-1; i++){
                servt_dict2.emplace(depo_idx2_temp2.at(i), 0);
                double servt = 0;

                for(int j = depo_idx2_temp.at(i)+1; j < depo_idx2_temp.at(i+1); j++){
                    if(customers.at(skv2_route.at(j)).requestType == "DELIVERY" && customers.at(skv2_route.at(j)).custno != customers.at(skv2_route.at(j-1)).custno
                      && customers.at(skv2_route.at(j)).custno.at(0) != 'b'){
                        servt += customers.at(skv2_route.at(j)).depotservt;
                    }
                }

                if(customers.at(vehicle2.route.at(depo_idx2_temp2.at(i)+1)).custno != "0"){
                    servt_dict2.at(depo_idx2_temp2.at(i)) += servt+vehicle2.depotservt; //デポが連続していない場合
                }
                else{
                    servt_dict2.at(depo_idx2_temp2.at(i)) += servt; //デポが連続している場合
                }
            }

            //集荷時間
            servt_dict2.emplace(distance(vehicle2.route.begin(), find(vehicle2.route.begin(), vehicle2.route.end(), skv2_route.at(skv2_route_size-1))), 0);
            for(int i = 1; i < depo_idx2_temp_size; i++){
                double servt = 0;

                for(int j = depo_idx2_temp.at(i-1)+1; j < depo_idx2_temp.at(i); j++){
                    if(customers.at(skv2_route.at(j)).requestType == "PICKUP" && customers.at(skv2_route.at(j)).custno != customers.at(skv2_route.at(j-1)).custno){
                        servt += customers.at(skv2_route.at(j)).depotservt;
                    }
                }

                servt_dict2.at(depo_idx2_temp2.at(i)) += servt;
            }


            //終了インデックスを計算
            int alast_idx1 = skv1_route_size-1;
            for(int i = skv1_route_size-2; i >= 0; i--){
                if((customers.at(skv1_route.at(i)).custno != "0")&&(customers.at(skv1_route.at(i)).custno[0] != 'b')&&(customers.at(skv1_route.at(i)).custno[0] != 'e')){
                    break;
                }
                else{
                    alast_idx1 = i;
                }
            }

            int alast_idx2 = skv2_route_size-1;
            for(int i = skv2_route_size-2; i >= 0; i--){
                if((customers.at(skv2_route.at(i)).custno != "0")&&(customers.at(skv2_route.at(i)).custno[0] != 'b')&&(customers.at(skv2_route.at(i)).custno[0] != 'e')){
                    break;
                }
                else{
                    alast_idx2 = i;
                }
            }

            vector<double> load11(skv1_route_size, 0), load12(skv1_route_size, 0);

            for(int i=0; i < skv1_route_size-1; i++){
                if(vector_finder(depo_idx1_temp, i) == true){
                    for(int j=i+1; j < depo_idx1_temp.at(distance(depo_idx1_temp.begin(), find(depo_idx1_temp.begin(), depo_idx1_temp.end(), i))+1); j++){
                        if(customers.at(skv1_route.at(j)).requestType == "DELIVERY"){
                            if(customers.at(skv1_route.at(j)).custno.at(0) != 'b'){  // 休憩スポットは積載量は無視する
                                load11.at(i) += customers.at(skv1_route.at(j)).dem;
                        
                                load12.at(i) += customers.at(skv1_route.at(j)).dem2;
                            }
                        }
                    }
                }
                else{
                    if(customers.at(skv1_route.at(i)).custno.at(0) != 'b'){  // 休憩スポットは積載量は無視する
                        if(customers.at(skv1_route.at(i)).requestType == "DELIVERY"){
                            load11.at(i) = load11.at(i-1)-customers.at(skv1_route.at(i)).dem;
                            load12.at(i) = load12.at(i-1)-customers.at(skv1_route.at(i)).dem2;
                        }
                        else{
                            load11.at(i) = load11.at(i-1)+customers.at(skv1_route.at(i)).dem;
                            load12.at(i) = load12.at(i-1)+customers.at(skv1_route.at(i)).dem2;
                        }
                    }
                }
            }

            for(int i = skv1_route_size-2; i >= 0; i--){
                if(vector_finder(depo_idx1_temp, i) == true && vector_finder(depo_idx1_temp, i+1) == true){
                    load11.at(i) = load11.at(i+1);
                    load12.at(i) = load12.at(i+1);
                }
            }

            //誤差対策
            for(int i = 0; i < skv1_route_size; i++){
                if(load11.at(i) < 1.0e-9){
                    load11.at(i) = 0.0;
                }
            }
            for(int i = 0; i < skv1_route_size; i++){
                if(load12.at(i) < 1.0e-9){
                    load12.at(i) = 0.0;
                }
            }

            vector<int> no_park_idx1;
            bool no_park_flag1 = true;

            for(int i = depo_idx1_temp.at(0)+1; i < depo_idx1_temp.at(1); i++){
                if(customers.at(skv1_route.at(i)).requestType == "DELIVERY"){
                    no_park_flag1 = false;
                    break;
                }
            }
            if(no_park_flag1 == true && load11.at(0) == 0 && load12.at(0) == 0){
                no_park_idx1.emplace_back(0);
            }

            for(int i = 1; i < depo_idx1_temp_size-1; i++){
                if(load11.at(depo_idx1_temp.at(i)-1) == 0 && load12.at(depo_idx1_temp.at(i)-1) == 0){
                    no_park_flag1 = true;

                    for(int j = depo_idx1_temp.at(i)+1; j < depo_idx1_temp.at(i+1); j++){
                        if(customers.at(skv1_route.at(j)).requestType == "DELIVERY"){
                            no_park_flag1 = false;
                            break;
                        }
                    }
                    if(no_park_flag1 == true && load11.at(depo_idx1_temp.at(i)) == 0 && load12.at(depo_idx1_temp.at(i)) == 0){
                        no_park_idx1.emplace_back(depo_idx1_temp2.at(i));
                    }
                }
            }

            if(load11.at(depo_idx1_temp.at(depo_idx1_temp_size-1)-1) == 0 && load12.at(depo_idx1_temp.at(depo_idx1_temp_size-1)-1) == 0){
                no_park_idx1.emplace_back(depo_idx1_temp2.at(depo_idx1_temp_size-1));
            }

            pick_opt_flag1 = false;
            bool non_depo = false;
            if(argvs.pickup_flag == true){
                for(int i = skv1_route_size-2; i >= 1; i--){
                    if(customers.at(skv1_route.at(i)).custno != "0"){
                        if(customers.at(skv1_route.at(i)).custno.at(0) != 'b'){
                            non_depo = true;
                            if(customers.at(skv1_route.at(i)).requestType == "PICKUP"){
                                pick_opt_flag1 = true;
                                break;
                            }
                        }
                    }
                    else if(non_depo == true){
                        break;
                    }
                }
            }

            vector<double> load21(skv2_route_size, 0), load22(skv2_route_size, 0);

            for(int i=0; i < skv2_route_size-1; i++){
                if(vector_finder(depo_idx2_temp, i) == true){
                    for(int j=i+1; j < depo_idx2_temp.at(distance(depo_idx2_temp.begin(), find(depo_idx2_temp.begin(), depo_idx2_temp.end(), i))+1); j++){
                        if(customers.at(skv2_route.at(j)).requestType == "DELIVERY"){
                            if(customers.at(skv2_route.at(j)).custno.at(0) != 'b'){  // 休憩スポットは積載量は無視する
                                load21.at(i) += customers.at(skv2_route.at(j)).dem;
                        
                                load22.at(i) += customers.at(skv2_route.at(j)).dem2;
                            }
                        }
                    }
                }
                else{
                    if(customers.at(skv2_route.at(i)).custno.at(0) != 'b'){  // 休憩スポットは積載量は無視する
                        if(customers.at(skv2_route.at(i)).requestType == "DELIVERY"){
                            load21.at(i) = load21.at(i-1)-customers.at(skv2_route.at(i)).dem;
                            load22.at(i) = load22.at(i-1)-customers.at(skv2_route.at(i)).dem2;
                        }
                        else{
                            load21.at(i) = load21.at(i-1)+customers.at(skv2_route.at(i)).dem;
                            load22.at(i) = load22.at(i-1)+customers.at(skv2_route.at(i)).dem2;
                        }
                    }
                }
            }

            for(int i = skv2_route_size-2; i >= 0; i--){
                if(vector_finder(depo_idx2_temp, i) == true && vector_finder(depo_idx2_temp, i+1) == true){
                    load21.at(i) = load21.at(i+1);
                    load22.at(i) = load22.at(i+1);
                }
            }

            //誤差対策
            for(int i = 0; i < skv2_route_size; i++){
                if(load21.at(i) < 1.0e-9){
                    load21.at(i) = 0.0;
                }
            }
            for(int i = 0; i < skv2_route_size; i++){
                if(load22.at(i) < 1.0e-9){
                    load22.at(i) = 0.0;
                }
            }

            vector<int> no_park_idx2;
            bool no_park_flag2 = true;

            for(int i = depo_idx2_temp.at(0)+1; i < depo_idx2_temp.at(1); i++){
                if(customers.at(skv2_route.at(i)).requestType == "DELIVERY"){
                    no_park_flag2 = false;
                    break;
                }
            }
            if(no_park_flag2 == true && load21.at(0) == 0 && load22.at(0) == 0){
                no_park_idx2.emplace_back(0);
            }

            for(int i = 1; i < depo_idx2_temp_size-1; i++){
                if(load21.at(depo_idx2_temp.at(i)-1) == 0 && load22.at(depo_idx2_temp.at(i)-1) == 0){
                    no_park_flag2 = true;

                    for(int j = depo_idx2_temp.at(i)+1; j < depo_idx2_temp.at(i+1); j++){
                        if(customers.at(skv2_route.at(j)).requestType == "DELIVERY"){
                            no_park_flag2 = false;
                            break;
                        }
                    }
                    if(no_park_flag2 == true && load21.at(depo_idx2_temp.at(i)) == 0 && load22.at(depo_idx2_temp.at(i)) == 0){
                        no_park_idx2.emplace_back(depo_idx2_temp2.at(i));
                    }
                }
            }

            if(load21.at(depo_idx2_temp.at(depo_idx2_temp_size-1)-1) == 0 && load22.at(depo_idx2_temp.at(depo_idx2_temp_size-1)-1) == 0){
                no_park_idx2.emplace_back(depo_idx2_temp2.at(depo_idx2_temp_size-1));
            }

            pick_opt_flag2 = false;
            non_depo = false;
            if(argvs.pickup_flag == true){
                for(int i = skv2_route_size-2; i >= 1; i--){
                    if(customers.at(skv2_route.at(i)).custno != "0"){
                        if(customers.at(skv2_route.at(i)).custno.at(0) != 'b'){
                            non_depo = true;
                            if(customers.at(skv2_route.at(i)).requestType == "PICKUP"){
                                pick_opt_flag2 = true;
                                break;
                            }
                        }
                    }
                    else if(non_depo == true){
                        break;
                    }
                }
            }

            double lv1, lv2;
            double st1, st2;
            double adist1, adist2;
            if(argvs.break_flag == false){
                //#### ここから休憩なし用の処理 ####
                //ペナルティ計算
                double lv_t, arr_t, st_t, lv, arr, st;
                bool passable = true;

                //vehicle1
                if(servt_dict1.count(start_idx1) == 1){
                    if((start_idx1 == 0 || customers.at(vehicle1.route.at(start_idx1)).spotid != customers.at(vehicle1.route.at(start_idx1-1)).spotid) && vector_finder(no_park_idx1, start_idx1) == false){
                        lv_t = max(vehicle1.arr.at(start_idx1)+customers.at(vehicle1.route.at(start_idx1)).parkt_arrive, customers.at(vehicle1.route.at(start_idx1)).ready_fs)+servt_dict1.at(start_idx1)*vehicle1.opskill;
                        st_t = max(vehicle1.arr.at(start_idx1)+customers.at(vehicle1.route.at(start_idx1)).parkt_arrive, customers.at(vehicle1.route.at(start_idx1)).ready_fs)+customers.at(vehicle1.route.at(start_idx1)).parkt_open;
                    }
                    else{
                        lv_t = max(vehicle1.arr.at(start_idx1), customers.at(vehicle1.route.at(start_idx1)).ready_fs)+servt_dict1.at(start_idx1)*vehicle1.opskill;
                        st_t = max(vehicle1.arr.at(start_idx1), customers.at(vehicle1.route.at(start_idx1)).ready_fs);
                    }
                }
                else{
                    if((start_idx1 == 0 || customers.at(vehicle1.route.at(start_idx1)).spotid != customers.at(vehicle1.route.at(start_idx1-1)).spotid) && vector_finder(no_park_idx1, start_idx1) == false){
                        lv_t = max(vehicle1.arr.at(start_idx1)+customers.at(vehicle1.route.at(start_idx1)).parkt_arrive, customers.at(vehicle1.route.at(start_idx1)).ready_fs)+customers.at(vehicle1.route.at(start_idx1)).servt*vehicle1.opskill;
                        st_t = max(vehicle1.arr.at(start_idx1)+customers.at(vehicle1.route.at(start_idx1)).parkt_arrive, customers.at(vehicle1.route.at(start_idx1)).ready_fs)+customers.at(vehicle1.route.at(start_idx1)).parkt_open;
                    }
                    else{
                        lv_t = max(vehicle1.arr.at(start_idx1), customers.at(vehicle1.route.at(start_idx1)).ready_fs)+customers.at(vehicle1.route.at(start_idx1)).servt*vehicle1.opskill;
                        st_t = max(vehicle1.arr.at(start_idx1), customers.at(vehicle1.route.at(start_idx1)).ready_fs);
                    }
                }
                if((start_idx1 == 0 || customers.at(vehicle1.route.at(start_idx1)).spotid != customers.at(vehicle1.route.at(start_idx1-1)).spotid) && vector_finder(no_park_idx1, start_idx1) == false){
                    lv_t += customers.at(vehicle1.route.at(start_idx1)).parkt_open; //駐車時間
                }

                if(start_idx1==0 || customers.at(vehicle1.route.at(start_idx1-1)).custno != "0" || customers.at(vehicle1.route.at(start_idx1)).custno != "0"){ //デポが連続しない場合
                    atpen += max(0.0, st_t-customers.at(vehicle1.route.at(start_idx1)).due_fs);
                }

                if(start_idx1 != 0 && customers.at(vehicle1.route.at(start_idx1)).custno == customers.at(vehicle1.route.at(start_idx1-1)).custno && customers.at(vehicle1.route.at(start_idx1)).custno != "0"){
                    lv_t = vehicle1.lv.at(start_idx1-1);
                }

                for(int i = start_idx1; i < pos1-1; i++){
                    Customer from_c = customers.at(vehicle1.route.at(i)), to_c = customers.at(vehicle1.route.at(i+1));

                    if(to_c.custno == from_c.custno && to_c.custno != "0"){
                        continue;
                    }

                    if(get_dtable(vehicle1.vtype, lv_t, argvs.management_dtable, from_c.spotno, to_c.spotno) == -1
                    || get_ttable(vehicle1.vtype, lv_t, argvs.management_ttable, from_c.spotno, to_c.spotno) == -1
                    || (!argvs.management_etable.empty() && get_etable(vehicle1.vtype, vehicle1.etype, lv_t, argvs.management_etable, from_c.spotno, to_c.spotno) == -1)
                    || (!argvs.management_xtable.empty() && get_xtable(vehicle1.vtype, lv_t, argvs.management_xtable, from_c.spotno, to_c.spotno) == -1)){
                        passable = false; //通行不可
                        break;
                    }

                    adist += get_dtable(vehicle1.vtype, lv_t, argvs.management_dtable, from_c.spotno, to_c.spotno);
                    if(!argvs.management_etable.empty()){
                        apower += get_etable(vehicle1.vtype, vehicle1.etype, lv_t, argvs.management_etable, from_c.spotno, to_c.spotno);
                    }
                    if(!argvs.management_xtable.empty()){
                        aexp += get_xtable(vehicle1.vtype, lv_t, argvs.management_xtable, from_c.spotno, to_c.spotno);
                    }

                    arr_t = lv_t+get_ttable(vehicle1.vtype, lv_t, argvs.management_ttable, from_c.spotno, to_c.spotno)*vehicle1.drskill;
                    if(servt_dict1.count(i+1) == 1){
                        if(to_c.spotid != from_c.spotid && vector_finder(no_park_idx1, i+1) == false){
                            lv_t = max(arr_t+to_c.parkt_arrive, to_c.ready_fs)+servt_dict1.at(i+1)*vehicle1.opskill;
                            st_t = max(arr_t+to_c.parkt_arrive, to_c.ready_fs)+to_c.parkt_open;
                        }
                        else{
                            lv_t = max(arr_t, to_c.ready_fs)+servt_dict1.at(i+1)*vehicle1.opskill;
                            st_t = max(arr_t, to_c.ready_fs);
                        }
                    }
                    else{
                        if(to_c.spotid != from_c.spotid && vector_finder(no_park_idx1, i+1) == false){
                            lv_t = max(arr_t+to_c.parkt_arrive, to_c.ready_fs)+to_c.servt*vehicle1.opskill;
                            st_t = max(arr_t+to_c.parkt_arrive, to_c.ready_fs)+to_c.parkt_open;
                        }
                        else{
                            lv_t = max(arr_t, to_c.ready_fs)+to_c.servt*vehicle1.opskill;
                            st_t = max(arr_t, to_c.ready_fs);
                        }
                    }
                    if(to_c.spotid != from_c.spotid && vector_finder(no_park_idx1, i+1) == false){
                        lv_t += to_c.parkt_open; //駐車時間
                    }

                    if(from_c.custno != "0" || to_c.custno != "0"){ //デポが連続しない場合
                        atpen += max(0.0, st_t-to_c.due_fs);
                    }
                }

                if(!passable){
                    continue; //通行不可
                }

                int depo_begin_idx1 = pos1;
                if(customers.at(vehicle1.route.at(pos1-1)).custno != customers.at(custs1.at(0)).custno || customers.at(custs1.at(0)).custno == "0"){
                    if(get_dtable(vehicle1.vtype, lv_t, argvs.management_dtable, customers.at(vehicle1.route.at(pos1-1)).spotno, customers.at(custs1.at(0)).spotno) == -1
                    || get_ttable(vehicle1.vtype, lv_t, argvs.management_ttable, customers.at(vehicle1.route.at(pos1-1)).spotno, customers.at(custs1.at(0)).spotno) == -1
                    || (!argvs.management_etable.empty() && get_etable(vehicle1.vtype, vehicle1.etype, lv_t, argvs.management_etable, customers.at(vehicle1.route.at(pos1-1)).spotno, customers.at(custs1.at(0)).spotno) == -1)
                    || (!argvs.management_xtable.empty() && get_xtable(vehicle1.vtype, lv_t, argvs.management_xtable, customers.at(vehicle1.route.at(pos1-1)).spotno, customers.at(custs1.at(0)).spotno) == -1)){
                        continue; //通行不可
                    }

                    adist += get_dtable(vehicle1.vtype, lv_t, argvs.management_dtable, customers.at(vehicle1.route.at(pos1-1)).spotno, customers.at(custs1.at(0)).spotno);
                    if(!argvs.management_etable.empty()){
                        apower += get_etable(vehicle1.vtype, vehicle1.etype, lv_t, argvs.management_etable, customers.at(vehicle1.route.at(pos1-1)).spotno, customers.at(custs1.at(0)).spotno);
                    }
                    if(!argvs.management_xtable.empty()){
                        aexp += get_xtable(vehicle1.vtype, lv_t, argvs.management_xtable, customers.at(vehicle1.route.at(pos1-1)).spotno, customers.at(custs1.at(0)).spotno);
                    }

                    arr_t = lv_t+get_ttable(vehicle1.vtype, lv_t, argvs.management_ttable, customers.at(vehicle1.route.at(pos1-1)).spotno, customers.at(custs1.at(0)).spotno)*vehicle1.drskill;
                    if(customers.at(custs1.at(0)).spotid != customers.at(vehicle1.route.at(pos1-1)).spotid && (customers.at(custs1.at(0)).custno != "0" || vector_finder(no_park_idx1, distance(vehicle1.route.begin(), find(vehicle1.route.begin()+depo_begin_idx1, vehicle1.route.end(), custs1.at(0)))) == false)){
                        lv_t = max(arr_t+customers.at(custs1.at(0)).parkt_arrive, customers.at(custs1.at(0)).ready_fs);
                        st_t = max(arr_t+customers.at(custs1.at(0)).parkt_arrive, customers.at(custs1.at(0)).ready_fs)+customers.at(custs1.at(0)).parkt_open;
                    }
                    else{
                        lv_t = max(arr_t, customers.at(custs1.at(0)).ready_fs);
                        st_t = max(arr_t, customers.at(custs1.at(0)).ready_fs);
                    }
                    if(customers.at(custs1.at(0)).spotid != customers.at(vehicle1.route.at(pos1-1)).spotid && (customers.at(custs1.at(0)).custno != "0" || vector_finder(no_park_idx1, distance(vehicle1.route.begin(), find(vehicle1.route.begin()+depo_begin_idx1, vehicle1.route.end(), custs1.at(0)))) == false)){
                        lv_t += customers.at(custs1.at(0)).parkt_open; //駐車時間
                    }
                    if (servt_dict1.count(distance(vehicle1.route.begin(), find(vehicle1.route.begin()+depo_begin_idx1, vehicle1.route.end(), custs1.at(0)))) == 1){
                        lv_t += servt_dict1.at(distance(vehicle1.route.begin(), find(vehicle1.route.begin()+depo_begin_idx1, vehicle1.route.end(), custs1.at(0))))*vehicle1.opskill;
                        depo_begin_idx1 = distance(vehicle1.route.begin(), find(vehicle1.route.begin()+depo_begin_idx1, vehicle1.route.end(), custs1.at(0)))+1;
                    }
                    else{
                        lv_t += customers.at(custs1.at(0)).servt*vehicle1.opskill;
                    }

                    if(customers.at(vehicle1.route.at(pos1-1)).custno != "0" || customers.at(custs1.at(0)).custno != "0"){ //デポが連続しない場合
                        atpen += max(0.0, st_t-customers.at(custs1.at(0)).due_fs);
                    }
                }
                
                for(int i = 0; i < custs1_size-1; i++){
                    if(custs1.at(i) == skv1_route.at(alast_idx1-1)){
                        break; //最終訪問先へのルートは考慮しない
                    }

                    Customer from_c = customers.at(custs1.at(i)), to_c = customers.at(custs1.at(i+1));

                    if(to_c.custno == from_c.custno && to_c.custno != "0"){
                        continue;
                    }

                    if(one_table_flag == false || argvs.multitrip_flag == true || pick_nondepo_flag == true || argvs.avedist_ae_flag == true || argvs.avedist_ape_flag == true){
                        if(get_dtable(vehicle1.vtype, lv_t, argvs.management_dtable, from_c.spotno, to_c.spotno) == -1
                        || get_ttable(vehicle1.vtype, lv_t, argvs.management_ttable, from_c.spotno, to_c.spotno) == -1
                        || (!argvs.management_etable.empty() && get_etable(vehicle1.vtype, vehicle1.etype, lv_t, argvs.management_etable, from_c.spotno, to_c.spotno) == -1)
                        || (!argvs.management_xtable.empty() && get_xtable(vehicle1.vtype, lv_t, argvs.management_xtable, from_c.spotno, to_c.spotno) == -1)){
                            passable = false; //通行不可
                            break;
                        }

                        adist += get_dtable(vehicle1.vtype, lv_t, argvs.management_dtable, from_c.spotno, to_c.spotno);
                        if(!argvs.management_etable.empty()){
                            apower += get_etable(vehicle1.vtype, vehicle1.etype, lv_t, argvs.management_etable, from_c.spotno, to_c.spotno);
                        }
                        if(!argvs.management_xtable.empty()){
                            aexp += get_xtable(vehicle1.vtype, lv_t, argvs.management_xtable, from_c.spotno, to_c.spotno);
                        }
                    }

                    arr_t = lv_t+get_ttable(vehicle1.vtype, lv_t, argvs.management_ttable, from_c.spotno, to_c.spotno)*vehicle1.drskill;
                    if(to_c.spotid != from_c.spotid && (to_c.custno != "0" || vector_finder(no_park_idx1, distance(vehicle1.route.begin(), find(vehicle1.route.begin()+depo_begin_idx1, vehicle1.route.end(), custs1.at(i+1)))) == false)){
                        lv_t = max(arr_t+to_c.parkt_arrive, to_c.ready_fs);
                        st_t = max(arr_t+to_c.parkt_arrive, to_c.ready_fs)+to_c.parkt_open;
                    }
                    else{
                        lv_t = max(arr_t, to_c.ready_fs);
                        st_t = max(arr_t, to_c.ready_fs);
                    }
                    if(to_c.spotid != from_c.spotid && (to_c.custno != "0" || vector_finder(no_park_idx1, distance(vehicle1.route.begin(), find(vehicle1.route.begin()+depo_begin_idx1, vehicle1.route.end(), custs1.at(i+1)))) == false)){
                        lv_t += to_c.parkt_open; //駐車時間
                    }
                    if(servt_dict1.count(distance(vehicle1.route.begin(), find(vehicle1.route.begin()+depo_begin_idx1, vehicle1.route.end(), custs1.at(i+1)))) == 1){
                        lv_t += servt_dict1.at(distance(vehicle1.route.begin(), find(vehicle1.route.begin()+depo_begin_idx1, vehicle1.route.end(), custs1.at(i+1))))*vehicle1.opskill;
                        depo_begin_idx1 = distance(vehicle1.route.begin(), find(vehicle1.route.begin()+depo_begin_idx1, vehicle1.route.end(), custs1.at(i+1)))+1;
                    }
                    else{
                        lv_t += to_c.servt*vehicle1.opskill;
                    }

                    if(from_c.custno != "0" || to_c.custno != "0"){ //デポが連続しない場合
                        atpen += max(0.0, st_t-to_c.due_fs);
                    }
                }

                if(!passable){
                    continue; //通行不可
                }
                if((argvs.bulkShipping_flag == false && pos2 == veh1_route_size-1 && customers.at(custs1.at(custs1_size-1)).custno != "0" && customers.at(vehicle1.route.at(veh1_route_size-1)).custno != "0" && pick_opt_flag1 == true) ||
                (argvs.bulkShipping_flag == true && pos2 == veh1_route_size-1 && customers.at(customers.at(custs1.at(custs1_size-1)).tieup_customer.at(0)).custno != "0" && customers.at(customers.at(vehicle1.route.at(veh1_route_size-1)).tieup_customer.at(0)).custno != "0" && pick_opt_flag1 == true)){ //集荷最適化
                    if(get_dtable(vehicle1.vtype, lv_t, argvs.management_dtable, customers.at(custs1.at(custs1_size-1)).spotno, customers.at(0).spotno) == -1
                    || get_ttable(vehicle1.vtype, lv_t, argvs.management_ttable, customers.at(custs1.at(custs1_size-1)).spotno, customers.at(0).spotno) == -1
                    || (!argvs.management_etable.empty() && get_etable(vehicle1.vtype, vehicle1.etype, lv_t, argvs.management_etable, customers.at(custs1.at(custs1_size-1)).spotno, customers.at(0).spotno) == -1)
                    || (!argvs.management_xtable.empty() && get_xtable(vehicle1.vtype, lv_t, argvs.management_xtable, customers.at(custs1.at(custs1_size-1)).spotno, customers.at(0).spotno) == -1)){
                        continue; //通行不可
                    }

                    arr = lv_t+get_ttable(vehicle1.vtype, lv_t, argvs.management_ttable, customers.at(custs1.at(custs1_size-1)).spotno, customers.at(0).spotno)*vehicle1.drskill;
                    lv = max(arr+customers.at(0).parkt_arrive, customers.at(0).ready_fs)+(servt_dict1.at(veh1_route_size-1)+vehicle1.depotservt)*vehicle1.opskill;
                    lv += customers.at(0).parkt_open;

                    if(get_dtable(vehicle1.vtype, lv, argvs.management_dtable, customers.at(0).spotno, customers.at(vehicle1.route.at(veh1_route_size-1)).spotno) == -1
                    || get_ttable(vehicle1.vtype, lv, argvs.management_ttable, customers.at(0).spotno, customers.at(vehicle1.route.at(veh1_route_size-1)).spotno) == -1
                    || (!argvs.management_etable.empty() && get_etable(vehicle1.vtype, vehicle1.etype, lv, argvs.management_etable, customers.at(0).spotno, customers.at(vehicle1.route.at(veh1_route_size-1)).spotno) == -1)
                    || (!argvs.management_xtable.empty() && get_xtable(vehicle1.vtype, lv, argvs.management_xtable, customers.at(0).spotno, customers.at(vehicle1.route.at(veh1_route_size-1)).spotno) == -1)){
                        continue; //通行不可
                    }
                }
                else if((argvs.bulkShipping_flag == false && pos2 == veh1_route_size-2 && customers.at(vehicle1.route.at(veh1_route_size-2)).custno == "0" && customers.at(vehicle1.route.at(veh1_route_size-1)).custno != "0" && pick_opt_flag1 == false) ||
                (argvs.bulkShipping_flag == true && pos2 == veh1_route_size-2 && customers.at(customers.at(vehicle1.route.at(veh1_route_size-2)).tieup_customer.at(0)).custno == "0" && customers.at(customers.at(vehicle1.route.at(veh1_route_size-1)).tieup_customer.at(0)).custno != "0" && pick_opt_flag1 == false)){ //集荷最適化
                    if(get_dtable(vehicle1.vtype, lv_t, argvs.management_dtable, customers.at(custs1.at(custs1_size-1)).spotno, customers.at(vehicle1.route.at(veh1_route_size-1)).spotno) == -1
                    || get_ttable(vehicle1.vtype, lv_t, argvs.management_ttable, customers.at(custs1.at(custs1_size-1)).spotno, customers.at(vehicle1.route.at(veh1_route_size-1)).spotno) == -1
                    || (!argvs.management_etable.empty() && get_etable(vehicle1.vtype, vehicle1.etype, lv_t, argvs.management_etable, customers.at(custs1.at(custs1_size-1)).spotno, customers.at(vehicle1.route.at(veh1_route_size-1)).spotno) == -1)
                    || (!argvs.management_xtable.empty() && get_xtable(vehicle1.vtype, lv_t, argvs.management_xtable, customers.at(custs1.at(custs1_size-1)).spotno, customers.at(vehicle1.route.at(veh1_route_size-1)).spotno) == -1)){
                        continue; //通行不可
                    }
                }
                else{
                    if(customers.at(custs1.at(custs1_size-1)).custno != customers.at(vehicle1.route.at(pos2)).custno || customers.at(vehicle1.route.at(pos2)).custno == "0"){
                        if(get_dtable(vehicle1.vtype, lv_t, argvs.management_dtable, customers.at(custs1.at(custs1_size-1)).spotno, customers.at(vehicle1.route.at(pos2)).spotno) == -1
                        || get_ttable(vehicle1.vtype, lv_t, argvs.management_ttable, customers.at(custs1.at(custs1_size-1)).spotno, customers.at(vehicle1.route.at(pos2)).spotno) == -1
                        || (!argvs.management_etable.empty() && get_etable(vehicle1.vtype, vehicle1.etype, lv_t, argvs.management_etable, customers.at(custs1.at(custs1_size-1)).spotno, customers.at(vehicle1.route.at(pos2)).spotno) == -1)
                        || (!argvs.management_xtable.empty() && get_xtable(vehicle1.vtype, lv_t, argvs.management_xtable, customers.at(custs1.at(custs1_size-1)).spotno, customers.at(vehicle1.route.at(pos2)).spotno) == -1)){
                            continue; //通行不可
                        }

                        if(pos2 < blast_idx1){
                            adist += get_dtable(vehicle1.vtype, lv_t, argvs.management_dtable, customers.at(custs1.at(custs1_size-1)).spotno, customers.at(vehicle1.route.at(pos2)).spotno);
                            if(!argvs.management_etable.empty()){
                                apower += get_etable(vehicle1.vtype, vehicle1.etype, lv_t, argvs.management_etable, customers.at(custs1.at(custs1_size-1)).spotno, customers.at(vehicle1.route.at(pos2)).spotno);
                            }
                            if(!argvs.management_xtable.empty()){
                                aexp += get_xtable(vehicle1.vtype, lv_t, argvs.management_xtable, customers.at(custs1.at(custs1_size-1)).spotno, customers.at(vehicle1.route.at(pos2)).spotno);
                            }
                        }

                        arr = lv_t+get_ttable(vehicle1.vtype, lv_t, argvs.management_ttable, customers.at(custs1.at(custs1_size-1)).spotno, customers.at(vehicle1.route.at(pos2)).spotno)*vehicle1.drskill;
                        if (servt_dict1.count(pos2) == 1){
                            if(customers.at(vehicle1.route.at(pos2)).spotid != customers.at(custs1.at(custs1_size-1)).spotid && vector_finder(no_park_idx1, pos2) == false){
                                lv = max(arr+customers.at(vehicle1.route.at(pos2)).parkt_arrive, customers.at(vehicle1.route.at(pos2)).ready_fs)+servt_dict1.at(pos2)*vehicle1.opskill;
                                st = max(arr+customers.at(vehicle1.route.at(pos2)).parkt_arrive, customers.at(vehicle1.route.at(pos2)).ready_fs)+customers.at(vehicle1.route.at(pos2)).parkt_open;
                            }
                            else{
                                lv = max(arr, customers.at(vehicle1.route.at(pos2)).ready_fs)+servt_dict1.at(pos2)*vehicle1.opskill;
                                st = max(arr, customers.at(vehicle1.route.at(pos2)).ready_fs);
                            }
                        }
                        else{
                            if(customers.at(vehicle1.route.at(pos2)).spotid != customers.at(custs1.at(custs1_size-1)).spotid && vector_finder(no_park_idx1, pos2) == false){
                                lv = max(arr+customers.at(vehicle1.route.at(pos2)).parkt_arrive, customers.at(vehicle1.route.at(pos2)).ready_fs)+customers.at(vehicle1.route.at(pos2)).servt*vehicle1.opskill;
                                st = max(arr+customers.at(vehicle1.route.at(pos2)).parkt_arrive, customers.at(vehicle1.route.at(pos2)).ready_fs)+customers.at(vehicle1.route.at(pos2)).parkt_open;
                            }
                            else{
                                lv = max(arr, customers.at(vehicle1.route.at(pos2)).ready_fs)+customers.at(vehicle1.route.at(pos2)).servt*vehicle1.opskill;
                                st = max(arr, customers.at(vehicle1.route.at(pos2)).ready_fs);
                            }
                        }
                        if(customers.at(vehicle1.route.at(pos2)).spotid != customers.at(custs1.at(custs1_size-1)).spotid && vector_finder(no_park_idx1, pos2) == false){
                            lv += customers.at(vehicle1.route.at(pos2)).parkt_open; //駐車時間
                        }
                    }
                    else{
                        arr = arr_t;
                        lv = lv_t;
                        st = st_t;
                    }

            
                    if(one_table_flag == false || argvs.multitrip_flag == true || pick_nondepo_flag == true || argvs.avedist_ae_flag == true || argvs.avedist_ape_flag == true){
                        passable = check_impassability(vehicle1, customers, lv, pos2+1, blast_idx1, servt_dict1, adist, apower, aexp, no_park_idx1, pick_opt_flag1, argvs);

                        if(!passable){
                            continue; //通行不可
                        }
                    }
                }
                adist1 = adist;

                lv1 = lv_t;
                st1 = st_t;

                if(pos2 >= blast_idx1){ //pos2が最終訪問先の場合
                    atpen += max(0.0, lv1-vehicle1.endhour);

                    if(compare_time(argvs.lowertime) > 0 || compare_time(argvs.uppertime) > 0){
                        return_lutime_penalty(lv1-vehicle1.arr.at(0), bv1final_t-vehicle1.arr.at(0), argvs.lowertime, argvs.uppertime, atpen_temp, btpen_temp);
                    }
                }
                else{
                    if(customers.at(custs1.at(custs1_size-1)).custno !=customers.at(vehicle1.route.at(pos2)).custno){
                        atpen += max(0.0, st-customers.at(vehicle1.route.at(pos2)).due_fs);
                    }

                    lv_t = lv;
                    st_t = st;

                    for(int i = pos2; i < blast_idx1-1; i++){
                        Customer from_c = customers.at(vehicle1.route.at(i)), to_c = customers.at(vehicle1.route.at(i+1));

                        if(to_c.custno == from_c.custno && to_c.custno != "0"){
                            continue;
                        }

                        //作業時間
                        double proc_time;
                        if(servt_dict1.count(i+1) == 1){
                            proc_time = servt_dict1.at(i+1);
                        }
                        else{
                            proc_time = customers.at(vehicle1.route.at(i+1)).servt;
                        }

                        bool no_park;
                        if(vector_finder(no_park_idx1, i+1) == true){
                            no_park = true;
                        }
                        else{
                            no_park = false;
                        }

                        double ttime_pen = calc_time_pen(vehicle1, from_c, to_c, lv_t, proc_time, argvs.management_ttable, no_park);

                        if(from_c.custno != "0" || to_c.custno != "0"){ //デポが連続しない場合
                            atpen += ttime_pen;
                        }
                    }

                    lv1 = lv_t;
                    st1 = st_t;

                    atpen += max(0.0, lv1-vehicle1.endhour);

                    if(compare_time(argvs.lowertime) > 0 || compare_time(argvs.uppertime) > 0){
                        return_lutime_penalty(lv1-vehicle1.arr.at(0), bv1final_t-vehicle1.arr.at(0), argvs.lowertime, argvs.uppertime, atpen_temp, btpen_temp);
                    }
                }
                
                //vehicle2
                if(servt_dict2.count(start_idx2) == 1){
                    if((start_idx2 == 0 || customers.at(vehicle2.route.at(start_idx2)).spotid != customers.at(vehicle2.route.at(start_idx2-1)).spotid) && vector_finder(no_park_idx2, start_idx2) == false){
                        lv_t = max(vehicle2.arr.at(start_idx2)+customers.at(vehicle2.route.at(start_idx2)).parkt_arrive, customers.at(vehicle2.route.at(start_idx2)).ready_fs)+servt_dict2.at(start_idx2)*vehicle2.opskill;
                        st_t = max(vehicle2.arr.at(start_idx2)+customers.at(vehicle2.route.at(start_idx2)).parkt_arrive, customers.at(vehicle2.route.at(start_idx2)).ready_fs)+customers.at(vehicle2.route.at(start_idx2)).parkt_open;
                    }
                    else{
                        lv_t = max(vehicle2.arr.at(start_idx2), customers.at(vehicle2.route.at(start_idx2)).ready_fs)+servt_dict2.at(start_idx2)*vehicle2.opskill;
                        st_t = max(vehicle2.arr.at(start_idx2), customers.at(vehicle2.route.at(start_idx2)).ready_fs);
                    }
                }
                else{
                    if((start_idx2 == 0 || customers.at(vehicle2.route.at(start_idx2)).spotid != customers.at(vehicle2.route.at(start_idx2-1)).spotid) && vector_finder(no_park_idx2, start_idx2) == false){
                        lv_t = max(vehicle2.arr.at(start_idx2)+customers.at(vehicle2.route.at(start_idx2)).parkt_arrive, customers.at(vehicle2.route.at(start_idx2)).ready_fs)+customers.at(vehicle2.route.at(start_idx2)).servt*vehicle2.opskill;
                        st_t = max(vehicle2.arr.at(start_idx2)+customers.at(vehicle2.route.at(start_idx2)).parkt_arrive, customers.at(vehicle2.route.at(start_idx2)).ready_fs)+customers.at(vehicle2.route.at(start_idx2)).parkt_open;
                    }
                    else{
                        lv_t = max(vehicle2.arr.at(start_idx2), customers.at(vehicle2.route.at(start_idx2)).ready_fs)+customers.at(vehicle2.route.at(start_idx2)).servt*vehicle2.opskill;
                        st_t = max(vehicle2.arr.at(start_idx2), customers.at(vehicle2.route.at(start_idx2)).ready_fs);
                    }
                }
                if((start_idx2 == 0 || customers.at(vehicle2.route.at(start_idx2)).spotid != customers.at(vehicle2.route.at(start_idx2-1)).spotid) && vector_finder(no_park_idx2, start_idx2) == false){
                    lv_t += customers.at(vehicle2.route.at(start_idx2)).parkt_open; //駐車時間
                }
                if(start_idx2==0 || customers.at(vehicle2.route.at(start_idx2-1)).custno != "0" || customers.at(vehicle2.route.at(start_idx2)).custno != "0"){ //デポが連続しない場合
                    atpen += max(0.0, st_t-customers.at(vehicle2.route.at(start_idx2)).due_fs);
                }

                if(start_idx2 != 0 && customers.at(vehicle2.route.at(start_idx2)).custno == customers.at(vehicle2.route.at(start_idx2-1)).custno && customers.at(vehicle2.route.at(start_idx2)).custno != "0"){
                    lv_t = vehicle2.lv.at(start_idx2-1);
                }

                for(int i = start_idx2; i < pos3-1; i++){
                    Customer from_c = customers.at(vehicle2.route.at(i)), to_c = customers.at(vehicle2.route.at(i+1));

                    if(to_c.custno == from_c.custno && to_c.custno != "0"){
                        continue;
                    }

                    if(get_dtable(vehicle2.vtype, lv_t, argvs.management_dtable, from_c.spotno, to_c.spotno) == -1
                    || get_ttable(vehicle2.vtype, lv_t, argvs.management_ttable, from_c.spotno, to_c.spotno) == -1
                    || (!argvs.management_etable.empty() && get_etable(vehicle2.vtype, vehicle2.etype, lv_t, argvs.management_etable, from_c.spotno, to_c.spotno) == -1)
                    || (!argvs.management_xtable.empty() && get_xtable(vehicle2.vtype, lv_t, argvs.management_xtable, from_c.spotno, to_c.spotno) == -1)){
                        passable = false; //通行不可
                        break;
                    }

                    adist += get_dtable(vehicle2.vtype, lv_t, argvs.management_dtable, from_c.spotno, to_c.spotno);
                    if(!argvs.management_etable.empty()){
                        apower += get_etable(vehicle2.vtype, vehicle2.etype, lv_t, argvs.management_etable, from_c.spotno, to_c.spotno);
                    }
                    if(!argvs.management_xtable.empty()){
                        aexp += get_xtable(vehicle2.vtype, lv_t, argvs.management_xtable, from_c.spotno, to_c.spotno);
                    }

                    arr_t = lv_t+get_ttable(vehicle2.vtype, lv_t, argvs.management_ttable, from_c.spotno, to_c.spotno)*vehicle2.drskill;
                    if(servt_dict2.count(i+1) == 1){
                        if(to_c.spotid != from_c.spotid && vector_finder(no_park_idx2, i+1) == false){
                            lv_t = max(arr_t+to_c.parkt_arrive, to_c.ready_fs)+servt_dict2.at(i+1)*vehicle2.opskill;
                            st_t = max(arr_t+to_c.parkt_arrive, to_c.ready_fs)+to_c.parkt_open;
                        }
                        else{
                            lv_t = max(arr_t, to_c.ready_fs)+servt_dict2.at(i+1)*vehicle2.opskill;
                            st_t = max(arr_t, to_c.ready_fs);
                        }
                    }
                    else{
                        if(to_c.spotid != from_c.spotid && vector_finder(no_park_idx2, i+1) == false){
                            lv_t = max(arr_t+to_c.parkt_arrive, to_c.ready_fs)+to_c.servt*vehicle2.opskill;
                            st_t = max(arr_t+to_c.parkt_arrive, to_c.ready_fs)+to_c.parkt_open;
                        }
                        else{
                            lv_t = max(arr_t, to_c.ready_fs)+to_c.servt*vehicle2.opskill;
                            st_t = max(arr_t, to_c.ready_fs);
                        }
                    }
                    if(to_c.spotid != from_c.spotid && vector_finder(no_park_idx2, i+1) == false){
                        lv_t += to_c.parkt_open; //駐車時間
                    }

                    if(from_c.custno != "0" || to_c.custno != "0"){ //デポが連続しない場合
                        atpen += max(0.0, st_t-to_c.due_fs);
                    }
                }

                if(!passable){
                    continue; //通行不可
                }

                int depo_begin_idx2 = pos3;
                if(customers.at(vehicle2.route.at(pos3-1)).custno != customers.at(custs2.at(0)).custno || customers.at(custs2.at(0)).custno == "0"){
                    if(get_dtable(vehicle2.vtype, lv_t, argvs.management_dtable, customers.at(vehicle2.route.at(pos3-1)).spotno, customers.at(custs2.at(0)).spotno) == -1
                    || get_ttable(vehicle2.vtype, lv_t, argvs.management_ttable, customers.at(vehicle2.route.at(pos3-1)).spotno, customers.at(custs2.at(0)).spotno) == -1
                    || (!argvs.management_etable.empty() && get_etable(vehicle2.vtype, vehicle2.etype, lv_t, argvs.management_etable, customers.at(vehicle2.route.at(pos3-1)).spotno, customers.at(custs2.at(0)).spotno) == -1)
                    || (!argvs.management_xtable.empty() && get_xtable(vehicle2.vtype, lv_t, argvs.management_xtable, customers.at(vehicle2.route.at(pos3-1)).spotno, customers.at(custs2.at(0)).spotno) == -1)){
                        continue; //通行不可
                    }

                    adist += get_dtable(vehicle2.vtype, lv_t, argvs.management_dtable, customers.at(vehicle2.route.at(pos3-1)).spotno, customers.at(custs2.at(0)).spotno);
                    if(!argvs.management_etable.empty()){
                        apower += get_etable(vehicle2.vtype, vehicle2.etype, lv_t, argvs.management_etable, customers.at(vehicle2.route.at(pos3-1)).spotno, customers.at(custs2.at(0)).spotno);
                    }
                    if(!argvs.management_xtable.empty()){
                        aexp += get_xtable(vehicle2.vtype, lv_t, argvs.management_xtable, customers.at(vehicle2.route.at(pos3-1)).spotno, customers.at(custs2.at(0)).spotno);
                    }

                    arr_t = lv_t+get_ttable(vehicle2.vtype, lv_t, argvs.management_ttable, customers.at(vehicle2.route.at(pos3-1)).spotno, customers.at(custs2.at(0)).spotno)*vehicle2.drskill;
                    if(customers.at(custs2.at(0)).spotid != customers.at(vehicle2.route.at(pos3-1)).spotid && (customers.at(custs2.at(0)).custno != "0" || vector_finder(no_park_idx2, distance(vehicle2.route.begin(), find(vehicle2.route.begin()+depo_begin_idx2, vehicle2.route.end(), custs2.at(0)))) == false)){
                        lv_t = max(arr_t+customers.at(custs2.at(0)).parkt_arrive, customers.at(custs2.at(0)).ready_fs);
                        st_t = max(arr_t+customers.at(custs2.at(0)).parkt_arrive, customers.at(custs2.at(0)).ready_fs);
                    }
                    else{
                        lv_t = max(arr_t, customers.at(custs2.at(0)).ready_fs);
                        st_t = max(arr_t, customers.at(custs2.at(0)).ready_fs);
                    }
                    if(customers.at(custs2.at(0)).spotid != customers.at(vehicle2.route.at(pos3-1)).spotid && (customers.at(custs2.at(0)).custno != "0" || vector_finder(no_park_idx2, distance(vehicle2.route.begin(), find(vehicle2.route.begin()+depo_begin_idx2, vehicle2.route.end(), custs2.at(0)))) == false)){
                        lv_t += customers.at(custs2.at(0)).parkt_open; //駐車時間
                        st_t += customers.at(custs2.at(0)).parkt_open; //駐車時間
                    }
                    if (servt_dict2.count(distance(vehicle2.route.begin(), find(vehicle2.route.begin()+depo_begin_idx2, vehicle2.route.end(), custs2.at(0)))) == 1){
                        lv_t += servt_dict2.at(distance(vehicle2.route.begin(), find(vehicle2.route.begin()+depo_begin_idx2, vehicle2.route.end(), custs2.at(0))))*vehicle2.opskill;
                        depo_begin_idx2 = distance(vehicle2.route.begin(), find(vehicle2.route.begin()+depo_begin_idx2, vehicle2.route.end(), custs2.at(0)))+1;
                    }
                    else{
                        lv_t += customers.at(custs2.at(0)).servt*vehicle2.opskill;
                    }

                    if(customers.at(vehicle2.route.at(pos3-1)).custno != "0" || customers.at(custs2.at(0)).custno != "0"){ //デポが連続しない場合
                        atpen += max(0.0, st_t-customers.at(custs2.at(0)).due_fs);
                    }
                }

                for(int i = 0; i < custs2_size-1; i++){
                    if(custs2.at(i) == skv2_route.at(alast_idx2-1)){
                        break; //最終訪問先へのルートは考慮しない
                    }

                    Customer from_c = customers.at(custs2.at(i)), to_c = customers.at(custs2.at(i+1));

                    if(to_c.custno == from_c.custno && to_c.custno != "0"){
                        continue;
                    }

                    if(one_table_flag == false || argvs.multitrip_flag == true || pick_nondepo_flag == true || argvs.avedist_ae_flag == true || argvs.avedist_ape_flag == true){
                        if(get_dtable(vehicle2.vtype, lv_t, argvs.management_dtable, from_c.spotno, to_c.spotno) == -1
                        || get_ttable(vehicle2.vtype, lv_t, argvs.management_ttable, from_c.spotno, to_c.spotno) == -1
                        || (!argvs.management_etable.empty() && get_etable(vehicle2.vtype, vehicle2.etype, lv_t, argvs.management_etable, from_c.spotno, to_c.spotno) == -1)
                        || (!argvs.management_xtable.empty() && get_xtable(vehicle2.vtype, lv_t, argvs.management_xtable, from_c.spotno, to_c.spotno) == -1)){
                            passable = false; //通行不可
                            break;
                        }

                        adist += get_dtable(vehicle2.vtype, lv_t, argvs.management_dtable, from_c.spotno, to_c.spotno);
                        if(!argvs.management_etable.empty()){
                            apower += get_etable(vehicle2.vtype, vehicle2.etype, lv_t, argvs.management_etable, from_c.spotno, to_c.spotno);
                        }
                        if(!argvs.management_xtable.empty()){
                            aexp += get_xtable(vehicle2.vtype, lv_t, argvs.management_xtable, from_c.spotno, to_c.spotno);
                        }
                    }

                    arr_t = lv_t+get_ttable(vehicle2.vtype, lv_t, argvs.management_ttable, from_c.spotno, to_c.spotno)*vehicle2.drskill;
                    if(to_c.spotid != from_c.spotid && (to_c.custno != "0" || vector_finder(no_park_idx2, distance(vehicle2.route.begin(), find(vehicle2.route.begin()+depo_begin_idx2, vehicle2.route.end(), custs2.at(i+1)))) == false)){
                        lv_t = max(arr_t+to_c.parkt_arrive, to_c.ready_fs);
                        st_t = max(arr_t+to_c.parkt_arrive, to_c.ready_fs) + to_c.parkt_open;
                    }
                    else{
                        lv_t = max(arr_t, to_c.ready_fs);
                        st_t = max(arr_t, to_c.ready_fs);
                    }
                    if(to_c.spotid != from_c.spotid && (to_c.custno != "0" || vector_finder(no_park_idx2, distance(vehicle2.route.begin(), find(vehicle2.route.begin()+depo_begin_idx2, vehicle2.route.end(), custs2.at(i+1)))) == false)){
                        lv_t += to_c.parkt_open; //駐車時間
                    }
                    if(servt_dict2.count(distance(vehicle2.route.begin(), find(vehicle2.route.begin()+depo_begin_idx2, vehicle2.route.end(), custs2.at(i+1)))) == 1){
                        lv_t += servt_dict2.at(distance(vehicle2.route.begin(), find(vehicle2.route.begin()+depo_begin_idx2, vehicle2.route.end(), custs2.at(i+1))))*vehicle2.opskill;
                        depo_begin_idx2 = distance(vehicle2.route.begin(), find(vehicle2.route.begin()+depo_begin_idx2, vehicle2.route.end(), custs2.at(i+1)))+1;
                    }
                    else{
                        lv_t += to_c.servt*vehicle2.opskill;
                    }

                    if(from_c.custno != "0" || to_c.custno != "0"){ //デポが連続しない場合
                        atpen += max(0.0, st_t-to_c.due_fs);
                    }
                }

                if(!passable){
                    continue; //通行不可
                }

                if((argvs.bulkShipping_flag == false && pos4 == veh2_route_size-1 && customers.at(custs2.at(custs2_size-1)).custno != "0" && customers.at(vehicle2.route.at(veh2_route_size-1)).custno != "0" && pick_opt_flag2 == true) ||
                (argvs.bulkShipping_flag == true && pos4 == veh2_route_size-1 && customers.at(customers.at(custs2.at(custs2_size-1)).tieup_customer.at(0)).custno != "0" && customers.at(customers.at(vehicle2.route.at(veh2_route_size-1)).tieup_customer.at(0)).custno != "0" && pick_opt_flag2 == true)){ //集荷最適化
                    if(get_dtable(vehicle2.vtype, lv_t, argvs.management_dtable, customers.at(custs2.at(custs2_size-1)).spotno, customers.at(0).spotno) == -1
                    || get_ttable(vehicle2.vtype, lv_t, argvs.management_ttable, customers.at(custs2.at(custs2_size-1)).spotno, customers.at(0).spotno) == -1
                    || (!argvs.management_etable.empty() && get_etable(vehicle2.vtype, vehicle2.etype, lv_t, argvs.management_etable, customers.at(custs2.at(custs2_size-1)).spotno, customers.at(0).spotno) == -1)
                    || (!argvs.management_xtable.empty() && get_xtable(vehicle2.vtype, lv_t, argvs.management_xtable, customers.at(custs2.at(custs2_size-1)).spotno, customers.at(0).spotno) == -1)){
                        continue; //通行不可
                    }

                    arr = lv_t+get_ttable(vehicle2.vtype, lv_t, argvs.management_ttable, customers.at(custs2.at(custs2_size-1)).spotno, customers.at(0).spotno)*vehicle2.drskill;
                    st = max(arr+customers.at(0).parkt_arrive, customers.at(0).ready_fs)+customers.at(0).parkt_open;
                    lv = max(arr+customers.at(0).parkt_arrive, customers.at(0).ready_fs)+(servt_dict2.at(veh2_route_size-1)+vehicle2.depotservt)*vehicle2.opskill;
                    lv += customers.at(0).parkt_open;

                    if(get_dtable(vehicle2.vtype, lv, argvs.management_dtable, customers.at(0).spotno, customers.at(vehicle2.route.at(veh2_route_size-1)).spotno) == -1
                    || get_ttable(vehicle2.vtype, lv, argvs.management_ttable, customers.at(0).spotno, customers.at(vehicle2.route.at(veh2_route_size-1)).spotno) == -1
                    || (!argvs.management_etable.empty() && get_etable(vehicle2.vtype, vehicle2.etype, lv, argvs.management_etable, customers.at(0).spotno, customers.at(vehicle2.route.at(veh2_route_size-1)).spotno) == -1)
                    || (!argvs.management_xtable.empty() && get_xtable(vehicle2.vtype, lv, argvs.management_xtable, customers.at(0).spotno, customers.at(vehicle2.route.at(veh2_route_size-1)).spotno) == -1)){
                        continue; //通行不可
                    }
                }
                else if((argvs.bulkShipping_flag == false && pos4 == veh2_route_size-2 && customers.at(vehicle2.route.at(veh2_route_size-2)).custno == "0" && customers.at(vehicle2.route.at(veh2_route_size-1)).custno != "0" && pick_opt_flag2 == false) ||
                (argvs.bulkShipping_flag == true && pos4 == veh2_route_size-2 && customers.at(customers.at(vehicle2.route.at(veh2_route_size-2)).tieup_customer.at(0)).custno == "0" && customers.at(customers.at(vehicle2.route.at(veh2_route_size-1)).tieup_customer.at(0)).custno != "0" && pick_opt_flag2 == false)){ //集荷最適化
                    if(get_dtable(vehicle2.vtype, lv_t, argvs.management_dtable, customers.at(custs2.at(custs2_size-1)).spotno, customers.at(vehicle2.route.at(veh2_route_size-1)).spotno) == -1
                    || get_ttable(vehicle2.vtype, lv_t, argvs.management_ttable, customers.at(custs2.at(custs2_size-1)).spotno, customers.at(vehicle2.route.at(veh2_route_size-1)).spotno) == -1
                    || (!argvs.management_etable.empty() && get_etable(vehicle2.vtype, vehicle2.etype, lv_t, argvs.management_etable, customers.at(custs2.at(custs2_size-1)).spotno, customers.at(vehicle2.route.at(veh2_route_size-1)).spotno) == -1)
                    || (!argvs.management_xtable.empty() && get_xtable(vehicle2.vtype, lv_t, argvs.management_xtable, customers.at(custs2.at(custs2_size-1)).spotno, customers.at(vehicle2.route.at(veh2_route_size-1)).spotno) == -1)){
                        continue; //通行不可
                    }
                }
                else{
                    if(customers.at(custs2.at(custs2_size-1)).custno != customers.at(vehicle2.route.at(pos4)).custno || customers.at(vehicle2.route.at(pos4)).custno == "0"){
                        if(get_dtable(vehicle2.vtype, lv_t, argvs.management_dtable, customers.at(custs2.at(custs2_size-1)).spotno, customers.at(vehicle2.route.at(pos4)).spotno) == -1
                        || get_ttable(vehicle2.vtype, lv_t, argvs.management_ttable, customers.at(custs2.at(custs2_size-1)).spotno, customers.at(vehicle2.route.at(pos4)).spotno) == -1
                        || (!argvs.management_etable.empty() && get_etable(vehicle2.vtype, vehicle2.etype, lv_t, argvs.management_etable, customers.at(custs2.at(custs2_size-1)).spotno, customers.at(vehicle2.route.at(pos4)).spotno) == -1)
                        || (!argvs.management_xtable.empty() && get_xtable(vehicle2.vtype, lv_t, argvs.management_xtable, customers.at(custs2.at(custs2_size-1)).spotno, customers.at(vehicle2.route.at(pos4)).spotno) == -1)){
                            continue; //通行不可
                        }

                        if(pos4 < blast_idx2){
                            adist += get_dtable(vehicle2.vtype, lv_t, argvs.management_dtable, customers.at(custs2.at(custs2_size-1)).spotno, customers.at(vehicle2.route.at(pos4)).spotno);
                            if(!argvs.management_etable.empty()){
                                apower += get_etable(vehicle2.vtype, vehicle2.etype, lv_t, argvs.management_etable, customers.at(custs2.at(custs2_size-1)).spotno, customers.at(vehicle2.route.at(pos4)).spotno);
                            }
                            if(!argvs.management_xtable.empty()){
                                aexp += get_xtable(vehicle2.vtype, lv_t, argvs.management_xtable, customers.at(custs2.at(custs2_size-1)).spotno, customers.at(vehicle2.route.at(pos4)).spotno);
                            }
                        }

                        arr = lv_t+get_ttable(vehicle2.vtype, lv_t, argvs.management_ttable, customers.at(custs2.at(custs2_size-1)).spotno, customers.at(vehicle2.route.at(pos4)).spotno)*vehicle2.drskill;
                        if (servt_dict2.count(pos4) == 1){
                            if(customers.at(vehicle2.route.at(pos4)).spotid != customers.at(custs2.at(custs2_size-1)).spotid && vector_finder(no_park_idx2, pos4) == false){
                                lv = max(arr+customers.at(vehicle2.route.at(pos4)).parkt_arrive, customers.at(vehicle2.route.at(pos4)).ready_fs)+servt_dict2.at(pos4)*vehicle2.opskill;
                                st = max(arr+customers.at(vehicle2.route.at(pos4)).parkt_arrive, customers.at(vehicle2.route.at(pos4)).ready_fs)+customers.at(vehicle2.route.at(pos4)).parkt_open;
                            }
                            else{
                                lv = max(arr, customers.at(vehicle2.route.at(pos4)).ready_fs)+servt_dict2.at(pos4)*vehicle2.opskill;
                                st = max(arr, customers.at(vehicle2.route.at(pos4)).ready_fs);
                            }
                        }
                        else{
                            if(customers.at(vehicle2.route.at(pos4)).spotid != customers.at(custs2.at(custs2_size-1)).spotid && vector_finder(no_park_idx2, pos4) == false){
                                lv = max(arr+customers.at(vehicle2.route.at(pos4)).parkt_arrive, customers.at(vehicle2.route.at(pos4)).ready_fs)+customers.at(vehicle2.route.at(pos4)).servt*vehicle2.opskill;
                                st = max(arr+customers.at(vehicle2.route.at(pos4)).parkt_arrive, customers.at(vehicle2.route.at(pos4)).ready_fs)+customers.at(vehicle2.route.at(pos4)).parkt_open;
                            }
                            else{
                                lv = max(arr, customers.at(vehicle2.route.at(pos4)).ready_fs)+customers.at(vehicle2.route.at(pos4)).servt*vehicle2.opskill;
                                st = max(arr, customers.at(vehicle2.route.at(pos4)).ready_fs);
                            }
                        }
                        if(customers.at(vehicle2.route.at(pos4)).spotid != customers.at(custs2.at(custs2_size-1)).spotid && vector_finder(no_park_idx2, pos4) == false){
                            lv += customers.at(vehicle2.route.at(pos4)).parkt_open; //駐車時間
                        }
                    }
                    else{
                        arr = arr_t;
                        lv = lv_t;
                        st = st_t;
                    }

                    if(one_table_flag == false || argvs.multitrip_flag == true || pick_nondepo_flag == true || argvs.avedist_ae_flag == true || argvs.avedist_ape_flag == true){
                        passable = check_impassability(vehicle2, customers, lv, pos4+1, blast_idx2, servt_dict2, adist, apower, aexp, no_park_idx2, pick_opt_flag2, argvs);

                        if(!passable){
                            continue; //通行不可
                        }
                    }
                }
                adist2 = adist-adist1;

                lv2 = lv_t;
                st2 = st_t;

                if(pos4 >= blast_idx2){ //pos4が最終訪問先の場合
                    atpen += max(0.0, lv2-vehicle2.endhour);

                    if(compare_time(argvs.lowertime) > 0 || compare_time(argvs.uppertime) > 0){
                        return_lutime_penalty(lv2-vehicle2.arr.at(0), bv2final_t-vehicle2.arr.at(0), argvs.lowertime, argvs.uppertime, atpen_temp, btpen_temp);
                    }
                }
                else{
                    if(customers.at(custs2.at(custs2_size-1)).custno != customers.at(vehicle2.route.at(pos4)).custno){
                        atpen += max(0.0, st-customers.at(vehicle2.route.at(pos4)).due_fs);
                    }

                    lv_t = lv;
                    st_t = st;

                    for(int i = pos4; i < blast_idx2-1; i++){
                        Customer from_c = customers.at(vehicle2.route.at(i)), to_c = customers.at(vehicle2.route.at(i+1));

                        if(to_c.custno == from_c.custno && to_c.custno != "0"){
                            continue;
                        }

                        //作業時間
                        double proc_time;
                        if(servt_dict2.count(i+1) == 1){
                            proc_time = servt_dict2.at(i+1);
                        }
                        else{
                            proc_time = customers.at(vehicle2.route.at(i+1)).servt;
                        }

                        bool no_park;
                        if(vector_finder(no_park_idx2, i+1) == true){
                            no_park = true;
                        }
                        else{
                            no_park = false;
                        }

                        double ttime_pen = calc_time_pen(vehicle2, from_c, to_c, lv_t, proc_time, argvs.management_ttable, no_park);

                        if(from_c.custno != "0" || to_c.custno != "0"){ //デポが連続しない場合
                            atpen += ttime_pen;
                        }
                    }

                    lv2 = lv_t;
                    st2 = st_t;

                    atpen += max(0.0, lv2-vehicle2.endhour);

                    if(compare_time(argvs.lowertime) > 0 || compare_time(argvs.uppertime) > 0){
                        return_lutime_penalty(lv2-vehicle2.arr.at(0), bv2final_t-vehicle2.arr.at(0), argvs.lowertime, argvs.uppertime, atpen_temp, btpen_temp);
                    }
                }
                //#### ここまで休憩機能なし用の処理
            }
            else{
				//#### ここから休憩機能あり用の処理 ####
                double lv_t, arr_t, lv, arr;
                bool passable = true;

                Vehicle veh1 = dcopy(vehicle1);
                veh1.route = skv1_route;
				if(argvs.pickup_flag){
                    if(pick_opt_flag1){
                        if(get_pick_opt_status(customers, veh1, argvs)==false){
                            veh1.route.insert(veh1.route.end()-1, 0);
                        }
                    }
                    else{
                        veh1 = pick_opt_rest(customers, veh1, pick_opt_flag1, argvs);
                    }
                }
                veh1.update_arrtime_tw(customers, argvs.management_ttable, argvs.break_flag);
                veh1.update_load_onbrd(customers);
                Vehicle veh2 = dcopy(vehicle2);
                veh2.route = skv2_route;
				if(argvs.pickup_flag){
                    if(pick_opt_flag2){
                        if(get_pick_opt_status(customers, veh2, argvs)==false){
                            veh2.route.insert(veh2.route.end()-1, 0);
                        }
                    }
                    else{
                        veh2 = pick_opt_rest(customers, veh2, pick_opt_flag2, argvs);
                    }
                }
                veh2.update_arrtime_tw(customers, argvs.management_ttable, argvs.break_flag);
                veh2.update_load_onbrd(customers);

                if(insert_breakspot(customers, veh1, vehicles, argvs, veh2) == false){
    				argvs.management_ttable = t_ttable;
	    			argvs.management_dtable = t_dtable;
		    		argvs.management_etable = t_etable;
			    	argvs.management_xtable = t_xtable;
                    customers = t_customers;
				    vehicles = r_restore(vehicles, t_route, argvs, customers);
                    continue;
                }
                skv1_route = veh1.route;
                skv1_route_size = skv1_route.size();
				alast_idx1=skv1_route.size();
                for(int i = skv1_route_size-1; i > 0; i--){
                    if((customers.at(skv1_route.at(i)).custno!="0")&&(customers.at(skv1_route.at(i)).custno.at(0)!='b')&&(customers.at(skv1_route.at(i)).custno.at(0)!='e')&&(i!=skv1_route_size-1)){
                        break;
                    }
                    else{
						alast_idx1 = i;
                    }
                }
				lv1 = veh1.lv.at(alast_idx1 - 1);
                depo_idx1_temp.clear();
                for(int i = 0; i < skv1_route_size-1; i++){
                    if(customers.at(skv1_route.at(i)).custno == "0"){
                        depo_idx1_temp.emplace_back(i); //routeにおけるデポのインデックス
                    }
                }
                depo_idx1_temp.emplace_back(skv1_route_size-1); //最終訪問先を追加
                depo_idx1_temp_size = depo_idx1_temp.size();
                veh1.update_totalcost_dtable(customers, argvs);
				adist1 = veh1.totalcost_wolast;

                for(int i = start_idx1; i < alast_idx1; i++){
                    if(i>0){
                        if(customers.at(veh1.route.at(i-1)).custno != customers.at(veh1.route.at(i)).custno){
                            atpen += max(0.0, veh1.st.at(i)-customers.at(veh1.route.at(i)).due_fs);
                        }
                    }
                    else{
                        atpen += max(0.0, veh1.st.at(i)-customers.at(veh1.route.at(i)).due_fs);
                    }
                }
                atpen += max(0.0, lv1-veh1.endhour);

                skv2_route = veh2.route;
                skv2_route_size = skv2_route.size();
				alast_idx2=skv2_route.size();
                for(int i = skv2_route_size-1; i > 0; i--){
                    if((customers.at(skv2_route.at(i)).custno!="0")&&(customers.at(skv2_route.at(i)).custno.at(0)!='b')&&(customers.at(skv2_route.at(i)).custno.at(0)!='e')&&(i!=skv2_route_size-1)){
                        break;
                    }
                    else{
						alast_idx2 = i;
                    }
                }
				lv2 = veh2.lv.at(alast_idx2 - 1);
                depo_idx2_temp.clear();
                for(int i = 0; i < skv2_route_size-1; i++){
                    if(customers.at(skv2_route.at(i)).custno == "0"){
                        depo_idx2_temp.emplace_back(i); //routeにおけるデポのインデックス
                    }
                }
                depo_idx2_temp.emplace_back(skv2_route_size-1); //最終訪問先を追加
                depo_idx2_temp_size = depo_idx2_temp.size();
                veh2.update_totalcost_dtable(customers, argvs);
				adist2 = veh2.totalcost_wolast;

                for(int i = start_idx2; i < alast_idx2; i++){
                    if(i>0){
                        if(customers.at(veh2.route.at(i-1)).custno != customers.at(veh2.route.at(i)).custno){
                            atpen += max(0.0, veh2.st.at(i)-customers.at(veh2.route.at(i)).due_fs);
                        }
                    }
                    else{
                        atpen += max(0.0, veh2.st.at(i)-customers.at(veh2.route.at(i)).due_fs);
                    }
                }
                atpen += max(0.0, lv2-veh2.endhour);

                //kaizen go no arrive time wo ltime,utime to hikaku
                if(compare_time(argvs.lowertime) > 0 || compare_time(argvs.uppertime) > 0){
                    return_lutime_penalty(lv1-vehicle1.arr.at(0), bv1final_t-vehicle1.arr.at(0), argvs.lowertime, argvs.uppertime, atpen_temp, btpen_temp);
                    return_lutime_penalty(lv2-vehicle2.arr.at(0), bv2final_t-vehicle2.arr.at(0), argvs.lowertime, argvs.uppertime, atpen_temp, btpen_temp);
                }
                //### 
                if(!argvs.management_etable.empty()){
                    apower = calc_power(veh1, customers, start_idx1+1, skv1_route_size, argvs.management_etable)
                        +calc_power(veh2, customers, start_idx2+1, skv2_route_size, argvs.management_etable);
                }

                if(!argvs.management_xtable.empty()){
                    aexp = calc_exp(veh1, customers, start_idx1+1, skv1_route_size, argvs.management_xtable)
                        +calc_exp(veh2, customers, start_idx2+1, skv2_route_size, argvs.management_xtable);
                }

                adist = adist1 + adist2;
				//#### ここまで休憩機能あり用の処理 ####
            }

            //集荷違反量を計算
            double max_load_pen1_1 = 0, max_load_pen1_2 = 0,max_load_pen2_1 = 0,max_load_pen2_2 = 0;
            if(argvs.pickup_flag == true){
                tie(max_load_pen1_1, max_load_pen1_2) = calc_vehicle_loadpickup_penalty(vehicle1, customers, skv1_route);
                tie(max_load_pen2_1, max_load_pen2_2) = calc_vehicle_loadpickup_penalty(vehicle2, customers, skv2_route);
            }


            //配送違反量を計算
            double load_pen = 0, load_pen2 = 0, load_onbrd = 0, load_onbrd2 = 0;
            for(int i = 0; i < depo_idx1_temp_size-1; i++){
                load_onbrd = 0;
                load_onbrd2 = 0;

                for(int j = depo_idx1_temp.at(i)+1; j < depo_idx1_temp.at(i+1); j++){
                    if(customers.at(skv1_route.at(j)).requestType == "DELIVERY"){
                       if(customers.at(skv1_route.at(j)).custno.at(0) != 'b'){  // 休憩スポットは積載量は無視する
                            load_onbrd += customers.at(skv1_route.at(j)).dem;
                    
                            load_onbrd2 += customers.at(skv1_route.at(j)).dem2;
                       }
                    }
                }

                load_pen += max(load_onbrd-vehicle1.cap,0.0);
                load_pen2 += max(load_onbrd2-vehicle1.cap2,0.0);
            }

            for(int i = 0; i < depo_idx2_temp_size-1; i++){
                load_onbrd = 0;
                load_onbrd2 = 0;

                for(int j = depo_idx2_temp.at(i)+1; j < depo_idx2_temp.at(i+1); j++){
                    if(customers.at(skv2_route.at(j)).requestType == "DELIVERY"){
                        if(customers.at(skv2_route.at(j)).custno.at(0) != 'b'){  // 休憩スポットは積載量は無視する
                            load_onbrd += customers.at(skv2_route.at(j)).dem;
                    
                            load_onbrd2 += customers.at(skv2_route.at(j)).dem2;
                        }
                    }
                }

                load_pen += max(load_onbrd-vehicle2.cap,0.0);
                load_pen2 += max(load_onbrd2-vehicle2.cap2,0.0);
            }

            aload = load_pen+max_load_pen1_1+max_load_pen2_1;
            aload2 = load_pen2+max_load_pen1_2+max_load_pen2_2;


            //時間平準化ペナルティを計算
            int vnum = vehicles.size();
            if(argvs.avetime_ape_flag == true || argvs.avetime_ae_flag == true || argvs.opt_t_weight != 0){
                if(argvs.opt_t_weight != 0){
                    atime = lv1+lv2;
                }

                map<int, double> mvehicles;
                
                for(int i = 0; i < vnum; i++){
                    if(vehicles.at(i).vehno == vehicle1.vehno){
                        mvehicles.emplace(i, lv1-vehicle1.arr.at(0));
                    }
                    if(vehicles.at(i).vehno == vehicle2.vehno){
                        mvehicles.emplace(i, lv2-vehicle2.arr.at(0));
                    }
                }

                if(argvs.avetime_ape_flag == true){
                    atime_error = calc_avetime_APE(vehicles, customers, mvehicles, argvs.avetime_ape, argvs.lastc_flag, argvs.break_flag);
                }
                else if(argvs.avetime_ae_flag == true){
                    atime_error = calc_avetime_AE(vehicles, customers, mvehicles, argvs.avetime_ae, argvs.lastc_flag);
                }
            }


            //配送件数平準化ペナルティを計算
            if(argvs.avevisits_ape_flag == true || argvs.avevisits_ae_flag == true){
                map<int, double> mvehicles;
                
                for(int i = 0; i < vnum; i++){
                    if(argvs.bulkShipping_flag == true){
                        if(vehicles.at(i).vehno == vehicle1.vehno){
                            int cust_num1 = 0;
                            for(int j = 1; j < skv1_route_size-1; j++){
                                int tieup_num = customers.at(skv1_route.at(j)).tieup_customer.size();

                                for(int k = 0; k < tieup_num; k++){
                                    if(customers.at(customers.at(skv1_route.at(j)).tieup_customer.at(k)).custno == "0" 
                                    || customers.at(customers.at(skv1_route.at(j)).tieup_customer.at(k)).custno.substr(0, 1) == "e"
                                    || customers.at(customers.at(skv1_route.at(j)).tieup_customer.at(k)).custno.substr(0, 1) == "b"){
                                        continue; //充電スポットとデポはスキップ
                                    }

                                    cust_num1++;
                                }
                            }

                            mvehicles.emplace(i, cust_num1);
                        }
                        if(vehicles.at(i).vehno == vehicle2.vehno){
                            int cust_num2 = 0;
                            for(int j = 1; j < skv2_route_size-1; j++){
                                int tieup_num = customers.at(skv2_route.at(j)).tieup_customer.size();

                                for(int k = 0; k < tieup_num; k++){
                                    if(customers.at(customers.at(skv2_route.at(j)).tieup_customer.at(k)).custno == "0" 
                                    || customers.at(customers.at(skv2_route.at(j)).tieup_customer.at(k)).custno.substr(0, 1) == "e"
                                    || customers.at(customers.at(skv2_route.at(j)).tieup_customer.at(k)).custno.substr(0, 1) == "b"){
                                        continue; //充電スポットとデポはスキップ
                                    }

                                    cust_num2++;
                                }
                            }

                            mvehicles.emplace(i, cust_num2);
                        }
                    }
                    else{
                        if(vehicles.at(i).vehno == vehicle1.vehno){
                            int cust_num1 = 0;
                            for(int j = 1; j < skv1_route_size-1; j++){
                                if(customers.at(skv1_route.at(j)).custno == "0" 
                                || customers.at(skv1_route.at(j)).custno.substr(0, 1) == "e"
                                || customers.at(skv1_route.at(j)).custno.substr(0, 1) == "b"
                                || customers.at(skv1_route.at(j)).custno == customers.at(skv1_route.at(j-1)).custno){
                                    continue; //充電スポットかデポの場合または同一CUSTNOが連続する場合はスキップ
                                }

                                cust_num1++;
                            }

                            mvehicles.emplace(i, cust_num1);
                        }
                        if(vehicles.at(i).vehno == vehicle2.vehno){
                            int cust_num2 = 0;
                            for(int j = 1; j < skv2_route_size-1; j++){
                                if(customers.at(skv2_route.at(j)).custno == "0" 
                                || customers.at(skv2_route.at(j)).custno.substr(0, 1) == "e"
                                || customers.at(skv2_route.at(j)).custno.substr(0, 1) == "b"
                                || customers.at(skv2_route.at(j)).custno == customers.at(skv2_route.at(j-1)).custno){
                                    continue; //充電スポットかデポの場合または同一CUSTNOが連続する場合はスキップ
                                }

                                cust_num2++;
                            }

                            mvehicles.emplace(i, cust_num2);
                        }
                    }
                }

                if(argvs.avevisits_ape_flag == true){
                    avisits_error = calc_avevisits_APE(vehicles, customers, mvehicles, argvs.avevisits_ape, argvs.bulkShipping_flag, argvs.break_flag);
                }
                else if(argvs.avevisits_ae_flag == true){
                    avisits_error = calc_avevisits_AE(vehicles, customers, mvehicles, argvs.avevisits_ae, argvs.bulkShipping_flag, argvs.break_flag);
                }
            }


            if(argvs.avedist_ape_flag == true || argvs.avedist_ae_flag == true){
                map<int, double> mvehicles;
                
                for(int i = 0; i < vnum; i++){
                    if(vehicles.at(i).vehno == vehicle1.vehno){
                        mvehicles.emplace(i, vehicle1.totalcost_wolast+adist1-bdist1);
                    }
                    if(vehicles.at(i).vehno == vehicle2.vehno){
                        mvehicles.emplace(i, vehicle2.totalcost_wolast+adist2-bdist2);
                    }
                }

                if(argvs.avedist_ape_flag == true){
                    adist_error = calc_avedist_APE(vehicles, customers, mvehicles, argvs.avedist_ape, argvs.lastc_flag, argvs.break_flag);
                }
                else if(argvs.avedist_ae_flag == true){
                    adist_error = calc_avedist_AE(vehicles, customers, mvehicles, argvs.avedist_ae, argvs.lastc_flag);
                }
            }

            //改善後未割り当て荷物最適化ペナルティの算出(重み付けはここではしない)
            if(argvs.opt_unassigned_flag==true){
                if(vehicle1.reserved_v_flag==true){
                    for(int rc=1; rc<=alast_idx1; rc++){
                        if(customers.at(skv1_route.at(rc)).custno != "0" && customers.at(skv1_route.at(rc)).custno.at(0) != 'b' && customers.at(skv1_route.at(rc)).custno.at(0) != 'e'){
                            aunass++;
                        }
                    }
                }
                if(vehicle2.reserved_v_flag==true){
                    for(int rc=0; rc<=alast_idx2; rc++){
                        if(customers.at(skv2_route.at(rc)).custno != "0" && customers.at(skv2_route.at(rc)).custno.at(0) != 'b' && customers.at(skv2_route.at(rc)).custno.at(0) != 'e'){
                            aunass++;
                        }
                    }
                }
            }

            //解が改善したかチェック
            double cdiff_temp;
            bool ires_temp = improved_or_not(atpen+atpen_temp, btpen_temp, adist, bdist, aload, bload, aload2, bload2, atime_error, btime_error, avisits_error, bvisits_error, adist_error, bdist_error, atime, btime, apower, bpower, aexp, bexp, aunass, bunass, cdiff_temp, argvs);

            if(ires_temp == true && cdiff_temp < cdiff_best){ //改善があった場合
                ires = true;

                cdiff_best = cdiff_temp;
                cdiff=cdiff_temp;

                custs1_best = custs1;
                custs2_best = custs2;

                pick_opt_flag1_best = pick_opt_flag1, pick_opt_flag2_best = pick_opt_flag2;

                if(argvs.break_flag == true){
                    route1_best = skv1_route;
                    route2_best = skv2_route;
					ttable_best = argvs.management_ttable;
					dtable_best = argvs.management_dtable;
					etable_best = argvs.management_etable;
					xtable_best = argvs.management_xtable;
                    vehicles.at(vehicle1.vehno).route = skv1_route;
                    vehicles.at(vehicle2.vehno).route = skv2_route;
					t_route_best = r_backup(vehicles);
                    customers_best = customers;
                }

            }
			if(argvs.break_flag){
				argvs.management_ttable = t_ttable;
				argvs.management_dtable = t_dtable;
				argvs.management_etable = t_etable;
				argvs.management_xtable = t_xtable;
                customers = t_customers;
				vehicles = r_restore(vehicles, t_route, argvs, customers);

            }
        }
    }
    
    return make_tuple(ires, custs1_best, custs2_best, pick_opt_flag1_best, pick_opt_flag2_best, route1_best, route2_best, ttable_best, dtable_best, etable_best, xtable_best, t_route_best, customers_best);
}



namespace{
    //vecにnumberが含まれているか調べる
    bool vector_finder(const vector<int> &vec, const int number){
        auto itr = find(vec.begin(), vec.end(), number);
        size_t index = distance(vec.begin(), itr);

        if(index != vec.size()){
            return true; //発見できたとき
        }
        else{
            return false; //発見できなかったとき
        }
    }

    //vecにstringrが含まれているか調べる
    bool vector_finder(const vector<string> &vec, const string str){
        auto itr = find(vec.begin(), vec.end(), str);
        size_t index = distance(vec.begin(), itr);

        if(index != vec.size()){
            return true; //発見できたとき
        }
        else{
            return false; //発見できなかったとき
        }
    }

    //重複順列を計算する
    vector<vector<int>> combination_with_replacement(const int n, const int m){
        vector<vector<int>> comb;

        if(n == 0){
            return comb;
        }

        const int maxM = 10000;
        int buf[maxM] = {0};

        for(;;){
            vector<int> comb_temp;
            int size = 0;

            bool is_comb = true;

            for(int i =0; i < n; i++){
                for(int j = 0; j < i; j++){
                    if(buf[i] < buf[j]){
                        is_comb = false;
                        break;
                    }
                }
                if(!is_comb){
                    break;
                }

                comb_temp.emplace_back(buf[i]);
            }

            if(is_comb){
                comb.emplace_back(comb_temp);
            }

            int index = n-1;

            for(;;){
                if(++buf[index] < m){
                    break;
                }

                buf[index] = 0;

                if(--index < 0){
                    return comb;
                }
            }
        }

        return comb;
    }
}

