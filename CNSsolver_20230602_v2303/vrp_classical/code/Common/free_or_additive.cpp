#include <algorithm>
#include <iostream>
#include <vector>
#include "free_or_additive.hpp"

namespace{
    bool vector_finder(const vector<int> &vec, const int number);
    bool vector_finder(const vector<string> &vec, const string str);
}

// #### 未割り当て荷物最適化関数 ####

// # 荷物が今ある車両でfreeかadditiveかを判定
// #  in  : customer, vehicle, argvs
// #  out : free、additive
// #
string free_or_additive(const Customer cust, const Vehicle veh, const vector<Vehicle> &vehicles, const vector<Customer> &customers, const Cppargs &argvs)
{
	if(argvs.opt_unassigned_flag==false){
		// 未割り当て荷物最適化がFalseならばそのまま
		return cust.changecust;
	}

	// 初期解を通常車両と予約車両でグルーピング
	vector<string> n_grp; 	// 通常車両の荷物
	vector<string> r_grp; // 予約車両の荷物
	for (auto ir = argvs.initroute.begin(); ir != argvs.initroute.end(); ++ir){
		int v_size = vehicles.size();
		for(int v=0; v < v_size; v++){
			if(vehicles.at(v).vehno == ir->first){
				if(vehicles.at(v).reserved_v_flag == true){
					int is = ir->second.size();
					for(int i=0; i<is; i++){
						r_grp.push_back(ir->second.at(i));
					}
				}
				else{
					int is = ir->second.size();
					for(int i=0; i<is; i++){
						n_grp.push_back(ir->second.at(i));
					}
				}

			}
		}
	}

	if(veh.reserved_v_flag==true){
		if(vector_finder(r_grp, cust.custno)==true){
			// 車両が予約車両で、荷物が初期解の予約車両に有ったならばchangecustを返す
			return(cust.changecust);
		}
	}
	else{
		if(vector_finder(n_grp, cust.custno)==true){
			// 車両が通常車両で、荷物が初期解の通常車両に有ったならばchangecustを返す
			return(cust.changecust);
		}
	}
	// 上記以外は全部free

	return "free";
}


// # 荷物がfreeからaddtitiveになる際に、additiveのルールを守っているか判定
// #  in  : cust 荷物、veh 移動先車両、skv 移動先の仮ルート、argvs
// #  out : True ルールを守っている、False ルールを守っていない
// #
bool in_additive(Customer const cust, const Vehicle veh, const vector<int> &skv, const vector<Customer> &customers, const vector<Vehicle> &vehicles, const Cppargs &argvs)
{
	if(free_or_additive(cust, veh, vehicles, customers, argvs)=="additive'"){
		// additive
		// 車両は合ってるか？
		if(vector_finder(argvs.initroute.at(veh.vehno), cust.custno)==false){
			// 元の車両じゃなかったらルールエラー
			return false;
		}

		// 回転の便数は合ってるか？
		// 初期解での便数の取得
		int mt_cnt = 0;
		vector<string> init_custs1;
		vector<string> init_custs2;
		bool loop_out = false;

		int route_size = argvs.initroute.at(veh.vehno).size();
		for(int c=1; c<route_size; c++){
		 	string custno = argvs.initroute.at(veh.vehno).at(c);
			if(custno == "0"){
				if(loop_out == true){
					break;
				}
				else{
					init_custs1.clear();
					init_custs2.clear();
					mt_cnt += 1;			// depoならば便数+1
				}
			}
			else{
				if(custno == cust.custno){
					loop_out = true;	// 荷物が見つかった
				}
				else{
					if(loop_out == true){
						init_custs2.push_back(custno); 	// 対象荷物より後ろにあるべき荷物
					}
					else{
						init_custs1.push_back(custno);	// 対象荷物より前にあるべき荷物
					}
				}
			}
		}

		// 移動先での便数の取得
		int skv_mt_cnt = 0;
		vector<string> skv_custs1;
		vector<string> skv_custs2;
		loop_out=false;

		route_size = skv.size();
		for(int c=1; c<route_size; c++){
		 	string custno = customers.at(skv.at(c)).custno;
			if(custno == "0"){
				if(loop_out == true){
					break;
				}
				else{
					skv_custs1.clear();
					skv_custs2.clear();
					skv_mt_cnt += 1;			// depoならば便数+1
				}
			}
			else{
				if(custno == cust.custno){
					loop_out = true;	// 荷物が見つかった
				}
				else{
					if(free_or_additive(customers.at(c), veh, vehicles, customers, argvs)=="additive"){
						if(loop_out == true){
							skv_custs2.push_back(custno); 	// 対象荷物より後ろにあるべき荷物
						}
						else{
							skv_custs1.push_back(custno);	// 対象荷物より前にあるべき荷物
						}
					}
				}
			}
		}
		
		if(mt_cnt != skv_mt_cnt){
			// 便が違うのでルールエラー
			return false;
		}

		// 便内での他の荷物との順序は合っているかチェック
		// 対象荷物より前のチェック
		int skv_size = skv_custs1.size();
		for(int c=0; c<skv_size; c++){
			string cust = skv_custs1.at(c);
			if(vector_finder(init_custs2, cust) == true){
				// 前にある荷物が初期解では後ろに有った
				return false;
			}
		}

		// 対象荷物より後ろのチェック
		skv_size = skv_custs2.size();
		for(int c=0; c<skv_size; c++){
			string cust = skv_custs2.at(c);
			if(vector_finder(init_custs1, cust) == true){
				// 後ろにある荷物が初期解では前に有った
				return false;
			}
		}
	}

	return true;
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
}

