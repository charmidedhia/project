#ifndef FAULT_H
#define FAULT_H

#include <string>

using namespace std;

class Fault {

public:

    string line;
    int stuckat;

    Fault(string l, int s) : line(l), stuckat(s) {}
    Fault() { stuckat = -1; }

    bool operator==(const Fault &other) const { 
        return (line.compare(other.line) == 0) && (stuckat == other.stuckat);
    }

    bool operator<(const Fault &other) const {
        return (line.compare(other.line) > 0) || ((line.compare(other.line) == 0) && (stuckat < other.stuckat));  
    }
};
#endif
