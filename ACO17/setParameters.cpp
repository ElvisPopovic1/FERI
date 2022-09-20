#include "ACO.h"
#include "defaultParams.h"

void SetDefaultParams(Parameters &params);
void WriteHint(std::string programName);

bool SetParameters(int argc, char *argv[], Parameters &params)
{
    int i, iValue;
    double dValue;
    std::string s;
    std::stringstream ss;
    SetDefaultParams(params); //reset to default
    if(argc < 2)
    {
        WriteHint(std::string(argv[0]));
        return false;
    }
    if(argv[1][0]!='-')
        params.filename = std::string(argv[1]);

    for(i=1; i<argc; i++)
    {
        s=argv[i];
        if(s=="-nt" || s=="--no-test")
        {
            params.test = false;
        }
        else if(i<argc-1)
        {
            ss.str(argv[i+1]); //set ss 
            ss.clear(); //clear errors
            if(s=="-p" || s=="--problemFile")
                ss >> params.filename;
            else if(s=="-rs" || s=="--randomSeed")
                ss >> params.seed;
            else if(s=="-alt" || s=="--algoType")
            {
                if(ss.str() == "old")
                    params.algorithmType = ALT_OLD;
                else if(ss.str() == "new")
                    params.algorithmType = ALT_NEW;
            }
            else if(s=="-sn" || s=="--startNode")
            {   
                ss >> iValue;
                if(iValue >= 0)
                    params.startNode = iValue; //elese default
            }
            else if(s=="-nf" || s=="--nFavorits")
            {
                ss >> iValue;
                if(iValue > 0)
                    params.nFavorits = iValue;
            } 
            else if(s=="-cr" || s=="--carRho")
            {
                ss >> dValue;
                if(dValue > 0)
                    params.carRho = dValue;
            } 
            else if(s=="-ca" || s=="--carAlpha")
            {
                ss >> dValue;
                if(dValue > 0.5)
                    params.carAlpha = dValue;
            }
            else if(s=="-cb" || s=="--carBeta")
            {
                ss >> dValue;
                if(dValue > 0.5)
                    params.carBeta = dValue;
            }
            else if(s=="-nr" || s=="--nodeRho")
            {
                ss >> dValue;
                if(dValue > 0)
                    params.nodeRho = dValue;
            } 
            else if(s=="-na" || s=="--nodeAlpha")
            {
                ss >> dValue;
                if(dValue > 0.5)
                    params.nodeAlpha = dValue;
            }
            else if(s=="-nb" || s=="--nodeBeta")
            {
                ss >> dValue;
                if(dValue > 0.5)
                    params.nodeBeta = dValue;
            }
            else if(s=="-pQ" || s=="--pheroQ")
            {
                ss >> dValue;
                if(dValue > 1.0)
                    params.pheroQ = dValue;
            }
            else if(s=="-nA" || s=="--nAnts")
            {
                ss >> iValue;
                if(iValue > 0)
                    params.nAnts = iValue;
            }
            else if(s=="-nIt" || s=="--nIterations")
            {
                ss >> iValue;
                if(iValue > 0)
                    params.nIterations = iValue;
            }
            else if(s=="-nK" || s=="--nKappa")
            {
                ss >> iValue;
                if(iValue > 1)
                    params.kappa = iValue;
            }
        } 
    }
    return true;
}

void SetDefaultParams(Parameters &params)
{
    params.seed = SEED;
    params.algorithmType = ALGORITHM_TYPE;
    params.test = PERFORM_TEST;
    params.startNode = START_NODE;
    params.nFavorits = N_FAVORITS;

    params.carRho = params.nodeRho = RHO;
    params.carAlpha = params.nodeAlpha = ALPHA;
    params.carBeta  = params.nodeBeta  = BETA;
    params.pheroQ = PHERO_Q;
    params.nAnts = N_ANTS;
    params.nIterations = N_ITERATIONS;
    params.kappa = KAPPA;
}
void WriteHint(std::string programName)
{
     std::cout << strUsage[0] << programName << strUsage[1] << std::endl;
}