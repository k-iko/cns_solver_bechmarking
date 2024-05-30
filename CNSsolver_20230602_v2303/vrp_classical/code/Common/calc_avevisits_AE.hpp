#ifndef INCLUDED_CALC_AVEVISITS_AE
#define INCLUDED_CALC_AVEVISITS_AE

#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include "../Vehicle.hpp"
#include "../Customer.hpp"
using namespace std;

double calc_avevisits_AE(const vector<Vehicle> &vehicles, const vector<Customer> &customers, const map<int, double> &mvehicles, const double avevisits_ae, const bool bulkShipping_flag, const int decr_veh = -1);
vector<double> calc_avevisits_AE_list(const vector<Vehicle> &vehicles, const vector<Customer> &customers, const double avevisits_ae, const bool bulkShipping_flag);

#endif