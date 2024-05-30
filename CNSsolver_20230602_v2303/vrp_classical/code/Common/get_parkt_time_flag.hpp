#ifndef INCLUDED_GET_PARKT_TIME_FLAG
#define INCLUDED_GET_PARKT_TIME_FLAG

#include "../Vehicle.hpp"
#include "../Customer.hpp"
#include "../Cppargs.hpp"
using namespace std;

vector<bool> get_parkt_time_flag(const vector<Customer> &customers, vector<int> &route);


#endif