/*
 * CNF.h
 *
 *  Created on: Dec 3, 2014
 *      Author: jdavies
 */

#ifndef CNF_H
#define CNF_H

#ifndef CIRCUIT_H
#include "Circuit.h"
#endif

#include <map>

class CNF {


public:

	int num_vars;

	vector<vector<int> > clauses;


	CNF(int nvars) { num_vars = nvars; }


	int newVar() { num_vars++; return num_vars; }

	void createANDClauses(vector<int> &inputs, int output);
	void createORClauses(vector<int> &inputs, int output);
	void createNOTClauses(vector<int> &inputs, int output);
	void createXORClauses(vector<int> &inputs, int output);
	void createNANDClauses(vector<int> &inputs, int output);
	void createBUFFClauses(vector<int> &inputs, int output);
	void createNORClauses(vector<int> &inputs, int output);
	void createSensitizationClauses(int output, int foutput, int soutput);
	void createClause(vector<int> &lits);

        void outputToFile(char *filename);
	void outputToFile(char *benchfile, Fault &fault, map<string, int> &extern_to_line, set<string> &inlines, set<string> &outlines, char *filename);
	void outputToFile(char *benchfile, map<string, int> &extern_to_line, set<string> &inlines, set<string> &outlines, char *filename);
        void outputToFile(char *benchfile, vector<Fault> &faults, vector<vector<int> > &testpatterns, char *filename);
        void outputToFileMaxsat(char *benchfile, set<int> &softclauseIDs, map<string, int> &inputLineToCnfVar, char *filename); 
};


#endif
