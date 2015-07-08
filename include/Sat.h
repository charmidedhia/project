#ifndef SAT_H
#define SAT_H

#ifndef Minisat_Solver_h
#include "Solver.h"
#endif

#include <vector>
#include <map>

using namespace std;
using namespace Minisat;
class CNF;


// Interface to minisat
class SATSolver {

private:

    Solver *minisat;
    CNF *cnf;
    map<int, Var> varToMinisatVar;

public:

    // We assume the caller will modify the cnf object
    SATSolver(CNF *cnf);
    ~SATSolver();

    // add clauses from this index to the end of cnf's clauses list
    void addClausesToMinisat(int clauseListIndex);

    void setPrefixVars(vector<int> &prefix);

    bool solve(vector<int> &model);
    bool solveWithAssumptions(vector<int> &assumps);
    bool solveWithAssumptions(vector<int> &assumps, vector<int> &model);
    bool solve(vector<int> &model, int budget);
};

#endif
