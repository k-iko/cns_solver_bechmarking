#ifndef INCLUDED_CALC_DIST
#define INCLUDED_CALC_DIST

#include <map>
#include <vector>
#include <string>
#include "../Vehicle.hpp"
#include "../Customer.hpp"
#include "get_dtable.hpp"
using namespace std;

double calc_dist(const Vehicle &vehicle, const vector<Customer> &customers, const int start_idx, const int end_idx, const map<tuple<string, double>, vector<vector<double>>> &management_dtable);

#endif