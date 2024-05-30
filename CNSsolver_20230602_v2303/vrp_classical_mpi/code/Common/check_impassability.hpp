#ifndef INCLUDED_CHECK_IMPASSABILITY
#define INCLUDED_CHECK_IMPASSABILITY

#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include "../Vehicle.hpp"
#include "../Customer.hpp"
#include "../Cppargs.hpp"
#include "get_dtable.hpp"
#include "get_ttable.hpp"
#include "get_etable.hpp"
#include "get_xtable.hpp"
using namespace std;

bool check_impassability(const Vehicle &vehicle, const vector<Customer> &customers, double lv_t, const int start_idx, const int last_idx, const map<int, double> &servt_dict, double &dist, double &power, double &exp, const vector<int> &no_park_idx, bool pick_opt_flag, const Cppargs &argvs);

#endif