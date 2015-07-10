#include "../include/MaxSat.h"

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>

MaxSATSolver::MaxSATSolver(CNF *formula, char* name){
  cnf = formula;
  solver_name = name;
}

MaxSATSolver::~MaxSATSolver(){}

bool MaxSATSolver::solve(vector<int> &model, set<int>& softclauses, int maxhs_time_limit){

  char* cnffile = "test_maxsat.cnf";
  ofstream outf(cnffile);

  vector<vector<int> > &clauses = cnf->clauses;
  int hardWeight = softclauses.size()+1;
  outf << "p wcnf " << cnf->num_vars << " " << clauses.size() << " " << hardWeight << endl;

  for (int i = 0; i < clauses.size(); i++) {
    if (softclauses.find(i) != softclauses.end()) {
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

  FILE *in;
  char intStr[10] ;
  snprintf(intStr, sizeof(intStr), "%d", maxhs_time_limit);
  char buff[512];
  char cmd[512];
  strcpy(cmd,solver_name);
  strcat(cmd," ");
  strcat(cmd,cnffile);
  strcat(cmd," -printBstSoln -cpu-lim=");
  strcat(cmd,intStr);
  strcat(cmd," 2>/dev/null");

  cout << cmd << endl;
   
  string output;
  if(!(in = popen(cmd, "r"))){
    cerr << "ERROR: could not execute " << cmd << endl;
    exit(1);
  }
  while(fgets(buff, sizeof(buff), in)!=NULL){
    output.append(buff);
  }
  pclose(in);

  string::size_type start = 0;
  string::size_type  end = 0;
  int i=0;
  do {
    start = output.find("\nv ",end);
    if (start != string::npos){
      end = output.find("\n",start+1);
      string vline = output.substr(start+2,end-start-3);
      
      string::size_type vdel = 0;

      do {
	vdel = vline.find(" ",vdel);

	if (vdel != string::npos){
	  i++;
	  vdel++;
	  if (vline[vdel] == '-')
	    model.push_back(-i);
	  else 
	    model.push_back(i);
	}

      } while (vdel != string::npos);
    } 
  } while (start != string::npos);
  if(i!=0){start = output.find("Final LB:",end);
    end=output.find("\n",start);
    cout<<output.substr(start,end-start)<<endl;
  }

  if (i == 0){
    //no optimum found}
    start = output.find("Best Model Found:");
    if (start != string::npos){
      start=output.find("\nc ",start);
      end=output.find("\n",start +1);
      string vline = output.substr(start+2,end-start-3);
      
      string::size_type vdel = 0;

        do {
          vdel = vline.find(" ",vdel);

          if (vdel != string::npos){
            i++;
            vdel++;
            if (vline[vdel] == '-')
              model.push_back(-i);
            else 
              model.push_back(i);
          }
        } while (vdel != string::npos);
    }
    if(i==0){//no best solution found
      cerr << "ERROR: could not parse model" << endl;
      exit(1);
    }
    else{
      start=output.find("Best UB Found:");
      end=output.find("\n",start);
      cout<<output.substr(start,end-start)<<endl;
    }
  }

  // cout << output << endl;
}
