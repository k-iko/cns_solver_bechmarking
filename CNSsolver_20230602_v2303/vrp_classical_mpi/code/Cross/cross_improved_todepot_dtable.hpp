#ifndef INCLUDED_CROSS_IMPROVED_TODEPOT_DTABLE
#define INCLUDED_CROSS_IMPROVED_TODEPOT_DTABLE

#include <map>
#include <vector>
#include <string>
#include "../Vehicle.hpp"
#include "../Customer.hpp"
#include "../Cppargs.hpp"
#include "../Common/calc_avetime_APE.hpp"
#include "../Common/calc_avetime_AE.hpp"
#include "../Common/calc_avevisits_APE.hpp"
#include "../Common/calc_avevisits_AE.hpp"
#include "../Common/calc_avedist_APE.hpp"
#include "../Common/calc_avedist_AE.hpp"
#include "../Common/return_lutime_penalty.hpp"
#include "../Common/calc_dist.hpp"
#include "../Common/calc_power.hpp"
#include "../Common/calc_exp.hpp"
#include "../Common/calc_time_pen.hpp"
#include "../Common/calc_vehicle_loadpickup_penalty.hpp"
#include "../Common/calc_visitnum.hpp"
#include "../Common/check_impassability.hpp"
#include "../Common/get_dtable.hpp"
#include "../Common/get_ttable.hpp"
#include "../Common/get_etable.hpp"
#include "../Common/get_xtable.hpp"
#include "../Common/compare_time.hpp"
#include "../Common/improved_or_not.hpp"
#include "../Common/dcopy.hpp"
#include "../Common/insert_breakspot.hpp"
#include "../Common/get_pick_opt_status.hpp"
#include "../Common/free_or_additive.hpp"
using namespace std;

tuple<bool, vector<int>, vector<int>, bool, bool, vector<int>, vector<int>, map<tuple<string, double>, vector<vector<double>>>, map<tuple<string, double>, vector<vector<double>>>, map<tuple<string, string, double>, vector<vector<double>>>, map<tuple<string, double>, vector<vector<double>>>, vector<vector<int>>, vector<Customer>> cross_improved_todepot_dtable(const Vehicle &vehicle1, const Vehicle &vehicle2, vector<Customer> &customers, const int pos1, const int pos2, const int pos3, const int pos4, vector<Vehicle> &vehicles, Cppargs &argvs, double &cdiff, bool no_depo_parkt_flag, bool one_table_flag);

#endif