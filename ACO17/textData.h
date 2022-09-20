#ifndef TEXTDATA_H
#define TEXTDATA_H

static const std::string strUsage[] = {"Usage: ", "(ACO17 ... -p problemFile | ACO17 problemFile) \
 [(-rs | --randomSeed) seed] [(-alt | --algoType) (old|new)] [(-sn | --startNode) startNode] [(-nf | --nFavorits) nFavorits] \
 [(-cr | --carRho) carRho] [(-nr | --nodeRho) nodeRho] [(-pQ | --pheroQ) pheroQ]\
 [(-ca | --carAlpha) carAlpha] [(-cb | --carBeta) carBeta] [(-na | --nodeAlpha) nodeAlpha] [(-nb | --nodeBeta) nodeBeta] \
 [(-nA | --nAnts) nAnts] [(-nIt | --nIterations) nIter] \
 [-nt | --no-test] [(-nK | --nKappa) nKappa]"};
static const std::string mainTest[] = {"All tests passed.","Load problem test failed.","Solution test failed.","Shared lib test failed."};
static const std::string strFileNotOpened = "file not opened.";
static const std::string strMatrixReadError = "matrix read error.";
static const std::string strCoordReadError = "node coordinates read error.";
static const std::string strVectorReadError = "vector read error.";

static const std::string strSharedLibLoad[] = {"Release shared library load.","Debug shared library load."};
static const std::string strSharedAlgos[] = {"Old algorithm loaded.","New algorithm loaded."};
static const std::string strSolutionLibError = "Solution library load error.";
static const std::string strSolutionFunctionLoadError = "Solution function load error.";

static const std::string strInitSolverError = "Init Solver Error.";

static const std::string problemDisplay[] = 
{"PROBLEM DATA","============","Problem name: ","Type: ","Comment: ","Nodes: ",", cars: ",", passengers: "};
static const std::string parametersDisplay[] = 
{"Problem","Seed","Car","Node","rho","alpha","beta","Start node","Favorites", "Ants","iterations","Test","yes","no","Phero Q","Kappa"};
static const std::string problemDisplay2[] =
{"Edge weight type: ","Edge weight format: ","NONE","EUCLID 2D","EXPLICIT","FULL MATRIX","VECTOR"};
static const std::string problemDisplay3[] = 
{"Edge weight: ","SYMMETRIC","ASYMMETRIC","Edge weight matrices:","Return rate matrices:","Car "," matrix:",};
static const std::string problemDisplay4[] = 
{"Car limits:","Passengers data:","C","P"," -> ",", budget:"};

static const std::string solutionDisplay[] = 
{"SOLUTION DATA", "============", "Problem name: ", "Iteration: ", "Cost: "};
static const std::string solutionDisplay2[] =
{"Nodes: ", "Cars: ", "Passengers: "};


#endif