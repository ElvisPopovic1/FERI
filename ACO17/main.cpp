#include "ACO.h"

using namespace std;

bool PerformTests(void *solutionLib);
bool SharedLibrarySolverTest(void *solutionLib, double(*CalculateSolutionCost)(ProblemData &problem, Solution &solution));

int main(int argc, char *argv[])
{
    ProblemData problem;
    Parameters params;
    Solution solution;

    /* shared library pointers */
    void *solutionLib;
    bool (*initSolver)(ProblemData &problem, Parameters &params);
    double (*calculateSolution)(double (*CalculateSolutionCost)(ProblemData &problem, Solution &solution), Solution &solution);
    const char *error;

        /* set parameters and load problem */
    if(!SetParameters(argc, argv, params) || !LoadProblem(params.filename, problem))
        return EXIT_FAILURE;
    DisplayParameters(params);


    /* load shared library */
    #ifndef DEBUG
        if(params.algorithmType == ALT_NEW)
            solutionLib = dlopen("bin/release/createSolution_new.so", RTLD_LAZY);
        else
            solutionLib = dlopen("bin/release/createSolution_old.so", RTLD_LAZY);
    #else
        if(params.algorithmType == ALT_NEW)
            solutionLib = dlopen("bin/debug/createSolution_new.so", RTLD_LAZY);
        else
            solutionLib = dlopen("bin/debug/createSolution_old.so", RTLD_LAZY);
        cout << strSharedLibLoad[1] << endl;
    #endif
    if(!solutionLib)
    {
        cerr << strSolutionLibError << endl;
        return EXIT_FAILURE;
    }
    else
    {
        if(params.algorithmType == ALT_NEW)
            DisplayAlgoType(ALT_NEW);
        else
            DisplayAlgoType(ALT_OLD);
    }
    /* obtain shared library functioons' pointers */
    dlerror(); //reset error
    initSolver = (bool (*)(ProblemData &problem, Parameters &params))dlsym(solutionLib, "InitSolver");
    calculateSolution = (double (*)(double(*CalculateSolutionCost)(ProblemData &problem, Solution &solution), Solution &solution))dlsym(solutionLib, "CalculateSolution");
    if((error = dlerror()))
    {
        cerr << strSolutionFunctionLoadError << ": " << error << endl;
        dlclose(solutionLib);
        return EXIT_FAILURE;
    }

    /* perform all tests - needs test switch parameter */
    if(params.test == true)
        if(PerformTests(solutionLib)==false)
        {
            dlclose(solutionLib);
            return EXIT_FAILURE;
        }

    /* solver initialization - must have params and problem */
    if(!initSolver(problem,params))
    {
        cerr << strInitSolverError << endl;
        dlclose(solutionLib);
        return EXIT_FAILURE;
    }

    /* start algorithm */
    calculateSolution(&CalculateSolutionCost, solution);

    /* temporary */
    cout << "Problem: " << problem.name << ", nFav: " << params.nFavorits << ", result = " << solution.cost << endl;

    dlclose(solutionLib);
    return EXIT_SUCCESS;
}

/**********************************/
/*                                */
/*           test part            */
/*                                */
/**********************************/
bool PerformTests(void *solutionLib)
{
    if(!LoadProblemTest())
    {
        cout << mainTest[1] << endl;
        return false;
    }
    if(!SolutionTest())
    {
        cout << mainTest[2] << endl;
        return false;
    }

    if(!SharedLibrarySolverTest(solutionLib,*CalculateSolutionCost))
    {
        cout << mainTest[3] << endl;
        return false;
    }
    cout << mainTest[0] << endl; //all tests passed
    return true;
}
bool SharedLibrarySolverTest(void *solutionLib, double(*CalculateSolutionCost)(ProblemData &problem, Solution &solution))
{
    bool (*initSolverTest)(ProblemData &problem, Solution &solution);
    bool (*solverTest)(Solution &solution, double (*CalculateSolutionCost)(ProblemData &problem, Solution &solution));
    ProblemData problem;
    Solution solution;
    initSolverTest = (bool (*)(ProblemData &problem, Solution &solution))dlsym(solutionLib,"InitSolverTest");
    solverTest = (bool   (*)(Solution &solution, double(*CalculateSolutionCost)(ProblemData &problem, Solution &solution)))dlsym(solutionLib,"SolverTest");

    /* test problem with passengers */
    if(!LoadProblem("testFiles/Chi6e.car", problem))
        return false;
    if(!LoadSolution("testFiles/Chi6e_solution.txt",solution))
        return false;
    cout << "Start test" << endl;
    if(!initSolverTest(problem, solution))
        return false;
    cout << "End test" << endl;
    if(!solverTest(solution, CalculateSolutionCost))
        return false;
    
    /* test problem without passengers */
    if(!LoadProblem("testFiles/BrasilRJ14n.car", problem))
        return false;
    if(!LoadSolution("testFiles/BrasilRJ14n_solution.txt",solution))
        return false;

    if(!initSolverTest(problem, solution))
        return false;
    if(!solverTest(solution, CalculateSolutionCost))
        return false;

    return true;
}