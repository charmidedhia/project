#ifndef ATPG_H
#define ATPG_H

#ifndef CIRCUIT_H
#include "Circuit.h"
#endif

#ifndef FAULT_H
#include "Fault.h"
#endif

#ifndef SAT_H
#include "Sat.h"
#endif

#ifndef MAXSAT_H
#include "MaxSat.h"
#endif


#include <vector>
#include <list>
#include <set>
#include <string>

using namespace std;
typedef list<Fault>::iterator flt_it;


class ATPG {

    bool g_incremental;
    bool g_breaksymm;

    double g_randseed;
    Circuit *g_circuit;
    vector<string> g_faultlines;

    CNF *g_cnfformula;
    SATSolver *g_satsolver;
    vector<vector<int> > g_testpatternvars;
    vector<vector<int> > g_selectorvars;
    vector<vector<int> > g_multioutputvars;
    vector<vector<int> > g_uvars;
    int g_relaxvar;
    vector<Fault> g_curfaults;
    int g_numUntestableFaults;
    vector<Fault> g_allfaults;

    // for the good circuit
    map<int, int> g_line_to_boolvar;

    void initCNF(bool shouldRelax, bool addToSoftClauseList);
    bool addUntestedFault(const vector<vector<int> > &patterns, vector<Fault> &curfaults);
    void patternsToAssumptions(const vector<vector<int> > &patternvars, const vector<vector<int> > &patterns, vector<int> &assumps);
    void modelToPatterns(const vector<vector<int> > &patternvars, const vector<int> &model, vector<vector<int> > &patterns);
    bool isFaultTested(Fault &fault, const vector<vector<int> > &testpatterns);
    void updateTestCNFWithFault(Fault &fault, bool shouldRelax, bool addToSoftClauseList);
    void updateTestCNFWithPattern();
    
    void addToCone(string faultyline, set<int> &cone, vector<int> gates);
    void buildCNF(set<int> &cone, Fault &fault, CNF *outCNF, bool addToSoftClauseList, bool addGoodCircuit);
    void buildCNF(set<int> &cone, Fault &fault, CNF *outCNF, map<int, int> &line_to_boolvar, set<int> &softclauses, bool addToSoftClauseList, bool addGoodCircuit);
    void addGateClauses(CNF *cnf, Gate::gate_type type, vector<int> &inputs, int output);
    void createMultiplexerClauses(CNF *cnf, vector<vector<int> > &patterns, vector<int> &selectors, vector<int> &outputvars, bool relax);
    void createUVarClauses(CNF *cnf, int uvar, vector<int> &s1, vector<int> &s2);
    void createLexClauses(CNF *cnf, vector<int> &patt, vector<int> &prevpatt);
    int getBoolVar(CNF *cnf, map<int, int> &line_to_boolvar, int line);
    bool isFaultTestable(Fault &f);

    void buildMaxSatCnf(list<Fault> &faults, CNF *cnf, set<int> &softclauses, vector<int> &patternvars);
    void getAllFaults(vector<Fault> &allfaults);//gives full fault list
    void getReducedFaults(vector<Fault> &allfaults);//gives fault list after equivalent fault collapsing


public:

    ATPG(char *benchname, bool incr, bool symm);
    ~ATPG();

    void getMinTestSet(vector<vector<int> > &patterns);

    CNF *test(Fault &fault);
    CNF *getFaultyCNF(Fault &fault);
    CNF *getCNF();
    //CNF *getTestFaultsCNF(vector<Fault> &faults,  int numPatterns, vector<vector<int> > &testpatterns);

    /* Use a greedy strategy to generate a small set of test patterns. */
    void getGreedyTestSet(vector<vector<int> > &patterns, char *solver_name);
    bool checkTestSet(vector<vector<int> > patterns);


    void getFaultList(vector<string> &faults);
    void getInputLines(set<string> &inlines) { g_circuit->getInputLines(inlines); }
    void getOutputLines(set<string> &outlines) { g_circuit->getOutputLines(outlines); }
    map<string, int> &getExternToLine() { return g_circuit->getExternToLine(); }

};

#endif
