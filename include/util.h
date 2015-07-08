#ifndef UTIL_H
#define UTIL_H

#include <string>

using namespace std;

string trim(const string& str, const string& whitespace = " \t");

double drand(double& seed);

// Returns a random integer 0 <= x < size. Seed must never be 0.
int irand(double& seed, int size);
#endif
