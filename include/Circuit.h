#ifndef CIRCUIT_H
#define CIRCUIT_H

#ifndef FAULT_H
#include "Fault.h"
#endif

#include <vector>
#include <map>
#include <string>
#include <set>
#include <iostream>

using namespace std;

class CNF;

class Line {
public:

    int id;
    string extern_name;
    bool is_input;
    bool is_output;

    vector<int> to_gates;
    int from_gate;

    Line(int line_num, string name, bool input, bool output) {
        id = line_num;
        extern_name = name;
        is_input = input;
        is_output = output;
        from_gate = -1;
    }

    // Copy constructor
    Line(const Line &l) {
    	id = l.id;
    	extern_name = l.extern_name;
    	is_input = l.is_input;
    	is_output = l.is_output;
    	to_gates = l.to_gates;
    	from_gate = l.from_gate;
    }

    void print() {
    	cout << "id = " << id << " extern_name = " << extern_name << " is_input = " << is_input << " is_output " << is_output;
    }
};

class Gate {
public:

    enum gate_type { AND = 0, OR, NOT, XOR, NAND, BUFF, NOR };

    int id;
    gate_type type;
    vector<string> input_lines;
    string output_line;



    Gate(int gate_num, gate_type t, vector<string> &inlines, string &outline) {
        id = gate_num;
        type = t;
        input_lines = inlines;
        output_line = outline;
    }

    // Copy constructor
    Gate(const Gate &g) {
    	id = g.id;
    	type = g.type;
    	input_lines = g.input_lines;
    	output_line = g.output_line;
    }

    void print() {

    	cout << "id = " << id << " type = " << type << " input_lines = ";
    	for (int i = 0; i < input_lines.size(); i++) {
    		cout << input_lines[i] << ", ";
    	}
    	cout << " output_line = " << output_line;
    }
};

class Circuit {

protected:
	map<string, int> extern_to_line;
	vector<Line> lines;
	vector<Gate> gates;
	int num_lines;
	int num_gates;

	vector<int> output_lines;
	vector<int> input_lines;

public:

	Circuit(char *benchFile);

	// Copy constructor
	Circuit(const Circuit &c) {
		extern_to_line = c.extern_to_line;
		lines = c.lines;
		gates = c.gates;
		num_lines = c.num_lines;
		num_gates = c.num_gates;
		output_lines = c.output_lines;
		input_lines = c.input_lines;
	}

	void getFaultList(vector<string> &faults);
    void getRTOPFaultList(vector<string> &faults);
    void getRandomFaultList(vector<string> &faults) ;

	int getNumLines() { return num_lines; }
        int getNumGates() { return num_gates; }
        int getNumInputLines() { return input_lines.size(); }
        int getNumOutputLines() { return output_lines.size(); }
        
        Line getLine(int i) { return lines[i]; }
        Gate getGate(int i) { return gates[i]; }
        int getInputLine(int i) { return input_lines[i]; }
        int getOutputLine(int i) { return output_lines[i]; }
        void getInputLines(set<string> &inlines);
        void getOutputLines(set<string> &outlines);
	int getLineID(string name) { return extern_to_line[name]; }
	map<string, int> &getExternToLine() { return extern_to_line; }

        void print();
};
#endif
