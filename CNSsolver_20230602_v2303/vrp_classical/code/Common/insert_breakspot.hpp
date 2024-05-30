#ifndef INCLUDED_INCLUDE_BREAKSPOT
#define INCLUDED_INCLUDE_BREAKSPOT

#include "../Vehicle.hpp"
#include "../Customer.hpp"
#include "../Cppargs.hpp"
#include "dcopy.hpp"
#include "check_impassability.hpp"
#include "get_parkt_time_flag.hpp"
#include "get_pick_opt_status.hpp"
using namespace std;

bool insert_breakspot(vector<Customer> &customers, Vehicle &veh, vector<Vehicle> &vehicles, Cppargs &argvs, Vehicle &veh2);
int get_breakspot_pattern(const vector<Vehicle> &vehicles, Cppargs &argvs);
Customer create_customer(string _custno, const Cppargs &argvs);


#endif