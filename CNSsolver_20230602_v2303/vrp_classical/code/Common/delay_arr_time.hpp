#ifndef INCLUDED_DELAY_ARR_TIME
#define INCLUDED_DELAY_ARR_TIME

#include <vector>
#include <algorithm>

#include "../Customer.hpp"
#include "../Vehicle.hpp"
#include "../Cppargs.hpp"
#include "compare_time.hpp"
#include "get_ttable.hpp"
#include "get_dtable.hpp"
#include "get_etable.hpp"
#include "get_xtable.hpp"

void delay_arr_time(const vector<Customer> &customers, vector<Vehicle> &vehicles, const Cppargs &argvs);

#endif