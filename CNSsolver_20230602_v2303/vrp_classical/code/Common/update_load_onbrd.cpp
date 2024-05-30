#include "update_remdist.hpp"

/**
 * @brief 総積載量および総集荷量を計算する関数
 * @param[in] customers 配送先リスト
 * @details 第1及び第2の総積載量および総集荷量を計算する関数。
 */
void Vehicle::update_load_onbrd(const vector<Customer> &customers){
    int route_size = (int) route.size();

    load_onbrd = 0;
    load_pickup = 0;
    load_onbrd2 = 0;
    load_pickup2 = 0;

    for(int i = 1; i < route_size-1; i++){
        if(customers.at(route.at(i)).custno == "0"){
            continue; //デポはスキップ
        }

        if(customers.at(route.at(i)).requestType == "DELIVERY"){
            load_onbrd += customers.at(route.at(i)).dem;
        }
        else{
            load_pickup += customers.at(route.at(i)).dem;
        }

        if(customers.at(route.at(i)).requestType == "DELIVERY"){
            load_onbrd2 += customers.at(route.at(i)).dem2;
        }
        else{
            load_pickup2 += customers.at(route.at(i)).dem2;
        }
    }
}