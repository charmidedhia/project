#ifndef UTIL_H
#include "../include/util.h"
#endif



string trim(const string& str, const string& whitespace)
{
    size_t strBegin = str.find_first_not_of(whitespace);
    if (strBegin == string::npos)
        return ""; // no content

    size_t strEnd = str.find_last_not_of(whitespace);
    size_t strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

double drand(double& seed) {
        seed *= 1389796;
        int q = (int)(seed / 2147483647);
        seed -= (double)q * 2147483647;
        return seed / 2147483647; }

// Returns a random integer 0 <= x < size. Seed must never be 0.
int irand(double& seed, int size) {
        return (int)(drand(seed) * size); }


