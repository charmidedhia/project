#ifndef BENCHFILEPARSER_H
#define BENCHFILEPARSER_H

#ifndef UTIL_H
#include "util.h"
#endif

#ifndef CIRCUIT_H
#include "Circuit.h"
#endif

#include <map>
#include <vector>

using namespace std;



void parseIOLine(string strInput, int prefixLen, bool is_input, map<string, int> &extern_to_line, int &num_lines, vector<Line> &lines, vector<int> &input_lines, vector<int> &output_lines);
void parseGate(string strInput, map<string, int> &extern_to_line, int &num_lines, vector<Line> &lines, int &num_gates, vector<Gate> &gates);
void parseDFF(string strInput, map<string, int> &extern_to_line, int &num_lines, vector<Line> &lines, vector<int> &input_lines, vector<int> &output_lines);


#endif
