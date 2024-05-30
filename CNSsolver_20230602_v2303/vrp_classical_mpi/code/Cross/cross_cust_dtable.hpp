#ifndef INCLUDED_CROSS_CUST_DTABLE
#define INCLUDED_CROSS_CUST_DTABLE

#include <map>
#include <vector>
#include <string>
#include "../Vehicle.hpp"
#include "../Customer.hpp"
#include "../Cppargs.hpp"
using namespace std;

void cross_cust_dtable(Vehicle &vehicle1, Vehicle &vehicle2, const vector<Customer> &customers, const int pos1, const int pos2, const int pos3, const int pos4, const vector<int> &custs1, const vector<int> &custs2, bool pick_opt_flag1, bool pick_opt_flag2, const vector<int> &route1, const vector<int> &route2, const Cppargs &argvs);

#endif