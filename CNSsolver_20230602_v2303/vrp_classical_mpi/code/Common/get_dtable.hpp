#ifndef INCLUDED_GET_DTABLE
#define INCLUDED_GET_DTABLE

#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include "compare_time.hpp"
using namespace std;

double get_dtable(const string vtype, const double start_time, const map<tuple<string, double>, vector<vector<double>>> &management_dtable, const int from_c, const int to_c);

#endif