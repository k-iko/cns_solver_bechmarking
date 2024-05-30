#include <algorithm>
#include <iostream>
#include <vector>
#include "get_pick_opt_status.hpp"

bool get_pick_opt_status(const vector<Customer> &customers, const Vehicle &vehicle, Cppargs &argvs)
{
	bool pick_opt_flag = false;

	if(argvs.pickup_flag == true){
		if(argvs.bulkShipping_flag == true){
			int last_nobrspot_idx = -1;
			int vehicle_route_size = vehicle.route.size();

			for(int idx = vehicle_route_size-2; idx > 0; idx--){
                if(customers.at(customers.at(vehicle.route.at(idx)).tieup_customer.at(0)).custno.at(0) != 'b'){
					last_nobrspot_idx = idx;
					break;
				}
			}
			if(last_nobrspot_idx != -1){
				if((customers.at(customers.at(vehicle.route.at(last_nobrspot_idx)).tieup_customer.at(0)).custno=="0") &&  (customers.at(customers.at(vehicle.route.at(vehicle_route_size - 1)).tieup_customer.at(0)).custno!="0")){
					for(int i = last_nobrspot_idx-1; i > 0; i--){
						if(customers.at(customers.at(vehicle.route.at(i)).tieup_customer.at(0)).custno=="0"){
							break;
						}
						if(customers.at(vehicle.route.at(i)).requestType=="PICKUP"){
							pick_opt_flag = true;
							break;
						}
					}
				}
			}
		}
		else{
			int last_nobrspot_idx = -1;
			int vehicle_route_size = vehicle.route.size();

			for(int idx = vehicle_route_size-2; idx > 0; idx--){
				if(customers.at(vehicle.route.at(idx)).custno.at(0) != 'b'){
					last_nobrspot_idx = idx;
					break;
				}
			}

			if(last_nobrspot_idx != -1){
				if((customers.at(vehicle.route.at(last_nobrspot_idx)).custno=="0") &&  (customers.at(vehicle.route.at(vehicle_route_size - 1)).custno!="0")){
					for(int i = last_nobrspot_idx-1; i > 0; i--){
						if(customers.at(vehicle.route.at(i)).custno=="0"){
							break;
						}
						if(customers.at(vehicle.route.at(i)).requestType=="PICKUP"){
							pick_opt_flag = true;
							break;
						}
					}
				}
			}	
		}
	}

	return pick_opt_flag;

}

int get_pick_opt_idx(const vector<Customer> &customers, const Vehicle &vehicle, Cppargs &argvs)
{
	bool pick_opt_flag = false;
	int last_nobrspot_idx = -1;

	if(argvs.pickup_flag == true){
		if(argvs.bulkShipping_flag == true){
			int vehicle_route_size = vehicle.route.size();

			for(int idx = vehicle_route_size-2; idx > 0; idx--){
                if(customers.at(customers.at(vehicle.route.at(idx)).tieup_customer.at(0)).custno.at(0) != 'b'){
					last_nobrspot_idx = idx;
					break;
				}
			}
			if(last_nobrspot_idx != -1){
				if((customers.at(customers.at(vehicle.route.at(last_nobrspot_idx)).tieup_customer.at(0)).custno=="0") &&  (customers.at(customers.at(vehicle.route.at(vehicle_route_size - 1)).tieup_customer.at(0)).custno!="0")){
					for(int i = last_nobrspot_idx-1; i > 0; i--){
						if(customers.at(customers.at(vehicle.route.at(i)).tieup_customer.at(0)).custno=="0"){
							break;
						}
						if(customers.at(vehicle.route.at(i)).requestType=="PICKUP"){
							pick_opt_flag = true;
							break;
						}
					}
				}
			}
		}
		else{
			int vehicle_route_size = vehicle.route.size();

			for(int idx = vehicle_route_size-2; idx > 0; idx--){
				if(customers.at(vehicle.route.at(idx)).custno.at(0) != 'b'){
					last_nobrspot_idx = idx;
					break;
				}
			}

			if(last_nobrspot_idx != -1){
				if((customers.at(vehicle.route.at(last_nobrspot_idx)).custno=="0") &&  (customers.at(vehicle.route.at(vehicle_route_size - 1)).custno!="0")){
					for(int i = last_nobrspot_idx-1; i > 0; i--){
						if(customers.at(vehicle.route.at(i)).custno=="0"){
							break;
						}
						if(customers.at(vehicle.route.at(i)).requestType=="PICKUP"){
							pick_opt_flag = true;
							break;
						}
					}
				}
			}	
		}
	}

	return last_nobrspot_idx;

}


Vehicle pick_opt_rest(const vector<Customer> &customers, Vehicle &vehicle, bool pick_opt_flag, Cppargs &argvs)
{
	if(argvs.pickup_flag == true){
		int last_nobrspot_idx = get_pick_opt_idx(customers, vehicle, argvs);
		int vehicle_route_size = vehicle.route.size();

		if(last_nobrspot_idx != -1){
			if((argvs.bulkShipping_flag==false && pick_opt_flag==false && customers.at(vehicle.route.at(last_nobrspot_idx)).custno=="0" && customers.at(vehicle.route.at(vehicle_route_size-1)).custno!="0") ||
				(argvs.bulkShipping_flag==true && customers.at(customers.at(vehicle.route.at(last_nobrspot_idx)).tieup_customer.at(0)).custno=="0" && customers.at(customers.at(vehicle.route.at(vehicle_route_size-1)).tieup_customer.at(0)).custno!="0")){
					vehicle.route.erase(vehicle.route.begin()+last_nobrspot_idx);
					//vehicle.update_totalcost_dtable(customers, argvs);
			}
		}
	}

	return vehicle;
}
