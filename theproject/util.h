#ifndef UTIL_H
#define UTIL_H
#include <cstdlib>
#include <cmath>
using namespace std;
#define HUGEEPSILON .005 // epsilon is really big.
#define SMALLEPSILON .00005 //smaller
bool kindOfEqual(double x, double y){

	if(fabs(x-y) <= HUGEEPSILON)
		return true;
	else
		return false;
}

bool almostEqual(double x, double y){

	if(fabs(x-y) <= SMALLEPSILON)
		return true;
	else
		return false;
}

#endif