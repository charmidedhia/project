/*
 * main.cpp
 *
 *  Created on: Nov 26, 2014
 *      Author: jdavies
 */

#ifndef CIRCUIT_H
#include "../include/Circuit.h"
#endif

#ifndef CNF_H
#include "../include/CNF.h"
#endif

#ifndef SAT_H
#include "../include/Sat.h"
#endif

#ifndef ATPG_H
#include "../include/ATPG.h"
#endif

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <map>

using namespace std;

void generateCircuits(char *benchname, ATPG& atpg);

int main(int argc, char *argv[]) {

    if (argc < 3) {
        cout << "Usage: panda [-i] [-s] [-f] <.bench file> <max sat solver>// -i for incremental SAT solving, -s for symmetry breaking predicates, -f for generating CNF instances for the good and faulty circuits" << endl;
        exit(1);
    }

    bool incremental = false; 
    bool symm = false;
    bool gen_circuits = false;

    for (int i=1; i<argc-2; i++){
      if (strcmp(argv[i], "-i") == 0)
	  incremental = true;

      if (strcmp(argv[i], "-s") == 0)
	  symm = true;

      if (strcmp(argv[i], "-f") == 0)
	  gen_circuits = true;
    }

    char *benchname = argv[argc-2];
    char *solver_name = argv[argc-1];
    ATPG atpg(benchname, true, false);

    // will output to test_maxsat.cnf
    vector<vector<int> > patterns;
    atpg.getGreedyTestSet(patterns, solver_name);

    if (gen_circuits) 
      generateCircuits(benchname, atpg);

#if 0
    ATPG atpg(benchname, incremental, symm);
    atpg.getMinTestSet(patterns);
#endif
    cout << endl << "Num patterns = " << patterns.size() << endl;
    for (int i = 0; i < patterns.size(); i++) {
        cout << "Pattern " << i << ": ";
        for (int j = 0; j < patterns[i].size(); j++) {
            cout << patterns[i][j] << " ";
        }    
        cout << endl;
    }

#if 0 
    Circuit circuit_good(benchname);
    vector<string> faults;
    circuit_good.getFaultList(faults);
    char outputfile[100];
    set<string> inlines;
    set<string> outlines;
    circuit_good.getInputLines(inlines);
    circuit_good.getOutputLines(outlines);


    for (int i = 0; i < faults.size(); i++) {
    	cout << "Processing fault " << faults[i] << endl;
    	fflush(stdout);

    	CNF *goodCNF = circuit_good.getCNF();
    	sprintf(outputfile, "%s/good.cnf", outputdir);
    	goodCNF->outputToFile(benchname, circuit_good.getExternToLine(), inlines, outlines, outputfile);

    	// Test circuit for stuck-at-0
        Fault f0(faults[i], 0);
    	CNF *testCNF0 = circuit_good.test(f0);
    	sprintf(outputfile, "%s/test%s_sa%d.cnf", outputdir, f0.line.c_str(), f0.stuckat);
    	testCNF0->outputToFile(benchname, f0, circuit_good.getExternToLine(), inlines, outlines, outputfile);

    	// Faulty circuit for stuck-at-0
    	CNF *faultyCNF0 = circuit_good.getFaultyCNF(f0);
    	sprintf(outputfile, "%s/faulty%s_sa%d.cnf", outputdir, f0.line.c_str(), f0.stuckat);
    	faultyCNF0->outputToFile(benchname, f0, circuit_good.getExternToLine(), inlines, outlines, outputfile);

    	// Test circuit for stuck-at-1
        Fault f1(faults[i], 1);
    	CNF *testCNF1 = circuit_good.test(f1);
    	sprintf(outputfile, "%s/test%s_sa%d.cnf", outputdir, f1.line.c_str(), f1.stuckat);
    	testCNF1->outputToFile(benchname, f1, circuit_good.getExternToLine(), inlines, outlines, outputfile);

    	// Faulty circuit for stuck-at-1
        CNF *faultyCNF1 = circuit_good.getFaultyCNF(f1);
        sprintf(outputfile, "%s/faulty%s_sa%d.cnf", outputdir, f1.line.c_str(), f1.stuckat);
        faultyCNF1->outputToFile(benchname, f1, circuit_good.getExternToLine(), inlines, outlines, outputfile);
    }

    // Try with a single fault first, and a single test pattern
    vector<Fault> faultsG;
    faultsG.push_back(Fault(faults[0], 0)); 
    faultsG.push_back(Fault(faults[1], 0));
    cout << "faults[0] = " << faults[0] << " faults[1] = " << faults[1] << endl;
    int numpatt = 2;

    vector<vector<int> > testpatterns;
    CNF *testCNF1 = circuit_good.getTestFaultsCNF(faultsG, numpatt, testpatterns);
    sprintf(outputfile, "%s/test_nfaults%d_npatt%d.cnf", outputdir, faultsG.size(), numpatt);
    string nofaultname = "";
    testCNF1->outputToFile(benchname, faultsG, testpatterns, outputfile);

    SATSolver satsolver(testCNF1);
    vector<int> model;
    bool satresult = satsolver.solve(model);
    cout << "SAT solver result = " << satresult << endl;
    if (satresult) {
        cout << "Model: "; 
        for (int i = 0; i < model.size(); i++) {
            cout << model[i] << " ";
        }
        cout << endl;
    }
#endif
}


/* Generate CNF instances for good and faulty circuits. */
void generateCircuits(char *benchname, ATPG& atpg){
  vector<string> faults;
  char outputfile[100];
  set<string> inlines;
  set<string> outlines;
   
  atpg.getFaultList(faults);
  atpg.getInputLines(inlines);
  atpg.getOutputLines(outlines);

  CNF *goodCNF = atpg.getCNF();
  sprintf(outputfile, "good.cnf");
  goodCNF->outputToFile(benchname, atpg.getExternToLine(), inlines, outlines, outputfile);

  for (unsigned int i = 0; i < faults.size(); i++) {
    cout << "Processing fault " << faults[i] << endl;
    fflush(stdout);
    Fault f0(faults[i], 0);

    // Faulty circuit for stuck-at-0
    CNF *faultyCNF0 = atpg.getFaultyCNF(f0);
    sprintf(outputfile, "faulty%s_sa%d.cnf", f0.line.c_str(), f0.stuckat);
    faultyCNF0->outputToFile(benchname, f0, atpg.getExternToLine(), inlines, outlines, outputfile);

    Fault f1(faults[i], 1);
    CNF *faultyCNF1 = atpg.getFaultyCNF(f1);
    sprintf(outputfile, "faulty%s_sa%d.cnf", f1.line.c_str(), f1.stuckat);
    faultyCNF1->outputToFile(benchname, f1, atpg.getExternToLine(), inlines, outlines, outputfile);
  }
}





