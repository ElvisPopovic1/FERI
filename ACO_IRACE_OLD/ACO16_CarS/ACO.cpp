#include "ACO.h"

using namespace std;


pass* passengers = nullptr;
node* nodes = nullptr;
car* cars = nullptr;
ant* ants = nullptr;
float pheroMin1, pheroMin2, pheroMax1, pheroMax2;
probabilityArrays probArrays;
bestPath bPath;

void resetAnts();
bool nodeTraversal(node *startNode, ant *currentAnt);

bool Solution(uint iter, node *startNode)
{
    uint j;
    ant *ptAnt;
    resetAnts();
    /* ant loop */
    for(j = 0, ptAnt = ants; j < parData.nAnts; j++, ptAnt++)
    {
        nodeTraversal(startNode, ptAnt); 
    }   

    /* overwrite first ant with predefined */
    return true;
}

bool nodeTraversal(node *startNode, ant *currentAnt)
{
    int pickedNode, pickedCar;
    float value;
    node *currentNode;
    antNode *currentAntNode;
    car *currentCar;

    /* start node is visited, and counted as passed */
    currentNode = startNode;
    currentAntNode = currentAnt->nodes+currentAnt->nodeCounter;

    currentAntNode->prevNode = nullptr;
    currentAntNode->curNode = currentNode;   
    currentAnt->nodesVisited[currentNode->index] = true;

    currentAntNode->carIn = nullptr;
    pickedCar = PickCar(currentAnt, currentNode, nullptr); // not yet current car
    if(pickedCar >= 0)
        currentCar = cars+pickedCar;
    else return -1;
    currentAntNode->carOut = currentCar;
    currentAnt->carsRented[currentCar->index] = true;
    currentAnt->carPickedNode = currentNode;

    /* for min purpose */
    /* nodes */
    currentAntNode->choices1 = 1.0; //predefined, only one choice
    currentAntNode->prob1 = 1.0;  // predefined, so probability is 1
    /* cars */
    currentAntNode->choices2 = probArrays.n; //n cars
    currentAntNode->prob2 = probArrays.probs[probArrays.selected]/probArrays.sum; //prob for chosen car
    /* end of min purpose */

    currentAnt->priceNoPassengers = 0.0;
    currentAnt->nodeCounter++;
    currentAnt->usedNeighbours = 0;
    currentAntNode++;

    do /* node traversal loop starts here */
    {
        pickedNode = PickNode(currentAnt, currentNode, currentCar);
        if(pickedNode >= 0) //can pick at least last node
        {
            /* for max-min, nodes - picked node */
            currentAntNode->choices1 = probArrays.n1;
            currentAntNode->prob1 = probArrays.probs[probArrays.selected]/probArrays.sum1;

            currentAnt->priceNoPassengers += (float)prData.edgeWeightMatrices[currentCar->index][currentNode->index][nodes[pickedNode].index];
            if(probArrays.selected >= 0 && currentAnt->usedNeighbours < (uint)probArrays.selected)
                currentAnt->usedNeighbours = probArrays.selected; 
                        
            currentAntNode->prevNode = currentNode;
            currentNode = nodes+pickedNode;
            (currentAntNode-1)->nextNode = currentNode;
            currentAntNode->curNode = currentNode;
            currentAnt->nodesVisited[pickedNode] = true;      

            currentAnt->nodes[currentAnt->nodeCounter].carIn = currentCar;
            pickedCar = PickCar(currentAnt, currentNode, currentCar);
            currentAntNode->prob2 = 1.0;
            if(pickedCar >= 0)
            {
                /* for max-min, cars - picked cars */
                currentAntNode->choices2 = probArrays.n;
                currentAntNode->prob2 = probArrays.probs[probArrays.selected]/probArrays.sum;
                if( (cars+pickedCar)->index != currentAntNode->carIn->index)
                {
                    currentAnt->priceNoPassengers += prData.returnRateMatrices[currentCar->index][currentNode->index][currentAnt->carPickedNode->index];
                    currentAnt->carPickedNode = currentNode;
                }
                currentCar = cars+pickedCar;
                currentAnt->carsRented[currentCar->index] = true;
            }
 
            currentAntNode->carOut = currentCar;
            currentAnt->nodeCounter++; 
            currentAntNode++;        
        }
        else //no node can be picked so it may be start node (circular) 
        {
            /* check if we have path to last node in matrix */
            value = prData.edgeWeightMatrices[currentCar->index][currentNode->index][startNode->index];

            if(value != 0.0 && value < 9999) //can we connect last node and start node?
            {
                currentAnt->priceNoPassengers += value;
                currentNode = startNode;
                (currentAntNode-1)->nextNode = currentNode;
                currentAnt->priceNoPassengers += prData.returnRateMatrices[currentCar->index][currentNode->index][currentAnt->carPickedNode->index];
                currentCar = nullptr;
                currentAnt->closedPath = true;
            } /* no, so there is not tour */
            else 
            {
                (currentAntNode-1)->nextNode = nullptr;
                currentAnt->closedPath = false;
            }
        }
    } while (pickedNode >= 0);

    if(currentAnt->nodeCounter == prData.dim && currentAnt->closedPath == true)
        return true;
    return false;
}

void PheromoneEvaporation()
{
    uint i, j;
    node *ptNode;
    float *ptFloat;
    for(j=0, ptNode=nodes; j<prData.dim; j++, ptNode++)
    {
        // evaporate neighbours pheromones 
        for(i=0, ptFloat=ptNode->pheroNeighbours; i<prData.dim; i++, ptFloat++)
            if(i!=j)
                *ptFloat = (1.0-parData.rho1) * (*ptFloat);
        // evaporate cars' pheromones
        for(i=0, ptFloat=ptNode->pheroCars; i<prData.nCars; i++, ptFloat++)
            *ptFloat = (1.0-parData.rho2) * (*ptFloat);
    }
}

bool updatePheromones(ant *bestAnt)
{
    uint i;
    float pheroUpdate;
    antNode *ptAntNode;
    if(bestAnt->nodeCounter < prData.dim || bestAnt-> closedPath == false || bestAnt->priceNoPassengers == numeric_limits<float>::max())
        return false;

    /* we don't use stage 2 (passengers') price for stage 1 phero update */
    if(bestAnt->haveOpt == true) // no passengers but have opt, so use opt price
    {
        pheroUpdate = 1.0/bestAnt->bestOptPrice;
        ptAntNode = bestAnt->optNodes;
    }
    else // no opts, so use basic price
    {
        pheroUpdate = 1.0/bestAnt->priceNoPassengers;
        ptAntNode = bestAnt->nodes;
    }
    for(i=0; i<bestAnt->nodeCounter; i++, ptAntNode++)
    {
        ptAntNode->curNode->pheroNeighbours[ptAntNode->nextNode->index]+=pheroUpdate;
        ptAntNode->curNode->pheroCars[ptAntNode->carOut->index]+=pheroUpdate;
    }
    
    return true;
}

void limitPheromoneTraces()
{
    uint i, j;
    node *ptNode;
    float *ptFloat;


    for(j=0, ptNode=nodes; j<prData.dim; j++, ptNode++)
    {
        for(i=0, ptFloat=ptNode->pheroNeighbours; i<prData.dim; i++, ptFloat++)
            if(i!=j)
            {
                if(*ptFloat > pheroMax1)
                    *ptFloat = pheroMax1;
                if(*ptFloat < pheroMin1)
                    *ptFloat = pheroMin1;
            }
        for(i=0, ptFloat=ptNode->pheroCars; i<prData.nCars; i++, ptFloat++)
        {
            if(*ptFloat > pheroMax2)
                *ptFloat = pheroMax2;
            if(*ptFloat < pheroMin2)
                *ptFloat = pheroMin2;
        }
    }
}

bool updateBestPath(uint iteration, ant *bestAnt, chrono::duration<double> sol_time)
{
    uint i, j;
    pathNodePass *ptPathNodePass1, *ptPathNodePass2;
    pass **pptPass1, **pptPass2;
    bool *ptBool, updateBestPath;
    float *ptFloat1, *ptFloat2;
    antNode *ptAntNode1, *ptAntNode2;
    if(bestAnt == nullptr || bestAnt->nodeCounter < prData.dim || bestAnt->closedPath == false)
        return false; //no updates

    updateBestPath = false;
    if(bestAnt->havePassengers)
    {
        if(bestAnt->passPrice < bPath.passPrice)
            updateBestPath = true;
    }
    else if(bestAnt->haveOpt)
    {
        if(bestAnt->bestOptPrice < bPath.optPrice)
            updateBestPath = true;
    }
    else
    {
        if(bestAnt->priceNoPassengers < bPath.price)
            updateBestPath = true;
    }
    if(updateBestPath)
    {
        bPath.solution_time = sol_time;
        bPath.iteration = iteration;
        bPath.nodeCounter = bestAnt->nodeCounter;

        //depends on best solution and is notmaximal used 
        bPath.usedNeighbours = bestAnt->usedNeighbours;
        /* non-opt path */
        bPath.price = bestAnt->priceNoPassengers;
        for(i=0, ptAntNode1=bPath.nodes, ptAntNode2=bestAnt->nodes; i<bPath.nodeCounter; i++, ptAntNode1++, ptAntNode2++)
            memcpy(ptAntNode1, ptAntNode2, sizeof(antNode));
        /* opt path */
        bPath.haveOpt = bestAnt->haveOpt;
        bPath.optPrice = bestAnt->bestOptPrice;
        bPath.passWithOpt = bestAnt->passWithOpt;
        if(bestAnt->haveOpt)
            for(i=0, ptAntNode1=bPath.optNodes, ptAntNode2=bestAnt->bestOptNodes; 
                i<bPath.nodeCounter; i++, ptAntNode1++, ptAntNode2++)
                    memcpy(ptAntNode1, ptAntNode2, sizeof(antNode));
        /* passengers */
        bPath.havePass = bestAnt->havePassengers;
        bPath.passPrice = bestAnt->passPrice;
        for(i=0, ptBool = bPath.pickedPassengers; i<prData.nPass; i++, ptBool++)
            *ptBool = false;
        bPath.passPrice = bestAnt->passPrice;
        bPath.passWithOpt = bestAnt->passWithOpt;
        if(bestAnt->havePassengers)
        {
            for(j=0, ptPathNodePass1=bestAnt->tourPassengers, ptPathNodePass2=bPath.tourPassengers;
                j<bestAnt->nodeCounter; j++, ptPathNodePass1++, ptPathNodePass2++)
            {
                ptPathNodePass2->nPassengers = ptPathNodePass1->nPassengers;

                for(i=0, pptPass1=ptPathNodePass1->passengers, pptPass2=ptPathNodePass2->passengers,
                    ptFloat1=ptPathNodePass1->budgets, ptFloat2=ptPathNodePass2->budgets; 
                    i<ptPathNodePass1->nPassengers; i++, pptPass1++, pptPass2++, ptFloat1++, ptFloat2++)
                    {
                        (*pptPass2) = (*pptPass1);
                        (*ptFloat2) = (*ptFloat1);
                        bPath.pickedPassengers[(*pptPass1)->index]=true;
                    }
            }
            for(j=0, ptFloat1=bestAnt->passPheromones, ptFloat2=bPath.passPheromones; 
                j<prData.nPass; j++, ptFloat1++, ptFloat2++)
                    *ptFloat2 = *ptFloat1;

            for(j=0, ptBool=bPath.pickedPassengers, bPath.nPickedPassengers=0; j<prData.nPass; j++, ptBool++)
                if(*ptBool == true)
                {
                    bPath.nPickedPassengers++;
                }
        }
        else if(bPath.tourPassengers != nullptr) //no passengers, switch or data, if switch only, we can set tour to zero
            for(j=0, ptPathNodePass2=bPath.tourPassengers; j<bestAnt->nodeCounter; j++, ptPathNodePass2++)
                ptPathNodePass2->nPassengers = 0;


        bPath.pheroMin1 = pheroMin1;
        bPath.pheroMax1 = pheroMax1;

        bPath.pheroMin2 = pheroMin2;
        bPath.pheroMax2 = pheroMax2;
    }
    return updateBestPath;      
}

// for best path since pheromone reset
bool updateBestPathSincePheroReset(ant *bestAnt)
{
    bool updateBestPath;
    updateBestPath = false;
    if(bestAnt->havePassengers)
    {
        if(bestAnt->passPrice < bPath.pheroResetPassPrice)
            updateBestPath = true;
    }
    else if(bestAnt->haveOpt)
    {
        if(bestAnt->bestOptPrice < bPath.pheroResetOptPrice)
            updateBestPath = true;
    }
    else
    {
        if(bestAnt->priceNoPassengers < bPath.pheroResetPrice)
            updateBestPath = true;
    }
    if(updateBestPath)
    {
        bPath.pheroResetPrice = bestAnt->priceNoPassengers;
        bPath.pheroResetOptPrice = bestAnt->bestOptPrice;
        bPath.pheroResetPassPrice = bestAnt->passPrice;
    }
    return updateBestPath;
}


float calculatePathCost(antNode *nodes, uint nodeCounter)
{
    uint i;
    antNode *ptAntNode;
    node *carPickNode;
    float value, price; 
    if(nodeCounter == 0)
        return numeric_limits<float>::max();
    carPickNode=nodes->curNode;
    for(i=1, price = 0.0, ptAntNode=nodes+1; i<nodeCounter; i++, ptAntNode++)
    {
        price += prData.edgeWeightMatrices[ptAntNode->carIn->index][ptAntNode->prevNode->index][ptAntNode->curNode->index];
        if(ptAntNode->carOut->index != ptAntNode->carIn->index)
        {
            price += prData.returnRateMatrices[ptAntNode->carIn->index][ptAntNode->curNode->index][carPickNode->index];
            carPickNode = ptAntNode->curNode;
        }
    }
    value = prData.edgeWeightMatrices[(ptAntNode-1)->carOut->index][(ptAntNode-1)->curNode->index][nodes->curNode->index];
    price += value;
    if(value != 0.0 && value < 9999)
        price += prData.returnRateMatrices[(ptAntNode-1)->carOut->index][nodes->curNode->index][carPickNode->index];
    return price;
}


ant* findBestAnt()
{
    uint i;
    ant* ptAnt, *bestAnt=nullptr;
    float minPrice = numeric_limits<float>::max();
    for(i=0, ptAnt=ants; i<parData.nAnts; i++, ptAnt++)
    {
        if(ptAnt->nodeCounter == prData.dim && ptAnt->closedPath == true) //if ant finished path
        {
            if(prData.nPass > 0 && parData.pass == true) // we have passengers, so we compare passengers' prices
            {
                if(ptAnt->havePassengers && ptAnt->passPrice < minPrice)
                {
                    bestAnt = ptAnt;
                    minPrice = ptAnt->passPrice;
                }
            }
            else if(prData.dim > 5 && parData.opt == true) //conditions for opt
            {
                if(ptAnt->haveOpt && ptAnt->bestOptPrice < minPrice)
                {
                    bestAnt = ptAnt;
                    minPrice = ptAnt->bestOptPrice;
                }
            }
            else
            {
                if(ptAnt->priceNoPassengers < minPrice)
                {
                    bestAnt = ptAnt;
                    minPrice = ptAnt->priceNoPassengers;
                }
            }
        }
    } // end of ants
    return bestAnt;
}

void resetAnts()
{
    uint i, j;
    ant *ptAnt;
    bool *ptBool;
    pathNodePass *ptPathNodePass;
    for(j=0, ptAnt=ants; j<parData.nAnts; j++, ptAnt++)
    {
        ptAnt->priceNoPassengers = numeric_limits<float>::max();
        ptAnt->bestOptPrice = numeric_limits<float>::max();
        ptAnt->passPrice = numeric_limits<float>::max();;

        ptAnt->usedNeighbours = 0;

        ptAnt->carPickedNode = nullptr;

        ptAnt->nodeCounter = 0;
        ptAnt->haveOpt = false;
        ptAnt->havePassengers = false;
        ptAnt->passWithOpt = false;
        for(i=0, ptBool=ptAnt->nodesVisited; i<prData.dim; i++, ptBool++)
            *ptBool = false;
        for(i=0, ptBool=ptAnt->carsRented; i<prData.nCars; i++, ptBool++)
            *ptBool = false;
        if(prData.nPass>0)
            for(i=0, ptPathNodePass=ptAnt->tourPassengers; i<prData.dim; i++, ptPathNodePass++)
                ptPathNodePass->nPassengers = 0;
    }
}

/* check for stall */
bool checkSolutionsStall(ant *bestAnt)
{
    static uint stallCounter = 0;
    static float lastSolutionPrice = numeric_limits<float>::max();
    float solutionPrice;
    if(parData.stall == -1)
        return false;   // no stall avoidance
    if(bestAnt->havePassengers)
        solutionPrice = bestAnt->passPrice;
    else if(bestAnt->haveOpt)
        solutionPrice = bestAnt->bestOptPrice;
    else
        solutionPrice = bestAnt->priceNoPassengers;

    if(solutionPrice == lastSolutionPrice)
        stallCounter++;
    else
    {
        lastSolutionPrice = solutionPrice;
        stallCounter = 0;
    }
    if((int)stallCounter > parData.stall)
    {
        stallCounter = 0;
        return true;
    }
    else
        return false;
}

/* calling when iterations' solutions stall */
void resetPheromones()
{
    node    *ptNode;
    float   *ptFloat;
    uint i, j;
    pheroMax1 = pheroMax2 = PHERO_MAX;
    pheroMin1 = parData.minRatioNodes * pheroMax1;
    pheroMin2 = parData.minRatioCars * pheroMax2;
    for(j=0, ptNode = nodes; j<prData.dim; j++, ptNode++)
    {
        for(i=0, ptFloat=ptNode->pheroNeighbours; i<prData.dim; i++, ptFloat++)
        {
            if(i==j)
                *ptFloat = 0.0;
            else
                *ptFloat = pheroMax1;
        }
        for(i=0, ptFloat=ptNode->pheroCars; i<prData.nCars; i++, ptFloat++)
            *ptFloat = pheroMax2;
    }

    //reset best path since phero reset prices
    bPath.pheroResetPrice = numeric_limits<float>::max();
    bPath.pheroResetOptPrice = numeric_limits<float>::max();
    bPath.pheroResetPassPrice = numeric_limits<float>::max();
}



