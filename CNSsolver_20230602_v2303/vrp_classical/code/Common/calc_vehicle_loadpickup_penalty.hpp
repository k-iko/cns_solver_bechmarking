#ifndef INCLUDED_CALC_VEHICLE_LOADPICKUP_PENALTY
#define INCLUDED_CALC_VEHICLE_LOADPICKUP_PENALTY

#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include "../Vehicle.hpp"
#include "../Customer.hpp"
using namespace std;

tuple<double, double> calc_vehicle_loadpickup_penalty(const Vehicle &vehicle, const vector<Customer> &customers, const vector<int> &route);

#endif