#include "check_impassability.hpp"

namespace{
    bool vector_finder(const vector<int> &vec, const int number);
}

/**
 * @brief 各種コストを更新し、通行不可ルートがないかチェックする関数
 * @param[in] vehicle 車両オブジェクト
 * @param[in] customers 配送先リスト
 * @param[in] lv_t start_idxの前の配送先の出発時刻
 * @param[in] start_idx 開始インデックス
 * @param[in] last_idx 終了インデックス
 * @param[in] servt_dict 作業時間用map
 * @param[in] dist 走行距離
 * @param[in] power 消費電力
 * @param[in] exp 経験コスト
 * @param[in] no_park_idx 駐車時間を計算しないインデックスリスト
 * @param[in] pick_opt_flag 集荷最適化を実施するかのフラグ(true : 実施する)
 * @param[in] argvs グローバル変数格納用
 * @return bool true : 通行可能、false : 通行不可
 * @details start_idx以降の各種コストを更新し、通行不可ルートがないかチェックする関数。
 */
bool check_impassability(const Vehicle &vehicle, const vector<Customer> &customers, double lv_t, const int start_idx, const int last_idx, const map<int, double> &servt_dict, double &dist, double &power, double &exp, const vector<int> &no_park_idx, bool pick_opt_flag, const Cppargs &argvs){
    int route_size = (int) vehicle.route.size();
    bool passable = true;
    double arr_t;
    
    if((argvs.bulkShipping_flag == false && pick_opt_flag == true && customers.at(vehicle.route.at(route_size-2)).custno != "0" && customers.at(vehicle.route.at(route_size-1)).custno!="0") ||
    (argvs.bulkShipping_flag == true && pick_opt_flag == true && customers.at(customers.at(vehicle.route.at(route_size-2)).tieup_customer.at(0)).custno != "0" && customers.at(customers.at(vehicle.route.at(route_size-1)).tieup_customer.at(0)).custno!="0")){ //集荷最適化
        for(int i = start_idx; i < route_size-1; i++){
            if(customers.at(vehicle.route.at(i-1)).custno == customers.at(vehicle.route.at(i)).custno && customers.at(vehicle.route.at(i)).custno != "0"){
				continue;
            }
            
            //通行不可チェック
            if(get_ttable(vehicle.vtype, lv_t, argvs.management_ttable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(i)).spotno) == -1){
                passable = false;
                break;
            }

            if(get_dtable(vehicle.vtype, lv_t, argvs.management_dtable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(i)).spotno) == -1){
                passable = false;
                break;
            }

            if(!argvs.management_etable.empty()){
                if(get_etable(vehicle.vtype, vehicle.etype, lv_t, argvs.management_etable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(i)).spotno) == -1){
                    passable = false;
                    break;
                }
            }

            if(!argvs.management_xtable.empty()){
                if(get_xtable(vehicle.vtype, lv_t, argvs.management_xtable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(i)).spotno) == -1){
                    passable = false;
                    break;
                }
            }


            //コストの更新
            if(argvs.lastc_flag == false || i < last_idx){
                dist += get_dtable(vehicle.vtype, lv_t, argvs.management_dtable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(i)).spotno);

                if(!argvs.management_etable.empty()){
                    power += get_etable(vehicle.vtype, vehicle.etype, lv_t, argvs.management_etable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(i)).spotno);
                }

                if(!argvs.management_xtable.empty()){
                    exp += get_xtable(vehicle.vtype, lv_t, argvs.management_xtable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(i)).spotno);
                }
            }


            //時刻の更新
            arr_t = lv_t+get_ttable(vehicle.vtype, lv_t, argvs.management_ttable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(i)).spotno)*vehicle.drskill;

            if(servt_dict.count(i) == 1){
                if(customers.at(vehicle.route.at(i)).spotid != customers.at(vehicle.route.at(i-1)).spotid && customers.at(vehicle.route.at(i)).custno.substr(0, 1) != "e" && vector_finder(no_park_idx, i) == false){
                    lv_t = max(arr_t+customers.at(vehicle.route.at(i)).parkt_arrive, customers.at(vehicle.route.at(i)).ready_fs)+servt_dict.at(i)*vehicle.opskill; //拠点作業時間+荷作業時間
                }
                else{
                    lv_t = max(arr_t, customers.at(vehicle.route.at(i)).ready_fs)+servt_dict.at(i)*vehicle.opskill; //拠点作業時間+荷作業時間
                }
            }
            else{
                if(customers.at(vehicle.route.at(i)).spotid != customers.at(vehicle.route.at(i-1)).spotid && customers.at(vehicle.route.at(i)).custno.substr(0, 1) != "e" && vector_finder(no_park_idx, i) == false){
                    lv_t = max(arr_t+customers.at(vehicle.route.at(i)).parkt_arrive, customers.at(vehicle.route.at(i)).ready_fs)+customers.at(vehicle.route.at(i)).servt*vehicle.opskill; //荷作業時間
                }
                else{
                    lv_t = max(arr_t, customers.at(vehicle.route.at(i)).ready_fs)+customers.at(vehicle.route.at(i)).servt*vehicle.opskill; //荷作業時間
                }
            }

            if(customers.at(vehicle.route.at(i)).spotid != customers.at(vehicle.route.at(i-1)).spotid && customers.at(vehicle.route.at(i)).custno.substr(0, 1) != "e" && vector_finder(no_park_idx, i) == false){
                lv_t += customers.at(vehicle.route.at(i)).parkt_open; //駐車時間
            }

            if(customers.at(vehicle.route.at(i)).custno.substr(0, 1) == "e"){
                lv_t += vehicle.chg_batt.at(i)/customers.at(vehicle.route.at(i)).e_rate; //充電時間
            }
        }

        //通行不可チェック
        if(get_ttable(vehicle.vtype, lv_t, argvs.management_ttable, customers.at(vehicle.route.at(route_size-2)).spotno, customers.at(0).spotno) == -1){
            passable = false;
            return passable;
        }

        if(get_dtable(vehicle.vtype, lv_t, argvs.management_dtable, customers.at(vehicle.route.at(route_size-2)).spotno, customers.at(0).spotno) == -1){
            passable = false;
            return passable;
        }

        if(!argvs.management_etable.empty()){
            if(get_etable(vehicle.vtype, vehicle.etype, lv_t, argvs.management_etable, customers.at(vehicle.route.at(route_size-2)).spotno, customers.at(0).spotno) == -1){
                passable = false;
                return passable;
            }
        }

        if(!argvs.management_xtable.empty()){
            if(get_xtable(vehicle.vtype, lv_t, argvs.management_xtable, customers.at(vehicle.route.at(route_size-2)).spotno, customers.at(0).spotno) == -1){
                passable = false;
                return passable;
            }
        }


        //コストの更新
        if(argvs.lastc_flag == false){
            dist += get_dtable(vehicle.vtype, lv_t, argvs.management_dtable, customers.at(vehicle.route.at(route_size-2)).spotno, customers.at(0).spotno);

            if(!argvs.management_etable.empty()){
                power += get_etable(vehicle.vtype, vehicle.etype, lv_t, argvs.management_etable, customers.at(vehicle.route.at(route_size-2)).spotno, customers.at(0).spotno);
            }

            if(!argvs.management_xtable.empty()){
                exp += get_xtable(vehicle.vtype, lv_t, argvs.management_xtable, customers.at(vehicle.route.at(route_size-2)).spotno, customers.at(0).spotno);
            }
        }


        //時刻の更新
        arr_t = lv_t+get_ttable(vehicle.vtype, lv_t, argvs.management_ttable, customers.at(vehicle.route.at(route_size-2)).spotno, customers.at(0).spotno)*vehicle.drskill;

        lv_t = max(arr_t+customers.at(0).parkt_arrive, customers.at(0).ready_fs)+(servt_dict.at(route_size-1)-customers.at(vehicle.route.at(route_size-1)).servt+customers.at(0).servt+vehicle.depotservt)*vehicle.opskill; //拠点作業時間+荷作業時間

        lv_t += customers.at(0).parkt_open; //駐車時間


        //通行不可チェック
        if(get_ttable(vehicle.vtype, lv_t, argvs.management_ttable, customers.at(0).spotno, customers.at(vehicle.route.at(route_size-1)).spotno) == -1){
            passable = false;
            return passable;
        }

        if(get_dtable(vehicle.vtype, lv_t, argvs.management_dtable, customers.at(0).spotno, customers.at(vehicle.route.at(route_size-1)).spotno) == -1){
            passable = false;
            return passable;
        }

        if(!argvs.management_etable.empty()){
            if(get_etable(vehicle.vtype, vehicle.etype, lv_t, argvs.management_etable, customers.at(0).spotno, customers.at(vehicle.route.at(route_size-1)).spotno) == -1){
                passable = false;
                return passable;
            }
        }

        if(!argvs.management_xtable.empty()){
            if(get_xtable(vehicle.vtype, lv_t, argvs.management_xtable, customers.at(0).spotno, customers.at(vehicle.route.at(route_size-1)).spotno) == -1){
                passable = false;
                return passable;
            }
        }


        //コストの更新
        if(argvs.lastc_flag == false){
            dist += get_dtable(vehicle.vtype, lv_t, argvs.management_dtable, customers.at(0).spotno, customers.at(vehicle.route.at(route_size-1)).spotno);

            if(!argvs.management_etable.empty()){
                power += get_etable(vehicle.vtype, vehicle.etype, lv_t, argvs.management_etable, customers.at(0).spotno, customers.at(vehicle.route.at(route_size-1)).spotno);
            }

            if(!argvs.management_xtable.empty()){
                exp += get_xtable(vehicle.vtype, lv_t, argvs.management_xtable, customers.at(0).spotno, customers.at(vehicle.route.at(route_size-1)).spotno);
            }
        }
    }
    else{
        for(int i = start_idx; i < route_size; i++){
            if(customers.at(vehicle.route.at(i-1)).custno == customers.at(vehicle.route.at(i)).custno && customers.at(vehicle.route.at(i)).custno != "0"){
				continue;
            }
            
            //デポ削除後の通行不可チェック
            bool is_only_depo = true;
            for(int j = i; j < route_size-1; j++){
                if(customers.at(vehicle.route.at(j)).custno != "0"){
                    is_only_depo = false;
                    break;
                }
            }
            if(is_only_depo){
                if(pick_opt_flag == false){
					if(customers.at(vehicle.route.at(i-1)).custno.at(0)=='b' && customers.at(vehicle.route.at(i-1)).spotid==""){
                        int l = -1;
						for(int ll=i-2; ll>=0; ll--){
							if(customers.at(vehicle.route.at(ll)).custno.at(0)!='b'){
                                l = ll;
								break;
                            }
                        }
                        if(l==-1){
                            passable = false;
                            break;
                        }

                        // from
                        double lv_from = lv_t;
                        if(get_ttable(vehicle.vtype, lv_from, argvs.management_ttable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(route_size-1)).spotno) == -1){
                            passable = false;
                            break;
                        }

                        if(get_dtable(vehicle.vtype, lv_from, argvs.management_dtable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(route_size-1)).spotno) == -1){
                            passable = false;
                            break;
                        }

                        if(!argvs.management_etable.empty()){
                            if(get_etable(vehicle.vtype, vehicle.etype, lv_from, argvs.management_etable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(route_size-1)).spotno) == -1){
                                passable = false;
                                break;
                            }
                        }

                        if(!argvs.management_xtable.empty()){
                            if(get_xtable(vehicle.vtype, lv_from, argvs.management_xtable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(route_size-1)).spotno) == -1){
                                passable = false;
                                break;
                            }
                        }

                        // to
                        double lv_to = vehicle.lv.at(i-1);
                        if(get_ttable(vehicle.vtype, lv_to, argvs.management_ttable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(route_size-1)).spotno) == -1){
                            passable = false;
                            break;
                        }

                        if(get_dtable(vehicle.vtype, lv_to, argvs.management_dtable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(route_size-1)).spotno) == -1){
                            passable = false;
                            break;
                        }

                        if(!argvs.management_etable.empty()){
                            if(get_etable(vehicle.vtype, vehicle.etype, lv_to, argvs.management_etable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(route_size-1)).spotno) == -1){
                                passable = false;
                                break;
                            }
                        }

                        if(!argvs.management_xtable.empty()){
                            if(get_xtable(vehicle.vtype, lv_to, argvs.management_xtable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(route_size-1)).spotno) == -1){
                                passable = false;
                                break;
                            }
                        }
                    }
                    else{
                        if(get_ttable(vehicle.vtype, lv_t, argvs.management_ttable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(route_size-1)).spotno) == -1){
                            passable = false;
                            break;
                        }

                        if(get_dtable(vehicle.vtype, lv_t, argvs.management_dtable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(route_size-1)).spotno) == -1){
                            passable = false;
                            break;
                        }

                        if(!argvs.management_etable.empty()){
                            if(get_etable(vehicle.vtype, vehicle.etype, lv_t, argvs.management_etable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(route_size-1)).spotno) == -1){
                                passable = false;
                                break;
                            }
                        }

                        if(!argvs.management_xtable.empty()){
                            if(get_xtable(vehicle.vtype, lv_t, argvs.management_xtable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(route_size-1)).spotno) == -1){
                                passable = false;
                                break;
                            }
                        }
                    }
                    if(argvs.lastc_flag == false){
                        dist += get_dtable(vehicle.vtype, lv_t, argvs.management_dtable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(route_size-1)).spotno);

                        if(!argvs.management_etable.empty()){
                            power += get_etable(vehicle.vtype, vehicle.etype, lv_t, argvs.management_etable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(route_size-1)).spotno);
                        }

                        if(!argvs.management_xtable.empty()){
                            exp += get_xtable(vehicle.vtype, lv_t, argvs.management_xtable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(route_size-1)).spotno);
                        }
                    }

                    break;
                }
            }


            //通行不可チェック
			if(customers.at(vehicle.route.at(i-1)).custno.at(0)=='b' && customers.at(vehicle.route.at(i-1)).spotid==""){
                int l = -1;
                for(int ll=i-2; ll>=0; ll--){
                    if(customers.at(vehicle.route.at(ll)).custno.at(0)!='b'){
                        l = ll;
                        break;
                    }
                }
                if(l==-1){
                    passable = false;
                    break;
                }

                int m = -1;
                int route_size = vehicle.route.size();
                for(int mm=i; mm<route_size; mm++){
                    if(customers.at(vehicle.route.at(mm)).custno.at(0)!='b'){
                        m = mm;
                        break;
                    }
                }
                if(l==-1){
                    passable = false;
                    break;
                }

                // from
                double lv_from = lv_t;
                if(get_ttable(vehicle.vtype, lv_from, argvs.management_ttable, customers.at(vehicle.route.at(l)).spotno, customers.at(vehicle.route.at(m)).spotno) == -1){
                    passable = false;
                    break;
                }

                if(get_dtable(vehicle.vtype, lv_from, argvs.management_dtable, customers.at(vehicle.route.at(l)).spotno, customers.at(vehicle.route.at(m)).spotno) == -1){
                    passable = false;
                    break;
                }

                if(!argvs.management_etable.empty()){
                    if(get_etable(vehicle.vtype, vehicle.etype, lv_from, argvs.management_etable, customers.at(vehicle.route.at(l)).spotno, customers.at(vehicle.route.at(m)).spotno) == -1){
                        passable = false;
                        break;
                    }
                }

                if(!argvs.management_xtable.empty()){
                    if(get_xtable(vehicle.vtype, lv_from, argvs.management_xtable, customers.at(vehicle.route.at(l)).spotno, customers.at(vehicle.route.at(m)).spotno) == -1){
                        passable = false;
                        break;
                    }
                }

                // to
                double lv_to = vehicle.lv.at(i-1);
                if(get_ttable(vehicle.vtype, lv_to, argvs.management_ttable, customers.at(vehicle.route.at(l)).spotno, customers.at(vehicle.route.at(m)).spotno) == -1){
                    passable = false;
                    break;
                }

                if(get_dtable(vehicle.vtype, lv_to, argvs.management_dtable, customers.at(vehicle.route.at(l)).spotno, customers.at(vehicle.route.at(m)).spotno) == -1){
                    passable = false;
                    break;
                }

                if(!argvs.management_etable.empty()){
                    if(get_etable(vehicle.vtype, vehicle.etype, lv_to, argvs.management_etable, customers.at(vehicle.route.at(l)).spotno, customers.at(vehicle.route.at(m)).spotno) == -1){
                        passable = false;
                        break;
                    }
                }

                if(!argvs.management_xtable.empty()){
                    if(get_xtable(vehicle.vtype, lv_to, argvs.management_xtable, customers.at(vehicle.route.at(l)).spotno, customers.at(vehicle.route.at(m)).spotno) == -1){
                        passable = false;
                        break;
                    }
                }
            }
            else{
                if(get_ttable(vehicle.vtype, lv_t, argvs.management_ttable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(i)).spotno) == -1){
                    passable = false;
                    break;
                }

                if(get_dtable(vehicle.vtype, lv_t, argvs.management_dtable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(i)).spotno) == -1){
                    passable = false;
                    break;
                }

                if(!argvs.management_etable.empty()){
                    if(get_etable(vehicle.vtype, vehicle.etype, lv_t, argvs.management_etable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(i)).spotno) == -1){
                        passable = false;
                        break;
                    }
                }

                if(!argvs.management_xtable.empty()){
                    if(get_xtable(vehicle.vtype, lv_t, argvs.management_xtable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(i)).spotno) == -1){
                        passable = false;
                        break;
                    }
                }
            }

            //コストの更新
            if(argvs.lastc_flag == false || i < last_idx){
                dist += get_dtable(vehicle.vtype, lv_t, argvs.management_dtable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(i)).spotno);

                if(!argvs.management_etable.empty()){
                    power += get_etable(vehicle.vtype, vehicle.etype, lv_t, argvs.management_etable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(i)).spotno);
                }

                if(!argvs.management_xtable.empty()){
                    exp += get_xtable(vehicle.vtype, lv_t, argvs.management_xtable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(i)).spotno);
                }
            }


            //時刻の更新
            arr_t = lv_t+get_ttable(vehicle.vtype, lv_t, argvs.management_ttable, customers.at(vehicle.route.at(i-1)).spotno, customers.at(vehicle.route.at(i)).spotno)*vehicle.drskill;

            if(servt_dict.count(i) == 1){
                if(customers.at(vehicle.route.at(i)).spotid != customers.at(vehicle.route.at(i-1)).spotid && customers.at(vehicle.route.at(i)).custno.substr(0, 1) != "e" && vector_finder(no_park_idx, i) == false){
                    lv_t = max(arr_t+customers.at(vehicle.route.at(i)).parkt_arrive, customers.at(vehicle.route.at(i)).ready_fs)+servt_dict.at(i)*vehicle.opskill; //拠点作業時間+荷作業時間
                }
                else{
                    lv_t = max(arr_t, customers.at(vehicle.route.at(i)).ready_fs)+servt_dict.at(i)*vehicle.opskill; //拠点作業時間+荷作業時間
                }
            }
            else{
                if(customers.at(vehicle.route.at(i)).spotid != customers.at(vehicle.route.at(i-1)).spotid && customers.at(vehicle.route.at(i)).custno.substr(0, 1) != "e" && vector_finder(no_park_idx, i) == false){
                    lv_t = max(arr_t+customers.at(vehicle.route.at(i)).parkt_arrive, customers.at(vehicle.route.at(i)).ready_fs)+customers.at(vehicle.route.at(i)).servt*vehicle.opskill; //荷作業時間
                }
                else{
                    lv_t = max(arr_t, customers.at(vehicle.route.at(i)).ready_fs)+customers.at(vehicle.route.at(i)).servt*vehicle.opskill; //荷作業時間
                }
            }

            if(customers.at(vehicle.route.at(i)).spotid != customers.at(vehicle.route.at(i-1)).spotid && customers.at(vehicle.route.at(i)).custno.substr(0, 1) != "e" && vector_finder(no_park_idx, i) == false){
                lv_t += customers.at(vehicle.route.at(i)).parkt_open; //駐車時間
            }

            if(customers.at(vehicle.route.at(i)).custno.substr(0, 1) == "e"){
                lv_t += vehicle.chg_batt.at(i)/customers.at(vehicle.route.at(i)).e_rate; //充電時間
            }
        }
    }

    return passable;
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