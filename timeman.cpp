#include <cmath>

#include "timeman.h"

#define BASE_TIME_PCT 0.07

// values calculated by desmos: https://www.desmos.com/calculator/57lwlgtxv8
// rough curve shape from lc0's old time algorithm: https://lczero.org/blog/2018/09/time-management/
double timeman::calc_base_time(int remainingTime, int x) {
	double out;
	if (x < 44)
		out = 0.00000038026*pow(x, 5) + 0.0000373386*pow(x, 4) - 0.00660532*pow(x, 3) + 0.191887*pow(x, 2) + 14.9884;
	else
		out = 107498 * pow(x, 1 / -0.445285) + 4.50529;
	out /= 50;
	return remainingTime * BASE_TIME_PCT * out;
}