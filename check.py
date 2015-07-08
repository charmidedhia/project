import re
import sys
import os
import string

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
    
def getSubModel(resultfile, vars):
    submodel = []
    fp = open(resultfile, 'r')
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



def runCircuitOnPattern(satsolver, goodcnf, inputpattern, outputvars):

    goodfilename = goodcnf + ".tmptest"
    outfp = open(goodfilename, 'w')
    fp = open(goodcnf, 'r')
     
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

    outfile = goodfilename + ".sat"
    os.system(satsolver + " " + goodfilename + " " + outfile)
    
    outputvals = getSubModel(outfile, outputvars) 
    return outputvals

 
def checkFault(satsolver, fault_line, stuckat):

    goodcnf = dir_to_check + "/" + "good.cnf" 
    testcnf = dir_to_check + "/" + "test" + fault_line + "_sa" + stuckat + ".cnf"
    faultycnf = dir_to_check + "/" + "faulty" + fault_line + "_sa" + stuckat + ".cnf"
    testresultfile = testcnf + ".sat" 
    cmd_str = satsolver + " " + testcnf + " " + testresultfile  
    os.system(cmd_str)

    # get the variables corresponding to the input and output lines
    [inputvars, outputvars] = getInputOutputVars(testcnf) 
 
    # take the satisfying assignment and run it through the good and faulty circuits
    inputpattern = getSubModel(testresultfile, inputvars)

    if inputpattern != "":
        goodoutput = runCircuitOnPattern(satsolver, goodcnf, inputpattern, outputvars) 
        faultyoutput = runCircuitOnPattern(satsolver, faultycnf, inputpattern, outputvars)
        if goodoutput == faultyoutput:
            print "ERROR: " + testcnf
            print "Good output: " + str(goodoutput)
            print "Faulty output: " + str(faultyoutput)
    else:
        print "Undetectable: " + testcnf  

if __name__ == '__main__':

    if len(sys.argv) < 3:
        print "Usage: check.py <minisat location> <dir name>"
        sys.exit(1)

    satsolver = sys.argv[1]
    dir_to_check = sys.argv[2]
    direntries = os.listdir(dir_to_check)
    for e in direntries:
        m = re.search("^test([^_]+)_sa(.).cnf", e)
        if m:
            fault_line = m.group(1)
            stuckat = m.group(2)

            checkFault(satsolver, fault_line, stuckat)
            
