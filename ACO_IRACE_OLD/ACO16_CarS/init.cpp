#include "ACO.h"
#include "passengers.h"

using namespace std;

extern passManager* pManager;

void initNodes();
void initCars();
void initPassengers();
void initAnts();
// int  divideCarChoices(int c, int nodeDim, int depth);
void initProbArrays();
void initBestPath();

void initPassManager();
void deletePassManager();

/* initialize and cleanup */
void init()
{
     mt19937::result_type seed;
    if(parData.seed == -1)
        parData.seed = time(0);
    seed = parData.seed;
    mersenneGenerator = new mt19937(seed);

    initPassengers();
    initCars();
    initNodes();
    initAnts();
    initPassManager();
    initProbArrays();
    initBestPath();
}


void cleanup()
{
    uint i, j;
    node* ptNode;
    ant* ptAnt;
    int **pptInt;
    pathNodePass *ptPathNodePass;
    if(nodes != nullptr)
    {
        for(j=0, ptNode=nodes; j<prData.dim; j++, ptNode++)
        {
            for(i=0, pptInt=ptNode->neighbourSortIndice; i<prData.nCars; i++, pptInt++)
                delete[] *pptInt;
            delete[] ptNode->neighbourSortIndice;
            delete[] ptNode->pheroNeighbours;
            delete[] ptNode->pheroCars;
            delete[] ptNode->passengers;
        }
        delete[] nodes; 
    }
    if(passengers != nullptr)
        delete[] passengers; 

    if(cars != nullptr)
        delete[] cars;
    
    if(ants != nullptr)
    {
        for(j=0, ptAnt=ants; j<parData.nAnts; j++, ptAnt++)
        {
            delete[] ptAnt->bestOptNodes;
            delete[] ptAnt->optNodes;
            delete[] ptAnt->nodes;
            delete[] ptAnt->nodesVisited;
            delete[] ptAnt->carsRented;
            if(ptAnt->tourPassengers != nullptr)
            {
                for(i=0, ptPathNodePass=ptAnt->tourPassengers; i<prData.dim; i++, ptPathNodePass++)
                {
                    delete[] ptPathNodePass->passengers;
                    delete[] ptPathNodePass->budgets;
                }
                delete[] ptAnt->tourPassengers;
            }
            if(ptAnt->passPicked !=nullptr)
                delete[] ptAnt->passPicked;
            if(ptAnt->passPheromones != nullptr)
                delete[] ptAnt->passPheromones;
        }
        delete[] ants;
    }
    deletePassManager();
    delete[] probArrays.indices;
    delete[] probArrays.probs;
    delete[] probArrays.cumulatives;

    if(bPath.pickedPassengers != nullptr)
        delete[] bPath.pickedPassengers;
    if(bPath.passPheromones != nullptr)
        delete[] bPath.passPheromones;

    if(bPath.tourPassengers != nullptr)
    {
        for(j=0, ptPathNodePass=bPath.tourPassengers; j<prData.dim; j++, ptPathNodePass++)
        {
            delete[] ptPathNodePass->passengers;
            delete[] ptPathNodePass->budgets;
        }
        delete[] bPath.tourPassengers;
    }

    delete[] bPath.nodes;
    delete[] bPath.optNodes;
    bPath.nodeCounter = 0;
    delete mersenneGenerator;
}

bool neighbourSortIndiceCompare(neighbourSort& ns1, neighbourSort& ns2)
{
    return ns1.edgeWeight < ns2.edgeWeight;
}

void createSortIndice(node *ptNode)
{
    neighbourSort ns;
    int** pptInt;
    int* ptInt;
    uint i, j;
    if(ptNode == nullptr)
        return;
    for(j=0, pptInt=ptNode->neighbourSortIndice; j<prData.nCars; j++, pptInt++)
    {
        vector<neighbourSort> v;
        for(i=0, ptInt=*pptInt; i<prData.dim; i++, ptInt++)
        {
            ns.edgeWeight = prData.edgeWeightMatrices[j][ptNode->index][i];
            ns.index = i;
            v.push_back(ns);
        }  
        sort(v.begin(), v.end(), neighbourSortIndiceCompare); 
        for(i=0, ptInt=*pptInt; i<prData.dim; i++, ptInt++)
            *ptInt = v[i].index;
    }
}

/* separated elements */
void initNodes()
{
    uint i, j;
    node* ptNode;
    pass* ptPass;
    int **pptInt;
    float* ptFloat;

    nodes = new node[prData.dim];
    for(j=0, ptNode=nodes; j<prData.dim; j++, ptNode++)     //for each node
    {
        sprintf(ptNode->name, "V%d", j);
        ptNode->index = j;
        ptNode->pheroNeighbours = new float[prData.dim];
        ptNode->pheroCars = new float[prData.nCars];
        for(i=0, ptFloat=ptNode->pheroNeighbours; i<prData.dim; i++, ptFloat++)
        {
            if(i==j)
                *ptFloat = 0.0;
            else
                *ptFloat = pheroMax1;        
        }
        for(i=0, ptFloat=ptNode->pheroCars; i<prData.nCars; i++, ptFloat++)
            *ptFloat = pheroMax2;
        ptNode->nPassengers = 0;

        //neighbour sort indice
        ptNode->neighbourSortIndice = new int*[prData.nCars];
        for(i=0, pptInt=ptNode->neighbourSortIndice; i<prData.nCars; i++, pptInt++)
            *pptInt = new int[prData.dim];
        createSortIndice(ptNode);
    }
    /* number of passengers */
    for(i=0, ptPass = passengers; i<prData.nPass; i++, ptPass++)
        nodes[ptPass->startNode].nPassengers++;

    
    /* max node pasengers in prData */
    for(i=0, ptNode=nodes; i<prData.dim; i++, ptNode++)
        if(ptNode->nPassengers > prData.maxNodePassengers)
            prData.maxNodePassengers = ptNode->nPassengers;

    /* init array of passengers */
    for(j=0, ptNode=nodes; j<prData.dim; j++, ptNode++) 
    {
        ptNode->passengers = new pass*[ptNode->nPassengers];
        ptNode->nPassengers = 0;
    }
    /* fill node's arrays of passengers */
    for(i=0, ptPass = passengers; i<prData.nPass; i++, ptPass++)
    {
        ptNode = &nodes[ptPass->startNode];
        ptNode->passengers[(ptNode->nPassengers)++] = ptPass;
    }
    /* calculate average available neighbour nodes in each path node */
    prData.nodeNAverage = (prData.dim-1)/2.0;
}

void initCars()
{
    uint i, max = 0;
    car *ptCar;
    uchar *ptUchar;
    cars = new car[prData.nCars];
    for(i=0, ptCar = cars, ptUchar = prData.carPassLimit; i<prData.nCars; i++, ptCar++, ptUchar++)
    {
        ptCar->index = i;
        sprintf(ptCar->name, "C%d", i);
        ptCar->carPassLimit = prData.carPassLimit[i];
        if(ptCar->carPassLimit > max)
            max = ptCar->carPassLimit;
    }
    prData.maxCarPassengers = max;
}

void initPassengers()
{
    uint i;
    pass* ptPass;
    dataPass* ptDataPass;
    if(prData.nPass > 0)
        passengers = new pass[prData.nPass];
    /* copy data from dataPass structures to pass structures */
    for(i=0, ptPass=passengers, ptDataPass=prData.passengers; i<prData.nPass; i++, ptPass++, ptDataPass++)
    {
        ptPass->index = i;
        sprintf(ptPass->name, "P%d", i);
        ptPass->budget = ptDataPass->budget;
        ptPass->startNode = ptDataPass->startNode;
        ptPass->endNode = ptDataPass->destinationNode;
    }
}

void initAnts()
{
    uint i, j;
    ant* ptAnt;
    bool* ptBool;
    pathNodePass *ptPathNodePass;

    ants = new ant[parData.nAnts];
    for(j=0, ptAnt=ants; j<parData.nAnts; j++, ptAnt++)
    {
        ptAnt->nodes = new antNode[prData.dim];
        ptAnt->optNodes = new antNode[prData.dim];
        ptAnt->bestOptNodes = new antNode[prData.dim];

        ptAnt->nodeCounter = 0;
        ptAnt->nodesVisited = new bool[prData.dim];
        for(i=0, ptBool=ptAnt->nodesVisited; i<prData.dim; i++, ptBool++)
            *ptBool = false;

        ptAnt->carsRented = new bool[prData.nCars];
        for(i=0, ptBool=ptAnt->carsRented; i<prData.nCars; i++, ptBool++)
            *ptBool = false;

        ptAnt->usedNeighbours = 0;

        if(prData.maxNodePassengers > 0)
        {
            ptAnt->havePassengers = true; // we have passengers
            ptAnt->passPicked = new bool[prData.nPass]; //temporary array for passengers' algorithm
            for(i=0, ptBool=ptAnt->passPicked; i<prData.nPass; i++, ptBool++)
                *ptBool = false;

            ptAnt->tourPassengers = new pathNodePass[prData.dim];
            for(i=0, ptPathNodePass=ptAnt->tourPassengers; 
                i<prData.dim; i++, ptPathNodePass++)
            {
                ptPathNodePass->passengers = new pass*[prData.maxCarPassengers];
                ptPathNodePass->budgets = new float[prData.maxCarPassengers];
                ptPathNodePass->nPassengers = 0;
            }
            ptAnt->passPheromones = new float[prData.nPass];
        }
        else
        {
            ptAnt->havePassengers = false; // we have not passengers at all
            ptAnt->passPicked = nullptr;
            ptAnt->tourPassengers = nullptr;
            ptAnt->passPheromones = nullptr;
        }
    }
}

/* resursive function - car selection numbers, not used now
*/
/*
int divideCarChoices(int c, int nodeDim, int depth)
{
    if(depth >= nodeDim || c < 2)
        return 1;
    return divideCarChoices(c,nodeDim,depth+1)+(c-1)*divideCarChoices(c-1,nodeDim,depth+1);
}
*/

void initProbArrays()
{
    uint max;
    max = prData.dim;
    if(prData.nCars > max)
        max = prData.nCars;
    if(prData.nPass > max)
        max = prData.nPass;
    
    probArrays.n = 0;
    probArrays.indices = new uint[max];
    probArrays.probs = new float[max];
    probArrays.cumulatives = new float[max];
}

void initBestPath()
{
    uint i;
    pathNodePass *ptPathNodePass;
    bPath.nodeCounter = 0;
    bPath.nodes = new antNode[prData.dim];
    bPath.haveOpt = false;
    bPath.optNodes = new antNode[prData.dim];
    
    if(prData.nPass > 0 && prData.maxNodePassengers > 0)
    {
        bPath.tourPassengers = new pathNodePass[prData.dim];
        for(i=0, ptPathNodePass=bPath.tourPassengers; i<prData.dim; i++, ptPathNodePass++)
        {
            ptPathNodePass->passengers = new pass*[prData.maxCarPassengers];
            ptPathNodePass->budgets = new float[prData.maxCarPassengers];
        }
        bPath.pickedPassengers = new bool[prData.nPass];
        bPath.passPheromones = new float[prData.nPass];
    }
    else
    {
        bPath.tourPassengers = nullptr;
        bPath.pickedPassengers = nullptr;
        bPath.passPheromones = nullptr;
    }

    bPath.price = numeric_limits<float>::max();
    bPath.optPrice = numeric_limits<float>::max();
    bPath.passPrice = numeric_limits<float>::max();

    bPath.pheroResetPrice = numeric_limits<float>::max();
    bPath.pheroResetOptPrice = numeric_limits<float>::max();
    bPath.pheroResetPassPrice = numeric_limits<float>::max();
    

}

/* init of second stage - all encapsulated into passenger manager */
void initPassManager()
{
    uint i, j, k;
    float max1, max2;
    passAnt *ptPassAnt;
    bool **pptBool;
    if(prData.nPass == 0)
        return;
    pManager = new passManager;
    /* transition passengers - passengers in a car */
    pManager->transPassNumber = new uchar[prData.dim];
    pManager->transPassengers = new bool*[prData.dim];
    /* 2D array of passenger pointers - transitions -> array of passenger pointers in each transition */
    for(i=0, pptBool=pManager->transPassengers; i<prData.dim; i++, pptBool++)
        *pptBool = new bool[prData.nPass];
    pManager->pickedPassengers = new bool[prData.nPass];
    /* array of thrownOut passengers at the begining - their pointers */
    pManager->initThrownOutPassengers = new pass*[prData.nPass];

    /* 2nd stage ants and best 2nd stage ant */
    pManager->ants = new passAnt[parData.nPassAnts];
    for(i=0, ptPassAnt=pManager->ants; i<parData.nPassAnts; i++, ptPassAnt++)  
        ptPassAnt->thrownOutPassengers = new pass*[prData.nPass];
    pManager->bestAnt.thrownOutPassengers = new pass*[prData.nPass];
    
    /* pheromones */
    pManager->pheromones = new float[prData.nPass];
    pManager->budgets = new float[prData.nPass];
    /* upper bound */
    for(k=0, max1=0; k<prData.nCars;k++)
        for(j=0; j<prData.dim; j++)
            for(i=0; i<prData.dim; i++)
                if(max1 < prData.edgeWeightMatrices[k][j][i])
                    max1 = prData.edgeWeightMatrices[k][j][i];
    
    for(k=0, max2=0; k<prData.nCars;k++)
        for(j=0; j<prData.dim; j++)
            for(i=0; i<prData.dim; i++)
                if(max2 < prData.returnRateMatrices[k][j][i])
                    max2 = prData.returnRateMatrices[k][j][i];
    
    pManager->upperBound = prData.dim*max1+prData.nCars*max2;
    
}

void deletePassManager()
{
    uint i, j;
    passAnt *ptPassAnt;
    bool **pptBool;
    if(pManager == nullptr)
        return;

    delete[] pManager->budgets;
    delete[] pManager->pheromones;
    delete[] pManager->bestAnt.thrownOutPassengers;
    
    for(j=0, ptPassAnt=pManager->ants; j<parData.nPassAnts; j++, ptPassAnt++)
        delete[] ptPassAnt->thrownOutPassengers;
    delete[] pManager->ants;

    delete[] pManager->initThrownOutPassengers;
    delete[] pManager->pickedPassengers;    
    for(i=0, pptBool=pManager->transPassengers; i<prData.dim; i++, pptBool++)
        delete[] *pptBool;
    delete[] pManager->transPassengers;
    delete[] pManager->transPassNumber;
    delete pManager;
}

