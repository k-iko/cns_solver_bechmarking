#ifndef INCLUDED_INCLUDE_FREE_OR_ADDITIVE
#define INCLUDED_INCLUDE_FREE_OR_ADDITIVE

#include "../Vehicle.hpp"
#include "../Customer.hpp"
#include "../Cppargs.hpp"
using namespace std;

string free_or_additive(const Customer cust, const Vehicle veh, const vector<Vehicle> &vehicles, const vector<Customer> &customers, const Cppargs &argvs);
bool in_additive(Customer const cust, const Vehicle veh, const vector<int> &skv, const vector<Customer> &customers, const vector<Vehicle> &vehicles, const Cppargs &argvs);


#endif