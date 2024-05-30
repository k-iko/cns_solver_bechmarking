#ifndef INCLUDED_CALC_AVEDIST_APE
#define INCLUDED_CALC_AVEDIST_APE

#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include "../Vehicle.hpp"
#include "../Customer.hpp"
#include "compare_time.hpp"
using namespace std;

double calc_avedist_APE(const vector<Vehicle> &vehicles, const vector<Customer> &customers, const map<int, double> &mvehicles, const double avedist_ape, const bool lastc_flag, const bool break_flag, const int decr_veh = -1);
vector<double> calc_avedist_APE_list(const vector<Vehicle> &vehicles, const vector<Customer> &customers, const double avedist_ape, const bool lastc_flag);

#endif