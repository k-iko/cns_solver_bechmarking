#ifndef INCLUDED_CALC_AVEVISITS_APE
#define INCLUDED_CALC_AVEVISITS_APE

#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include "../Vehicle.hpp"
#include "../Customer.hpp"
#include "compare_time.hpp"
using namespace std;

double calc_avevisits_APE(const vector<Vehicle> &vehicles, const vector<Customer> &customers, const map<int, double> &mvehicles, const double avevisits_ape, const bool bulkShipping_flag, const bool break_flag, const int decr_veh = -1);
vector<double> calc_avevisits_APE_list(const vector<Vehicle> &vehicles, const vector<Customer> &customers, const double avevisits_ape, const bool bulkShipping_flag);

#endif