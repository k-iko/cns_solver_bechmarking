#ifndef INCLUDED_CALC_AVETIME_AE
#define INCLUDED_CALC_AVETIME_AE

#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include "../Vehicle.hpp"
#include "../Customer.hpp"
using namespace std;

double calc_avetime_AE(const vector<Vehicle> &vehicles, const vector<Customer> &customers, const map<int, double> &mvehicles, const double avetime_ae, const bool lastc_flag, const int decr_veh = -1);
vector<double> calc_avetime_AE_list(const vector<Vehicle> &vehicles, const vector<Customer> &customers, const double avetime_ae, const bool lastc_flag);

#endif