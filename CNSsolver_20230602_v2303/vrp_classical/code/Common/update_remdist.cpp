#include "update_remdist.hpp"

/**
 * @brief 各配送先からデポに戻るまでの距離を計算する関数
 * @param[in] customers 配送先リスト
 * @param[in] argvs グローバル変数格納用
 * @details 各配送先からデポに戻るまでの距離を計算し、その値でremdistを更新する関数。
 */
void Vehicle::update_remdist(const vector<Customer> &customers, const Cppargs &argvs){
    int route_size = (int) route.size();

    remdist = vector<double>(route_size, 0);

    double totaldist = 0;
    for(int i = 0; i < route_size-1; i++){
        totaldist += get_dtable(vtype, lv.at(i), argvs.management_dtable, customers.at(route.at(i)).spotno, customers.at(route.at(i+1)).spotno);
    }

    remdist.at(0) = totaldist;

    for(int i = 1; i < route_size; i++){
        remdist.at(i) = remdist.at(i-1)-get_dtable(vtype, lv.at(i-1), argvs.management_dtable, customers.at(route.at(i-1)).spotno, customers.at(route.at(i)).spotno);
    }

    if(!delay_arr.empty() && argvs.break_flag==false){
        delay_remdist = vector<double>(route_size, 0);

        double totaldist = 0;
        for(int i = 0; i < route_size-1; i++){
            totaldist += get_dtable(vtype, delay_lv.at(i), argvs.management_dtable, customers.at(route.at(i)).spotno, customers.at(route.at(i+1)).spotno);
        }

        delay_remdist.at(0) = totaldist;

        for(int i = 1; i < route_size; i++){
            delay_remdist.at(i) = delay_remdist.at(i-1)-get_dtable(vtype, delay_lv.at(i-1), argvs.management_dtable, customers.at(route.at(i-1)).spotno, customers.at(route.at(i)).spotno);
        }
    }

	if(reserved_v_flag==true){
		delay_remdist=remdist;
    }

}