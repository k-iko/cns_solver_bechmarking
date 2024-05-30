#ifndef INCLUDED_GET_ETABLE
#define INCLUDED_GET_ETABLE

#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include "compare_time.hpp"
using namespace std;

double get_etable(const string vtype, const string etype, const double start_time, const map<tuple<string, string, double>, vector<vector<double>>> &management_etable, const int from_c, const int to_c);

#endif