#ifndef INCLUDED_RETURN_LUTIME_PENALTY
#define INCLUDED_RETURN_LUTIME_PENALTY

#include "compare_time.hpp"

void return_lutime_penalty(const double aservice_time, const double bservice_time, const double lowertime, const double uppertime, double &atpen, double &btpen);

#endif