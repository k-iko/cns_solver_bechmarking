#ifndef INCLUDED_MAX_DIST_FROM_TABLE
#define INCLUDED_MAX_DIST_FROM_TABLE

#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include "compare_time.hpp"
#include "../Customer.hpp"
using namespace std;

double max_dist_from_table(const string vtype, const double start_time, const Customer &from_c, const Customer &to_c, const map<tuple<string, double>, vector<vector<double>>> &management_dtable);

#endif