#ifndef INCLUDED_CALC_VISITNUM
#define INCLUDED_CALC_VISITNUM

#include <map>
#include <vector>
#include <string>
#include "../Vehicle.hpp"
#include "../Customer.hpp"
#include "get_dtable.hpp"
using namespace std;

int calc_visitnum(const vector<int> &route, const vector<Customer> &customers);

#endif