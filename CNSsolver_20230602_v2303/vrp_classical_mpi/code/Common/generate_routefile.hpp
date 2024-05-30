#ifndef INCLUDED_GENERATE_ROUTEFILE
#define INCLUDED_GENERATE_ROUTEFILE

#include <fstream>

#include "../Vehicle.hpp"
#include "../Customer.hpp"

void generate_routefile(const string filename, const vector<Vehicle> &vehicles, const vector<Customer> &customers, const bool bulkShipping_flag);

#endif