#ifndef INCLUDED_CALC_TIME_PEN
#define INCLUDED_CALC_TIME_PEN

#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include "../Vehicle.hpp"
#include "../Customer.hpp"
#include "get_ttable.hpp"
using namespace std;

double calc_time_pen(const Vehicle &vehicle, const Customer &from_c, const Customer &to_c, double &lv_t, const double proc_time, const map<tuple<string, double>, vector<vector<double>>> &management_ttable, bool no_park);

#endif