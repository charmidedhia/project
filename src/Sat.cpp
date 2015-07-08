#ifndef SAT_H
#include "../include/Sat.h"
#endif

#ifndef CNF_H
#include "../include/CNF.h"
#endif

#ifndef Minisat_System_h
#include "../include/System.h"
#endif


SATSolver::SATSolver(CNF *formula) {

    cnf = formula;
    minisat = new Solver();
    addClausesToMinisat(0);
}

SATSolver::~SATSolver() {

    delete minisat;
}

void SATSolver::addClausesToMinisat(int clauseListIndex) {

    // keep the variables in order 
    for (int i = 0; i < cnf->num_vars; i++) {
        if (varToMinisatVar.find(i+1) == varToMinisatVar.end()) {
            varToMinisatVar[i+1] = minisat->newVar();
        }
    }

    for (int i = clauseListIndex; i < cnf->clauses.size(); i++) {
        vec<Lit> cls;
        for (int j = 0; j < cnf->clauses[i].size(); j++) {
            int lit = cnf->clauses[i][j];
            int var = lit > 0 ? lit : -1*lit;

            // Sanity check that the variable is known
            if (varToMinisatVar.find(var) == varToMinisatVar.end()) { 
                cout << "ERROR: unexpected var appears in clause: " << var << endl;
                fflush(stdout);
                exit(1); 
            } 
            Var mvar = varToMinisatVar[var]; 
            Lit mlit = mkLit(mvar, lit < 0);
            cls.push(mlit);
        }
        minisat->addClause_(cls); 
    }
}

void SATSolver::setPrefixVars(vector<int> &prefix) {

    vec<Var> mprefix;
    for (int i = 0; i < prefix.size(); i++) {
        mprefix.push(varToMinisatVar[prefix[i]]);
    }
    minisat->setPrefixVars(mprefix);
}

bool SATSolver::solve(vector<int> &model, int budget) {

    cout << "SATSolver::solve enter" << endl;
    
    double starttime = cpuTime();
 
    lbool satres = minisat->solveTimeout(budget); 
    if (satres == l_True) {
        for (int i = 0; i < cnf->num_vars; i++) {
            Var mvar = varToMinisatVar[i+1];
            lbool val = minisat->modelValue(mvar);
            if (val == l_True || val == l_Undef) {
                model.push_back(i+1);
            } else if (val == l_False) {
                model.push_back(-1*(i+1));
            } 
        }
    } else if (satres == l_Undef) {
        cout << "SAT Solver timed out, considering it to be UNSAT" << endl;
    }  
    
    cout << "SAT time: " << cpuTime() - starttime << endl;
    // will consider timeouts as UNSAT
    return (satres == l_True);
}
bool SATSolver::solve(vector<int> &model) {

    cout << "SATSolver::solve enter" << endl;
    
    double starttime = cpuTime();
  
    bool satres = minisat->solve(); 
    if (satres) {
        for (int i = 0; i < cnf->num_vars; i++) {
            Var mvar = varToMinisatVar[i+1];
            lbool val = minisat->modelValue(mvar);
            if (val == l_True || val == l_Undef) {
                model.push_back(i+1);
            } else if (val == l_False) {
                model.push_back(-1*(i+1));
            } 
        }
    } 

    cout << "SAT time: " << cpuTime() - starttime << endl;

    return satres;
}

bool SATSolver::solveWithAssumptions(vector<int> &assumps) {

    cout << "SATSolver::solveWithAssumptions enter" << endl;

    vec<Lit> massumps;
    for (int i = 0; i < assumps.size(); i++) {
        int lit = assumps[i];
        int var = (lit > 0) ? lit : -1*lit; 
        Var mvar = varToMinisatVar[var];
        Lit mlit = mkLit(mvar, (lit < 0));
        massumps.push(mlit);
    }

    double starttime = cpuTime();

    bool satres = minisat->solve(massumps);
    cout << "SAT time: " << cpuTime() - starttime << endl;
    return satres;
}

bool SATSolver::solveWithAssumptions(vector<int> &assumps, vector<int> &model) {

    cout << "SATSolver::solveWithAssumptions and return model enter" << endl;

    vec<Lit> massumps;
    for (int i = 0; i < assumps.size(); i++) {
        int lit = assumps[i];
        int var = (lit > 0) ? lit : -1*lit; 
        Var mvar = varToMinisatVar[var]; // this was a bug??
        Lit mlit = mkLit(mvar, (lit < 0));
        massumps.push(mlit);
    }

    double starttime = cpuTime();

    bool satres = minisat->solve(massumps);
    cout << "SAT time: " << cpuTime() - starttime << endl;

    if (satres) {
        for (int i = 0; i < cnf->num_vars; i++) {
            Var mvar = varToMinisatVar[i+1];
            lbool val = minisat->modelValue(mvar);
            if (val == l_True || val == l_Undef) {
                model.push_back(i+1);
            } else if (val == l_False) {
                model.push_back(-1*(i+1));
            } 
        }
    }
    return satres;
}
