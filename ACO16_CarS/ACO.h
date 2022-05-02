#ifndef ACO_H
#define ACO_H

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <sstream>
#include <random>
#include <limits>
#include <algorithm>
#include <chrono>

/* default only - if value from command line not available */
#define SEED -1
#define START_NODE 0
#define RHO1 0.1 //node pheromone evaporation
#define RHO2 0.1 //car pheromone evaporation
#define RHO3 0.7 //passenger iteration phero transfer
#define ALPHA1 0.6 //node pheromone exponent >= 0
#define BETA1 1.6 //node heuristic exponent >= 1
#define ALPHA2 0.6 //car pheromone exponent >= 0
#define BETA2 1.6 //car heuristic exponent >= 1
#define N_ANTS 15 //number of ants
#define N_ITER 100 //number of iterations
#define STALL 10
#define ALPHA3 1.4 //passengers' pheromone exponent >= 0
#define BETA3 2.9 //passengers' heuristic exponent >= 1
#define PHERO_MAX 10000
#define N_PASS_ANTS 16 //number complete pass searches
#define N_PASS_ITER 15 //number of passenger iterations
#define N_FAVORITS -1 //number of favorits to be picked

#define N_EXEC 50 //number of executions

typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned short ushort;

enum symmetry:unsigned char {SYMMETRIC, ASYMMETRIC};

struct dataPass;    //passengers from readed data

/* parameters */
struct parameters
{
    uint argc;
    int seed;
    int startNode; //-1 means random, if >= dim then dim-1
    float rho1; //stage 1 node evaporation
    float rho2; //stage 1 car evaporation
    float rho3; //stage 2 (passengers') pheromone evaporation
    float alpha1, beta1; // stage 1 node exponents
    float alpha2, beta2; // stage 1 car exponents
    float alpha3, beta3; // stage 2 (passengers) exponents
    uint nAnts;
    uint nIter;
    int stall;
    uint nPassAnts;
    uint nPassIters;
    uint nExecutions;
    int nFavorits;
    bool writeData;
    bool opt;  //do we calculate opt
    bool pass; //do we calculate passengers
    char fileName[128];
};


/* file data structure */
struct problemData
{
    char name[32]; //problem name
    char type[12]; //type
    char comment[64]; //comment
    uint dim; //dimensionality (number of nodes)
    ushort nCars; //number of cars
    ushort nPass; //number of passengers
    symmetry edgeWeight; //symmetry of edge weight matrices
    symmetry returnRate; //symmetry of return rate matrices
    float*** edgeWeightMatrices = nullptr; //array of edge weight matrices
    float*** returnRateMatrices = nullptr; //array of return rate matrices
    uchar* carPassLimit; //array of car capacities
    dataPass* passengers; //array of passengers (file data)
    /* not in original paper */
    float rentFee = 0.0;
    /* used by ants during passengers pick */
    uint maxNodePassengers = 0;
    uint maxCarPassengers = 0;
    float nodeNAverage; // for max-min Navg value - nodes
    // float carNAverage; for max-min Navg value - cars -- not used now
};

/* file data passenger structure */
struct dataPass
{
    uint startNode;
    uint destinationNode;
    float budget;
};

/* algorithm passengers structure */
struct pass
{
    char name[6];
    uint index;
    uint startNode;
    uint endNode;
    float phero;
    float budget;
    uint radius;
};

/* special structure for picked passengers on tours (ant and best tour) */
struct pathNodePass
{
    uchar nPassengers;
    pass **passengers;
    float *budgets;
};

struct neighbourSort
{
    float edgeWeight;
    int index;
};

/* problem node structure - static */
struct node
{
    char name[6];
    uint index;
    float *pheroNeighbours;
    float *pheroCars;
    uint nPassengers;
    pass** passengers;

    //neighbour sort indices (by distance)
    int** neighbourSortIndice; //2D, cars and neighbours
};

struct car
{
    char name[6];
    uint index;
    uchar carPassLimit;
};

/* ant node structure - dynamic */
/* uses memcpy, so no internal arrays allowed */
struct antNode
{
    node *curNode;     // pointer to problem node
    node *prevNode, *nextNode; // previous and next node
    car *carIn, *carOut; // old and new car in node
    float prob1, prob2; //used for max min calculus (probabilities), for nodes and for cars
    float choices1, choices2; //used for max min calculus (average), for nodes and cars
};


/* ant structure */
struct ant
{
    /* nodes, same ant on stage 2 achieves multiple opt tours and multiple passenger problems
       so, it has current value for prices, node array nodes and passenger nodes, 
       and also best prices, node arrays and pass nodes
    */
    uint nodeCounter;  //nodes counter for all - nodes, opt nodes and node pass arrays
    antNode *nodes;    //passed nodes
    antNode *optNodes;  //array of opt nodes - various opt tries
    antNode *bestOptNodes; // array of best opt nodes - final best opt

    /* prices - current prices passed as return values, so no need to be here */
    float priceNoPassengers; //price withous passengers and opt
    float bestOptPrice; //price with best opt
    float passPrice;

    /* pheromones */
    float pheroUpdate;
    
    /* passengers in nodes, same ant on stage 2 achieves multiple opt tours and multiple passenger problems
       so, it has current value for prices, node array nodes and passenger nodes, 
       and also best prices, node arrays and pass nodes
    */
    pathNodePass *tourPassengers; //passengers on board for each node on tour ???

    /* passengers' pheromones - current and best (stage 2) */
    float *passPheromones;

    /* flags */
    bool havePassengers; // do we calculated passengers?
    bool haveOpt; // do we have opt nodes?
    bool passWithOpt; // whether pass calculation use opt nodes or basic nodes
    bool closedPath;   //if this ant achieved closed path

    /* car stuff */
    node *carPickedNode; // where a particular car has been picked
   
    /* flag arrays */
    bool *nodesVisited; //all nodes visited flags
    bool *carsRented;   //all cars rented flags

    /* picked passengers - flags */
    bool *passPicked = nullptr;
};

struct transitionData
{
    uint nodeIndex;
    uint nextNodeIndex;
    uint carIndex;
    uint carPassLimit;

    float priceNoPass; //price without passengers
    float priceWithPass; //price with passengers
    uint nPassengers;
    pass **passengers; //passengers on board during transition
    float *budgets; //on board passengers' budgets during transition
};



struct bestPath
{
    uint iteration;
    uint nodeCounter;
    antNode *nodes;
    antNode *optNodes;

    bool haveOpt; // do we have opt nodes
    bool havePass; // do we have passengers

    // best path costs
    float price;
    float optPrice;
    float passPrice;

    //best path since pheromone reset costs
    float pheroResetPrice;
    float pheroResetOptPrice;
    float pheroResetPassPrice;


    bool passWithOpt;
    /* passengers in nodes */
    uint nPickedPassengers;
    pathNodePass *tourPassengers; //passengers on board for each node on tour
    bool *pickedPassengers; //for display purposes, calculated at the end from tourPassengers

    float pheroMin1, pheroMax1, pheroRatio1; //nodes pheromones min and max values and ratio
    float pheroMin2, pheroMax2, pheroRatio2; //cars pheromones min and max values and ratio

    float *passPheromones;

    std::chrono::duration<double> solution_time;
};

/* all ants use it - make it thread safe if ants uses threads */
struct probabilityArrays
{
    uint n; //number of picks in favorits
    uint n1; //number of all possible picks (out of favorites)

    uint *indices; // elements indices (prob. arrays have elements and their probabilities)
    int selected; // 
    float *probs; // probabilities
    float *cumulatives; // cumulative probabilities

    float sum;
    float sum1;
};


extern std::mt19937* mersenneGenerator;
extern parameters parData;
extern problemData prData;
extern probabilityArrays probArrays;
extern bestPath bPath;
/* declaration in ACO.cpp */
extern node* nodes;
extern car* cars;
extern pass* passengers;
extern ant* ants;

extern float pheroMin1, pheroMin2, pheroMax1, pheroMax2;
extern float pheroRatio1, pheroRatio2;

/* chrono timer stuff */
extern std::chrono::duration<double> elapsed_time;
extern std::chrono::system_clock::time_point start_time, current_time;

/* final price for IRACE - can be executed after cleanup */
void displayFinalPrice();


/* parser functions */
bool setParameters(int argc, char** argv);
bool loadData(const char* filename);
void freeData();
int parseHeaderToken(const char* name);
bool parseMatrix(std::ifstream& dataFile, float*** matrix, int nMatrix, int dim);


/* initializer */
void init();
void cleanup();

/* algorithm part */
bool Solution(uint iter, node *startNode);
void opt2_5();
void PheromoneEvaporation();
bool updatePheromones(ant *bestAnt);
void limitPheromoneTraces();
ant* findBestAnt();
bool updateBestPath(uint iteration, ant *bestAnt, std::chrono::duration<double> sol_time);
bool updateBestPathSincePheroReset(ant *bestAnt);
float calculatePathCost(antNode *nodes, uint nodeCounter);

/* 2nd stage algorithm part */


/* probabilitiy */
int PickNode(ant *currentAnt, node *currentNode, car *currentCar);
int PickCar(ant *currentAnt, node *currentNode, car *currentCar);

uint selectFromFreqArray(float sum, uint n, float *probabilities);
void calculateMaxMin();


/* display part */
void displayProblemData();
void displayBestPath(std::chrono::duration<double> cur_time);

/* display part for passengers */
extern void initPassManager();
extern void deletePassManager();


/* calling when iterations' solutions stall */
bool checkSolutionsStall(ant *bestAnt);
void resetPheromones();

#endif // ACO_H_INCLUDED
