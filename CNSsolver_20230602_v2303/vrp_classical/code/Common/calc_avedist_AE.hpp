#ifndef INCLUDED_CALC_AVEDIST_AE
#define INCLUDED_CALC_AVEDIST_AE

#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include "../Vehicle.hpp"
#include "../Customer.hpp"
using namespace std;

double calc_avedist_AE(const vector<Vehicle> &vehicles, const vector<Customer> &customers, const map<int, double> &mvehicles, const double avedist_ae, const bool lastc_flag, const int decr_veh = -1);
vector<double> calc_avedist_AE_list(const vector<Vehicle> &vehicles, const vector<Customer> &customers, const double avedist_ae, const bool lastc_flag);

#endif