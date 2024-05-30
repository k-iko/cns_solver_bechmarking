#ifndef INCLUDED_MAX_E_FROM_TABLE
#define INCLUDED_MAX_E_FROM_TABLE

#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include "compare_time.hpp"
#include "../Customer.hpp"
using namespace std;

double max_e_from_table(const string vtype, const string etype, const double start_time, const Customer &from_c, const Customer &to_c, const map<tuple<string, string, double>, vector<vector<double>>> &management_etable);

#endif