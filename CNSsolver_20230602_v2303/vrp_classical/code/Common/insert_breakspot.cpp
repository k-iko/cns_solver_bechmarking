#include <algorithm>
#include <iostream>
#include <vector>
#include "insert_breakspot.hpp"

namespace{
    bool vector_finder(const vector<int> &vec, const int number);
}

Customer create_customer(string _custno, const Cppargs &argvs)
/**
 * @brief パターン3の時に休憩スポットを作成する
 * @param[in] _custno
 * @param[in] argvs
 * @return Customer
 * @details パターン3の時に休憩スポットを作成する
 */
{
    vector<int> tieup_customer;
    vector<int> reje_ve;
    string custno = _custno;
    string spotid = "";
    int spotno = 0;
    double y = 0.0;
    double x = 0.0;
    double dem = 0.0;
    double dem2 = 0.0;
    double servt = argvs.break_time / 60;
    double e_rate = 0.0;
    double parkt_arrive = 0.0;
    double parkt_open = 0.0;
    double depotservt = 0.0;
    double ready_fs = -8765817.0;
    double due_fs = 8765823.0;
    string requestType = "DELIVERY";
    bool deny_unass = false;
    string changecust = "";

	return Customer(tieup_customer, reje_ve,custno,spotid,spotno,y,x,dem,dem2,servt,e_rate,parkt_arrive,parkt_open,depotservt,ready_fs,due_fs,requestType,deny_unass,changecust);
}


vector<int> get_no_park_idx(Vehicle& veh, vector<Customer>& customers)
{
    vector<int> depo_idx;
    map<int, double> servt_dict;
    vector<int> route = veh.route;
    vector<int> no_park_idx;

    int route_size = (int) route.size();

    if(route_size < 2){
        return no_park_idx;
    }

    for(int i = 0; i < route_size-1; i++){
        if(customers.at(route.at(i)).custno == "0"){
            depo_idx.emplace_back(i); //routeにおけるデポのインデックス
        }
    }
    depo_idx.emplace_back(route_size-1); //最終訪問先を追加

    //荷作業時間を計算
    int depo_idx_size = (int) depo_idx.size();
    vector<double> load(route_size, 0), load2(route_size, 0);

    for(int i=0; i < route_size-1; i++){
        if(vector_finder(depo_idx, i) == true){
            for(int j = i+1; j < depo_idx.at(distance(depo_idx.begin(), find(depo_idx.begin(), depo_idx.end(), i))+1); j++){
                if(customers.at(route.at(j)).requestType == "DELIVERY"){
                    load.at(i) += customers.at(route.at(j)).dem;
                    load2.at(i) += customers.at(route.at(j)).dem2;
                }
            }
        }
        else{
            if(customers.at(route.at(i)).requestType == "DELIVERY"){
                load.at(i) = load.at(i-1)-customers.at(route.at(i)).dem;
                load2.at(i) = load2.at(i-1)-customers.at(route.at(i)).dem2;
            }
            else{
                load.at(i) = load.at(i-1)+customers.at(route.at(i)).dem;
                load2.at(i) = load2.at(i-1)+customers.at(route.at(i)).dem2;
            }
        }
    }

    for(int i = route_size-2; i >= 0; i--){
        if(vector_finder(depo_idx, i) == true && vector_finder(depo_idx, i+1) == true){
            load.at(i) = load.at(i+1);
            load2.at(i) = load2.at(i+1);
        }
    }

    //誤差対策
    for(int i = 0; i < route_size; i++){
        if(load.at(i) < 1.0e-9){
            load.at(i) = 0.0;
        }
    }
    for(int i = 0; i < route_size; i++){
        if(load2.at(i) < 1.0e-9){
            load2.at(i) = 0.0;
        }
    }

    bool no_park_flag = true;

    for(int i = depo_idx.at(0)+1; i < depo_idx.at(1); i++){
        if(customers.at(route.at(i)).requestType == "DELIVERY"){
            no_park_flag = false;
            break;
        }
    }
    if(no_park_flag == true && load.at(0) == 0 && load2.at(0) == 0){
        no_park_idx.emplace_back(0);
    }

    for(int i = 1; i < depo_idx_size-1; i++){
        if(load.at(depo_idx.at(i)-1) == 0 && load2.at(depo_idx.at(i)-1) == 0){
            no_park_flag = true;

            for(int j = depo_idx.at(i)+1; j < depo_idx.at(i+1); j++){
                if(customers.at(route.at(j)).requestType == "DELIVERY"){
                    no_park_flag = false;
                    break;
                }
            }
            if(no_park_flag == true && load.at(depo_idx.at(i)) == 0 && load2.at(depo_idx.at(i)) == 0){
                no_park_idx.emplace_back(depo_idx.at(i));
            }
        }
    }

    if(load.at(depo_idx.at(depo_idx_size-1)-1) == 0 && load2.at(depo_idx.at(depo_idx_size-1)-1) == 0){
        no_park_idx.emplace_back(depo_idx.at(depo_idx_size-1));
    }

    return no_park_idx;
}


int add_table(Vehicle &veh, int from_spotno, int to_spotno, double from_time, double to_time, int arr_p, Cppargs &argvs)
/**
 * @brief パターン3の時にテーブルに作成した休憩スポットを追加する
 * @param[in] veh
 * @param[in] from_spotno
 * @param[in] to_spotno
 * @param[in] from_time
 * @param[in] to_time
 * @param[in] argvs
 * @return spotno 追加したテーブルのspotno
 * @details パターン3の時にテーブルに作成した休憩スポットを追加する
 */
{
	string vtype = veh.vtype;
    double start_time1 = veh.lv[arr_p-1];
    double start_time2 = start_time1 + argvs.break_time / 60;

	// ##### ttable #####
    //table検索(配送時刻がある場合)
    // ----- table1 -----
    bool exf = false;
    std::vector<std::vector<double>> table1;
    for(auto table = argvs.management_ttable.rbegin(); table != argvs.management_ttable.rend(); table++){
        if(get<0>(table->first) != "common" && get<0>(table->first) == vtype && compare_time(get<1>(table->first), start_time1) <= 0){
            table1 = table->second;
            exf = true;
            break;
        }
    }

    if(exf == false){
        for(auto table = argvs.management_ttable.rbegin(); table != argvs.management_ttable.rend(); table++){
            if(get<0>(table->first) == "time_common" && compare_time(get<1>(table->first), start_time1) <= 0){
                table1 = table->second;
                exf = true;
                break;
            }
        }
    }

    //table検索(配送時刻がない場合)
    if(exf == false){
        for(auto table = argvs.management_ttable.rbegin(); table != argvs.management_ttable.rend(); table++){
            if(get<0>(table->first) == "common"){
                table1 = table->second;
                exf = true;
                break;
            }
        }
    }

    exf = false;
    std::vector<std::vector<double>> table2;
    for(auto table = argvs.management_ttable.rbegin(); table != argvs.management_ttable.rend(); table++){
        if(get<0>(table->first) != "common" && get<0>(table->first) == vtype && compare_time(get<1>(table->first), start_time2) <= 0){
            table2 = table->second;
            exf = true;
            break;
        }
    }

    if(exf == false){
        for(auto table = argvs.management_ttable.rbegin(); table != argvs.management_ttable.rend(); table++){
            if(get<0>(table->first) == "time_common" && compare_time(get<1>(table->first), start_time2) <= 0){
                table2 = table->second;
                exf = true;
                break;
            }
        }
    }

    //table検索(配送時刻がない場合)
    if(exf == false){
        for(auto table = argvs.management_ttable.rbegin(); table != argvs.management_ttable.rend(); table++){
            if(get<0>(table->first) == "common"){
                table2 = table->second;
                exf = true;
                break;
            }
        }
    }

    bool sameTable = false;
    if(table1==table2){
        sameTable = true;
    }

    // from_timeの追加(列追加)
    int table1_size=table1.size();
    for(int idx=0; idx < table1_size; idx++){
        table1.at(idx).push_back(0);
    }
    int tl = table1.at(from_spotno).size();
    table1.at(from_spotno).at(tl - 1) = from_time;

    if(sameTable == true){
	    // to_timeの追加(行追加)
        std::vector<double> tt(table1.at(0).size(), 0); 
	    tt.at(to_spotno) = to_time;
	    table1.push_back(tt);
    }
    else{
	    // to_timeの追加(行追加)
        std::vector<double> tt(table2.at(0).size(), 0); 
	    tt.at(to_spotno) = to_time;
	    table2.push_back(tt);
    }

    exf = false;
    for(auto table = argvs.management_ttable.rbegin(); table != argvs.management_ttable.rend(); table++){
        if(get<0>(table->first) != "common" && get<0>(table->first) == vtype && compare_time(get<1>(table->first), start_time1) <= 0){
            argvs.management_ttable[table->first] = table1;
            exf = true;
            break;
        }
    }

    if(exf == false){
        for(auto table = argvs.management_ttable.rbegin(); table != argvs.management_ttable.rend(); table++){
            if(get<0>(table->first) == "time_common" && compare_time(get<1>(table->first), start_time1) <= 0){
                argvs.management_ttable[table->first] = table1;
                exf = true;
                break;
            }
        }
    }

    //table検索(配送時刻がない場合)
    if(exf == false){
        for(auto table = argvs.management_ttable.rbegin(); table != argvs.management_ttable.rend(); table++){
            if(get<0>(table->first) == "common"){
                argvs.management_ttable[table->first] = table1;
                exf = true;
                break;
            }
        }
    }

    if(sameTable == false){
        //table検索(配送時刻がない場合)
        exf = false;
        for(auto table = argvs.management_ttable.rbegin(); table != argvs.management_ttable.rend(); table++){
            if(get<0>(table->first) != "common" && get<0>(table->first) == vtype && compare_time(get<1>(table->first), start_time2) <= 0){
                argvs.management_ttable[table->first] = table2;
                exf = true;
                break;
            }
        }

        if(exf == false){
            for(auto table = argvs.management_ttable.rbegin(); table != argvs.management_ttable.rend(); table++){
                if(get<0>(table->first) == "time_common" && compare_time(get<1>(table->first), start_time2) <= 0){
                    argvs.management_ttable[table->first] = table2;
                    exf = true;
                    break;
                }
            }
        }

        if(exf == false){
            for(auto table = argvs.management_ttable.rbegin(); table != argvs.management_ttable.rend(); table++){
                if(get<0>(table->first) == "common"){
                    argvs.management_ttable[table->first] = table2;
                    exf = true;
                    break;
                }
            }
        }
    }

	int last_spotno = table1.size()-1;

    for(auto table = argvs.management_ttable.rbegin(); table != argvs.management_ttable.rend(); table++){
        if((sameTable == true && table->second != table1)||(table->second != table1 && table->second != table2)){
            int table_second_size = table->second.size();
            for(int idx=0; idx < table_second_size; idx++){
                argvs.management_ttable[table->first].at(idx).push_back(0);
            }
            std::vector<double> tt(table->second.at(0).size(), 0); 
            argvs.management_ttable[table->first].push_back(tt);
        }
    }

	// ##### dtable #####
	// table検索(配送時刻がある場合)
    exf = false;
    for(auto table = argvs.management_dtable.rbegin(); table != argvs.management_dtable.rend(); table++){
        if(get<0>(table->first) != "common" && get<0>(table->first) == vtype && compare_time(get<1>(table->first), start_time1) <= 0){
            table1 = table->second;
            exf = true;
            break;
        }
    }

    if(exf == false){
        for(auto table = argvs.management_dtable.rbegin(); table != argvs.management_dtable.rend(); table++){
            if(get<0>(table->first) == "time_common" && compare_time(get<1>(table->first), start_time1) <= 0){
                table1 = table->second;
                exf = true;
                break;
            }
        }
    }

    //table検索(配送時刻がない場合)
    if(exf == false){
        for(auto table = argvs.management_dtable.rbegin(); table != argvs.management_dtable.rend(); table++){
            if(get<0>(table->first) == "common"){
                table1 = table->second;
                exf = true;
                break;
            }
        }
    }

    exf = false;
    for(auto table = argvs.management_dtable.rbegin(); table != argvs.management_dtable.rend(); table++){
        if(get<0>(table->first) != "common" && get<0>(table->first) == vtype && compare_time(get<1>(table->first), start_time2) <= 0){
            table2 = table->second;
            exf = true;
            break;
        }
    }

    if(exf == false){
        for(auto table = argvs.management_dtable.rbegin(); table != argvs.management_dtable.rend(); table++){
            if(get<0>(table->first) == "time_common" && compare_time(get<1>(table->first), start_time2) <= 0){
                table2 = table->second;
                exf = true;
                break;
            }
        }
    }

    //table検索(配送時刻がない場合)
    if(exf == false){
        for(auto table = argvs.management_dtable.rbegin(); table != argvs.management_dtable.rend(); table++){
            if(get<0>(table->first) == "common"){
                table2 = table->second;
                exf = true;
                break;
            }
        }
    }

	double to_val = table1.at(from_spotno).at(to_spotno);

    if(table1 == table2){
        sameTable = true;
    }
    else{
        sameTable = false;
    }

    // from_timeの追加(列追加)
    table1_size = table1.size();
    for(int idx=0; idx < table1_size; idx++){
        table1.at(idx).push_back(0);
    }
    if(to_time!=0.0){
        table1.at(from_spotno).at(tl - 1) = to_val / 2;
    }
    else{
        table1.at(from_spotno).at(tl - 1) = 0;
    }

    if(sameTable == true){
	    // to_timeの追加(行追加)
        std::vector<double> td(table1.at(0).size(), 0); 
        if(to_time!=0.0){
	        td.at(to_spotno) = to_val / 2;
        }
        else{
	        td.at(to_spotno) = 0;
        }
	    table1.push_back(td);
    }
    else{
	    // to_timeの追加(行追加)
        std::vector<double> td(table2.at(0).size(), 0); 
        if(to_time!=0.0){
	        td.at(to_spotno) = to_val / 2;
        }
        else{
	        td.at(to_spotno) = 0;
        }
	    table2.push_back(td);
    }

    exf = false;
    for(auto table = argvs.management_dtable.rbegin(); table != argvs.management_dtable.rend(); table++){
        if(get<0>(table->first) != "common" && get<0>(table->first) == vtype && compare_time(get<1>(table->first), start_time1) <= 0){
            argvs.management_dtable[table->first] = table1;
            exf = true;
            break;
        }
    }

    if(exf == false){
        for(auto table = argvs.management_dtable.rbegin(); table != argvs.management_dtable.rend(); table++){
            if(get<0>(table->first) == "time_common" && compare_time(get<1>(table->first), start_time1) <= 0){
                argvs.management_dtable[table->first] = table1;
                exf = true;
                break;
            }
        }
    }

    //table検索(配送時刻がない場合)
    if(exf == false){
        for(auto table = argvs.management_dtable.rbegin(); table != argvs.management_dtable.rend(); table++){
            if(get<0>(table->first) == "common"){
                argvs.management_dtable[table->first] = table1;
                exf = true;
                break;
            }
        }
    }

    if(sameTable == false){
        exf = false;
        for(auto table = argvs.management_dtable.rbegin(); table != argvs.management_dtable.rend(); table++){
            if(get<0>(table->first) != "common" && get<0>(table->first) == vtype && compare_time(get<1>(table->first), start_time2) <= 0){
                argvs.management_dtable[table->first] = table2;
                exf = true;
                break;
            }
        }

        if(exf == false){
            for(auto table = argvs.management_dtable.rbegin(); table != argvs.management_dtable.rend(); table++){
                if(get<0>(table->first) == "time_common" && compare_time(get<1>(table->first), start_time2) <= 0){
                    argvs.management_dtable[table->first] = table2;
                    exf = true;
                    break;
                }
            }
        }

        //table検索(配送時刻がない場合)
        if(exf == false){
            for(auto table = argvs.management_dtable.rbegin(); table != argvs.management_dtable.rend(); table++){
                if(get<0>(table->first) == "common"){
                    argvs.management_dtable[table->first] = table2;
                    exf = true;
                    break;
                }
            }
        }
    }

    for(auto table = argvs.management_dtable.rbegin(); table != argvs.management_dtable.rend(); table++){
        int table_size = argvs.management_dtable[table->first].at(0).size();
        if(table1_size >= table_size){
            int table_second_size = table->second.size();
            for(int idx=0; idx < table_second_size; idx++){
                argvs.management_dtable[table->first].at(idx).push_back(0);
            }
        }
        table_size = argvs.management_dtable[table->first].size();
        if(table1_size >= table_size){
            std::vector<double> tt(table->second.at(0).size(), 0); 
            argvs.management_dtable[table->first].push_back(tt);
        }
    }

	// ##### xtable #####
    if(argvs.management_xtable.size() > 0){
        // table検索(配送時刻がある場合)
        exf = false;
        for(auto table = argvs.management_xtable.rbegin(); table != argvs.management_xtable.rend(); table++){
            if(get<0>(table->first) != "common" && get<0>(table->first) == vtype && compare_time(get<1>(table->first), start_time1) <= 0){
                table1 = table->second;
                exf = true;
                break;
            }
        }

        if(exf == false){
            for(auto table = argvs.management_xtable.rbegin(); table != argvs.management_xtable.rend(); table++){
                if(get<0>(table->first) == "time_common" && compare_time(get<1>(table->first), start_time1) <= 0){
                    table1 = table->second;
                    exf = true;
                    break;
                }
            }
        }

        //table検索(配送時刻がない場合)
        if(exf == false){
            for(auto table = argvs.management_xtable.rbegin(); table != argvs.management_xtable.rend(); table++){
                if(get<0>(table->first) == "common"){
                    table1 = table->second;
                    exf = true;
                    break;
                }
            }
        }

        // table検索(配送時刻がある場合)
        exf = false;
        for(auto table = argvs.management_xtable.rbegin(); table != argvs.management_xtable.rend(); table++){
            if(get<0>(table->first) != "common" && get<0>(table->first) == vtype && compare_time(get<1>(table->first), start_time2) <= 0){
                table2 = table->second;
                exf = true;
                break;
            }
        }

        if(exf == false){
            for(auto table = argvs.management_xtable.rbegin(); table != argvs.management_xtable.rend(); table++){
                if(get<0>(table->first) == "time_common" && compare_time(get<1>(table->first), start_time2) <= 0){
                    table2 = table->second;
                    exf = true;
                    break;
                }
            }
        }

        //table検索(配送時刻がない場合)
        if(exf == false){
            for(auto table = argvs.management_xtable.rbegin(); table != argvs.management_xtable.rend(); table++){
                if(get<0>(table->first) == "common"){
                    table2 = table->second;
                    exf = true;
                    break;
                }
            }
        }

        to_val = table1.at(from_spotno).at(to_spotno);

        if(table1 == table2){
            sameTable = true;
        }
        else{
            sameTable = false;
        }

        // from_timeの追加(列追加)
        table1_size = table1.size();
        for(int idx=0; idx < table1_size; idx++){
            table1.at(idx).push_back(0);
        }
        if(to_time!=0.0){
            table1.at(from_spotno).at(tl - 1) = to_val / 2;
        }
        else{
            table1.at(from_spotno).at(tl - 1) = 0;
        }

        if(sameTable == true){
            // to_timeの追加(行追加)
            std::vector<double> tx(table1.at(0).size(), 0); 
            if(to_time!=0.0){
                tx.at(to_spotno) = to_val / 2;
            }
            else{
                tx.at(to_spotno) = 0;
            }
            table1.push_back(tx);
        }
        else{
            // to_timeの追加(行追加)
            std::vector<double> tx(table2.at(0).size(), 0); 
            if(to_time!=0.0){
                tx.at(to_spotno) = to_val / 2;
            }
            else{
                tx.at(to_spotno) = 0;
            }
            table2.push_back(tx);
        }

        exf = false;
        for(auto table = argvs.management_xtable.rbegin(); table != argvs.management_xtable.rend(); table++){
            if(get<0>(table->first) != "common" && get<0>(table->first) == vtype && compare_time(get<1>(table->first), start_time1) <= 0){
                argvs.management_xtable[table->first] = table1;
                exf = true;
                break;
            }
        }

        if(exf == false){
            for(auto table = argvs.management_xtable.rbegin(); table != argvs.management_xtable.rend(); table++){
                if(get<0>(table->first) == "time_common" && compare_time(get<1>(table->first), start_time1) <= 0){
                    argvs.management_xtable[table->first] = table1;
                    exf = true;
                    break;
                }
            }
        }

        //table検索(配送時刻がない場合)
        if(exf == false){
            for(auto table = argvs.management_xtable.rbegin(); table != argvs.management_xtable.rend(); table++){
                if(get<0>(table->first) == "common"){
                    argvs.management_xtable[table->first] = table1;
                    exf = true;
                    break;
                }
            }
        }

        if(sameTable == false){
            exf = false;
            for(auto table = argvs.management_xtable.rbegin(); table != argvs.management_xtable.rend(); table++){
                if(get<0>(table->first) != "common" && get<0>(table->first) == vtype && compare_time(get<1>(table->first), start_time2) <= 0){
                    argvs.management_xtable[table->first] = table2;
                    exf = true;
                    break;
                }
            }

            if(exf == false){
                for(auto table = argvs.management_xtable.rbegin(); table != argvs.management_xtable.rend(); table++){
                    if(get<0>(table->first) == "time_common" && compare_time(get<1>(table->first), start_time2) <= 0){
                        argvs.management_xtable[table->first] = table2;
                        exf = true;
                        break;
                    }
                }
            }

            //table検索(配送時刻がない場合)
            if(exf == false){
                for(auto table = argvs.management_xtable.rbegin(); table != argvs.management_xtable.rend(); table++){
                    if(get<0>(table->first) == "common"){
                        argvs.management_xtable[table->first] = table2;
                        exf = true;
                        break;
                    }
                }
            }
        }

        for(auto table = argvs.management_xtable.rbegin(); table != argvs.management_xtable.rend(); table++){
            int table_size = argvs.management_xtable[table->first].at(0).size();
            if(table1_size >= table_size){
                int table_second_size = table->second.size();
                for(int idx=0; idx < table_second_size; idx++){
                    argvs.management_xtable[table->first].at(idx).push_back(0);
                }
            }
            table_size = argvs.management_xtable[table->first].size();
            if(table1_size >= table_size){
                std::vector<double> tt(table->second.at(0).size(), 0); 
                argvs.management_xtable[table->first].push_back(tt);
            }
        }
    }

	// ##### etable #####
    if(argvs.management_etable.size() > 0){
        // table検索(配送時刻がある場合)
        exf = false;
        for(auto table = argvs.management_etable.rbegin(); table != argvs.management_etable.rend(); table++){
            if(get<0>(table->first) != "common" && get<1>(table->first) == vtype && compare_time(get<2>(table->first), start_time1) <= 0){
                table1 = table->second;
                exf = true;
                break;
            }
        }

        if(exf == false){
            for(auto table = argvs.management_etable.rbegin(); table != argvs.management_etable.rend(); table++){
                if(get<0>(table->first) == "time_common" && compare_time(get<2>(table->first), start_time1) <= 0){
                    table1 = table->second;
                    exf = true;
                    break;
                }
            }
        }

        //table検索(配送時刻がない場合)
        if(exf == false){
            for(auto table = argvs.management_etable.rbegin(); table != argvs.management_etable.rend(); table++){
                if(get<0>(table->first) == "common"){
                    table1 = table->second;
                    exf = true;
                    break;
                }
            }
        }

        exf = false;
        for(auto table = argvs.management_etable.rbegin(); table != argvs.management_etable.rend(); table++){
            if(get<0>(table->first) != "common" && get<1>(table->first) == vtype && compare_time(get<2>(table->first), start_time2) <= 0){
                table2 = table->second;
                exf = true;
                break;
            }
        }

        if(exf == false){
            for(auto table = argvs.management_etable.rbegin(); table != argvs.management_etable.rend(); table++){
                if(get<0>(table->first) == "time_common" && compare_time(get<2>(table->first), start_time2) <= 0){
                    table2 = table->second;
                    exf = true;
                    break;
                }
            }
        }

        //table検索(配送時刻がない場合)
        if(exf == false){
            for(auto table = argvs.management_etable.rbegin(); table != argvs.management_etable.rend(); table++){
                if(get<0>(table->first) == "common"){
                    table2 = table->second;
                    exf = true;
                    break;
                }
            }
        }

        to_val = table1.at(from_spotno).at(to_spotno);

        if(table1 == table2){
            sameTable = true;
        }
        else{
            sameTable = false;
        }

        // from_timeの追加(列追加)
        table1_size = table1.size();
        for(int idx=0; idx < table1_size; idx++){
            table1.at(idx).push_back(0);
        }
        if(to_time!=0.0){
            table1.at(from_spotno).at(tl - 1) = to_val / 2;
        }
        else{
            table1.at(from_spotno).at(tl - 1) = 0;
        }

        if(sameTable == true){
            // to_timeの追加(行追加)
            std::vector<double> te(table1.at(0).size(), 0); 
            if(to_time!=0.0){
                te.at(to_spotno) = to_val / 2;
            }
            else{
                te.at(to_spotno) = 0;
            }
            table1.push_back(te);
        }
        else{
            // to_timeの追加(行追加)
            std::vector<double> te(table2.at(0).size(), 0); 
            if(to_time!=0.0){
                te.at(to_spotno) = to_val / 2;
            }
            else{
                te.at(to_spotno) = 0;
            }
            table2.push_back(te);
        }

        exf = false;
        for(auto table = argvs.management_etable.rbegin(); table != argvs.management_etable.rend(); table++){
            if(get<0>(table->first) != "common" && get<0>(table->first) == vtype && compare_time(get<2>(table->first), start_time1) <= 0){
                argvs.management_etable[table->first] = table1;
                exf = true;
                break;
            }
        }

        if(exf == false){
            for(auto table = argvs.management_etable.rbegin(); table != argvs.management_etable.rend(); table++){
                if(get<0>(table->first) == "time_common" && compare_time(get<2>(table->first), start_time1) <= 0){
                    argvs.management_etable[table->first] = table1;
                    exf = true;
                    break;
                }
            }
        }

        //table検索(配送時刻がない場合)
        if(exf == false){
            for(auto table = argvs.management_etable.rbegin(); table != argvs.management_etable.rend(); table++){
                if(get<0>(table->first) == "common"){
                    argvs.management_etable[table->first] = table1;
                    exf = true;
                    break;
                }
            }
        }

        if(sameTable == false){
            exf = false;
            for(auto table = argvs.management_etable.rbegin(); table != argvs.management_etable.rend(); table++){
                if(get<0>(table->first) != "common" && get<0>(table->first) == vtype && compare_time(get<2>(table->first), start_time2) <= 0){
                    argvs.management_etable[table->first] = table2;
                    exf = true;
                    break;
                }
            }

            if(exf == false){
                for(auto table = argvs.management_etable.rbegin(); table != argvs.management_etable.rend(); table++){
                    if(get<0>(table->first) == "time_common" && compare_time(get<2>(table->first), start_time2) <= 0){
                        argvs.management_etable[table->first] = table2;
                        exf = true;
                        break;
                    }
                }
            }

            //table検索(配送時刻がない場合)
            if(exf == false){
                for(auto table = argvs.management_etable.rbegin(); table != argvs.management_etable.rend(); table++){
                    if(get<0>(table->first) == "common"){
                        argvs.management_etable[table->first] = table2;
                        exf = true;
                        break;
                    }
                }
            }
        }

        for(auto table = argvs.management_etable.rbegin(); table != argvs.management_etable.rend(); table++){
            int table_size = argvs.management_etable[table->first].at(0).size();
            if(table1_size >= table_size){
                int table_second_size = table->second.size();
                for(int idx=0; idx < table_second_size; idx++){
                    argvs.management_etable[table->first].at(idx).push_back(0);
                }
            }
            table_size = argvs.management_etable[table->first].size();
            if(table1_size >= table_size){
                std::vector<double> tt(table->second.at(0).size(), 0); 
                argvs.management_etable[table->first].push_back(tt);
            }
        }
    }

	return last_spotno;
}

Vehicle remove_breakspot_pat3(vector<Customer> &customers, Vehicle &veh, vector<Vehicle> &vehicles, Cppargs &argvs, bool cuserase=true)
/**
 * @brief パターン3で休憩スポットを削除する関数
 * @param[in] veh
 * @param[in] vehhicles
 * @param[in] argvs
 * @return vehicle
 * @details 休憩スポットを削除する関数
 */
{
	std::vector<int> bspotnos;
	std::vector<double> blvs;
	
	int rlen = veh.route.size();
    int br_pattern = get_breakspot_pattern(vehicles, argvs);

	for(int ridx=rlen-1; ridx>=0; ridx--){
        int vri = veh.route.at(ridx);
        if(customers.at(vri).custno[0] == 'b' && customers.at(vri).spotid == ""){
            veh.route.erase(veh.route.begin() + ridx);
            bspotnos.push_back(customers.at(vri).spotno);
            blvs.push_back(veh.lv.at(ridx));

            if(br_pattern == 3){
                customers.erase(customers.begin() + vri);
                // ##### customer idxの更新 #####
                int veh_route_size = veh.route.size();
                for(int ridx2=0; ridx2 < veh_route_size; ridx2++){
                    if(veh.route.at(ridx2) > vri){
                        veh.route.at(ridx2)--;
                    }
                    int tsize = veh.takebreak.size();
                    for(int tidx=0; tidx<tsize; tidx++){
                        if(veh.takebreak.at(tidx) >  vri){
                            veh.takebreak.at(tidx)--;
                        }
                    }
                }
                int vehicle_size = vehicles.size();
                for(int vidx=0; vidx < vehicle_size; vidx++){
                    int vehicles_vidx_route_size = vehicles.at(vidx).route.size();
                    for(int ridx2=0; ridx2 < vehicles_vidx_route_size; ridx2++){
                        if(vehicles.at(vidx).route.at(ridx2) > vri){
                            vehicles.at(vidx).route.at(ridx2)--;
                        }
                    }
                    int tsize = veh.takebreak.size();
                    for(int tidx=0; tidx<tsize; tidx++){
                        if(vehicles.at(vidx).takebreak.at(tidx) >  vri){
                            vehicles.at(vidx).takebreak.at(tidx)--;
                        }
                    }
                }
                // ##### tieup customer idxの更新 #####
                if(argvs.bulkShipping_flag==true){
                    int customers_size = customers.size();
                    for(int idx=0; idx < customers_size; idx++){
                        int tieup_customer_size = customers.at(idx).tieup_customer.size();
                        for(int tidx=0; tidx < tieup_customer_size; tidx++){
                            if(customers.at(idx).tieup_customer.at(tidx) > vri){
                                customers.at(idx).tieup_customer.at(tidx)--;
                            }
                        }
                    }
                }
            }
        }
    }

	string vtype = veh.vtype;

    int bspotnos_size = bspotnos.size();
    for(int i=0; i<bspotnos_size-1; i++){
        for(int j=i+1; j<bspotnos_size; j++){
            if(bspotnos.at(i) > bspotnos.at(j)){
                swap(bspotnos.at(i), bspotnos.at(j));
                swap(blvs.at(i), blvs.at(j));
            }
        }
    }

    for(int idx=bspotnos_size-1; idx>=0; idx--){
        int sn = bspotnos.at(idx);
		double start_time1 = blvs.at(idx);
		double start_time2 = start_time1 + argvs.break_time / 60;

	    // ##### ttable #####
        for(auto table = argvs.management_ttable.rbegin(); table != argvs.management_ttable.rend(); table++){
            // from_timeの削除(列削除)
            int table_second_size = table->second.size();
            for(int idx=0; idx < table_second_size; idx++){
                argvs.management_ttable[table->first].at(idx).erase(table->second.at(idx).begin() + sn);
            }
            
            // to_timeの削除(行削除)
            argvs.management_ttable[table->first].erase(table->second.begin() + sn);
        }

	    // ##### dtable #####
        for(auto table = argvs.management_dtable.rbegin(); table != argvs.management_dtable.rend(); table++){
            // from_timeの削除(列削除)
            int table_second_size = table->second.size();
            for(int idx=0; idx < table_second_size; idx++){
                argvs.management_dtable[table->first].at(idx).erase(table->second.at(idx).begin() + sn);
            }
            
            // to_timeの削除(行削除)
            argvs.management_dtable[table->first].erase(table->second.begin() + sn);
        }

	    // ##### xtable #####
        if(argvs.management_xtable.size()>0){
            for(auto table = argvs.management_xtable.rbegin(); table != argvs.management_xtable.rend(); table++){
                // from_timeの削除(列削除)
                int table_second_size = table->second.size();
                for(int idx=0; idx < table_second_size; idx++){
                    argvs.management_xtable[table->first].at(idx).erase(table->second.at(idx).begin() + sn);
                }
                
                // to_timeの削除(行削除)
                argvs.management_xtable[table->first].erase(table->second.begin() + sn);
            }
        }

	    // ##### etable #####
        if(argvs.management_etable.size()>0){
            for(auto table = argvs.management_etable.rbegin(); table != argvs.management_etable.rend(); table++){
                // from_timeの削除(列削除)
                int table_second_size = table->second.size();
                for(int idx=0; idx < table_second_size; idx++){
                    argvs.management_etable[table->first].at(idx).erase(table->second.at(idx).begin() + sn);
                }
                
                // to_timeの削除(行削除)
                argvs.management_etable[table->first].erase(table->second.begin() + sn);
            }
        }

        int clen = customers.size();
        for(int cidx=0; cidx<clen; cidx++){
            if(customers.at(cidx).spotno > sn){
                customers.at(cidx).spotno--;
            }
        }
    }

    veh.update_arrtime_tw(customers, argvs.management_ttable, argvs.break_flag);
    veh.update_load_onbrd(customers);

	return veh;
}


Vehicle remove_breakspot(vector<Customer> &customers, Vehicle &veh, vector<Vehicle> vehicles, Cppargs &argvs)
/**
 * @brief 休憩スポットを削除する関数
 * @param[in] customers
 * @param[in] veh
 * @param[in] argvs
 * @return vehicle
 * @details 休憩スポットを削除する関数
 */
{
	bool no_spotid = false;
	vector<int> rmlst;

    int route_size = veh.route.size();
	for(int rcnt=0; rcnt<route_size; rcnt++){
        int r = veh.route.at(rcnt);
		if(customers.at(r).custno.at(0)=='b'){
			if(customers.at(r).spotid != ""){
				rmlst.push_back(rcnt);
            }
			else{
				no_spotid = true;
            }
        }
    }

    int rmlst_size = rmlst.size();
    for(int rmcnt=rmlst_size-1; rmcnt>=0; rmcnt--){
        veh.route.erase(veh.route.begin() + rmlst.at(rmcnt));
    }

    veh.update_arrtime_tw(customers, argvs.management_ttable, argvs.break_flag);
    veh.update_load_onbrd(customers);
	if(no_spotid){
        remove_breakspot_pat3(customers, veh, vehicles, argvs, false);
    }

	return veh;
}

bool pattern1(vector<Customer> &customers, Vehicle &veh, vector<Vehicle> vehicles, Cppargs &argvs)
/**
 * @brief 休憩スポットを挿入する関数パターン1
 * @param[in] customers
 * @param[in] veh
 * @param[in] argvs
 * @return vehicle
 * @details 休憩スポットを挿入する関数
 */
{
	// 一旦全ての休憩スポットを削除する
	veh = remove_breakspot(customers, veh, vehicles, argvs);

	// 休憩ノードを休憩期間開始時刻順に並べる
    vector<int>takebreak_sorted = veh.takebreak;
    int takebreak_sorted_size = takebreak_sorted.size();
    for(int i=0; i<takebreak_sorted_size-1; i++){
        for(int n=i+1; n<takebreak_sorted_size; n++){
            if(customers.at(takebreak_sorted.at(i)).ready_fs > customers.at(takebreak_sorted.at(n)).ready_fs){
                std::swap(takebreak_sorted.at(i), takebreak_sorted.at(n));
            }
        }
    }

    int bbreak_p = 0;

	double abreak_start = 0.0;
    double abreak_end = 0.0;
    double bbreak_start = 0.0;
    double bbreak_end = 0.0;

	//休憩期間開始時刻を初めて超える配送先の直前に休憩ノードを追加
	for(int i=0;  i<takebreak_sorted_size; i++){
        Vehicle tveh = dcopy(veh);
		vector<double> arr_ts = veh.arr;
		vector<double> lv_ts = veh.lv;
		int arr_t_len = arr_ts.size();

        vector<int> no_park_idx1 = get_no_park_idx(veh, customers);
		vector<bool> parkt_time_flag = get_parkt_time_flag(customers, veh.route);

		//開始前の休憩は捨てる
        if(customers.at(takebreak_sorted.at(i)).due_fs < lv_ts.at(0)){
            continue;
        }

		// 休憩期間開始時刻を初めて超える配送先を求める
        int arr_p = 1;
        bool break_f = false;
		for(; arr_p<arr_t_len; arr_p++){
            double _bstime;
			if((parkt_time_flag.at(arr_p)==false) || (vector_finder(no_park_idx1, arr_p)==true)){
                _bstime = max(arr_ts.at(arr_p), customers.at(veh.route.at(arr_p)).ready_fs);
            }
            else{
                _bstime = max(arr_ts.at(arr_p) + customers.at(veh.route.at(arr_p)).parkt_arrive, customers.at(veh.route.at(arr_p)).ready_fs) + customers.at(veh.route.at(arr_p)).parkt_open;
            }
			if(_bstime > customers.at(takebreak_sorted.at(i)).ready_fs){
                break_f = true;
                break;
            }
        }
        if(break_f == false){
			// 休憩開始時間より早く配送が終了した
			continue;   //次の休憩スポットの挿入処理へ
        }
		
        // 直前に休憩ノードを追加
		while(true){

			if(arr_p <= bbreak_p){
				// 一つ前の休憩時間にまで戻ってしまった
                return false;
            }
			// 休憩ノードを追加
			if(customers.at(takebreak_sorted.at(i)).spotid == ""){
				// spotid無し対応
				int from_spotno = customers.at(veh.route.at(arr_p - 1)).spotno;
				int to_spotno = customers.at(veh.route.at(arr_p)).spotno;
				double tdiff = arr_ts.at(arr_p) - lv_ts.at(arr_p - 1);
				double from_time;
                from_time = customers.at(takebreak_sorted.at(i)).ready_fs - lv_ts.at(arr_p - 1);
				double to_time = tdiff - from_time;
				int last_spotno = add_table(veh, from_spotno, to_spotno, from_time, to_time, arr_p, argvs);	//時間テーブルなどに休憩スポットを追加
				customers.at(takebreak_sorted.at(i)).spotno = last_spotno;
            }
            
            if(argvs.bulkShipping_flag == false){
                veh.route.insert(veh.route.begin()+arr_p, takebreak_sorted.at(i));
            }
            else{
                string _custno = customers.at(takebreak_sorted.at(i)).custno;
                int custsize = customers.size();
                for(int idx=0; idx < custsize; idx++){
                    if(customers.at(takebreak_sorted.at(idx)).tieup_customer.size() > 0){
                        if(customers.at(takebreak_sorted.at(idx)).custno == _custno){
                            veh.route.insert(veh.route.begin()+arr_p, takebreak_sorted.at(idx));
                            break;
                        }
                    }
                }
            }

            veh.update_arrtime_tw(customers, argvs.management_ttable, argvs.break_flag);
            veh.update_load_onbrd(customers);
			abreak_start = veh.arr.at(arr_p);
			abreak_end = veh.lv.at(arr_p);
			
            no_park_idx1 = get_no_park_idx(veh, customers);
    		parkt_time_flag = get_parkt_time_flag(customers, veh.route);

            double  _bstime_b;
			if((parkt_time_flag.at(arr_p)==false) || (vector_finder(no_park_idx1, arr_p)==true)){
                _bstime_b = max(veh.arr.at(arr_p), customers.at(veh.route.at(arr_p)).ready_fs);
            }
            else{
                _bstime_b = max(veh.arr.at(arr_p) + customers.at(veh.route.at(arr_p)).parkt_arrive, customers.at(veh.route.at(arr_p)).ready_fs) + customers.at(veh.route.at(arr_p)).parkt_open;
            }
            // dueチェック
			if(_bstime_b > customers.at(takebreak_sorted.at(i)).due_fs){
				// 休憩スポット到着が間に合わなかった
				veh = tveh;
				arr_p -= 1;
                continue;
            }
			
            // 通行止チェック
            bool pick_opt_flag1 = get_pick_opt_status(customers, veh, argvs);

            bool no_park_flag1 = true;

            vector<int> depo_idx1_temp;
            int veh_route_size = veh.route.size();
            for(int i = 0; i < veh_route_size-1; i++){
                if(customers.at(veh.route.at(i)).custno == "0"){
                    depo_idx1_temp.emplace_back(i); //routeにおけるデポのインデックス
                }
            }
            depo_idx1_temp.emplace_back(veh.route.size()-1); //最終訪問先を追加

            for(int i = depo_idx1_temp.at(0)+1; i < depo_idx1_temp.at(1); i++){
                if(customers.at(veh.route.at(i)).requestType == "DELIVERY"){
                    no_park_flag1 = false;
                    break;
                }
            }

            map<int, double> servt_dict1;
            //配送時間
            int depo_idx1_temp_size=depo_idx1_temp.size();
            for(int i = 0; i < depo_idx1_temp_size-1; i++){
                servt_dict1.emplace(depo_idx1_temp.at(i), 0);
                double servt = 0;

                for(int j = depo_idx1_temp.at(i)+1; j < depo_idx1_temp.at(i+1); j++){
                    if(customers.at(veh.route.at(j)).requestType == "DELIVERY" && customers.at(veh.route.at(j)).custno != customers.at(veh.route.at(j-1)).custno
                       && customers.at(veh.route.at(j)).custno.at(0) != 'b'){
                        servt += customers.at(veh.route.at(j)).depotservt;
                    }
                }

                if(customers.at(veh.route.at(depo_idx1_temp.at(i)+1)).custno != "0"){
                    servt_dict1.at(depo_idx1_temp.at(i)) += servt+veh.depotservt; //デポが連続していない場合
                }
                else{
                    servt_dict1.at(depo_idx1_temp.at(i)) += servt; //デポが連続している場合
                }
            }

            //集荷時間
            servt_dict1.emplace(distance(veh.route.begin(), find(veh.route.begin(), veh.route.end(), veh.route.at(veh.route.size()-1))), 0);
            for(int i = 1; i < depo_idx1_temp_size; i++){
                double servt = 0;

                for(int j = depo_idx1_temp.at(i-1)+1; j < depo_idx1_temp.at(i); j++){
                    if(customers.at(veh.route.at(j)).requestType == "PICKUP" && customers.at(veh.route.at(j)).custno != customers.at(veh.route.at(j-1)).custno){
                        servt += customers.at(veh.route.at(j)).depotservt;
                    }
                }

                servt_dict1.at(depo_idx1_temp.at(i)) += servt;
            }

            double adist = veh.totalcost;
            double power = 0;
            double exp = 0;

            bool passable = check_impassability(veh, customers, veh.lv.at(0), 1, veh.route.size()-1, servt_dict1, adist, power, exp, no_park_idx1, pick_opt_flag1, argvs);

			if(passable){
                bbreak_p = arr_p;
				break;	// 通行可能だったら挿入成功
            }
			else{
				// 通行止なので次を探索
				veh = dcopy(tveh);
				arr_p -= 1;
            }
        }
    }

	return true;
}

bool pattern2(vector<Customer> &customers, Vehicle &veh, vector<Vehicle> vehicles, Cppargs &argvs)
/**
 * @brief 休憩スポットを挿入する関数パターン2
 * @param[in] customers
 * @param[in] veh
 * @param[in] argvs
 * @return vehicle
 * @details 休憩スポットを挿入する関数
 */
{
	// 一旦全ての休憩スポットを削除する
	veh = remove_breakspot(customers, veh, vehicles, argvs);

	// 休憩ノードを休憩期間開始時刻順に並べる
    vector<int>takebreak_sorted = veh.takebreak;
    int takebreak_sorted_size = takebreak_sorted.size();
    for(int i=0; i<takebreak_sorted_size-1; i++){
        for(int n=i+1; n<takebreak_sorted_size; n++){
            if(customers.at(takebreak_sorted.at(i)).ready_fs > customers.at(takebreak_sorted.at(n)).ready_fs){
                std::swap(takebreak_sorted.at(i), takebreak_sorted.at(n));
            }
        }
    }

    // 休憩間隔用変数の初期化
    double abreak_start = 0.0;
    double abreak_end = 0.0;
    double bbreak_start = 0.0;
    double bbreak_end = 0.0;

    int bbreak_p = 0;

	// 休憩期間開始時刻を初めて超える配送先の直前に休憩ノードを追加
	for(int i=0;  i<takebreak_sorted_size; i++){
        Vehicle tveh = dcopy(veh);
		vector<double> arr_ts = veh.arr;
		vector<double> lv_ts = veh.lv;
		int arr_t_len = arr_ts.size();

        vector<int> no_park_idx1 = get_no_park_idx(veh, customers);
		vector<bool> parkt_time_flag = get_parkt_time_flag(customers, veh.route);

		//開始前の休憩は捨てる
        if(customers.at(takebreak_sorted.at(i)).due_fs < lv_ts.at(0)){
            continue;
        }

        // 休憩期間開始時刻を初めて超える配送先を求める
        int arr_p = 1;
        bool break_f = false;
        double _bstime = 0.0;
		for(; arr_p<arr_t_len; arr_p++){
            double _bstime;
			if((parkt_time_flag.at(arr_p)==false) || (vector_finder(no_park_idx1, arr_p)==true)){
                _bstime = max(arr_ts.at(arr_p), customers.at(veh.route.at(arr_p)).ready_fs);
            }
            else{
                _bstime = max(arr_ts.at(arr_p) + customers.at(veh.route.at(arr_p)).parkt_arrive, customers.at(veh.route.at(arr_p)).ready_fs) + customers.at(veh.route.at(arr_p)).parkt_open;
            }
			if(_bstime > customers.at(takebreak_sorted.at(i)).ready_fs){
                break_f = true;
                break;
            }
        }
        if(break_f == false){
			// 休憩開始時間より早く配送が終了した
			continue;   //次の休憩スポットの挿入処理へ
        }
		
        // 直前に休憩ノードを追加
		while(true){

			if(arr_p <= bbreak_p){
				// 一つ前の休憩時間にまで戻ってしまった
                return false;
            }
			// 休憩ノードを追加
			if(customers.at(takebreak_sorted[i]).spotid == ""){
				// spotid無し対応
				int from_spotno = customers.at(veh.route.at(arr_p - 1)).spotno;
				int to_spotno = customers.at(veh.route.at(arr_p)).spotno;
				double tdiff = arr_ts.at(arr_p) - lv_ts.at(arr_p - 1);
                if(tdiff == 0.0){
                    // 移動時間が0の場合は一つ前に行く
                    arr_p -= 1;
                    continue;
                }
                double from_time = 0.0;
				if((_bstime <= bbreak_end + argvs.break_interval)&&(customers.at(takebreak_sorted[i]).ready_fs > lv_ts.at(arr_p - 1))){
					// 移動中に休憩間隔を超えずかつ休憩開始時間枠より前に出発する場合
                    if(customers.at(veh.route.at(arr_p - 1)).spotid == customers.at(takebreak_sorted.at(i)).spotid){
                        from_time = customers.at(takebreak_sorted.at(i)).ready_fs - lv_ts.at(arr_p - 1);
                    }
                    else{
                        from_time = customers.at(takebreak_sorted.at(i)).ready_fs - (lv_ts[arr_p - 1] + customers.at(takebreak_sorted.at(i)).parkt_arrive + customers.at(takebreak_sorted.at(i)).parkt_open);
                    }
                }
				else if((_bstime > bbreak_end + argvs.break_interval)&&(customers.at(takebreak_sorted[i]).ready_fs > lv_ts.at(arr_p - 1))){
					// 移動中に休憩間隔を超えてかつ休憩開始時間枠より前に出発する場合
					from_time = bbreak_end + argvs.break_interval - lv_ts.at(arr_p - 1);
                }
				double to_time = tdiff - from_time;
				int last_spotno = add_table(veh, from_spotno, to_spotno, from_time, to_time, arr_p, argvs);
				customers.at(takebreak_sorted[i]).spotno = last_spotno;
            }
            if(argvs.bulkShipping_flag == false){
                veh.route.insert(veh.route.begin()+arr_p, takebreak_sorted.at(i));
            }
            else{
                string _custno = customers.at(takebreak_sorted.at(i)).custno;
                int custsize = customers.size();
                for(int idx=0; idx < custsize; idx++){
                    if(customers.at(takebreak_sorted.at(idx)).tieup_customer.size() > 0){
                        if(customers.at(takebreak_sorted.at(idx)).custno == _custno){
                            veh.route.insert(veh.route.begin()+arr_p, takebreak_sorted.at(idx));
                            break;
                        }
                    }
                }
            }
            veh.update_arrtime_tw(customers, argvs.management_ttable, argvs.break_flag);
            veh.update_load_onbrd(customers);
			abreak_start = veh.arr.at(arr_p);
			abreak_end = veh.lv.at(arr_p);

            no_park_idx1 = get_no_park_idx(veh, customers);
            parkt_time_flag = get_parkt_time_flag(customers, veh.route);

            double  _bstime_b;
			if((parkt_time_flag.at(arr_p)==false) || (vector_finder(no_park_idx1, arr_p)==true)){
                _bstime_b = max(veh.arr.at(arr_p), customers.at(veh.route.at(arr_p)).ready_fs);
            }
            else{
                _bstime_b = max(veh.arr.at(arr_p) + customers.at(veh.route.at(arr_p)).parkt_arrive, customers.at(veh.route.at(arr_p)).ready_fs) + customers.at(veh.route.at(arr_p)).parkt_open;
            }

            // dueチェック
			if(_bstime_b > customers.at(takebreak_sorted[i]).due_fs){
				// 休憩スポット到着が間に合わなかった
				veh = tveh;
				arr_p -= 1;
                continue;
            }
			
			// intervalチェック
			abreak_start = _bstime_b;
			abreak_end= veh.lv.at(arr_p);
			if(bbreak_end + argvs.break_interval < abreak_start){
				// 休憩間隔を超えてしまった
				veh = tveh;
				arr_p -= 1;
				continue;
            }

            // 通行止チェック
            bool pick_opt_flag1 = get_pick_opt_status(customers, veh, argvs);

            bool no_park_flag1 = true;

            vector<int> depo_idx1_temp;
            int veh_route_size = veh.route.size();
            for(int i = 0; i < veh_route_size-1; i++){
                if(customers.at(veh.route.at(i)).custno == "0"){
                    depo_idx1_temp.emplace_back(i); //routeにおけるデポのインデックス
                }
            }
            depo_idx1_temp.emplace_back(veh.route.size()-1); //最終訪問先を追加

            for(int i = depo_idx1_temp.at(0)+1; i < depo_idx1_temp.at(1); i++){
                if(customers.at(veh.route.at(i)).requestType == "DELIVERY"){
                    no_park_flag1 = false;
                    break;
                }
            }

            map<int, double> servt_dict1;
            //配送時間
            int depo_idx1_temp_size = depo_idx1_temp.size();
            for(int i = 0; i < depo_idx1_temp_size-1; i++){
                servt_dict1.emplace(depo_idx1_temp.at(i), 0);
                double servt = 0;

                for(int j = depo_idx1_temp.at(i)+1; j < depo_idx1_temp.at(i+1); j++){
                    if(customers.at(veh.route.at(j)).requestType == "DELIVERY" && customers.at(veh.route.at(j)).custno != customers.at(veh.route.at(j-1)).custno
                       && customers.at(veh.route.at(j)).custno.at(0) != 'b'){
                        servt += customers.at(veh.route.at(j)).depotservt;
                    }
                }

                if(customers.at(veh.route.at(depo_idx1_temp.at(i)+1)).custno != "0"){
                    servt_dict1.at(depo_idx1_temp.at(i)) += servt+veh.depotservt; //デポが連続していない場合
                }
                else{
                    servt_dict1.at(depo_idx1_temp.at(i)) += servt; //デポが連続している場合
                }
            }

            //集荷時間
            servt_dict1.emplace(distance(veh.route.begin(), find(veh.route.begin(), veh.route.end(), veh.route.at(veh.route.size()-1))), 0);
            for(int i = 1; i < depo_idx1_temp_size; i++){
                double servt = 0;

                for(int j = depo_idx1_temp.at(i-1)+1; j < depo_idx1_temp.at(i); j++){
                    if(customers.at(veh.route.at(j)).requestType == "PICKUP" && customers.at(veh.route.at(j)).custno != customers.at(veh.route.at(j-1)).custno){
                        servt += customers.at(veh.route.at(j)).depotservt;
                    }
                }

                servt_dict1.at(depo_idx1_temp.at(i)) += servt;
            }

            double adist = veh.totalcost;
            double power = 0;
            double exp = 0;

            bool passable = check_impassability(veh, customers, veh.lv.at(0), 1, veh.route.size()-1, servt_dict1, adist, power, exp, no_park_idx1, pick_opt_flag1, argvs);

			if(passable){
                bbreak_p = arr_p;
                bbreak_start = abreak_start;	// 最新の休憩開始時間と終了時間の保存
                bbreak_end = abreak_end;
				break;	// 通行可能だったら挿入成功
            }
			else{
				// 通行止なので次を探索
				veh = dcopy(tveh);
				arr_p -= 1;
            }
        }
    }

	// 配送終了時間まで休憩間隔を守られているかチェック
    double lv_last = 0.0;
    if(argvs.lastc_flag == true){
        int veh_route_size = veh.route.size();
        int alast_idx = veh_route_size;
        for(int i = veh_route_size-1; i > 0; i--){
            if((customers.at(veh.route.at(i)).custno!="0")&&(customers.at(veh.route.at(i)).custno.at(0)!='b')&&(i!=veh_route_size-1)){
                break;
            }
            else{
                alast_idx = i;
            }
        }
        lv_last = veh.lv.at(alast_idx-1);
    }
    else{
        int veh_route_size = veh.route.size();
        lv_last = veh.lv.at(veh_route_size-1);
    }

    if(bbreak_end + argvs.break_interval < lv_last){
   		// 最後の休憩から配送終了時間までの時間が休憩間隔を超えてしまった
        return false;
    }

    return true;
}

bool pattern3(vector<Customer> &customers, Vehicle &veh, vector<Vehicle> &vehicles, Cppargs &argvs)
/**
 * @brief 休憩スポットを挿入する関数パターン3
 * @param[in] customers
 * @param[in] veh
 * @param[in] argvs
 * @return vehicle
 * @details 休憩スポットを挿入する関数
 */
{
    // 一旦全ての休憩スポットを削除し、時間テーブルから追加した休憩スポットのデータを削除する
    veh = remove_breakspot_pat3(customers, veh, vehicles, argvs);

    int bbreak_p = 0;

    // 休憩間隔用変数の初期化
    double abreak_start = 0.0;
    double abreak_end = 0.0;
    double bbreak_start = 0.0;
    double bbreak_end = 0.0;

    // 挿入した休憩スポットのカウンタ
    int br_cnt = 0;

    // 最終立ち寄り先のインデックスの取得
    int alast_idx = veh.route.size();
    if(argvs.lastc_flag == true){
        // lastc
        int alast_idx = veh.route.size();
        for(int i=veh.route.size()-1; i>=0; i--){
            if((customers.at(veh.route.at(i)).custno!="0") && (customers.at(veh.route.at(i)).custno.at(0)!='b') &&  (i!=veh.route.size()-1)){
                break;
            }
            else{
                alast_idx = i;
            }
        }
    }

    bool exf = false;

	bool mainloop_breakf = true;
    while(mainloop_breakf){
        Vehicle tveh = dcopy(veh);
        vector<double> arr_ts = veh.arr;
        int arr_t_len = arr_ts.size();
        vector<double> lv_ts = veh.lv;

        vector<int> no_park_idx1 = get_no_park_idx(veh, customers);
		vector<bool> parkt_time_flag = get_parkt_time_flag(customers, veh.route);

        // 休憩期間開始時刻を初めて超える配送先を求める
        int arr_p = 1;
        exf = false;
        for(; arr_p < arr_t_len; arr_p++){
            double _bstime;
			if((parkt_time_flag.at(arr_p)==false) || (vector_finder(no_park_idx1, arr_p)==true)){
                _bstime = max(arr_ts.at(arr_p), customers.at(veh.route.at(arr_p)).ready_fs);
            }
            else{
                _bstime = max(arr_ts.at(arr_p) + customers.at(veh.route.at(arr_p)).parkt_arrive, customers.at(veh.route.at(arr_p)).ready_fs) + customers.at(veh.route.at(arr_p)).parkt_open;
            }
			if(_bstime >= bbreak_end + argvs.break_interval){
                exf = true;
                break;
            }
        }
        if(exf == false){
            // 休憩スポットの挿入を完了した
            break;
        }

        // 直前に休憩ノードを追加
        exf = false;
        while(true){
            if(arr_p <= bbreak_p){
                // 一つ前の休憩時間にまで戻ってしまった
                return false;;
            }

            if((lv_ts.at(arr_p) == bbreak_end + argvs.break_interval)&&(lv_ts.at(arr_p) == bbreak_end + argvs.break_interval)){
                // 休憩時間=休憩間隔の場合
				mainloop_breakf = false;
                break;
            }

            if(lv_ts.at(arr_p - 1) > bbreak_end + argvs.break_interval){
                // 配送先異滞在中に休憩間隔が過ぎた
                arr_p -= 1;
                continue;
            }

            if(veh.st.at(arr_p) - arr_ts.at(arr_p) >= argvs.break_interval){
                // もしparktが休憩間隔より長い場合は一つ前に行く
                arr_p -= 1;
                continue;
            }

            // 休憩スポットを作成して追加
            Customer br_spot = create_customer('b'+std::to_string(veh.vehno)+'_'+std::to_string(br_cnt), argvs);
            int br_spot_idx = customers.size();
            double tdiff = arr_ts.at(arr_p) - lv_ts.at(arr_p - 1);
            if(tdiff == 0.0){
                // 移動時間が0の場合は一つ前に行く
				if(customers.at(veh.route.at(arr_p)).spotno != customers.at(veh.route.at(arr_p-1)).spotno){
                    arr_p -= 1;
                    continue;
                }
            }
            int from_spotno = customers.at(veh.route.at(arr_p - 1)).spotno;
            int to_spotno = customers.at(veh.route.at(arr_p)).spotno;
			double from_time = min(bbreak_end + argvs.break_interval, arr_ts.at(arr_p)) - lv_ts.at(arr_p - 1);
            double to_time = tdiff - from_time;
            int last_spotno = add_table(veh, from_spotno, to_spotno, from_time, to_time, arr_p, argvs);	// 時間テーブルなどに休憩スポットを追加
            br_spot.spotno = last_spotno;
            if(argvs.bulkShipping_flag == false){
                customers.push_back(br_spot);
                veh.route.insert(veh.route.begin()+arr_p, br_spot_idx);
            }
            else{
                int tcnum = 0;
                vector<int> _tc;
                _tc.push_back(br_spot_idx);
                Customer tied_customers(
                    _tc,
                    br_spot.reje_ve,
                    br_spot.custno,
                    br_spot.spotid,
                    br_spot.spotno,
                    br_spot.y,
                    br_spot.x,
                    br_spot.dem,
                    br_spot.dem2,
                    br_spot.servt,
                    br_spot.e_rate,
                    br_spot.parkt_arrive,
                    br_spot.parkt_open,
                    br_spot.depotservt,
                    br_spot.ready_fs,
                    br_spot.due_fs,
                    br_spot.requestType,
                    br_spot.deny_unass,
                    br_spot.changecust
                );
                customers.push_back(tied_customers);
                veh.route.insert(veh.route.begin()+arr_p, br_spot_idx);
            }
            veh.update_arrtime_tw(customers, argvs.management_ttable, argvs.break_flag);
            veh.update_load_onbrd(customers);
            abreak_start = veh.arr.at(arr_p);
            abreak_end = veh.lv.at(arr_p);

            // 通行止チェック
            bool pick_opt_flag1 = get_pick_opt_status(customers, veh, argvs);

            no_park_idx1 = get_no_park_idx(veh, customers);

            bool no_park_flag1 = true;

            vector<int> depo_idx1_temp;
            int veh_route_size = veh.route.size();
            for(int i = 0; i < veh_route_size-1; i++){
                if(customers.at(veh.route.at(i)).custno == "0"){
                    depo_idx1_temp.emplace_back(i); //routeにおけるデポのインデックス
                }
            }
            depo_idx1_temp.emplace_back(veh.route.size()-1); //最終訪問先を追加

            for(int i = depo_idx1_temp.at(0)+1; i < depo_idx1_temp.at(1); i++){
                if(customers.at(veh.route.at(i)).requestType == "DELIVERY"){
                    no_park_flag1 = false;
                    break;
                }
            }

            map<int, double> servt_dict1;
            //配送時間
            int depo_idx1_temp_size = depo_idx1_temp.size();
            for(int i = 0; i < depo_idx1_temp_size-1; i++){
                servt_dict1.emplace(depo_idx1_temp.at(i), 0);
                double servt = 0;

                for(int j = depo_idx1_temp.at(i)+1; j < depo_idx1_temp.at(i+1); j++){
                    if(customers.at(veh.route.at(j)).requestType == "DELIVERY" && customers.at(veh.route.at(j)).custno != customers.at(veh.route.at(j-1)).custno
                       && customers.at(veh.route.at(j)).custno.at(0) != 'b'){
                        servt += customers.at(veh.route.at(j)).depotservt;
                    }
                }

                if(customers.at(veh.route.at(depo_idx1_temp.at(i)+1)).custno != "0"){
                    servt_dict1.at(depo_idx1_temp.at(i)) += servt+veh.depotservt; //デポが連続していない場合
                }
                else{
                    servt_dict1.at(depo_idx1_temp.at(i)) += servt; //デポが連続している場合
                }
            }

            //集荷時間
            servt_dict1.emplace(distance(veh.route.begin(), find(veh.route.begin(), veh.route.end(), veh.route.at(veh.route.size()-1))), 0);
            for(int i = 1; i < depo_idx1_temp_size; i++){
                double servt = 0;

                for(int j = depo_idx1_temp.at(i-1)+1; j < depo_idx1_temp.at(i); j++){
                    if(customers.at(veh.route.at(j)).requestType == "PICKUP" && customers.at(veh.route.at(j)).custno != customers.at(veh.route.at(j-1)).custno){
                        servt += customers.at(veh.route.at(j)).depotservt;
                    }
                }

                servt_dict1.at(depo_idx1_temp.at(i)) += servt;
            }

            double adist = veh.totalcost;
            double power = 0;
            double exp = 0;

            bool passable = check_impassability(veh, customers, veh.lv.at(0), 1, veh.route.size()-1, servt_dict1, adist, power, exp, no_park_idx1, pick_opt_flag1, argvs);

			if(passable){
                //exf = true;
                bbreak_p = arr_p;
                bbreak_start = abreak_start;	// 最新の休憩開始時間と終了時間の保存
                bbreak_end = abreak_end;
                br_cnt++;
				break;	// 通行可能だったら挿入成功
            }
			else{
				// 通行止なので次を探索
				veh = dcopy(tveh);
				arr_p -= 1;
            }
        }
    }

    return !exf;

}

int indexof_vehno(int vehno, vector<Vehicle> &vehicles)
{
    int index = 0;

    int vehicles_szie = vehicles.size();
    for(; index < vehicles_szie; index++){
        if(vehicles.at(index).vehno == vehno){
            return index;
        }
    }

    return -1;
}


bool insert_breakspot(vector<Customer> &customers, Vehicle &veh, vector<Vehicle> &vehicles, Cppargs &argvs, Vehicle &veh2)
/**
 * @brief 休憩スポットを挿入する関数
 * @param[in] customers
 * @param[in] veh
 * @param[in] argvs
 * @return vehicle
 * @details 休憩スポットを挿入する関数
 */
{
	if(veh.takebreak.size()==0 && argvs.break_interval!=0 && argvs.break_time!=0){
        // パターン3
        vehicles.at(indexof_vehno(veh.vehno, vehicles)).route = veh.route;
        vehicles.at(indexof_vehno(veh2.vehno, vehicles)).route = veh2.route;
	    if(pattern3(customers, veh, vehicles, argvs) == false){
            return false;
        }
	    vehicles.at(indexof_vehno(veh.vehno, vehicles)).route = veh.route;
        veh2.route = vehicles.at(indexof_vehno(veh2.vehno, vehicles)).route;
	    if(pattern3(customers, veh2, vehicles, argvs) == false){
            return false;
        } 
	    veh.route = vehicles.at(indexof_vehno(veh.vehno, vehicles)).route;
	    return true;
    }
    else if(veh.takebreak.size()!=0 && argvs.break_interval==0){
        // パターン1
        vehicles.at(indexof_vehno(veh.vehno, vehicles)).route = veh.route;
        vehicles.at(indexof_vehno(veh2.vehno, vehicles)).route = veh2.route;
        if(pattern1(customers, veh, vehicles, argvs) == true){
        	vehicles.at(indexof_vehno(veh.vehno, vehicles)).route = veh.route;
            veh2.route = vehicles.at(indexof_vehno(veh2.vehno, vehicles)).route;
            if(pattern1(customers, veh2, vehicles, argvs) == false){
                return false;
            } 
	        veh.route = vehicles.at(indexof_vehno(veh.vehno, vehicles)).route;
            return true;
        }
        else{
            return false;
        }
    }
	else if(veh.takebreak.size()!=0 && argvs.break_interval!=0){
        // パターン2
        vehicles.at(indexof_vehno(veh.vehno, vehicles)).route = veh.route;
        vehicles.at(indexof_vehno(veh2.vehno, vehicles)).route = veh2.route;
        if(pattern2(customers, veh, vehicles, argvs) == true){
        	vehicles.at(indexof_vehno(veh.vehno, vehicles)).route = veh.route;
            veh2.route = vehicles.at(indexof_vehno(veh2.vehno, vehicles)).route;
            if(pattern2(customers, veh2, vehicles, argvs) == false){
                return false;
            } 
	        veh.route = vehicles[indexof_vehno(veh.vehno, vehicles)].route;
            return true;
        }
        else{
            return false;
        }
    }
	else{
		return true;					//休憩は取らない(休憩処理を動かさない)。⇒パターン0
    }
}

// 休憩スポットのパターン番号を返す
int get_breakspot_pattern(const vector<Vehicle> &vehicles, Cppargs &argvs)
{
	bool no_takebreak = true;
    int vehicles_size = vehicles.size();

	for(int v=0; v<vehicles_size; v++){
		if(vehicles.at(v).takebreak.size()>0){
			no_takebreak = false;
			break;
        }
    }

	if(no_takebreak==true && argvs.break_interval!=0 && argvs.break_time!=0){
		return 3;
    }
	else if(no_takebreak==false && argvs.break_interval==0){
		return 1;
    }
	else if(no_takebreak==false && argvs.break_interval!=0){
		return 2;
    }
	else{
		return 0;
    }
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
}