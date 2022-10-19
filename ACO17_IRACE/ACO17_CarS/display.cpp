#include "ACO.h"

using namespace std;


/***********************************************/
/*                                             */
/*        Problem data terminal display        */
/*                                             */
/***********************************************/
void DisplayProblemData(ProblemData &problem)
{
    int i;
    cout << problemDisplay[0] << endl << problemDisplay[1] << endl;
    cout << problemDisplay[2] << problem.name << endl;
    cout << problemDisplay[3] << problem.type << endl;
    cout << problemDisplay[4] << problem.comment << endl;
    cout << problemDisplay[5] << problem.dimension << problemDisplay[6] << problem.cars_number;
    if(problem.pass_number > 0)
        cout << problemDisplay[7] << problem.pass_number;
    cout << endl;
    cout << problemDisplay2[0];
    switch(problem.edge_weight_type)
    {
        case ET_NONE: cout << problemDisplay2[2]; break;
        case ET_EUC_2D: cout << problemDisplay2[3]; break;
        case ET_EXPLICIT: cout << problemDisplay2[4]; 
    }
    cout << endl;
    cout << problemDisplay2[1];
    switch(problem.edge_weight_format)
    {
        case EF_NONE: cout << problemDisplay2[2]; break;
        case EF_FULL_MATRIX: cout << problemDisplay2[5]; break;
        case EF_VECTOR: cout << problemDisplay2[6]; 
    }
    cout << endl;
    if(problem.edge_weight_format == EF_FULL_MATRIX)
    {
        cout << problemDisplay3[0];
        switch(problem.edge_weight)
        {
            case SM_NONE: cout << problemDisplay2[2]; break;
            case SM_SYMMETRIC: cout << problemDisplay3[1]; break;
            case SM_ASIMMETRIC: cout << problemDisplay3[2]; 
        }
        cout << endl << endl;
        cout << problemDisplay3[3] << endl;
        vector< vector< vector<int> > >::iterator carIt;
        vector< vector<int> >::iterator nodeIt1;
        vector<int>::iterator nodeIt2;
        for(carIt=problem.nonEuclidSection.edgeWeight.begin(), i=0; carIt!=problem.nonEuclidSection.edgeWeight.end(); ++carIt, i++)
        {
            cout << problemDisplay3[5] << i << problemDisplay3[6] << endl;
            for(nodeIt1=carIt->begin(); nodeIt1!=carIt->end(); ++nodeIt1)
            {
                for(nodeIt2=nodeIt1->begin(); nodeIt2!=nodeIt1->end(); ++nodeIt2)
                    cout << *nodeIt2 << "\t";
                cout << endl;
            }
        }
        cout << endl << problemDisplay3[4] << endl;
        for(carIt=problem.nonEuclidSection.returnRate.begin(), i=0; carIt!=problem.nonEuclidSection.returnRate.end(); ++carIt, i++)
        {
            cout << problemDisplay3[5] << i << problemDisplay3[6] << endl;
            for(nodeIt1=carIt->begin(); nodeIt1!=carIt->end(); ++nodeIt1)
            {
                for(nodeIt2=nodeIt1->begin(); nodeIt2!=nodeIt1->end(); ++nodeIt2)
                    cout << *nodeIt2 << "\t";
                cout << endl;
            }
        }
    }
    if(problem.pass_number > 0)
    {
        int i;
        vector<int>::iterator it;
        cout << problemDisplay4[0] << endl;
        for(it=problem.carLimits.begin(), i=0; it!=problem.carLimits.end(); ++it, i++)
            cout << problemDisplay4[2] << i << "=" << *it << "; ";
        cout << endl << problemDisplay4[1] << endl;
        vector<Budget>::iterator budgetIt;
        for(budgetIt=problem.financialLimits.begin(), i=0; budgetIt!=problem.financialLimits.end(); ++budgetIt, i++)
            cout << problemDisplay4[3] << i << ": " << budgetIt->nodeStart << problemDisplay4[4] << budgetIt->nodeEnd << problemDisplay4[5] << budgetIt->budget << endl;
    }
}

/***********************************************/
/*                                             */
/*             Display parameters              */
/*                                             */
/***********************************************/

void DisplayParameters(Parameters &parameters)
{
    cout << parametersDisplay[0] << ": " << parameters.filename << std::endl;
    cout << parametersDisplay[1] << ": " << parameters.seed << std::endl;
    cout << parametersDisplay[2] << " " << parametersDisplay[4] << ": " << parameters.carRho << ", " << 
    parametersDisplay[5] << ": " << parameters.carAlpha << ", " << parametersDisplay[6] << ": " << parameters.carBeta << std::endl;
    cout << parametersDisplay[3] << " " << parametersDisplay[4] << ": " << parameters.nodeRho << ", " << 
    parametersDisplay[5] << ": " << parameters.nodeAlpha << ", " << parametersDisplay[6] << ": " << parameters.nodeBeta << std::endl;
    cout << parametersDisplay[7] << ": " << parameters.startNode << std::endl;
    cout << parametersDisplay[8] << ": " << parameters.nFavorits << std::endl;
    cout << parametersDisplay[9] << ": " << parameters.nAnts << ", " << parametersDisplay[10] << ": " << parameters.nIterations << std::endl;
    cout << parametersDisplay[11] << ": " << (parameters.test?parametersDisplay[12]:parametersDisplay[13]) << std::endl;
    cout << parametersDisplay[14] << ": " << parameters.pheroQ << std::endl;
    cout << parametersDisplay[15] << ": " << parameters.kappa << std::endl;
    cout << parametersDisplay3[0] << ": " << parameters.stall << std::endl;
    cout << parametersDisplay3[1] << ": " << (parameters.writeFile?parametersDisplay[12]:parametersDisplay[13]) << std::endl;
    cout << parametersDisplay2[0] << ": " << parameters.carMinRatio << ", " << parametersDisplay2[1] << ": " << parameters.nodeMinRatio;
    if(parameters.algorithmType == ALT_NEW)
        cout << " " <<parametersDisplay2[5];
    cout << endl;
    cout << parametersDisplay2[2] << ": " << parameters.carMinRatioExp << ", " << parametersDisplay2[3] << ": " << parameters.nodeMinRatioExp;
    if(parameters.algorithmType == ALT_OLD)
        cout << " " << parametersDisplay2[4];
    cout << endl;
}

/***********************************************/
/*                                             */
/*       Solution data terminal display        */
/*                                             */
/***********************************************/
void DisplaySolution(Solution solution)
{
    vector<int>::iterator it, it1;
    cout << solutionDisplay[0] << endl << solutionDisplay[1] << endl;
    cout << solutionDisplay[2] << solution.problemName << endl;
    cout << solutionDisplay[3] << solution.iteration << endl;
    cout << solutionDisplay[4] << solution.cost << endl;
    cout << solutionDisplay2[0] << endl;
    for(it=solution.nodeList.begin(); it!=solution.nodeList.end(); ++it)
        cout << *it << " ";
    cout << endl;
    cout << solutionDisplay2[1] << endl;
    for(it=solution.carList.begin(); it!=solution.carList.end(); ++it)
        cout << *it << " ";
    cout << endl;
    if(solution.passengersNodeList.size() > 0)
    {
        cout << solutionDisplay2[2] << endl;
        for(it=solution.nodeList.begin(); it!=solution.nodeList.end(); ++it)
        {
            vector<int> v = solution.passengersNodeList[*it];
            cout << *it << endl << "  ";
            for(it1=v.begin(); it1!=v.end(); ++it1)
                cout << *it1 << " ";
            cout << endl;
        }
    }
}

/***********************************************/
/*                                             */
/*         Display final solution cost         */
/*            for IRACE goal result            */
/*                                             */
/***********************************************/
void DisplayFinalPrice(Solution &solution)
{
    cout << "Final best price: ";
    cout << solution.globalCost;
    cout << ", found at iteration: " << solution.iteration << endl;
}


/***********************************************/
/*                                             */
/*     Algorithm type shared lib loaded        */
/*                                             */
/***********************************************/
void DisplayAlgoType(algoType type)
{
    if(type == ALT_OLD)
        cout << strSharedAlgos[0] << endl;
    else if(type == ALT_NEW)
        cout << strSharedAlgos[1] << endl;
}