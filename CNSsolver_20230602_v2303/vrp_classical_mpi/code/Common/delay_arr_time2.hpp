#ifndef INCLUDED_DELAY_ARR_TIME2
#define INCLUDED_DELAY_ARR_TIME2

#include "../Customer.hpp"
#include "../Vehicle.hpp"
#include "../Cppargs.hpp"
#include "../Common/delay_arr_time.hpp"
#include "../Common/improved_or_not.hpp"
#include "../Common/calc_avetime_AE.hpp"
#include "../Common/calc_avetime_APE.hpp"
#include "../Common/calc_avedist_AE.hpp"
#include "../Common/calc_avedist_APE.hpp"
#include "../Common/return_lutime_penalty.hpp"

void delay_arr_time2(const vector<Customer> &customers, vector<Vehicle> &vehicles, const Cppargs &argvs);

#endif