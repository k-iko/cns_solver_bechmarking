#include "update_arrtime_tw.hpp"

namespace{
    bool vector_finder(const vector<int> &vec, const int number);
}

/**
 * @brief 各配送先に到着する時間等を計算する関数
 * @param[in] customers 配送先リスト
 * @param[in] management_ttable 時間テーブル一覧
 * @details 各配送先に到着する時間等を計算し、更新する関数。
 */
void Vehicle::update_arrtime_tw(const vector<Customer> &customers, const map<tuple<string, double>, vector<vector<double>>> &management_ttable, const bool break_flag, const bool delayst2_flag){
    vector<int> depo_idx;
    map<int, double> servt_dict;

    int route_size = (int) route.size();

    if(route_size < 2){
        return;
    }

    for(int i = 0; i < route_size-1; i++){
        if(customers.at(route.at(i)).custno == "0"){
            depo_idx.emplace_back(i); //routeにおけるデポのインデックス
        }
    }
    depo_idx.emplace_back(route_size-1); //最終訪問先を追加

    //荷作業時間を計算
    int depo_idx_size = (int) depo_idx.size();
    //配送時間
    for(int i = 0; i < depo_idx_size-1; i++){
        servt_dict.emplace(depo_idx.at(i), 0);
        double servt = 0;

        for(int j = depo_idx.at(i)+1; j < depo_idx.at(i+1); j++){
            if(customers.at(route.at(j)).requestType == "DELIVERY" && customers.at(route.at(j)).custno != customers.at(route.at(j-1)).custno
              && customers.at(route.at(j)).custno.at(0) != 'b'){
                servt += customers.at(route.at(j)).depotservt;
            }
        }

        if(customers.at(route.at(depo_idx.at(i)+1)).custno != "0"){
            servt_dict.at(depo_idx.at(i)) += servt+depotservt; //デポが連続していない場合
        }
        else{
            servt_dict.at(depo_idx.at(i)) += servt; //デポが連続している場合
        }
    }

    //集荷時間
    servt_dict.emplace(route_size-1, 0);
    for(int i = 1; i < depo_idx_size; i++){
        double servt = 0;

        for(int j = depo_idx.at(i-1)+1; j < depo_idx.at(i); j++){
            if(customers.at(route.at(j)).requestType == "PICKUP" && customers.at(route.at(j)).custno != customers.at(route.at(j-1)).custno){
                servt += customers.at(route.at(j)).depotservt;
            }
        }

        servt_dict.at(depo_idx.at(i)) += servt;
    }

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


    if((arr.empty() || delayst2_flag == true)&& break_flag == false){
        arr = vector<double>(route_size, 0);
        arr.at(0) = starthour;
    }
    else{
        double arr_temp = arr.at(0);
        arr = vector<double>(route_size, 0);
        arr.at(0) = arr_temp;
    }
    st = vector<double>(route_size, 0);
    lv = vector<double>(route_size, 0);
    slack_t = vector<double>(route_size, 0);

    vector<bool> parkt_time_flag = get_parkt_time_flag(customers, route);

    if(load.at(0) != 0 || load2.at(0) != 0){
        st.at(0) = max(arr.at(0)+customers.at(route.at(0)).parkt_arrive, customers.at(route.at(0)).ready_fs) + customers.at(route.at(0)).parkt_open;
    }
    else{
        st.at(0) = max(arr.at(0), customers.at(route.at(0)).ready_fs);
    }
    
    lv.at(0) = st.at(0)+servt_dict.at(0)*opskill;
    
    for(int i = 1; i < route_size; i++){
        Customer from_c = customers.at(route.at(i-1)), to_c = customers.at(route.at(i));

        if((from_c.spotid != "") && (to_c.spotid != "")){
            arr.at(i) = lv.at(i-1)+get_ttable(vtype, lv.at(i-1), management_ttable, from_c.spotno, to_c.spotno)*drskill;    // 休憩スポット以外
        }
        else{
            arr.at(i) = lv.at(i-1)+get_ttable(vtype, lv.at(i-1), management_ttable, from_c.spotno, to_c.spotno);    // 休憩スポットはスキルは関係ない
        }
        if(parkt_time_flag.at(i) &&
        ((to_c.spotid != "0" && i != route_size-1) || load.at(i) != 0 || load.at(i-1) != 0 || load2.at(i) != 0 || load2.at(i-1) != 0)){
            st.at(i) = max(arr.at(i)+to_c.parkt_arrive, to_c.ready_fs) + to_c.parkt_open; //駐車時間
        }
        else{
            st.at(i) = max(arr.at(i), to_c.ready_fs);
        }
        
        lv.at(i) = st.at(i);
        if(servt_dict.count(i) == 1){
            lv.at(i) += servt_dict.at(i)*opskill; //デポの場合の荷作業時間
        }
        else{
            if(customers.at(route.at(i)).custno.at(0) != 'b'){
                lv.at(i) += to_c.servt*opskill; //デポ以外の場合の荷作業時間
            }
            else{
                lv.at(i) += to_c.servt; //デポ以外の場合で休憩スポットはスキルは関係ない
            }
        }
        if(chg_batt_dict.count(route.at(i)) == 1){
            lv.at(i) += chg_batt_dict.at(route.at(i))/to_c.e_rate; //充電時間
        }

        if(to_c.custno == from_c.custno && to_c.custno!="0" && i!= route_size-1){
            arr.at(i) = arr.at(i-1);
            lv.at(i) = lv.at(i-1);
            st.at(i) = st.at(i-1);
        }
    }

    slack_t.at(route_size-1) = customers.at(route.at(route_size-1)).due_fs-arr.at(route_size-1);
    for(int i = route_size-2; i >= 0; i--){
        slack_t.at(i) = min(customers.at(route.at(i)).due_fs-st.at(i), slack_t.at(i+1));
    }

    if(!delay_arr.empty() && break_flag==false){
        delay_st = vector<double>(route_size, 0);
        delay_lv = vector<double>(route_size, 0);
        delay_slack_t = vector<double>(route_size, 0);

        if(parkt_time_flag.at(0) == true){
            delay_st.at(0) = max(delay_arr.at(0)+customers.at(route.at(0)).parkt_arrive, customers.at(route.at(0)).ready_fs) + customers.at(route.at(0)).parkt_open;
        }
        else{
            delay_st.at(0) = max(delay_arr.at(0), customers.at(route.at(0)).ready_fs);
        }
        delay_arr.resize(route_size);
        
        delay_lv.at(0) = delay_st.at(0)+servt_dict.at(0)*opskill;

        for(int i = 1; i < route_size; i++){
            Customer from_c = customers.at(route.at(i-1)), to_c = customers.at(route.at(i));

            if((from_c.spotid != "") && (to_c.spotid != "")){
                delay_arr.at(i) = delay_lv.at(i-1)+get_ttable(vtype, delay_lv.at(i-1), management_ttable, from_c.spotno, to_c.spotno)*drskill;  // 休憩スポット以外
            }
            else{
                delay_arr.at(i) = delay_lv.at(i-1)+get_ttable(vtype, delay_lv.at(i-1), management_ttable, from_c.spotno, to_c.spotno);  // 休憩スポットはスキルは関係ない
            }
            if(parkt_time_flag.at(i) == true){
                delay_st.at(i) = max(delay_arr.at(i)+to_c.parkt_arrive, to_c.ready_fs) + to_c.parkt_open;
            }
            else{
                delay_st.at(i) = max(delay_arr.at(i), to_c.ready_fs);
            }
            
            delay_lv.at(i) = delay_st.at(i);
            if(servt_dict.count(i) == 1){
                delay_lv.at(i) += servt_dict.at(i)*opskill; //デポの場合の荷作業時間
            }
            else{
                if(customers.at(route.at(i)).custno.at(0) != 'b'){
                    delay_lv.at(i) += to_c.servt*opskill; //デポ以外の場合の荷作業時間
                }
                else{
                    delay_lv.at(i) += to_c.servt; //デポ以外で休憩スポットの場合はスキルは関係ない
                }
            }
            if(delay_chg_batt_dict.count(route.at(i)) == 1){
                delay_lv.at(i) += delay_chg_batt_dict.at(route.at(i))/to_c.e_rate; //充電時間
            }

            if(to_c.custno == from_c.custno && to_c.custno!="0" && i!= route_size-1){
                delay_arr.at(i) = delay_arr.at(i-1);
                delay_lv.at(i) = delay_lv.at(i-1);
                delay_st.at(i) = delay_st.at(i-1);
            }
        }

        delay_slack_t.at(route_size-1) = customers.at(route.at(route_size-1)).due_fs-delay_st.at(route_size-1);
        for(int i = route_size-2; i >= 0; i--){
            delay_slack_t.at(i) = min(customers.at(route.at(i)).due_fs-delay_st.at(i), delay_slack_t.at(i+1));
        }
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