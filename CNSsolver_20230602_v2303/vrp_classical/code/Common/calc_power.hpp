#ifndef INCLUDED_CALC_POWER
#define INCLUDED_CALC_POWER

#include <map>
#include <vector>
#include <string>
#include "../Vehicle.hpp"
#include "../Customer.hpp"
#include "get_etable.hpp"
using namespace std;

double calc_power(const Vehicle &vehicle, const vector<Customer> &customers, const int start_idx, const int end_idx, const map<tuple<string, string, double>, vector<vector<double>>> &management_etable);

#endif