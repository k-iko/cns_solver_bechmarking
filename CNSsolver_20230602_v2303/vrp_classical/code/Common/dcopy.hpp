#ifndef INCLUDED_DCOPY
#define INCLUDED_DCOPY

#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include "../Vehicle.hpp"
using namespace std;

Vehicle dcopy(Vehicle sv);
vector<vector<int>> r_backup(vector<Vehicle> &vehicles);
vector<Vehicle> r_restore(vector<Vehicle> &vehicles, vector<vector<int>> rs, Cppargs &argvs, vector<Customer> &customer);

#endif