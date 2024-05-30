#include <algorithm>
#include <iostream>
#include <vector>
#include "get_parkt_time_flag.hpp"

vector<bool> get_parkt_time_flag(const vector<Customer> &customers, vector<int> &route)
{
	vector<bool> parkt_time_flag;
	string bspotid = "-1";
	string no_breakspot_bspotid = "-1";
		
	int route_size = route.size();

	for(int r=0; r<route_size; r++){
		Customer c = customers.at(route.at(r));
		if((c.custno.at(0) == 'e') || (c.custno.at(0) == 'b')){
			parkt_time_flag.push_back(false);
			bspotid = c.spotid;
		}
		else{
			if((c.spotid != bspotid) || (c.spotid != no_breakspot_bspotid)){
				parkt_time_flag.push_back(true);
			}
			else{
				parkt_time_flag.push_back(false);
			}
			bspotid = c.spotid;
			no_breakspot_bspotid = c.spotid;
		}
	}

	return parkt_time_flag;
}
