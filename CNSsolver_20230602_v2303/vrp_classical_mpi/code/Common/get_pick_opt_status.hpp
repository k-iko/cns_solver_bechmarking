#ifndef INCLUDED_GET_PICK_OPT_STATUS
#define INCLUDED_GET_PICK_OPT_STATUS

#include "../Vehicle.hpp"
#include "../Customer.hpp"
#include "../Cppargs.hpp"
using namespace std;

bool get_pick_opt_status(const vector<Customer> &customers, const Vehicle &vehicle, Cppargs &argvs);
int get_pick_opt_idx(const vector<Customer> &customers, const Vehicle &vehicle, Cppargs &argvs);
Vehicle pick_opt_rest(const vector<Customer> &customers,  Vehicle &vehicle, bool pick_opt_flag, Cppargs &argvs);


#endif