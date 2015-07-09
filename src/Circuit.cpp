
#ifndef CIRCUIT_H
#include "../include/Circuit.h"
#endif

#ifndef BENCHFILEPARSER_H
#include "../include/BenchFileParser.h"
#endif

#ifndef CNF_H
#include "../include/CNF.h"
#endif

#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <algorithm>
#include <queue> 
#include <cstdlib>    

using namespace std;

Circuit::Circuit(char *benchFile) {

	num_lines = 0;
	num_gates = 0;

	ifstream inf(benchFile);

	while (inf) {
		string strInput;
		getline(inf, strInput);
		size_t firstNonWhitespace = strInput.find_first_not_of(" \t");

		if (firstNonWhitespace == string::npos || strInput[firstNonWhitespace] == '#') {
			// comment, ignore
		} else if (strInput.find("INPUT") == firstNonWhitespace) {
			parseIOLine(strInput, 5, true, extern_to_line, num_lines, lines, input_lines, output_lines);
		} else if (strInput.find("OUTPUT") == firstNonWhitespace) {
			parseIOLine(strInput, 6, false, extern_to_line, num_lines, lines, input_lines, output_lines);
		} else if (strInput.find("DFF") != string::npos) {
			parseDFF(strInput, extern_to_line, num_lines, lines, input_lines, output_lines);
		} else if (strInput.find("=") != string::npos) {
			parseGate(strInput, extern_to_line, num_lines, lines, num_gates, gates);
		}
	}
}

void Circuit::print() {

	cout << "num_lines = " << num_lines << endl;
	cout << "num_gates = " << num_gates << endl;

	cout << "lines: " << endl;
	for (int i = 0; i < lines.size(); i++) {
		cout << "lines[" << i << "]= ";
		lines[i].print();
	}

	cout << "gates: " << endl;
	for (int i = 0; i < gates.size(); i++) {
		cout << "gates[" << i << "]= ";
		gates[i].print();
	}

	cout << "extern_to_line: ";
	for (map<string, int>::iterator iter = extern_to_line.begin(); iter != extern_to_line.end(); iter++) {
		cout << iter->first << " : " << iter->second << endl;
	}
}

void Circuit::getFaultList(vector<string> &faults) {

	// For now, don't allow faults on input and output lines
	for (int i = 0; i < lines.size(); i++) {
		if (!lines[i].is_input && !lines[i].is_output) {
			faults.push_back(lines[i].extern_name);
		}
	}
}

void Circuit::getRTOPFaultList(vector<string> &faults){
	cout<<"RTOP";
	vector<bool> inList;
	for(int i=0;i<lines.size();i++){
		inList.push_back(false);
	}
	queue<int> q;
	for(int i=0;i<output_lines.size();i++){
		q.push(output_lines[i]);
		inList[output_lines[i]]=true;
	}
	while(!q.empty()){
		int it=q.front();
		q.pop();
		Line curr(lines[it]);
		if(!curr.is_input && !curr.is_output) faults.push_back(curr.extern_name);
		
		if(!curr.is_input){
			Gate g = gates[curr.from_gate];
			for(int i=0;i<g.input_lines.size();i++){
				int j=getLineID(g.input_lines[i]);
				if(!inList[j]){
					q.push(j);
					inList[j]=true;
				}
			}
		}
	}
	cout<<"faultlist.size "<<faults.size()<<endl;
}

void Circuit::getRandomFaultList(vector<string> &faults) {
	for (int i = 0; i < lines.size(); i++) {
		if (!lines[i].is_input && !lines[i].is_output) {
			faults.push_back(lines[i].extern_name);
		}
	}
	random_shuffle ( faults.begin(), faults.end() );
}

void Circuit::getInputLines(set<string> &inlines) {

    for (int i = 0; i < input_lines.size(); i++) {
        inlines.insert(lines[input_lines[i]].extern_name);
    }
}

void Circuit::getOutputLines(set<string> &outlines) {

    for (int i = 0; i < output_lines.size(); i++) {
        outlines.insert(lines[output_lines[i]].extern_name);
    }
}
