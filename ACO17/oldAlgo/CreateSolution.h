#ifndef CREATE_SOLUTION_H
#define CREATE_SOLUTION_H

#include <random>
#include "../ACO.h"
#include <queue>

#define PHERO_MAX 1000000

 /**************************
 *                         *
 *     data structures     *
 *                         *
 **************************/


/***************************
*    problem tensor        *
***************************/

//deepest tensor cell - node neighbors
//neighbors are sorted, so we need stored index
struct TensorCell
{
    int index; // neighbor node index
    int distance;
};

// in each tensor car has the car data and node subtensors
struct TensorCar
{
    int capacity;
    std::vector< std::vector<TensorCell> > nodes;
};


/***************************
*    pheromone tensor      *
***************************/

// nodes are in natural order (not sorted)
struct PheroCell
{
    double tau; //pheromones
    double eta; //heuristics
};

struct PheroNode
{
    std::vector<PheroCell> neighborsPhero; //node neighbors pheromones for each car, dimension nodes
    std::vector<PheroCell> carsPhero; //node cars pheromones for each car, cars_number cars
};

/***************************
*  for probability array   *
***************************/
struct ProbabilityCell
{
    int index;
    double value;
    double cumulative;
};

struct SolutionData
{
    ProblemData problemData;
    Parameters parameters;
    std::vector<TensorCar> tensor;
    std::vector<PheroNode> pheromones;
    std::deque<Solution> kappa;
    double carPheroMax, nodePheroMax;
    std::mt19937 mersenneGenerator;
};

 /********************
 *                   *
 *     functions     *
 *                   *
 ********************/

extern "C" bool InitSolver(ProblemData &problem, Parameters &params);
extern "C" double CalculateSolution(double (*calculateSolutionCost)(ProblemData &problem, Solution &solution), Solution &solution);
extern "C" bool InitSolverTest(ProblemData &problem, Solution &solution);
extern "C" bool SolverTest(Solution &solution, double (*calculateSolutionCost)(ProblemData &problem, Solution &solution));

void resetSolver();
void resetSolution(Solution &solution);
void createTensors();
void calculateCarHeuristics();
void calculateNodeHeuristics();
void CopySolution(Solution &source, Solution &destination); 
void AddKappa(Solution s);
Solution GetMinKappa();
void deleteStates(std::vector<bool> &carsUsed, std::vector<bool> &nodesVisited);

void AddProbability(std::vector<ProbabilityCell> &cells, int cellIndex, int itemIndex, double value);
void AddProbability(std::vector<ProbabilityCell> &cells, int cellIndex);
int PickSegment(std::vector<ProbabilityCell> &cells, int size, std::mt19937 &mersenneGenerator);

bool AntRun(Solution &antSolution);
int pickCar(std::vector<ProbabilityCell> &probs, std::vector<bool> &carsUsed, int currentCar, int currentNode);
int pickNode(std::vector<ProbabilityCell> &probs, std::vector<bool> &nodesVisited, int currentCar, int currentNode, int startNeighbor);
void UpdatePheromones(Solution &solution);
void CalculateMaxMin(double cost);
void LimitPheromones();

bool ProbabTest(std::mt19937 &mersenneGenerator);
#endif