#ifndef ACO_H
#define ACO_H

#include <iostream>
#include <cstring>
#include <vector>
#include <sstream>
#include <fstream>
#include <dlfcn.h>
#include <algorithm>
#include "textData.h"

/* symmetry, type and format enumerations */
enum symmetry {SM_NONE, SM_SYMMETRIC, SM_ASIMMETRIC};
enum edgeType {ET_NONE, ET_EXPLICIT, ET_EUC_2D};
enum edgeFormat {EF_NONE, EF_VECTOR, EF_FULL_MATRIX};
enum algoType {ALT_OLD, ALT_NEW};

 /**************************
 *                         *
 *     data structures     *
 *                         *
 **************************/

/* euclidean problem (coordinates) data substructure */
struct EuclidSection
{
    std::vector< std::vector<float> > nodeCoordSection; 
    std::vector< std::vector<int> > edgeWeight;
    std::vector< std::vector<int> > returnRate;
};

/* non euclidean problem (matrices) data substructure */
struct NonEuclidSection
{
    std::vector< std::vector< std::vector<int> > > edgeWeight;
    std::vector< std::vector< std::vector<int> > > returnRate;
};

/* passenger start node, end node and financial limit data substructure */
struct Budget
{
    int nodeStart;
    int nodeEnd;
    float budget;
};

struct Parameters
{
    std::string filename;
    int seed;
    algoType algorithmType;
    bool test;
    bool writeFile;
    int startNode;
    int nFavorits;
    double carRho, carAlpha, carBeta;
    double nodeRho, nodeAlpha, nodeBeta;
    double pheroQ;
    int nAnts, nIterations;
    int kappa;
    double carMinRatio, nodeMinRatio;
    int carMinRatioExp, nodeMinRatioExp;
    int stall;
};

/* problem data structure */
struct ProblemData
{
    std::string name;
    std::string type;
    std::string comment;
    int dimension;
    int cars_number;
    int pass_number;
    symmetry edge_weight;
    symmetry return_rate;
    edgeType edge_weight_type;
    edgeFormat edge_weight_format;
    EuclidSection euclidSection;
    NonEuclidSection nonEuclidSection;
    std::vector<int> carLimits;
    std::vector<Budget> financialLimits;
};

struct Solution
{
    std::string problemName;
    int dimension;
    int iteration;
    std::vector<int> nodeList;
    std::vector<int> carList;
    std::vector< std::vector<int> > passengersNodeList;
    double cost; //universal cost, including cost since reset
    double globalCost; //global best cost
};

 /********************
 *                   *
 *     functions     *
 *                   *
 ********************/

/* in loadProblem.cpp */
bool LoadProblem(std::string filename, ProblemData &problem);
bool LoadProblemTest();

/* in setparameters.cpp */
bool SetParameters(int argc, char *argv[], Parameters &params);

/* in solution.cpp */
bool LoadSolution(std::string filename, Solution &solution);
bool SaveSolution(std::string filename, Solution solution);
double CalculateSolutionCost(ProblemData &problem, Solution &solution);
bool SolutionTest();

/* in display.cpp */
void DisplayProblemData(ProblemData &problem);
void DisplayParameters(Parameters &parameters);
void DisplaySolution(Solution solution);
void DisplayFinalPrice(Solution &solution);
void DisplayAlgoType(algoType type);

/* auxiliary functions */
bool parseColon(std::string &token, std::stringstream &ss);
void clearStringStream(std::stringstream &ss, std::string lineString);

#endif