#ifndef INCLUDED_CALC_EXP
#define INCLUDED_CALC_EXP

#include <map>
#include <vector>
#include <string>
#include "../Vehicle.hpp"
#include "../Customer.hpp"
#include "get_xtable.hpp"
using namespace std;

double calc_exp(const Vehicle &vehicle, const vector<Customer> &customers, const int start_idx, const int end_idx, const map<tuple<string, double>, vector<vector<double>>> &management_xtable);

#endif