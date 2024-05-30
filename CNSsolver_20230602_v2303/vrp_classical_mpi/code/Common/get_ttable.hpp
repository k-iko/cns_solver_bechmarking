#ifndef INCLUDED_GET_TTABLE
#define INCLUDED_GET_TTABLE

#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include "compare_time.hpp"
using namespace std;

double get_ttable(const string vtype, const double start_time, const map<tuple<string, double>, vector<vector<double>>> &management_ttable, const int from_c, const int to_c);

#endif