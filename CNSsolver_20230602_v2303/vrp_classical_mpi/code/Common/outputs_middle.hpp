#ifndef INCLUDED_OUTPUTS_MIDDLE
#define INCLUDED_OUTPUTS_MIDDLE

#include <time.h>
#include <algorithm>
#include <cmath>

#include "../Vehicle.hpp"
#include "../Customer.hpp"
#include "../Cppargs.hpp"
#include "generate_routefile.hpp"
#include "generate_resultfile.hpp"

void outputs_middle(const vector<Vehicle> &vehicles, const vector<Customer> &customers, const Cppargs &argvs, string &sLogBuf);

#endif