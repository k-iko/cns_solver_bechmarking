#include "cross_cust_dtable.hpp"

/**
 * @brief 2題の車両の配送先を交換する関数
 * @param[in] vehicle1 車両オブジェクト
 * @param[in] vehicle2 車両オブジェクト
 * @param[in] customers 配送先リスト
 * @param[in] pos1 1台目の交換開始インデックス
 * @param[in] pos2 1台目の交換終了インデックス
 * @param[in] pos3 2台目の交換開始インデックス
 * @param[in] pos4 2台目の交換終了インデックス
 * @param[in] custs1 1台目に挿入する配送先
 * @param[in] custs2 2台目に挿入する配送先
 * @param[in] pick_opt_flag1 1台目に対して集荷最適化するかのフラグ(true : 最適化する)
 * @param[in] pick_opt_flag2 2台目に対して集荷最適化するかのフラグ(true : 最適化する)
 * @param[in] argvs グローバル変数格納用
 * @details 1台目のpos1~pos2-1までをcusts1の配送先、2台目のpos3~pos4-1までをcusts2の配送先で置き換える関数。
 */
void cross_cust_dtable(Vehicle &vehicle1, Vehicle &vehicle2, const vector<Customer> &customers, const int pos1, const int pos2, const int pos3, const int pos4, const vector<int> &custs1, const vector<int> &custs2, bool pick_opt_flag1, bool pick_opt_flag2, const vector<int> &route1, const vector<int> &route2, const Cppargs &argvs){
    if(argvs.break_flag==false){
        //交換用リスト作成
        int route_size1 = (int) vehicle1.route.size();

        int custs1_size = (int) custs1.size();
        vector<int> v1swap;
        for (int i = 0; i < pos1; i++) {
            v1swap.emplace_back(vehicle1.route.at(i));
        }
        for (int i = 0; i < custs1_size; i++) {
            v1swap.emplace_back(custs1.at(i));
        }
        for (int i = pos2; i < route_size1; i++) {
            v1swap.emplace_back(vehicle1.route.at(i));
        }


        int route_size2 = (int) vehicle2.route.size();

        int custs2_size = (int) custs2.size();
        vector<int> v2swap;
        for (int i = 0; i < pos3; i++) {
            v2swap.emplace_back(vehicle2.route.at(i));
        }
        for (int i = 0; i < custs2_size; i++) {
            v2swap.emplace_back(custs2.at(i));
        }
        for (int i = pos4; i < route_size2; i++) {
            v2swap.emplace_back(vehicle2.route.at(i));
        }

        //交換
        vehicle1.route = v1swap;
        vehicle2.route = v2swap;

        //車両情報更新
        vehicle1.update_totalcost_dtable(customers, argvs);
        vehicle2.update_totalcost_dtable(customers, argvs);

        int veh1_route_size = vehicle1.route.size();
        int veh2_route_size = vehicle2.route.size();

        //集荷最適化
        if(argvs.bulkShipping_flag == false && pick_opt_flag1 == true && customers.at(vehicle1.route.at(veh1_route_size-2)).custno != "0" && customers.at(vehicle1.route.at(veh1_route_size-1)).custno != "0"){
            vehicle1.route.insert(vehicle1.route.begin()+veh1_route_size-1, 0);
            vehicle1.update_totalcost_dtable(customers, argvs);
        }
        else if(argvs.bulkShipping_flag == true && pick_opt_flag1 == true && customers.at(customers.at(vehicle1.route.at(veh1_route_size-2)).tieup_customer.at(0)).custno != "0" && customers.at(customers.at(vehicle1.route.at(veh1_route_size-1)).tieup_customer.at(0)).custno != "0"){
            vehicle1.route.insert(vehicle1.route.begin()+veh1_route_size-1, 0);
            vehicle1.update_totalcost_dtable(customers, argvs);
        }
        else if((argvs.bulkShipping_flag == false && pick_opt_flag1 == false && customers.at(vehicle1.route.at(veh1_route_size-2)).custno == "0" && customers.at(vehicle1.route.at(veh1_route_size-1)).custno != "0") ||
        (argvs.bulkShipping_flag == true && pick_opt_flag1 == false && customers.at(customers.at(vehicle1.route.at(veh1_route_size-2)).tieup_customer.at(0)).custno == "0" && customers.at(customers.at(vehicle1.route.at(veh1_route_size-1)).tieup_customer.at(0)).custno != "0")){
            vehicle1.route.erase(vehicle1.route.begin()+veh1_route_size-2);
            vehicle1.update_totalcost_dtable(customers, argvs);
        }

        if(argvs.bulkShipping_flag == false && pick_opt_flag2 == true && customers.at(vehicle2.route.at(veh2_route_size-2)).custno != "0" && customers.at(vehicle2.route.at(veh2_route_size-1)).custno != "0"){
            vehicle2.route.insert(vehicle2.route.begin()+veh2_route_size-1, 0);
            vehicle2.update_totalcost_dtable(customers, argvs);
        }
        else if(argvs.bulkShipping_flag == true && pick_opt_flag2 == true && customers.at(customers.at(vehicle2.route.at(veh2_route_size-2)).tieup_customer.at(0)).custno != "0" && customers.at(customers.at(vehicle2.route.at(veh2_route_size-1)).tieup_customer.at(0)).custno != "0"){
            vehicle2.route.insert(vehicle2.route.begin()+veh2_route_size-1, 0);
            vehicle2.update_totalcost_dtable(customers, argvs);
        }
        else if((argvs.bulkShipping_flag == false && pick_opt_flag2 == false && customers.at(vehicle2.route.at(veh2_route_size-2)).custno == "0" && customers.at(vehicle2.route.at(veh2_route_size-1)).custno != "0") ||
        (argvs.bulkShipping_flag == true && pick_opt_flag2 == false && customers.at(customers.at(vehicle2.route.at(veh2_route_size-2)).tieup_customer.at(0)).custno == "0" && customers.at(customers.at(vehicle2.route.at(veh2_route_size-1)).tieup_customer.at(0)).custno != "0")){
            vehicle2.route.erase(vehicle2.route.begin()+veh2_route_size-2);
            vehicle2.update_totalcost_dtable(customers, argvs);
        }
    }    
    else{
        //交換
        vehicle1.route = route1;
        vehicle2.route = route2;

        //車両情報更新
        vehicle1.update_totalcost_dtable(customers, argvs);
        vehicle2.update_totalcost_dtable(customers, argvs);
    }
}
