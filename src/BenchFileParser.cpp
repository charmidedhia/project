
#ifndef BENCHFILEPARSER_H
#include "../include/BenchFileParser.h"
#endif

#ifndef CIRCUIT_H
#include "../include/Circuit.h"
#endif

#include <vector>
#include <map>
#include <sstream>
#include <stdlib.h>
#include <iostream>

using namespace std;

void parseIOLine(string strInput, int prefixLen, bool is_input, map<string, int> &extern_to_line, int &num_lines, vector<Line> &lines, vector<int> &input_lines, vector<int> &output_lines) {
    size_t left_bracket = strInput.find_first_of('(');
    size_t right_bracket = strInput.find_first_of(')');

    size_t name_len = right_bracket - left_bracket - 1;

    char *name = new char[name_len+1];
    strInput.copy(name, name_len, left_bracket+1);
    name[name_len] = '\0';

    string trimmed_name = trim(name);

    int line_num = -1;
    if (extern_to_line.find(trimmed_name) == extern_to_line.end()) {
        line_num = num_lines++;
        extern_to_line[trimmed_name] = line_num;
        lines.push_back(Line(line_num, trimmed_name, is_input, !is_input));
    } else {
        line_num = extern_to_line[trimmed_name];
        lines[line_num].is_input = is_input;
        lines[line_num].is_output = !is_input;
    }
    if (is_input) {
        input_lines.push_back(line_num);
    } else {
        output_lines.push_back(line_num);
    }

    delete name;
}

void parseGate(string strInput, map<string, int> &extern_to_line, int &num_lines, vector<Line> &lines, int &num_gates, vector<Gate> &gates) {
    size_t equals_index = strInput.find("=");

    char *left_name = new char[equals_index+1];
    strInput.copy(left_name, equals_index, 0);
    left_name[equals_index] = '\0';
    string trimmed_left = trim(left_name);

    size_t left_bracket = strInput.find_first_of("(");
    size_t right_bracket = strInput.find_first_of(")");
    size_t inputs_len = right_bracket - left_bracket - 1;
    char *inputs = new char[inputs_len+1];
    strInput.copy(inputs, inputs_len, left_bracket+1);
    inputs[inputs_len] = '\0';

    istringstream iss(inputs);
    vector<string> inputs_vec;
    while (iss) {
        string sub;
        iss >> sub;
        // If there is a zero-length string we're done
        if (sub.length() == 0) {
        	break;
        }
        // Remove the comma if there is one
        size_t comma_location = sub.find_first_of(",");
        if (comma_location != string::npos) {
        	string new_sub = sub.substr(0, comma_location);
        	inputs_vec.push_back(new_sub);
        } else {
        	inputs_vec.push_back(sub);
        }
    }

    size_t gate_len = left_bracket - equals_index - 1;
    char *gate_name = new char[gate_len+1];
    strInput.copy(gate_name, gate_len, equals_index+1);
    gate_name[gate_len] = '\0';
    string gate = trim(gate_name);

    Gate::gate_type type = Gate::AND;
    if (gate.compare("AND") == 0) {
        type = Gate::AND;
    } else if (gate.compare("OR") == 0) {
        type = Gate::OR;
    } else if (gate.compare("XOR") == 0) {
        type = Gate::XOR;
    } else if (gate.compare("BUFF") == 0) {
        type = Gate::BUFF;
    } else if (gate.compare("NAND") == 0) {
        type = Gate::NAND;
    } else if (gate.compare("NOT") == 0) {
        type = Gate::NOT;
    } else if (gate.compare("NOR") == 0) {
        type = Gate::NOR;
    } else {
        cerr << "ERROR: unrecognized gate: " << gate << endl;
        exit(1);
    }

    // Use trimmed_left, inputs_vec and gate
    int gate_num = num_gates++;
    gates.push_back(Gate(gate_num, type, inputs_vec, trimmed_left));

    // Create the mentioned lines if they don't already exist, and update
    // their to and from gates.
    if (extern_to_line.find(trimmed_left) == extern_to_line.end()) {
        int line_num = num_lines++;
        extern_to_line[trimmed_left] = line_num;
        lines.push_back(Line(line_num, trimmed_left, false, false));
    }
    lines[extern_to_line[trimmed_left]].from_gate = gate_num;


    /*cout << "extern_to_line: ";
    	for (map<string, int>::iterator iter = extern_to_line.begin(); iter != extern_to_line.end(); iter++) {
    		cout << iter->first << " : " << iter->second << endl;
    	}*/

    for (int i = 0; i < inputs_vec.size(); i++) {
        if (extern_to_line.find(inputs_vec[i]) == extern_to_line.end()) {
            int line_num = num_lines++;
            extern_to_line[inputs_vec[i]] = line_num;
            lines.push_back(Line(line_num, inputs_vec[i], false, false));
        }
        lines[extern_to_line[inputs_vec[i]]].to_gates.push_back(gate_num);
    }

    delete left_name;
    delete inputs;
    delete gate_name;
}
void parseDFF(string strInput, map<string, int> &extern_to_line, int &num_lines, vector<Line> &lines, vector<int> &input_lines, vector<int> &output_lines){
    size_t equals_index = strInput.find("=");

    char *left_name = new char[equals_index+1];
    strInput.copy(left_name, equals_index, 0);
    left_name[equals_index] = '\0';
    string trimmed_left = trim(left_name);

    size_t left_bracket = strInput.find_first_of("(");
    size_t right_bracket = strInput.find_first_of(")");
    size_t inputs_len = right_bracket - left_bracket - 1;
    char *inputs = new char[inputs_len+1];
    strInput.copy(inputs, inputs_len, left_bracket+1);
    inputs[inputs_len] = '\0';
    int line_num = -1;
    if (extern_to_line.find(trimmed_left) == extern_to_line.end()) {
        line_num = num_lines++;
        extern_to_line[trimmed_left] = line_num;
        lines.push_back(Line(line_num, trimmed_left, true, false));//left is output of dff , so input of circuit
    } else {
        line_num = extern_to_line[trimmed_left];
        lines[line_num].is_input = true;
    }
    input_lines.push_back(line_num);
    line_num=-1;
    if (extern_to_line.find(inputs) == extern_to_line.end()) {
        line_num = num_lines++;
        extern_to_line[inputs] = line_num;
        lines.push_back(Line(line_num, inputs, false, true));//left is output of dff , so input of circuit
    } else {
        line_num = extern_to_line[inputs];
        lines[line_num].is_output = true;
    }
    output_lines.push_back(line_num);
    
}