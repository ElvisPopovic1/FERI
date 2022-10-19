#include "ACO.h"

bool ParsePassengerList(std::vector< std::vector<int> > &passList, std::ifstream &dataFile, int dimension, int passNodes);
void ClearSolution(Solution &solution);
bool SolutionTest(std::string problemFile, std::string solutionFile, std::string name, int dimension, int ass1, double ass2, int iteration, double cost);

/*********************************************************************/
/*                                                                   */
/*                         solution functions                        */
/*                                                                   */
/*********************************************************************/

/* load solution from file */
bool LoadSolution(std::string filename, Solution &solution)
{
    std::string line, token, svalue;
    int value;
    std::stringstream ss;
    std::ifstream dataFile(filename);
    ClearSolution(solution);
    if(!dataFile.is_open())
    {
        std::cerr << filename << ": " << strFileNotOpened << std::endl;
        return false;
    }
    while(std::getline(dataFile, line))
    {
        clearStringStream(ss,line);
        token = "";
        ss >> token;
        if((token == "NAME") && parseColon(token, ss))
            ss >> solution.problemName;
        else if((token == "ITERATION") && parseColon(token, ss))
            ss >> solution.iteration;
        else if((token == "COST") && parseColon(token, ss))
            ss >> solution.cost;
        else if(token == "NODES")
        {
            if(!getline(dataFile, line))
            {
                dataFile.close();
                return false; 
            }
            clearStringStream(ss,line);
            while (ss)
            {
                if(ss >> value)
                    solution.nodeList.push_back(value);
            }
            solution.dimension = solution.nodeList.size();
        }
        else if(token == "CARS")
        {
            bool result = true;
            if(!getline(dataFile, line))
            {
                dataFile.close();
                result = false;
            }
            clearStringStream(ss,line);
            while (ss)
            {
                if(ss >> value)
                    solution.carList.push_back(value);
            }
            if(solution.carList.size() != (size_t)solution.dimension)
                result = false;
            if(result==false)
            {
                dataFile.close();
                return false;
            }
        }
        else if((token == "PASSENGERS") && (parseColon(token, ss)))
        {
            bool result = true;
            int passNodes;
            ss >> passNodes;
            if(passNodes < 0 || passNodes > solution.dimension)
                result = false;
            if(ParsePassengerList(solution.passengersNodeList, dataFile, solution.dimension, passNodes)==false)
                result = false;
            if(result == false)
            {
                dataFile.close();
                return false;
            }
        }
    }
    dataFile.close();
    return true;
}

/* save solution to file */
bool SaveSolution(std::string filename, Solution solution)
{
    int n;
    std::ofstream resultStream;
    std::vector<int>::iterator it, it1;
    std::vector< std::vector<int> >::iterator it2;
    resultStream.open(filename, std::ios::out);
    if(!resultStream.is_open())
    {
        std::cerr << filename << ": " << strFileNotOpened << std::endl;
        return false;
    }
    resultStream << "NAME : " << solution.problemName << std::endl;
    resultStream << "ITERATION : " << solution.iteration << std::endl;
    resultStream << "COST : " << solution.cost << std::endl;
    resultStream << "NODES" << std::endl << "  ";
    for(it=solution.nodeList.begin(); it!=solution.nodeList.end(); ++it)
        resultStream << *it << " ";
    resultStream << std::endl << "CARS" << std::endl << "  ";
    for(it=solution.carList.begin(); it!=solution.carList.end(); ++it)
        resultStream << *it << " ";
    for(n=0, it2=solution.passengersNodeList.begin(); it2!=solution.passengersNodeList.end(); ++it2)
        if(it2->size()>0)
            n++;
    if(solution.passengersNodeList.size()>0)
    {
        resultStream << std::endl << "PASSENGERS : " << n << std::endl;
        for(it=solution.nodeList.begin(); it!=solution.nodeList.end(); ++it)
            if(solution.passengersNodeList[*it].size()>0)
            {
                resultStream << *it << std::endl << "  ";
                for(it1=solution.passengersNodeList[*it].begin(); it1!=solution.passengersNodeList[*it].end(); ++it1)
                    resultStream << *it1 << " ";
                resultStream << std::endl;
            }
    }
    resultStream << "EOF" << std::endl;
    resultStream.close();
    return true;
}

/* clears solution */
void ClearSolution(Solution &solution)
{
    solution.cost = 0.0;
    solution.dimension = 0;
    solution.iteration = 0;
    solution.nodeList.clear();
    solution.carList.clear();
    solution.passengersNodeList.clear();
}

/* calculate solution cost */
double CalculateSolutionCost(ProblemData &problem, Solution &solution)
{
    // n1 current node, n2 next node
    // c1 last car, c2 current car
    // np number of passengers in the car
    int i, n1, n2, c1, c2, cp, np;
    double result;
    if(solution.dimension < 2 || solution.nodeList.size() != (size_t)solution.dimension)
        return 0.0;
    if(problem.edge_weight_format != EF_FULL_MATRIX || problem.edge_weight_type != ET_EXPLICIT)
        return 0.0; //not implemented
    cp = solution.nodeList[0];
    c1 = solution.carList[0];
    result = 0.0;
    for(i=0; i<solution.dimension; i++)
    {
        n1 = solution.nodeList[i];
        if(i<solution.dimension-1)
            n2 = solution.nodeList[i+1];
        else
            n2 = solution.nodeList[0];
        if(i>0)
            c1 = solution.carList[i-1];
        c2= solution.carList[i];
        if(solution.passengersNodeList.size()>0)
            np = solution.passengersNodeList[n1].size()+1;
        else
            np = 1;
        result += (double)problem.nonEuclidSection.edgeWeight[c2][n1][n2]/np;
        if(c1 != c2)
        {
            result += problem.nonEuclidSection.returnRate[c1][n1][cp];
            cp = n1;
        }
    }
    result += problem.nonEuclidSection.returnRate[c1][0][cp];
    solution.cost = result;
    return result;
}

/*********************************************************************/
/*                                                                   */
/*   auxiliary functions required by the loadProblem funcion         */
/*                                                                   */
/*********************************************************************/

bool ParsePassengerList(std::vector< std::vector<int> > &passList, std::ifstream &dataFile, int dimension, int passNodes)
{
    int i, node, passenger;
    std::string line;
    std::stringstream ss;
    for(i=0; i<dimension; i++)
    {
        std::vector<int> v;
        passList.push_back(v);
    }
    for(i=0; i<passNodes; i++)
    {
        if(!getline(dataFile, line))
            return false;
        clearStringStream(ss,line);
        ss >> node;
        if(!getline(dataFile, line))
            return false;
        clearStringStream(ss,line);
        while(ss)
        {
            if(ss >> passenger)
                passList[node].push_back(passenger);
        }
    }
    return true;
}

std::string getFileName(std::string filePath, bool withExtension = true, char seperator = '/')
{
    // Get last dot position
    std::size_t dotPos = filePath.rfind('.');
    std::size_t sepPos = filePath.rfind(seperator);
    if(sepPos != std::string::npos)
    {
        return filePath.substr(sepPos + 1, filePath.size() - (withExtension || dotPos != std::string::npos ? 1 : dotPos) );
    }
    return "";
}

/**********************************/
/*                                */
/*           test part            */
/*                                */
/**********************************/
bool SolutionTest()
{
    /* passengers example test */
    if(!SolutionTest("testFiles/Chi6e.car","testFiles/Chi6e_solution.txt","Chile6e",6,19, 128.0, 36,264.15))
        return false;
    /* no passengers example test */
    if(!SolutionTest("testFiles/BrasilRJ14n.car","testFiles/BrasilRJ14n_solution.txt","BrasilRJ14n",14,96, 0.0, 106,175.0))
        return false;
    if(!SolutionTest("testFiles/BrasilRJ14n.car","testFiles/BrasilRJ14n_solution_best.txt","BrasilRJ14n",14,97,0.0,45,167.0))
        return false;
    return true;
}

/* problem name, solution name, name, dimension, solution nodes and cars sum, passenger sum, num of ioterations, solution cost */
bool SolutionTest(std::string problemFile, std::string solutionFile, std::string name, int dimension, int ass1, double ass2, int iteration, double cost)
{
    int sum;
    Solution solution1, solution2;
    ProblemData problem;
    std::vector<int>::iterator it;
    std::vector< std::vector<int> >::iterator it2;

    /* load solution from test folder */
    if(!LoadSolution(solutionFile.c_str(), solution1))
        return false;
    if((solution1.problemName != name) || (solution1.dimension != dimension) || 
        (solution1.iteration != iteration) || (std::abs(solution1.cost - cost)>0.001))
        return false;
    sum = 0;
    for(it=solution1.nodeList.begin(); it!=solution1.nodeList.end(); ++it)
        sum += *it;
    for(it=solution1.carList.begin(); it!=solution1.carList.end(); ++it)
        sum += *it;
    if(sum != ass1)
        return false;
    sum = 0;
    for(it2=solution1.passengersNodeList.begin(); it2!=solution1.passengersNodeList.end(); ++it2)
        for(it=it2->begin(); it!=it2->end(); ++it)
            sum += *it;
    if(sum != ass2)
        return false;
    std::string filename;
    std::stringstream ss;
    filename = getFileName(solutionFile.c_str(), false, '/');
    ss << filename << ".tmp";
    /* save solution */
    if(SaveSolution(ss.str(), solution1)==false)
        return false;
    /* load problem */
    if(LoadProblem(problemFile.c_str(), problem)==false)
        return false;
    /* load saved solution */
    if(!LoadSolution(ss.str(), solution2))
        return false;
    remove(ss.str().c_str());
    /* check some solution2 items */
    if((solution2.problemName != name) || (solution2.dimension != dimension) || 
        (solution2.iteration != iteration) || (std::abs(solution2.cost - cost)>0.001))
        return false;
    /* calculate cost */
    cost = CalculateSolutionCost(problem, solution2);
    /* compare calculated cost to stored costs in both solutions */
    if(std::abs(solution1.cost-cost)>0.001 || std::abs(solution2.cost-cost)>0.001)
        return false;
    return true;
}

