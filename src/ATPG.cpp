#ifndef ATPG_H
#include "../include/ATPG.h"
#endif

#ifndef SAT_H
#include "../include/Sat.h"
#endif

#ifndef CNF_H
#include "../include/CNF.h"
#endif

#ifndef UTIL_H
#include "../include/util.h"
#endif

#include <stdlib.h>
#include <ctime>

ATPG::ATPG(char *benchname, bool incr, bool symm) {

    g_incremental = incr;
    g_breaksymm = symm;
    
    g_randseed = 1357;

    g_circuit = new Circuit(benchname);

    g_circuit->getFaultList(g_faultlines);

    g_cnfformula = new CNF(0);
    g_satsolver = new SATSolver(g_cnfformula);
    //g_relaxvar = g_cnfformula->newVar();
    g_numUntestableFaults = 0;

    // *** we don't want the uvars when we're doing maxsat ***
    // the first fault has no uvars of its own, so for ease of indexing,
    // add a dummy vector for the first fault
    //vector<int> empty;
    //g_uvars.push_back(empty);
}

ATPG::~ATPG() {

    delete g_circuit;
    delete g_cnfformula;
    delete g_satsolver;
}

void ATPG::getFaultList(vector<string> &flts) {
    g_circuit->getFaultList(flts);
}

void ATPG::getMinTestSet(vector<vector<int> > &patterns) {

    int numpatt = 1;

    Fault firstfault(g_faultlines[0], 0);
    g_curfaults.push_back(firstfault); 
    bool newFault = true;
    vector<int> model;
    bool satresult = false;
    int iternum = 1;

    initCNF(true, false);

    for (int i = 0; i < g_faultlines.size(); i++) {
        g_allfaults.push_back(Fault(g_faultlines[i], 0));
        g_allfaults.push_back(Fault(g_faultlines[i], 1));
    }

    cout << "ATPG: starting getMinTestSet. Total number of faults is " << g_faultlines.size()*2 << endl;
 
    while (true) {
        cout << "ATPG: iteration " << iternum++ << " num faults " << g_curfaults.size() << " numpatt = " << numpatt << endl;
        for (int i = 0; i < g_curfaults.size(); i++) {
            cout << g_curfaults[i].line << "=" << g_curfaults[i].stuckat << " ";
        }
        cout << endl;
        
        model.clear();
        vector<int> assumps;
        assumps.push_back(-1*g_relaxvar);
 
        vector<int> prefixvars;
        for (int i = 0; i < g_uvars.size(); i++) {
            for (int j = 0; j < g_uvars[i].size(); j++) {
                prefixvars.push_back(g_uvars[i][j]);
            }
        }
        if (g_incremental) {
            g_satsolver->setPrefixVars(prefixvars);
            satresult = g_satsolver->solveWithAssumptions(assumps, model);
        } else {
            cout << "NOT INCREMENTAL"  << endl; 
            SATSolver *tmpsolver = new SATSolver(g_cnfformula);
            tmpsolver->setPrefixVars(prefixvars);
            satresult = tmpsolver->solveWithAssumptions(assumps, model);
            delete tmpsolver;
        }
        if (satresult) {
            patterns.clear();
            modelToPatterns(g_testpatternvars, model, patterns);
            if (addUntestedFault(patterns, g_curfaults) == false) {
                break;
            } else {
                newFault = true;
            }
        } else {
            numpatt++;
            newFault = false;
        }

        if (newFault) {
            updateTestCNFWithFault(g_curfaults[g_curfaults.size()-1], true, false);
        } else {
            updateTestCNFWithPattern();
        }
    }
}

/*
 * patterns contains 0's and 1's representing False and True
 * patternvars contains the cnf variable corresponding to each input in each test pattern
 */
void ATPG::patternsToAssumptions(const vector<vector<int> > &patternvars, const vector<vector<int> > &patterns, vector<int> &assumps) {

    for (int i = 0; i < patterns.size(); i++) {
        for (int j = 0; j < patterns[i].size(); j++) {
            int cnfvar = patternvars[i][j];
            int cnflit = (patterns[i][j] == 0) ? -1*cnfvar : cnfvar;
            assumps.push_back(cnflit);     
        }
    }
}

void ATPG::modelToPatterns(const vector<vector<int> > &patternvars, const vector<int> &model, vector<vector<int> > &patterns) {

    for (int i = 0; i < patternvars.size(); i++) { 
        vector<int> patt;
        for (int j = 0; j < patternvars[i].size(); j++) {
            int v = patternvars[i][j];
            // check what value this variable is given in the model. Assume the model is sorted.
            int mlit = model[v-1];
            int val = (mlit < 0) ? 0 : 1; 
            patt.push_back(val);      
        }
        patterns.push_back(patt);
    }
}

bool ATPG::addUntestedFault(const vector<vector<int> > &patterns, vector<Fault> &curfaults) {

    Fault untestedFault;

    cout << "ATPG: addUntestedFault enter, curfaults.size() = " << curfaults.size() << endl;
    set<Fault> testedFaults;
    testedFaults.insert(curfaults.begin(), curfaults.end());

    while ((g_allfaults.size() > 0) && (testedFaults.size() < g_allfaults.size())) {
        int randfault = irand(g_randseed, g_allfaults.size());  
       
        Fault frand = g_allfaults[randfault];  
        if (testedFaults.find(frand) != testedFaults.end()) {
            cout << "Something already tested" << endl;
            continue;
        } 
        // check if the fault is tested by the patterns
        bool istested = isFaultTested(frand, patterns); 
        if (!istested) {
            // check if the fault *can* be tested
            bool testable = isFaultTestable(frand);
            if (testable) { 
                untestedFault.line = frand.line;
                untestedFault.stuckat = frand.stuckat; 
                break;
            } else {
                cout << "Untestable fault detected." << endl; 
                // remove untestable faults
                g_numUntestableFaults++;
                g_allfaults[randfault] = g_allfaults[g_allfaults.size()-1];
                g_allfaults.pop_back();
            }
        } else {
            testedFaults.insert(frand); 
        }
    }

    if (untestedFault.stuckat >= 0) {
        curfaults.push_back(untestedFault);
    }
    cout << "ATPG: addUntestedFault exit, found fault = " << (untestedFault.stuckat >= 0) << endl;
    return (untestedFault.stuckat >= 0);
}

int ATPG::getBoolVar(CNF *cnf, map<int, int> &line_to_boolvar, int line) {

    if (line_to_boolvar.find(line) == line_to_boolvar.end()) {
        line_to_boolvar[line] = cnf->newVar();  
    }
    return line_to_boolvar[line];
}

void ATPG::buildCNF(set<int> &cone, Fault &fault, CNF *outCNF, map<int, int> &line_to_boolvar, set<int> &softclauses, bool addToSoftClauseList, bool addGoodCircuit) {
	map<string, int> extern_to_faultvar;

	for (int i = 0; i < g_circuit->getNumGates(); i++) {

		vector<int> inputs;
		vector<int> finputs;
		int output = 0;
		int foutput = 0;
                Gate curgate = g_circuit->getGate(i);
 
		bool inCone = (cone.find(curgate.id) != cone.end());

		output = getBoolVar(outCNF, line_to_boolvar, g_circuit->getLineID(curgate.output_line));

		if (inCone) {
			if (extern_to_faultvar.find(curgate.output_line) == extern_to_faultvar.end()) {
				extern_to_faultvar[curgate.output_line] = outCNF->newVar();
			}
			foutput = extern_to_faultvar[curgate.output_line];
		}

		for (int j = 0; j < curgate.input_lines.size(); j++) {
			string input = curgate.input_lines[j];
			inputs.push_back(getBoolVar(outCNF, line_to_boolvar, g_circuit->getLineID(input)));

			if ((input.compare(fault.line) == 0) || (inCone && (cone.find(g_circuit->getLine(g_circuit->getLineID(input)).from_gate) != cone.end()))) {
			    if (extern_to_faultvar.find(input) == extern_to_faultvar.end()) {
			        extern_to_faultvar[input] = outCNF->newVar();
			    }
			    finputs.push_back(extern_to_faultvar[input]);
			} else {
			    finputs.push_back(getBoolVar(outCNF, line_to_boolvar, g_circuit->getLineID(input)));
			}
		}

		/*if (inCone) {
			outCNF->createSensitizationClauses(output, foutput, soutput);
		}*/

                if (addGoodCircuit) {
		    addGateClauses(outCNF, curgate.type, inputs, output);
                }
		if (inCone) {
			addGateClauses(outCNF, curgate.type, finputs, foutput);
		}
	}

	// Assert that the stuck-at line is stuck!
	vector<int> stuck_unit;
	int stucklit = 0;

	if (extern_to_faultvar.find(fault.line) == extern_to_faultvar.end()) {
		cerr << "ERROR: fault line not found: " << fault.line << endl;
		exit(1);
	}
	stucklit = extern_to_faultvar[fault.line];
	if (fault.stuckat == 0) {
		stucklit = -1*stucklit;
	}
	stuck_unit.push_back(stucklit);
	outCNF->createClause(stuck_unit);

	// Assert that one of the output lines is sensitized
	/*vector<int> sensitized_outputs;
	for (int i = 0; i < outlines.size(); i++) {
		string outline = circuit->getLine(outlines[i]).extern_name;
		int svar = extern_to_sensvar[outline];
		sensitized_outputs.push_back(svar);
	}
	outCNF->createClause(sensitized_outputs);*/
	vector<int> outOR;
	for(int i = 0; i < g_circuit->getNumOutputLines(); i++) {
		vector<int> outxor;
		outxor.push_back(getBoolVar(outCNF, line_to_boolvar, g_circuit->getOutputLine(i)));
		string outname = (g_circuit->getLine(g_circuit->getOutputLine(i))).extern_name;
		// Some output variables might not be in the fault cone
		if (extern_to_faultvar.find(outname) != extern_to_faultvar.end()) {
			outxor.push_back(extern_to_faultvar[outname]);
			int orvar = outCNF->newVar();
			outCNF->createXORClauses(outxor, orvar);
			outOR.push_back(orvar);
		}
	}
	if (outOR.size() > 0) {
		outCNF->createClause(outOR);
                if (addToSoftClauseList) {
                    /*cout << "Soft clause: ";
                    for (int i = 0; i < outOR.size(); i++) {
                        cout << outOR[i] << " ";
                    }
                    cout << endl;*/
                    softclauses.insert(outCNF->clauses.size()-1);
                }
	} else {
		cerr << "ERROR: the output disjunction is empty!" << endl;
		exit(1);
	}


}

void ATPG::buildCNF(set<int> &cone, Fault &fault, CNF *outCNF, bool addToSoftClauseList, bool addGoodCircuit) {

    map<int, int> line_to_boolvar;
    set<int> dummy;
    buildCNF(cone, fault, outCNF, line_to_boolvar, dummy, addToSoftClauseList, addGoodCircuit);
}


void ATPG::buildMaxSatCnf(list<Fault> &faults, CNF *cnf, set<int> &softclauses, vector<int> &patternvars){
  int clauseIndex = cnf->clauses.size();
  
  int numInputs = g_circuit->getNumInputLines();
  int numPatterns = 1; // one pattern initially 
  int numSelectors = numPatterns; // simple multiplexer for now
  vector<vector<int> > testpatternvars;
  vector<vector<int> > selectorvars;
  vector<vector<int> > multioutputvars;
      
  for (int i = 0; i < numInputs; i++) { 
    patternvars.push_back(cnf->newVar());     
  }    
  testpatternvars.push_back(patternvars); 
    
  map<int, int> line_to_boolvar;

  // Initialize the boolean vars for the input lines to be the outputs of the multiplexer
    
  for (int i = 0; i < numInputs; i++) {
      line_to_boolvar[g_circuit->getInputLine(i)] = patternvars[i];
  }

  int iternum = 0;
  for (list<Fault>::iterator it_flt = faults.begin(); it_flt != faults.end(); it_flt++){
    Fault &fault = *it_flt;

    // gather all gates in the fanout-cone of the fault
    int faulty_line_id = g_circuit->getLineID(fault.line);
    set<int> cone;
    addToCone(fault.line, cone, (g_circuit->getLine(faulty_line_id)).to_gates);

    // only add clauses for the good circuit if it's the first fault
    buildCNF(cone, fault, cnf, line_to_boolvar, softclauses, true, (iternum == 0));
     
    iternum++;
  }


}


// Assume g_curfaults contains one fault
void ATPG::initCNF(bool shouldRelax, bool addToSoftClauseList) {

    int clauseIndex = g_cnfformula->clauses.size();

    int numInputs = g_circuit->getNumInputLines();
    int numPatterns = 1; // one pattern initially 
    /*int numSelectors = numPatterns;*/ // simple multiplexer for now
    
    vector<int> patt;
    for (int i = 0; i < numInputs; i++) { 
        patt.push_back(g_cnfformula->newVar());     
    }    
    g_testpatternvars.push_back(patt); 

    /*vector<int> selectorvars;
    selectorvars.push_back(g_cnfformula->newVar());
    g_selectorvars.push_back(selectorvars);*/ 

    /*vector<int> multioutput;
    for (int i = 0; i < numInputs; i++) {
        multioutput.push_back(g_cnfformula->newVar());
    }
    g_multioutputvars.push_back(multioutput);*/

    //createMultiplexerClauses(g_cnfformula, g_testpatternvars, selectorvars, multioutput, shouldRelax);

    // gather all gates in the fanout-cone of the fault
    int faulty_line_id = g_circuit->getLineID(g_curfaults[0].line);
    set<int> cone;
    addToCone(g_curfaults[0].line, cone, (g_circuit->getLine(faulty_line_id)).to_gates);

    // Initialize the boolean vars for the input lines to be the pattern vars 
    //map<int, int> line_to_boolvar;
    for (int i = 0; i < numInputs; i++) {
        g_line_to_boolvar[g_circuit->getInputLine(i)] = patt[i];
    }
    set<int> dummy; 
    buildCNF(cone, g_curfaults[0], g_cnfformula, g_line_to_boolvar, dummy, addToSoftClauseList, true);

    g_satsolver->addClausesToMinisat(clauseIndex); 

}

// If selectors=k then ouputvars=patterns[k]
// \Sigma_selectors = 1
// selectors[k]=1 implies outputvars = patterns[k]
void ATPG::createMultiplexerClauses(CNF *cnf, vector<vector<int> > &patterns, vector<int> &selectors, vector<int> &outputvars, bool relax) {

    // \Sigma_selectors = 1
    // >= 1
    if (relax) {
        vector<int> disj = selectors;
        disj.push_back(g_relaxvar);
        cnf->createClause(disj);
    } else {
        cnf->createClause(selectors);
    }
    // <= 1 (use simple quadratic encoding for now)
    for (int i = 0; i < selectors.size(); i++) {
        for (int j = i+1; j < selectors.size(); j++) {
            vector<int> binary;
            binary.push_back(-1*selectors[i]);
            binary.push_back(-1*selectors[j]);
            cnf->createClause(binary);
        }
    } 

    
    for (int i = 0; i < selectors.size(); i++) {
            for (int j = 0; j < patterns[i].size(); j++) {
                int patt = patterns[i][j];
                int output = outputvars[j];

                // selector[i] => (patt -> output)
                vector<int> cls3;
                cls3.push_back(-1*selectors[i]);
                cls3.push_back(-1*patt);
                cls3.push_back(output);
                cnf->createClause(cls3);

                // selector[i] => (output -> patt)
                vector<int> cls4;
                cls4.push_back(-1*selectors[i]);
                cls4.push_back(patt);
                cls4.push_back(-1*output);
                cnf->createClause(cls4);
        }
    }
}

// Assumes that the circuit has no cycles (i.e., that it is combinational, not sequential)
void ATPG::addToCone(string faultyline, set<int> &cone, vector<int> gates_toadd) {

	for (int i = 0; i < gates_toadd.size(); i++) {
		int cur_gate = gates_toadd[i];
		if (cone.find(cur_gate) != cone.end()) {
			break;
		}
		cone.insert(cur_gate);

		// fan-in of cur_gate
		/*for (int j = 0; j < gates[cur_gate].input_lines.size(); j++) {
			string iline_str = gates[cur_gate].input_lines[j];
			Line &iline = lines[circuit->getLineID(iline_str)];
			// Don't want to add the parent of the faulty line to the cone
			if (! iline.is_input && (iline_str.compare(faultyline) != 0)) {
				int from_gate = iline.from_gate;
				cone.insert(from_gate);
			}
		}*/

		// recurse on the gates downstream of cur_gate
		if (! g_circuit->getLine(g_circuit->getLineID(g_circuit->getGate(cur_gate).output_line)).is_output) {
			Gate g = g_circuit->getGate(cur_gate);
			string outline = g.output_line;
			int outline_id = g_circuit->getLineID(outline);

			addToCone(faultyline, cone, g_circuit->getLine(outline_id).to_gates);
		}
	}
}

// Create a circuit that is the same, except the faulty line is now an additional input
// to the circuit, whose value is already assigned to 'fault.stuckat'
CNF *ATPG::getFaultyCNF(Fault &fault) {

	CNF *faultyCNF = new CNF(g_circuit->getNumLines());

	int faulty_line_id = g_circuit->getLineID(fault.line);
	Line fline = g_circuit->getLine(faulty_line_id);
	int fline_cnfvar = faultyCNF->newVar();

	vector<int> unitFaultCls;
	unitFaultCls.push_back(fault.stuckat == 0 ? -1*fline_cnfvar : fline_cnfvar);
	faultyCNF->createClause(unitFaultCls);

	for (int i = 0; i < g_circuit->getNumGates(); i++) {
		Gate g = g_circuit->getGate(i);

		int output = g_circuit->getLineID(g.output_line) + 1;

		vector<int> inputs;
		for (int j = 0; j < g.input_lines.size(); j++) {
			if (g_circuit->getLineID(g.input_lines[j]) != faulty_line_id) {
				inputs.push_back(g_circuit->getLineID(g.input_lines[j]) + 1);
			} else {
				inputs.push_back(fline_cnfvar);
			}
		}


		addGateClauses(faultyCNF, g.type, inputs, output);
	}

	return faultyCNF;
}

void ATPG::addGateClauses(CNF *cnf, Gate::gate_type type, vector<int> &inputs, int output) {
	switch (type) {
		case(Gate::AND):
			cnf->createANDClauses(inputs, output);
			break;
		case(Gate::OR):
			cnf->createORClauses(inputs, output);
			break;
		case(Gate::NOT):
			cnf->createNOTClauses(inputs, output);
			break;
		case(Gate::XOR):
			cnf->createXORClauses(inputs, output);
			break;
		case(Gate::NAND):
			cnf->createNANDClauses(inputs, output);
			break;
		case(Gate::BUFF):
			cnf->createBUFFClauses(inputs, output);
			break;
		case(Gate::NOR):
			cnf->createNORClauses(inputs, output);
		break;

		default:
			cerr << "ERROR: unknown gate type " << type << endl;
			exit(1);
	}
}

CNF *ATPG::getCNF() {

	CNF *testCNF = new CNF(g_circuit->getNumLines());

	for (int i = 0; i < g_circuit->getNumGates(); i++) {
		Gate g = g_circuit->getGate(i);

		int output = g_circuit->getLineID(g.output_line) + 1;

		vector<int> inputs;
		for (int j = 0; j < g.input_lines.size(); j++) {
			inputs.push_back(g_circuit->getLineID(g.input_lines[j]) + 1);
		}

		addGateClauses(testCNF, g.type, inputs, output);

	}

	return testCNF;
}

CNF *ATPG::test(Fault &fault) {


	// gather all gates in the fanout-cone of the fault
	int faulty_line_id = g_circuit->getLineID(fault.line);

	set<int> cone;
	addToCone(fault.line, cone, g_circuit->getLine(faulty_line_id).to_gates);

	// now that we have the cone, start building the CNF
	CNF *testCNF = new CNF(g_circuit->getNumLines());
	buildCNF(cone, fault, testCNF, false, true);

	return testCNF;
}


// check whether a set of patterns tests a given fault
bool ATPG::isFaultTested(Fault &fault, const vector<vector<int> > &testpatterns) {

    CNF *tempcnf = new CNF(0);
    int numInputs = g_circuit->getNumInputLines();
    int numPatterns = testpatterns.size();
    int numSelectors = numPatterns; // simple multiplexer for now

    vector<vector<int> > testpatternvars;
    for (int i = 0; i < numPatterns; i++) {
        vector<int> patt;
        for (int j = 0; j < numInputs; j++) { 
            patt.push_back(tempcnf->newVar());     
        }    
        testpatternvars.push_back(patt); 
    }


    vector<int> selectorvars;
    for (int i = 0; i < numSelectors; i++) {
       selectorvars.push_back(tempcnf->newVar());
    } 

    vector<int> multioutput;
    for (int i = 0; i < numInputs; i++) {
        multioutput.push_back(tempcnf->newVar());
    }

    // Create clauses for the multiplexer
    createMultiplexerClauses(tempcnf, testpatternvars, selectorvars, multioutput, false);

    // gather all gates in the fanout-cone of the fault
    int faulty_line_id = g_circuit->getLineID(fault.line);
    set<int> cone;
    addToCone(fault.line, cone, (g_circuit->getLine(faulty_line_id)).to_gates);

    // Initialize the boolean vars for the input lines to be the outputs of the multiplexer
    map<int, int> line_to_boolvar;
    for (int i = 0; i < numInputs; i++) {
        line_to_boolvar[g_circuit->getInputLine(i)] = multioutput[i];
    }
    set<int> dummy; 
    buildCNF(cone, fault, tempcnf, line_to_boolvar, dummy, false, true);

    vector<int> assumps; 
    patternsToAssumptions(testpatternvars, testpatterns, assumps);
     
    SATSolver *tempsatsolver = new SATSolver(tempcnf);
    bool satresult = tempsatsolver->solveWithAssumptions(assumps);
    delete tempsatsolver;
    tempsatsolver = NULL;
    delete tempcnf;
    tempcnf = NULL;

    return satresult;
}

// Allow one more pattern to be used
void ATPG::updateTestCNFWithPattern() {

    int clauseIndex = g_cnfformula->clauses.size();

    vector<int> newpatternvars;
    for (int i = 0; i < g_circuit->getNumInputLines(); i++) {
        newpatternvars.push_back(g_cnfformula->newVar());
    }
    g_testpatternvars.push_back(newpatternvars);

    // the same relaxation var is used for all faults
    int newRelaxVar = g_cnfformula->newVar();
    // permanently relax all previous selector disjunctions 
    vector<int> oldRelax;
    oldRelax.push_back(g_relaxvar);
    g_cnfformula->createClause(oldRelax);
    g_relaxvar = newRelaxVar;
   
    // Add a new input to all the multiplexers
    for (int i = 0; i < g_curfaults.size(); i++) {
        int newSelector = g_cnfformula->newVar();
        vector<int> disj;
        disj.push_back(newRelaxVar);
        for (int j = 0; j < g_selectorvars[i].size(); j++) {
            vector<int> binary;
            binary.push_back(-1*newSelector);
            binary.push_back(-1*g_selectorvars[i][j]);
            g_cnfformula->createClause(binary);
           
            disj.push_back(g_selectorvars[i][j]);
        }
        g_selectorvars[i].push_back(newSelector);
        disj.push_back(newSelector);
        g_cnfformula->createClause(disj);
        

        // hook up the multiplexer inputs and outputs
        for (int j = 0; j < newpatternvars.size(); j++) {
            int patt = newpatternvars[j];
            int output = g_multioutputvars[i][j];

            // selector => (patt -> output)
            vector<int> cls3;
            cls3.push_back(-1*newSelector);
            cls3.push_back(-1*patt);
            cls3.push_back(output);
            g_cnfformula->createClause(cls3);

            // selector => (output -> patt)
            vector<int> cls4;
            cls4.push_back(-1*newSelector);
            cls4.push_back(patt);
            cls4.push_back(-1*output);
            g_cnfformula->createClause(cls4);
        }
    }

    // For each pair of faults add u_{i,j} => s1==s2 for the newest selectors
    for (int i = 0; i < g_curfaults.size(); i++) {
        vector<int> s1;
        s1.push_back(g_selectorvars[i][g_selectorvars[i].size()-1]);
        for (int j = 0; j < i; j++) { 
            vector<int> s2;
            s2.push_back(g_selectorvars[j][g_selectorvars[j].size()-1]);
            createUVarClauses(g_cnfformula, g_uvars[i][j], s1, s2);
        } 
    }

    if (g_breaksymm) { 
        // Add symmetry breaking predicates
        // between this pattern and the previous pattern
        createLexClauses(g_cnfformula, newpatternvars, g_testpatternvars[g_testpatternvars.size()-2]);
    }

    g_satsolver->addClausesToMinisat(clauseIndex); 
}

// prevpatt <= patt
void ATPG::createLexClauses(CNF *cnf, vector<int> &patt, vector<int> &prevpatt) {

    vector<int> eqvars;
    for (int i = 0; i < patt.size()-1; i++) {
        int eqvar = cnf->newVar();
        eqvars.push_back(eqvar);

        // e -> (patt -> prev)
        vector<int> cls1;
        cls1.push_back(-1*eqvar);
        cls1.push_back(-1*patt[i]);
        cls1.push_back(prevpatt[i]);
        cnf->createClause(cls1);

        // e -> (prev -> patt)
        vector<int> cls2;
        cls2.push_back(-1*eqvar);
        cls2.push_back(patt[i]);
        cls2.push_back(-1*prevpatt[i]);
        cnf->createClause(cls2);

        // (prev & patt) -> e
        vector<int> cls3;
        cls3.push_back(-1*patt[i]);
        cls3.push_back(-1*prevpatt[i]);
        cls3.push_back(eqvar);
        cnf->createClause(cls3);

        // (-prev & -patt) -> e 
        vector<int> cls4;
        cls4.push_back(patt[i]);
        cls4.push_back(prevpatt[i]);
        cls4.push_back(eqvar);
        cnf->createClause(cls4);
    }
    for (int i = 1; i < patt.size(); i++) {
        vector<int> cls;
        for (int j = 0; j < i; j++) {
            cls.push_back(-1*eqvars[j]);
        }        
        cls.push_back(patt[i]);
        cls.push_back(-1*prevpatt[i]);
        cnf->createClause(cls);
    }
}

// See if the current number of patterns can also test this additional fault
void ATPG::updateTestCNFWithFault(Fault &fault, bool shouldRelax, bool addToSoftClauseList) {

    int clauseIndex = g_cnfformula->clauses.size();
    int numInputs = g_circuit->getNumInputLines();
    int numPatterns = g_testpatternvars.size();
    /*int numSelectors = numPatterns;*/ // simple multiplexer for now
   
    /*vector<int> selectors;
    for (int i = 0; i < numSelectors; i++) {
        selectors.push_back(g_cnfformula->newVar());
    }
    g_selectorvars.push_back(selectors);

    vector<int> multioutput;
    for (int i = 0; i < numInputs; i++) {
        multioutput.push_back(g_cnfformula->newVar());
    }
    g_multioutputvars.push_back(multioutput);

    createMultiplexerClauses(g_cnfformula, g_testpatternvars, selectors, multioutput, shouldRelax); 
    */

    // gather all gates in the fanout-cone of the fault
    int faulty_line_id = g_circuit->getLineID(fault.line);
    set<int> cone;
    addToCone(fault.line, cone, (g_circuit->getLine(faulty_line_id)).to_gates);

    // Initialize the boolean vars for the input lines to be the test pattern vars 
    /*map<int, int> line_to_boolvar;
    for (int i = 0; i < numInputs; i++) {
        line_to_boolvar[g_circuit->getInputLine(i)] = g_testpatternvars[0][i];
    }*/

    set<int> dummy;
    // Don't duplicate the clauses of the good circuit, but use their variables via g_line_to_boolvar
    buildCNF(cone, fault, g_cnfformula, g_line_to_boolvar, dummy, addToSoftClauseList, false);

    // *** We don't need uvars for the maxsat case ***
#if 0
    // Add variable u_{i,j} for this fault i and all previous faults j
    vector<int> newuvars;
    for (int i = 0; i < g_curfaults.size()-1; i++) {
        newuvars.push_back(g_cnfformula->newVar()); 
        createUVarClauses(g_cnfformula, newuvars[newuvars.size()-1], selectors, g_selectorvars[i]);
    }
    g_uvars.push_back(newuvars);
#endif
 
    g_satsolver->addClausesToMinisat(clauseIndex);
}

// Add clauses saying that the u_{i,j} implies faults i and j have equal selectors
void ATPG::createUVarClauses(CNF *cnf, int uvar, vector<int> &s1, vector<int> &s2) {

    if (s1.size() != s2.size()) {
        cout << "ERROR: in createUVarClauses, s1.size() = " << s1.size() << " s2.size() = " << s2.size() << endl;
        fflush(stdout);
        exit(1); 
    }
 
    for (int i = 0; i < s1.size(); i++) { 

        // uvar => (s1 -> s2)
        vector<int> cls3;
        cls3.push_back(-1*uvar);
        cls3.push_back(-1*s1[i]);
        cls3.push_back(s2[i]);
        cnf->createClause(cls3);

        // uvar => (s2 -> s1)
        vector<int> cls4;
        cls4.push_back(-1*uvar);
        cls4.push_back(s1[i]);
        cls4.push_back(-1*s2[i]);
        cnf->createClause(cls4);
    }
}

bool ATPG::isFaultTestable(Fault &f) {

    CNF *tempcnf = test(f);
    SATSolver *tempsolver = new SATSolver(tempcnf);
    vector<int> model;
    bool satres = tempsolver->solve(model);
    delete tempcnf;
    delete tempsolver;
    return satres;
}


void ATPG::getGreedyTestSet(vector<vector<int> > &patterns, char *solver_name) {
    int iternum = 1;

    time_t t1,t2;
    vector<Fault> allfaults;
    list<Fault> curfaults;

    for (int i = 0; i < g_faultlines.size(); i++) {
      Fault f0(g_faultlines[i], 0);
      if (isFaultTestable(f0)) {
          allfaults.push_back(f0);
      }
      Fault f1(g_faultlines[i], 1);
      if (isFaultTestable(f1)) {
          allfaults.push_back(f1);
      }
    }
   
    cout << "Number of testable faults: " << allfaults.size() << endl; 
    fflush(stdout);

    for (int i= 0 ; i < allfaults.size(); i++) {
      curfaults.push_back(allfaults[i]);
    }

    // map<string, int> externLineToCnfVar;
    // for (int i = 0; i < g_circuit->getNumInputLines(); i++) {
    //     int lineID = g_circuit->getInputLine(i);
    //     externLineToCnfVar[g_circuit->getLine(lineID).extern_name] = g_testpatternvars[0][i]; 
    // } 
    // g_cnfformula->outputToFileMaxsat("whatever", g_soft_clid, externLineToCnfVar, "test_maxsat.cnf");

    while (curfaults.size() > 0) {
      cout << "iteration " << iternum++ << ", remaning faults=" << curfaults.size() 
	   << ", test patterns=" << patterns.size() << endl;

      vector<int> patternvars;
      CNF *cnf = new CNF(0);
      set<int> softclauses;
      buildMaxSatCnf(curfaults, cnf, softclauses, patternvars);

      vector<int> model;
      MaxSATSolver *maxsatsolver = new MaxSATSolver(cnf, solver_name);      

      time(&t1);
      maxsatsolver->solve(model, softclauses);
      time(&t2);

      cout << "Solving time=" << difftime(t2,t1) << "s" << endl;

      vector<vector<int> > testpatternvars;
      testpatternvars.push_back(patternvars);
      modelToPatterns(testpatternvars, model, patterns);

      vector<flt_it> tested_faults;

      // find and remove tested faults
      for (flt_it it=curfaults.begin(); it!=curfaults.end(); it++){
	if (isFaultTested(*it, patterns)){
	  tested_faults.push_back(it);
	}
      }
	
      cout << "Tested faults: ";
      for (vector<flt_it>::iterator it=tested_faults.begin(); it!=tested_faults.end(); it++){
	curfaults.erase(*it);
	cout << (*it)->line << ", ";
      }
      cout << endl;
      
    }
}

