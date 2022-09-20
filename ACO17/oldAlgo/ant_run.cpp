#include "CreateSolution.h"

extern SolutionData solData;

bool AntRun(Solution &antSolution)
{
    int node, currentCar, currentNode, startNeighbor, pick;
    std::vector<ProbabilityCell> probsCar(solData.problemData.cars_number+1); //picks n cars+last element
    std::vector<ProbabilityCell> probsNode(solData.problemData.dimension); //picks n-1 nodes+last element
    std::vector<bool> carsUsed(solData.problemData.cars_number);
    std::vector<bool> nodesVisited(solData.problemData.dimension);
    
    deleteStates(carsUsed, nodesVisited);

    currentCar = -1; //starting car - no car
    currentNode = solData.parameters.startNode; //starting node
    nodesVisited[currentNode]=true;
    for(node=0; node<solData.problemData.dimension; node++)
    {
        pick = pickCar(probsCar, carsUsed, currentCar, node);
        if(pick != currentCar)
        {
            if(currentCar!=-1)
                carsUsed[currentCar] = true;
            currentCar = pick;
        }
        antSolution.carList[node] = currentCar;
        antSolution.nodeList[node] = currentNode;

        startNeighbor = 1; //jump over self (0th element in tensor)
        if(node==solData.problemData.dimension-1)
            nodesVisited[solData.parameters.startNode]=false; //start node can be picked again
        do 
        {
            pick = pickNode(probsNode,nodesVisited,currentCar,currentNode,startNeighbor);
            if(pick >= 0)
            {
                nodesVisited[currentNode] = true;
                currentNode = pick;
            }
            else
                startNeighbor+=solData.parameters.nFavorits;
        }
        while(pick == -1 && startNeighbor < solData.problemData.dimension);
        if(pick == -1) //can't complete path (currently not possible)
            break;
    }
    antSolution.dimension = node;
    if(antSolution.dimension != solData.problemData.dimension)
        return false;
    return true;
}

void deleteStates(std::vector<bool> &carsUsed, std::vector<bool> &nodesVisited)
{
    int i;
    for(i=0; i<static_cast<int>(carsUsed.size()); i++)
        carsUsed[i] = false;
    //we don't reset pheromones
    for(i=0; i<static_cast<int>(nodesVisited.size()); i++)
        nodesVisited[i] = false;
}


/********************************************/
/*                                          */
/*           select (pick)                  */
/*          nodes and cars                  */
/*                                          */
/********************************************/

int pickCar(std::vector<ProbabilityCell> &probs, std::vector<bool> &carsUsed, int currentCar, int currentNode)
{
    int result, i, j;
    double p, alpha, beta; //probability
    TensorCar tc;
    PheroCell pc;
    result = 0;
    alpha = solData.parameters.carAlpha;
    beta = solData.parameters.carBeta;

    /* j - cell number, i - car index */
    for(i=0, j=0; i<solData.problemData.cars_number; i++)
    {
        if(carsUsed[i] == false)
        {
            tc = solData.tensor[i];
            pc = solData.pheromones[currentNode].carsPhero[i];
            p = pow(pc.tau, alpha)*pow(pc.eta, beta); 
            AddProbability(probs,j++, i, p);
        }
    }
    AddProbability(probs,j++);
    i = PickSegment(probs,j,solData.mersenneGenerator);
    result = probs[i].index;
    return result;
}

int pickNode(std::vector<ProbabilityCell> &probs, std::vector<bool> &nodesVisited, int currentCar, int currentNode, int startNeighbor)
{
    int result, i, j, k;
    double p, alpha, beta; //probability
    TensorCell tc;
    PheroCell pc;
    result = 0;
    alpha = solData.parameters.nodeAlpha;
    beta = solData.parameters.nodeBeta;
    /* run through sorted neighbors. Also sorted pheromone neighbors */
    for(i=startNeighbor, j=0; 
        i<startNeighbor+solData.parameters.nFavorits && i<solData.problemData.dimension; i++)
    {
        tc = solData.tensor[currentCar].nodes[currentNode][i];
        k = tc.index;
        pc = solData.pheromones[currentNode].neighborsPhero[k];
        if(nodesVisited[k] == false)
        {
            p = pow(pc.tau, alpha)*pow(pc.eta, beta); 
            AddProbability(probs,j++, k, p);
        }
    }
    AddProbability(probs,j++);
    i = PickSegment(probs,j,solData.mersenneGenerator);
    if(i>=0)
        result = probs[i].index;
    else
        result = -1;
    return result;
}


/********************************************/
/*                                          */
/*           pheromone update               */
/*                                          */
/********************************************/
void UpdatePheromones(Solution &solution)
{
    int i, j, c, n;
    double one_minus_rho, update;
    /* evaporation */
    one_minus_rho = 1.0-solData.parameters.carRho;
    /* evaporation cars */
    for(j=0; j<solData.problemData.dimension; j++)
        for(i=0; i<solData.problemData.cars_number; i++)
            solData.pheromones[j].carsPhero[i].tau *= one_minus_rho;
    
    one_minus_rho = 1.0-solData.parameters.nodeRho;
    /* evaporation nodes */
    for(j=0; j<solData.problemData.dimension; j++)
        for(i=0; i<solData.problemData.dimension; i++)
            solData.pheromones[j].neighborsPhero[i].tau *= one_minus_rho;

    if(solution.dimension != solData.problemData.dimension)
        return; //uncomplete path
    update = solData.parameters.pheroQ/solution.cost;
    for(i=0; i<solution.dimension; i++) //current nodes
    {
        c = solution.carList[i];
        j = solution.nodeList[i];
        if(i<solution.dimension-1)
            n = solution.nodeList[i+1];
        else
            n = solution.nodeList[0];
        solData.pheromones[j].carsPhero[c].tau += update;
        solData.pheromones[j].neighborsPhero[n].tau += update;
    }
}

void CalculateMaxMin(double cost)
{
    solData.carPheroMax = 1.0/(solData.parameters.carRho * cost);
    solData.nodePheroMax = 1.0/(solData.parameters.nodeRho * cost);
}

void LimitPheromones()
{
    int i, j;
    for(j=0; j<solData.problemData.dimension; j++)
    {
        for(i=0; i<solData.problemData.cars_number; i++)
            if(solData.pheromones[j].carsPhero[i].tau > solData.carPheroMax)
                solData.pheromones[j].carsPhero[i].tau = solData.carPheroMax;
        for(i=0; i<solData.problemData.dimension; i++)
            if(solData.pheromones[j].neighborsPhero[i].tau > solData.nodePheroMax)
                solData.pheromones[j].neighborsPhero[i].tau = solData.nodePheroMax;
    }
}
