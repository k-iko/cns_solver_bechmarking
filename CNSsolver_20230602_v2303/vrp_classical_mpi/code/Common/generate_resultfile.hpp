#ifndef INCLUDED_GENERATE_RESULTFILE
#define INCLUDED_GENERATE_RESULTFILE

#include <fstream>
#include <cmath>

#include "../Vehicle.hpp"
#include "../Customer.hpp"
#include "../Cppargs.hpp"
#include "get_dtable.hpp"
#include "get_etable.hpp"
#include "get_xtable.hpp"

void generate_resultfile(const string filename, const vector<Vehicle> &vehicles, const vector<Customer> &customers, const Cppargs &argvs);

#endif