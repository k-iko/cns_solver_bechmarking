#ifndef INCLUDED_GET_XTABLE
#define INCLUDED_GET_XTABLE

#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include "compare_time.hpp"
using namespace std;

double get_xtable(const string vtype, const double start_time, const map<tuple<string, double>, vector<vector<double>>> &management_xtable, const int from_c, const int to_c);

#endif