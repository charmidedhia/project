import re
import sys
import os
import string
import subprocess

dir_to_check = ""

def getInputOutputVars(testcnf):

    inputvars = []
    outputvars = []

    fp = open(testcnf, 'r')
    if fp:
        b = fp.readline()
        while b != "":
            m = re.search("^c ([\S]+) : ([\S]+) : input", b)
            if m:
                inputvars.append(m.group(2))
            else:
                m = re.search("^c ([\S]+) : ([\S]+) : output", b)
                if m:
                    outputvars.append(m.group(2))
            m = re.search("^p cnf", b)
            if m:
                break
            b = fp.readline()
    
        fp.close()
    return [inputvars, outputvars]
    
def getSubModel(testresultsfile, vars):
    submodel = []
    fp = open(testresultsfile, 'r')
    b = fp.readline() 
    while b != "":
        m = re.search("^UNSAT", b)
        if m:
            submodel = ""
            break
        else:
            m = re.search("^[-\d]", b)
            if m:
                model = string.split(b.rstrip()) 
                for v in vars:
                    submodel.append(model[int(v)-1])
        b = fp.readline()
    return submodel    

def getSubModelMaxsat(results, vars):
    submodel = []
    lines = results.splitlines()
    for b in lines:
        m = re.search("^v", b)
        if m:
            model = string.split(b.rstrip()) 
            for v in vars:
                submodel.append(model[int(v)])
    return submodel    




def runCircuitOnPattern(satsolver, cnf, inputpattern, outputvars):

    cnffilename = cnf + ".tmptest"
    outfp = open(cnffilename, 'w')
    fp = open(cnf, 'r')
     
    b = fp.readline()
    while b != "":
        m = re.search("^p cnf ([\d]+) ([\d]+)", b)
        if m:
            nclauses = int(m.group(2)) 
            print nclauses 
            nclauses = nclauses + len(inputpattern)
            print nclauses
            outfp.write("p cnf " + m.group(1) + " " + str(nclauses) + "\n")
        else:  
            outfp.write(b)
        b = fp.readline() 
    fp.close()

    for inputvar in inputpattern:
        outfp.write(inputvar + " 0\n")
    outfp.close() 

    outfile = cnffilename + ".sat"
    #os.system(satsolver + " " + cnffilename + " " + outfile)
   
    proc = subprocess.Popen(satsolver + " " + cnffilename + " " + outfile, stdout=subprocess.PIPE, shell=True)
    (out, err) = proc.communicate()
    
 
    outputvals = getSubModel(outfile, outputvars) 
    return outputvals

 
def checkFault(satsolver, pattern, faultycnf, inputvars, outputvars):
    
    faultyoutput = runCircuitOnPattern(satsolver, faultycnf, pattern, outputvars)
    return faultyoutput 


    
def getPattern(maxsat_solver, dir_to_check):

    testcnf = dir_to_check + "/" + "test_maxsat.cnf"
    testresultfile = testcnf + ".maxsat" 
    cmd_str = maxsat_solver + " " + testcnf   
    #os.system(cmd_str)
    proc = subprocess.Popen(cmd_str, stdout=subprocess.PIPE, shell=True)
    (maxout, err) = proc.communicate()
    [inputvars, outputvars] = getInputOutputVars(testcnf)
 
    # take the satisfying assignment and extract the test pattern from it 
    inputpattern = getSubModelMaxsat(maxout, inputvars)
    patt = []
    for e in inputpattern:
        m = re.search("^-", e)
        if m:
            patt.append("0")
        else:
            patt.append("1")
             
    return patt


# 'dir' should contain the following files:
# good.cnf - the output of Circuit::getCNF()->outputToFile(benchname, circuit_good.getExternToLine(), inlines, outlines, outputfile)
# test_maxsat.cnf - the maxsat problem outputted to a file, also with getExternToLine, inlines, outlines
# faultyN_saM.cnf - for each fault on line N and stuck-at-M, the output of circuit_good.getFaultyCNF(Fault(N, M))->outputToFile...;   
if __name__ == '__main__':

    if len(sys.argv) < 4:
        print "Usage: check.py <minisat location> <maxsat solver location> <dir name>"
        sys.exit(1)

    satsolver = sys.argv[1]
    maxsat_solver = sys.argv[2]
    dir_to_check = sys.argv[3]

    numTested = 0

    goodcnf = dir_to_check + "/" + "good.cnf" 
   
    pattern = getPattern(maxsat_solver, dir_to_check)
    print "Test pattern returned by maxsat solver: " + str(pattern)

   
    [goodin, goodout] = getInputOutputVars(goodcnf) 
    inputpattern = []
    for i in range(0, len(goodin)):
        if pattern[i] == "0":
            inputpattern.append("-" + goodin[i])
        else:
            inputpattern.append(goodin[i])
    print "Input pattern to good and faulty circuits: " + str(inputpattern)
    goodoutput = runCircuitOnPattern(satsolver, goodcnf, inputpattern, goodout) 
    
   
    direntries = os.listdir(dir_to_check)
    for e in direntries:
        m = re.search("^faulty([^_]+)_sa(.).cnf$", e)
        if m:
            faultycnf = dir_to_check + "/" + e  
            
            faultyoutput = checkFault(satsolver, inputpattern, faultycnf, goodin, goodout)
            if faultyoutput != goodoutput:
                numTested = numTested + 1

    print "Total number of faults tested by the maxsat solution: " + str(numTested)            
