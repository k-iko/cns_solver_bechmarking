#include "calc_visitnum.hpp"

/**
 * @brief 車両の訪問数を計算する関数
 * @param[in] vehicle 車両オブジェクト
 * @param[in] customers 配送先リスト
 * @return double 車両の区間内の総走行距離
 * @details 車両のstart_idx～end_idxの区間内の総走行距離を計算する関数。
 */
int calc_visitnum(const vector<int> &route, const vector<Customer> &customers)
{
	int visitnum = 0;
	bool bbreak = false;
	string bspotid = "-1";
	vector<string> lstbb;

    int route_size = route.size();
	for(int r=1; r<route_size-1; r++){
		if((customers.at(route.at(r)).custno.at(0) == 'e') || (customers.at(route.at(r)).custno == "0")){
			bspotid = customers.at(r).spotid;
			lstbb.clear();
			continue;
        }

		if(customers.at(route.at(r)).custno.at(0) == 'b'){
			lstbb.push_back(customers.at(r).custno);
			continue;
        }
		else if(lstbb.size() > 0 && bspotid == customers.at(r).spotid){
			bspotid = customers.at(route.at(r)).spotid;
            bool bfl = false;
            int lstbb_size = lstbb.size();
			for(int bb=0; bb<lstbb_size; bb++){
				if((lstbb.at(bb) != customers.at(route.at(r)).spotid) && (lstbb.at(bb) != "")){
                    bfl = true;
					break;
                }
            }
            if(bfl){
                visitnum += 1;
                lstbb.clear();
            }
			else{
				lstbb.clear();
				continue;
            }
        }
		else{
			if(customers.at(route.at(r)).spotid == bspotid){	// 同じスポットIDが連続したらカウントしない
				continue;
            }
			visitnum += 1;
			bspotid = customers.at(route.at(r)).spotid;
        }
    }

	return visitnum;

}

