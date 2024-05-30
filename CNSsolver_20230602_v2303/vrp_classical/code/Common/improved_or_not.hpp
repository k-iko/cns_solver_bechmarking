#ifndef INCLUDED_IMPROVED_OR_NOT
#define INCLUDED_IMPROVED_OR_NOT

#include "../Cppargs.hpp"
#include <cmath>
#include <algorithm>

bool improved_or_not(const double atpen, const double btpen, const double adist, const double bdist, const double aload, const double bload, const double aload2, const double bload2, const double atime, const double btime, const double avisits, const double bvisits, const double adist_error, const double bdist_error, const double aservice_time, const double bservice_time, const double apower, const double bpower, const double aexp, const double bexp, const double aunass, const double bunass, double &cdiff, const Cppargs &argvs);

#endif