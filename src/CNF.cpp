/*
 * CNF.cpp
 *
 *  Created on: Dec 3, 2014
 *      Author: jdavies
 */

#ifndef CNF_H
#include "../include/CNF.h"
#endif

#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

using namespace std;

void CNF::outputToFileMaxsat(char *benchfile, set<int> &softclauseIDs, map<string, int> &inputLineToCnfVar, char *filename) {

    ofstream outf(filename);

    outf << "c .bench file: " << benchfile << endl;
    outf << "c .bench file line ID : CNF variable (only for input variables)" << endl;

    for (map<string, int>::iterator iter = inputLineToCnfVar.begin(); iter != inputLineToCnfVar.end(); iter++) {
	outf << "c " << iter->first << " : " << (iter->second) << " : input" << endl;
    }

    outf << "c" << endl;
    outf << "c" << endl;

    int hardWeight = softclauseIDs.size()+1;
    outf << "p wcnf " << num_vars << " " << clauses.size() << " " << hardWeight << endl;

    for (int i = 0; i < clauses.size(); i++) {
        if (softclauseIDs.find(i) != softclauseIDs.end()) {
            outf << "1 ";
        } else {
            outf << hardWeight << " ";
        }
	for (int j = 0; j < clauses[i].size(); j++) {
   	    outf << clauses[i][j] << " ";
	}
	outf << "0" << endl;
    }

    outf.close();
}

void CNF::outputToFile(char *filename) {

        ofstream outf(filename);

        outf << "p cnf " << num_vars << " " << clauses.size() << endl;

        for (int i = 0; i < clauses.size(); i++) {
                for (int j = 0; j < clauses[i].size(); j++) {
                        outf << clauses[i][j] << " ";
                }
                outf << "0" << endl;
        }
        outf.close();
}
void CNF::outputToFile(char *benchfile, Fault &fault, map<string, int> &extern_to_line, set<string> &inlines, set<string> &outlines, char *filename) {

	ofstream outf(filename);

	outf << "c .bench file: " << benchfile << endl;
	outf << "c faulty line: " << fault.line << endl;
	outf << "c stuck-at: " << fault.stuckat << endl;

	outf << "c" << endl;
	outf << "c" << endl;

	outf << "c .bench file line ID : CNF variable" << endl;


	for (map<string, int>::iterator iter = extern_to_line.begin(); iter != extern_to_line.end(); iter++) {
		outf << "c " << iter->first << " : " << iter->second + 1;
		if (inlines.find(iter->first) != inlines.end()) {
			outf << " : input";
		} else if (outlines.find(iter->first) != outlines.end()) {
			outf << " : output";
		}
		outf << endl;
	}

	outf << "c" << endl;
	outf << "c" << endl;

        outf << "p cnf " << num_vars << " " << clauses.size() << endl;

	for (int i = 0; i < clauses.size(); i++) {
		for (int j = 0; j < clauses[i].size(); j++) {
			outf << clauses[i][j] << " ";
		}
		outf << "0" << endl;
	}
}

void CNF::outputToFile(char *benchfile, map<string, int> &extern_to_line, set<string> &inlines, set<string> &outlines, char *filename) {

	ofstream outf(filename);

	outf << "c .bench file: " << benchfile << endl;

	outf << "c" << endl;
	outf << "c" << endl;

	outf << "c .bench file line ID : CNF variable" << endl;


	for (map<string, int>::iterator iter = extern_to_line.begin(); iter != extern_to_line.end(); iter++) {
		outf << "c " << iter->first << " : " << iter->second + 1;
		if (inlines.find(iter->first) != inlines.end()) {
			outf << " : input";
		} else if (outlines.find(iter->first) != outlines.end()) {
			outf << " : output";
		}
		outf << endl;
	}

	outf << "c" << endl;
	outf << "c" << endl;

	outf << "p cnf " << num_vars << " " << clauses.size() << endl;

	for (int i = 0; i < clauses.size(); i++) {
		for (int j = 0; j < clauses[i].size(); j++) {
			outf << clauses[i][j] << " ";
		}
		outf << "0" << endl;
	}

}

void CNF::outputToFile(char *benchfile, vector<Fault> &faults, vector<vector<int> > &testpatterns, char *filename) {

	ofstream outf(filename);

	outf << "c .bench file: " << benchfile << endl;
        outf << "c num faults: " << faults.size() << ", num patterns: " << testpatterns.size() << endl;
        outf << "c Fault : Stuck-At " << endl; 
        for (int i = 0; i < faults.size(); i++) {
            outf << "c " << faults[i].line << " : " << faults[i].stuckat << endl;
        }
        outf << "c Test Pattern : Vars" << endl;
        for (int i = 0; i < testpatterns.size(); i++) {
            outf << "c " << i << " : ";
            for (int j = 0; j < testpatterns[i].size(); j++) {
                outf << testpatterns[i][j] << " ";
            }
            outf << endl;
        } 

        outf << "p cnf " << num_vars << " " << clauses.size() << endl;

	for (int i = 0; i < clauses.size(); i++) {
		for (int j = 0; j < clauses[i].size(); j++) {
			outf << clauses[i][j] << " ";
		}
		outf << "0" << endl;
	}
}

void CNF::createANDClauses(vector<int> &inputs, int output) {

	vector<int> c1;
	for (int i = 0; i < inputs.size(); i++) {
		c1.push_back(-1*inputs[i]);
	}
	c1.push_back(output);
	clauses.push_back(c1);

	// binary clauses
	for (int i = 0; i < inputs.size(); i++) {
		vector<int> c2;
		c2.push_back(-1*output);
		c2.push_back(inputs[i]);
		clauses.push_back(c2);
	}
}

void CNF::createORClauses(vector<int> &inputs, int output) {

	for (int i = 0; i < inputs.size(); i++) {
		vector<int> c1;
		c1.push_back(-1*inputs[i]);
		c1.push_back(output);
		clauses.push_back(c1);
	}

	vector<int> c2;
	for (int i = 0; i < inputs.size(); i++) {
		c2.push_back(inputs[i]);
	}
	c2.push_back(-1*output);
	clauses.push_back(c2);
}

void CNF::createNOTClauses(vector<int> &inputs, int output) {

	assert(inputs.size() == 1);
	vector<int> c1;
	c1.push_back(-1*inputs[0]);
	c1.push_back(-1*output);
	clauses.push_back(c1);

	vector<int> c2;
	c2.push_back(inputs[0]);
	c2.push_back(output);
	clauses.push_back(c2);

}

void CNF::createXORClauses(vector<int> &inputs, int output) {

	if (inputs.size() > 2) {
		int newvar = num_vars+1;
		num_vars++;
		int half = ceil(inputs.size()/2.0);
		vector<int> firsthalf(inputs.begin(), inputs.begin()+half);
		vector<int> secondhalf(inputs.begin()+half+1, inputs.end());
		createXORClauses(firsthalf, newvar);
		secondhalf.push_back(newvar);
		createXORClauses(secondhalf, output);

	} else {
		assert(inputs.size() == 2);
		vector<int> c1;
		c1.push_back(-1*output);
		c1.push_back(inputs[0]);
		c1.push_back(inputs[1]);
		clauses.push_back(c1);

		vector<int> c2;
		c2.push_back(-1*output);
		c2.push_back(-1*inputs[0]);
		c2.push_back(-1*inputs[1]);
		clauses.push_back(c2);

                vector<int> c3;
                c3.push_back(output);
                c3.push_back(-1*inputs[0]);
                c3.push_back(inputs[1]);
                clauses.push_back(c3);

                vector<int> c4;
                c4.push_back(output);
                c4.push_back(inputs[0]);
                c4.push_back(-1*inputs[1]);
                clauses.push_back(c4);
	}
}

void CNF::createNANDClauses(vector<int> &inputs, int output) {


	for (int i = 0; i < inputs.size(); i++) {
		vector<int> c1;
		c1.push_back(inputs[i]);
		c1.push_back(output);
		clauses.push_back(c1);
	}

	vector<int> c2;
	for (int i = 0; i < inputs.size(); i++) {
		c2.push_back(-1*inputs[i]);
	}
	c2.push_back(-1*output);
	clauses.push_back(c2);
}

void CNF::createBUFFClauses(vector<int> &inputs, int output) {

	assert(inputs.size() == 1);
	vector<int> c1;
	c1.push_back(inputs[0]);
	c1.push_back(-1*output);
	clauses.push_back(c1);
	vector<int> c2;
	c2.push_back(-1*inputs[0]);
	c2.push_back(output);
	clauses.push_back(c2);
}

void CNF::createNORClauses(vector<int> &inputs, int output) {

	vector<int> c1;
	for (int i = 0; i < inputs.size(); i++) {
		c1.push_back(inputs[i]);
	}
	c1.push_back(output);
	clauses.push_back(c1);

	for (int i = 0; i < inputs.size(); i++) {
		vector<int> c2;
		c2.push_back(-1*inputs[i]);
		c2.push_back(-1*output);
		clauses.push_back(c2);
	}
}

// As in TG-GRASP model: soutput <=> (output \neq foutput)
void CNF::createSensitizationClauses(int output, int foutput, int soutput) {

	vector<int> c1;
	c1.push_back(soutput);
	c1.push_back(output);
	c1.push_back(-1*foutput);
	clauses.push_back(c1);

	vector<int> c2;
	c2.push_back(soutput);
	c2.push_back(-1*output);
	c2.push_back(foutput);
	clauses.push_back(c2);


	int xAndnoty = newVar();
	vector<int> inputs;
	inputs.push_back(output);
	inputs.push_back(-1*foutput);
	createANDClauses(inputs, xAndnoty);


	int notxAndy = newVar();
	inputs.clear();
	inputs.push_back(-1*output);
	inputs.push_back(foutput);
	createANDClauses(inputs, notxAndy);

	vector<int> c3;
	c3.push_back(-1*soutput);
	c3.push_back(xAndnoty);
	c3.push_back(notxAndy);
	clauses.push_back(c3);

}

void CNF::createClause(vector<int> &lits) {
	clauses.push_back(lits);
}


