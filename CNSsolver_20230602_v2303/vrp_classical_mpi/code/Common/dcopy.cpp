
#include "dcopy.hpp"

std::vector<std::string> split(std::string str, char del) {
    int first = 0;
    int last = str.find_first_of(del);
 
    std::vector<std::string> result;
 
    int str_size = str.size();
    while (first < str_size) {
        std::string subStr(str, first, last - first);
 
        result.push_back(subStr);
 
        first = last + 1;
        last = str.find_first_of(del, first);
 
        if (last == std::string::npos) {
            last = str.size();
        }
    }
 
    return result;
}


Vehicle dcopy(Vehicle sv)
/**
 * @brief Vehicleのdeep copy
 * @param[in] sv Vehicle src
 * @return Vehicle コピーされた Vehicle
 * @details Vehicleのdeep copyを行う関数。
 */
{
    Vehicle dv(
        sv.route,
        sv.arr,
        sv.rem_batt,
        sv.chg_batt,
        sv.slack_t,
        sv.st,
        sv.lv,
        sv.remdist,
        sv.vehno,
        sv.cap,
        sv.cap2,
        sv.totalcost,
        sv.totalcost_wolast,
        sv.load_onbrd,
        sv.load_onbrd2,
        sv.load_pickup,
        sv.load_pickup2,
        sv.opskill,
        sv.drskill,
        sv.e_capacity,
        sv.e_initial,
        sv.e_cost,
        sv.e_margin,
        sv.time_pen,
        sv.load_pen,
        sv.load_pen2,
        sv.vtype,
        sv.etype,
        sv.direct_flag,
        sv.starthour,
        sv.endhour,
        sv.maxvisit,
        sv.maxrotate,
        sv.depotservt,
        sv.rmpriority,
        sv.chg_batt_dict,
        sv.delay_arr,
        sv.delay_totalcost,
        sv.delay_totalcost_wolast,
        sv.delay_rem_batt,
        sv.delay_chg_batt,
        sv.delay_chg_batt_dict,
        sv.delay_slack_t,
        sv.takebreak,
        sv.reserved_v_flag,
        sv.delay_remdist,
        sv.delay_lv,
        sv.delay_st
    );

	return dv;
}

vector<vector<int>> r_backup(vector<Vehicle> &vehicles)
/**
 * @brief Vehicleのrouteのバックアップ
 * @param[in] Vehicle &vehicle Vehicle src
 * @return b_route バックアップされたroute
 * @details Vehicleのdeep copyを行う関数。
 */
{
    vector<vector<int>> b_route;

    int vehicles_size = vehicles.size();
    for(int i=0; i < vehicles_size; i++){
        vector<int> r = vehicles.at(i).route;
        b_route.push_back(r);
    }

    return b_route;
}

vector<Vehicle> r_restore(vector<Vehicle> &vehicles, vector<vector<int>> rs, Cppargs &argvs, vector<Customer> &customer)
/**
 * @brief Vehicleのrouteのレストア
 * @param[in] Vehicle &vehicle Vehicle src
 * @param[in] vector<vector<int>> rs route
 * @param[in] Cppargs &argvs
 * @param[in] vector<Customer> &customer

 * @return b_route バックアップされたroute
 * @details Vehicleのdeep copyを行う関数。
 */
{
	int rsidx = 0;

    int vehicles_size = vehicles.size();
    for(int i=0; i < vehicles_size; i++){
        vehicles[i].route = rs.at(rsidx);
		rsidx++;
		vehicles[i].update_totalcost_dtable(customer, argvs);
    }

	return vehicles;
}

