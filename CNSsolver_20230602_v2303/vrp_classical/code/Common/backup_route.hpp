#ifndef INCLUDED_BACKUP_ROUTE
#define INCLUDED_BACKUP_ROUTE

#include "../Vehicle.hpp"
#include "../Cppargs.hpp"
#include "compare_time.hpp"
#include "insert_breakspot.hpp"

void backup_route(const vector<Vehicle> &vehicles, const vector<Customer> &customers, Cppargs &argvs);

#endif