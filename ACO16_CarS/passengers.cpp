#include "passengers.h"

using namespace std;

/* test */
uint testNodes[][3] = {{0,0,4},{0,4,5},{4,5,2},{5,2,3},{2,3,1},{3,1,0}}; 
uint testCars[][2] = {{1,1},{1,1},{1,1},{1,1},{1,0},{0,0}};

passManager* pManager = nullptr;

/*
nodes - ant nodes, non opt or opt
node counter - number of and nodes
passArray - array of passengers onboard
passPicked - temporary array of picked flags, no need to limit it by counter
*/
float calculatePassengers(ant *currentAnt, bool useOpt)
{
    uint i, nIt, nAnt;
    bool valid;
    float *ptFloat1;
    pass **pptPass1, **pptPass2, *ptPass;
    passAnt *ptAnt;
    if(currentAnt == nullptr)
        return numeric_limits<float>::max();
    pManager->nodeCounter = currentAnt->nodeCounter;
    if(!useOpt) //no opt nodes
    {
        pManager->nodes = currentAnt->nodes;
        pManager->passWithOpt = false;
    }
    else
    {
        pManager->nodes = currentAnt->optNodes;
        pManager->passWithOpt = true;
    }
    
    /* test if needed, set outer iterations and ants to 1 */
    //makeTestAnt(currentAnt);

    /* starting algorithm */
    resetPassPheromones(); //pheromones's reset must be outside of solution loop
    pManager->bestPrice = numeric_limits<float>::max();

    for(nIt = 0; nIt < parData.nPassIters; nIt++)
    {
        pManager->bestAnt.price = numeric_limits<float>::max(); //best ant for this iteration (copy)
        for(nAnt=0, ptAnt=pManager->ants; nAnt<parData.nPassAnts; nAnt++, ptAnt++)
        {
            /* update temporary budgets in pManager */
            for(i=0, ptPass=passengers, ptFloat1=pManager->budgets; i<prData.nPass; i++, ptPass++, ptFloat1++)
                *ptFloat1 = ptPass->budget;

            checkPassengers(); //check if passengers travel forward and have enough budget (optimistic price share)
            ptAnt->thrownOutPassNumber = 0;
            while(1)
            {
                valid = passNodeTraversal();
                if(valid)
                    break;
                thrownOutPassenger();
                if(probArrays.selected >= 0)
                    ptAnt->thrownOutPassengers[(ptAnt->thrownOutPassNumber)++]=&passengers[probArrays.indices[probArrays.selected]];
            }
            ptAnt->price = calculateTourPrice();
            if(pManager->bestAnt.price > ptAnt->price)
            {
                pManager->bestAnt.price = ptAnt->price;
                pManager->bestAnt.nIt = nIt;
                pManager->bestAnt.thrownOutPassNumber = ptAnt->thrownOutPassNumber;
                for(i=0, pptPass1=ptAnt->thrownOutPassengers, pptPass2=pManager->bestAnt.thrownOutPassengers; 
                    i<ptAnt->thrownOutPassNumber; 
                    i++, pptPass1++, pptPass2++)
                    *pptPass2 = *pptPass1; //copy passengers pointers to best ant
            }
        } //end of ants
        updatePheromones(); //best ant updates pheromones in pManager using his price
        if(pManager->bestPrice > pManager->bestAnt.price)
        {
            pManager->bestPrice = pManager->bestAnt.price; //if best price, best ant updates pManager best price
            calculatePassMaxMin(); //uses best price
            limitPassPheromones();
            updateUpperAnt(currentAnt);
        }
        else
            limitPassPheromones();

    }
    return pManager->bestPrice; //we provide pass price passed as return value
}

void checkPassengers()
{
    uint i, j, k;
    int start, end;
    bool *ptBool;
    car *ptCar;
    pass *ptPass, **pptPass;
    antNode *ptAntNode;
    float budget, share;
    pManager->initThrownOutPassNumber = 0; //initially thrown out passengers
    for(j=0, k=0, ptBool=pManager->pickedPassengers, pptPass=pManager->initThrownOutPassengers, ptPass=passengers; 
        j<prData.nPass; j++, ptBool++, ptPass++)
    {
        *ptBool = true;
        /* check passenger's direction */
        for(i=0, start=end=0, ptAntNode=pManager->nodes; i<pManager->nodeCounter; i++, ptAntNode++)
        {
            if(ptPass->startNode == ptAntNode->curNode->index)
                start = i; //start node (tour index)
            if(ptPass->endNode == ptAntNode->curNode->index)
                end = i; //end node (tour index)
        }
        if((end-start)<0 && end !=0) //can't reach destination on this tour
        {
            *ptBool = false; //update thrown out pass flag array
            *(pptPass++) = ptPass; //update initially thrown pass array
            k++; //update counter
        }
        /* check optimistic price (full car)*/
        if(*ptBool)
        {
            if(end > 0) //not circular
                ptPass->radius = end-start;
            else
                ptPass->radius = prData.dim-start;
            budget = pManager->budgets[ptPass->index]; //temp variable, we don't decrease passengers' budgets (just test it)
            for(i=start, ptAntNode=pManager->nodes+start; i<(end==0?pManager->nodeCounter:end); i++, ptAntNode++)
            {
                ptCar = ptAntNode->carOut;
                share = prData.edgeWeightMatrices[ptCar->index][ptAntNode->curNode->index][ptAntNode->nextNode->index]/
                (ptCar->carPassLimit+1.0);
                budget -= share;
            }
            if(budget < 0.0)
            {
                *ptBool = false;
                *(pptPass++) = ptPass;
                k++;
            }
        }
    }
    pManager->initThrownOutPassNumber=k; //update init pass throw
}

bool passNodeTraversal()
{
    uint i, j;
    bool valid;
    int deltaCap; //can be negative
    uchar *ptUchar;
    float *ptFloat, share;
    pass *ptPass, **pptPass;
    antNode *ptAntNode;
    bool *ptBool1, *ptBool2, **pptBool;
    /* reset transitions */
    for(j=0, ptUchar=pManager->transPassNumber, pptBool=pManager->transPassengers; 
        j<pManager->nodeCounter; j++, ptUchar++,pptBool++)
    {
        *ptUchar = 0; //number of transition passengers
        for(i=0, ptBool1=*pptBool; i<prData.nPass; i++, ptBool1++)
            *ptBool1 = false; //reset transition pass flags - no passengers in transitions (cars)
    }
    /* reset temporary passengers' budget stored in pManager budgets array */
    for(j=0, ptFloat=pManager->budgets, ptPass=passengers; j<prData.nPass; j++, ptFloat++, ptPass++)
        *ptFloat = ptPass->budget;
    pManager->penalisation = 0; //reset penalisation
    valid = true; //valid is true for now
    /* run through nodes */
    for(j=0, ptUchar=pManager->transPassNumber, pptBool=pManager->transPassengers, ptAntNode=pManager->nodes; 
        j<pManager->nodeCounter; j++, ptUchar++, pptBool++, ptAntNode++)
    {
        if(j>0) //if not first node
        {
            /* copying last transition condition to current */
            *ptUchar=*(ptUchar-1); //number of passengers, we need it in price share calc
            for(i=0, ptBool2=*pptBool, ptBool1=*(pptBool-1); i<prData.nPass; i++, ptBool1++, ptBool2++)
                *ptBool2 = *ptBool1; //passengers (their flags)
            /* passengers leaving car */
            for(i=0, ptBool1=*pptBool, ptPass=passengers; i<prData.nPass; i++, ptBool1++, ptPass++)
                if(*ptBool1 && ptPass->endNode==ptAntNode->curNode->index)
                {
                    *ptUchar=(*ptUchar)-1; //passengers' counter
                    *ptBool1 = false; //passenger's flag in transition flag array
                }
        }
        /* pick up node passengers (if their flag is set in selection array) */
        for(i=0, pptPass=ptAntNode->curNode->passengers; i<ptAntNode->curNode->nPassengers; i++, pptPass++)
        {
            if(pManager->pickedPassengers[(*pptPass)->index])
            {
                *ptUchar=(*ptUchar)+1;
                (*pptBool)[(*pptPass)->index]=true;
            }
        }
        deltaCap = ptAntNode->carOut->carPassLimit-(*ptUchar); //car capacity deviation (if exists)
        share = prData.edgeWeightMatrices[ptAntNode->carOut->index][ptAntNode->curNode->index][ptAntNode->nextNode->index]/(1.0+(*ptUchar));
        if(deltaCap >= 0) //if capacity is O.K. then no deviation
            deltaCap = 0;
        else
            valid = false; //if deviation, the solution is not valid
        /* decrease pass budgets */
        for(i=0, ptBool1=*pptBool, ptFloat=pManager->budgets; i<prData.nPass; i++, ptBool1++, ptFloat++)
            if(*ptBool1)
                (*ptFloat) -= share; //decrease transitions passengers' budget
        pManager->penalisation -= (deltaCap * share);
    }
    /* update penalisation */
    for(i=0, ptBool1=pManager->pickedPassengers, ptFloat=pManager->budgets; i<prData.nPass; i++, ptBool1++, ptFloat++)
        if(*ptBool1 && *ptFloat<0.0)
        {
            pManager->penalisation -= *ptFloat;
            valid = false;
        }
    return valid;
}


void resetPassPheromones()
{
    uint i;
    float *ptFloat;
    for(i=0, ptFloat=pManager->pheromones; i<prData.nPass; i++, ptFloat++)
        *ptFloat = numeric_limits<float>::max(); //all pheromones to max, we use max-min
}


void updatePheromones()
{
    int i;
    float *ptFloat, pheroUpdate;
    pass **pptPass;
    /* evaporation */
    for(i=0, ptFloat=pManager->pheromones; i<prData.nPass; i++, ptFloat++)
        (*ptFloat) *= (1.0-parData.rho3);
    /* updating, best ant updates pheromones (elite ant)*/
    pheroUpdate = pManager->bestAnt.price/pManager->upperBound;
    for(i=0, pptPass=pManager->bestAnt.thrownOutPassengers; i<pManager->bestAnt.thrownOutPassNumber; i++, pptPass++)
        pManager->pheromones[(*pptPass)->index] += pheroUpdate;  
}

void calculatePassMaxMin()
{
    float c, k, avg, pbest, pdec;
    c = 1.0/pManager->bestPrice;
    pManager->max = 1.0/(parData.rho3)*c;
    k = prData.nPass-pManager->initThrownOutPassNumber;
    avg = k/2.0;
    pbest = pow(2.0,-k); // 1/2^k (binary)
    pdec = pow(pow(pbest,parData.alpha3),1.0/k);
    if(avg>1.0)
        pManager->min = pManager->max*(1.0-pdec)/((avg-1.0)*pdec);
    else
        pManager->min = 0.0;
    if(pManager->min > pManager->max)
        pManager->min = pManager->max;
}

void limitPassPheromones()
{
    int i;
    float *ptFloat, max, min;
    max = pManager->max;
    min = pManager->min;
    for(i=0, ptFloat=pManager->pheromones; i<prData.nPass; i++, ptFloat++)
    {
        if(*ptFloat < min)
            *ptFloat = min;
        if(*ptFloat > max)
            *ptFloat = max;
    }
}

void thrownOutPassenger()
{
    uint i, n, *ptUint;
    bool *ptBool;
    float *ptFloat1, *ptFloat2;
    pass *ptPass;
    float sum, tau, eta, p, *ptFloat;
    for(i=0, n=0, sum=0.0, ptBool=pManager->pickedPassengers, ptFloat=pManager->pheromones, ptPass=passengers,
        ptUint=probArrays.indices, ptFloat1=probArrays.probs, ptFloat2=probArrays.cumulatives; 
        i<prData.nPass; i++, ptBool++, ptFloat++, ptPass++)
        if(*ptBool)
        {
            *(ptUint++) = i;
            tau = *ptFloat;
            eta = ptPass->radius/(float)(ptPass->budget);
            p = pow(tau,parData.alpha3)*pow(eta,parData.beta3);
            *(ptFloat1++) = p;
            *(ptFloat2++) = sum;
            sum+=p;
            n++;
        }
    probArrays.n = n;
    probArrays.sum = sum;
    if(n == 0)
        probArrays.selected = -1;
    else if(n == 1)
    {
        probArrays.selected = 0;
        pManager->pickedPassengers[probArrays.indices[0]]=false;
    }
    else
    {
        i = selectFromFreqArray(sum, n, probArrays.cumulatives);
        probArrays.selected = i;
        pManager->pickedPassengers[probArrays.indices[i]]=false;
    }
}

/* tour with passengers calculation 
     uses global pManager structure
*/
float calculateTourPrice() 
{
    uint i;
    float tourPrice = 0.0;
    uchar *ptUchar;
    antNode *ptAntNode, *rentNode;

    rentNode = pManager->nodes; //first node in array of nodes
    for(i=0, ptAntNode=pManager->nodes, ptUchar=pManager->transPassNumber; i<pManager->nodeCounter; i++, ptAntNode++, ptUchar++)
    {
        /* changing car */
        if(i>0 && ptAntNode->carOut->index != ptAntNode->carIn->index)
        {
            tourPrice += prData.returnRateMatrices[ptAntNode->carIn->index][ptAntNode->curNode->index][rentNode->curNode->index];
            rentNode = ptAntNode;
        }
        tourPrice += (float)(prData.edgeWeightMatrices[ptAntNode->carOut->index][ptAntNode->curNode->index][ptAntNode->nextNode->index])/
        (1.0+*ptUchar);  
    }
    tourPrice += prData.returnRateMatrices[(ptAntNode-1)->carOut->index][pManager->nodes->curNode->index][rentNode->curNode->index];
    return tourPrice;
}

/* update first stage ant */
void updateUpperAnt(ant *currentAnt)
{   
    uint i, j;
    uchar *ptUchar;
    antNode *ptAntNode;
    pathNodePass *ptPathNodePass;
    bool *ptBool1, *ptBool2, **pptBool;
    float *ptFloat1, *ptFloat2, share;
    pass *ptPass, **pptPass;
    /* reset pass picked flag array and update pheromones */

    for(i=0, ptBool1=currentAnt->passPicked, ptFloat1=pManager->pheromones, ptFloat2=currentAnt->passPheromones; 
        i<prData.nPass; i++, ptBool1++, ptFloat1++, ptFloat2++)
    {
        *ptBool1 = true;
        *ptFloat2 = *ptFloat1;
    }

    /* set pass picked flag for picked passengers */
    for(i=0, pptPass=pManager->initThrownOutPassengers; i<pManager->initThrownOutPassNumber; i++, pptPass++)
        currentAnt->passPicked[(*pptPass)->index]=false;
    for(i=0, pptPass=pManager->bestAnt.thrownOutPassengers; i<pManager->bestAnt.thrownOutPassNumber; i++, pptPass++)
        currentAnt->passPicked[(*pptPass)->index]=false;

    /* update upper ant node passengers */
    /* reset transitions */
    for(j=0, ptUchar=pManager->transPassNumber, pptBool=pManager->transPassengers; 
        j<pManager->nodeCounter; j++, ptUchar++,pptBool++)
    {
        *ptUchar = 0; //number of transition passengers
        for(i=0, ptBool1=*pptBool; i<prData.nPass; i++, ptBool1++)
            *ptBool1 = false; //reset transition pass flags - no passengers in transitions (cars)
    }
    /* node traversal again */
    /* reset budgets array */
    for(j=0, ptFloat1=pManager->budgets, ptPass=passengers; j<prData.nPass; j++, ptFloat1++, ptPass++)
        *ptFloat1 = ptPass->budget;
    /* node traversal */
    for(j=0, ptUchar=pManager->transPassNumber, pptBool=pManager->transPassengers, 
        ptPathNodePass=currentAnt->tourPassengers, ptAntNode=pManager->nodes; 
        j<pManager->nodeCounter; j++, ptUchar++, pptBool++, ptPathNodePass++, ptAntNode++)
    {
        if(j>0) //if not first node
        {
            /* copying last transition condition to current */
            *ptUchar=*(ptUchar-1); //number of passengers, we need it in price share calc
            for(i=0, ptBool2=*pptBool, ptBool1=*(pptBool-1); i<prData.nPass; i++, ptBool1++, ptBool2++)
                *ptBool2 = *ptBool1; //passengers (their pointers)
            /* passengers leaving car */
            for(i=0, ptBool1=*pptBool, ptPass=passengers; i<prData.nPass; i++, ptBool1++, ptPass++)
                if(*ptBool1 && ptPass->endNode==ptAntNode->curNode->index)
                {
                    *ptUchar=(*ptUchar)-1; //passengers' counter
                    *ptBool1 = false; //passenger's flag in transition flag array
                }
        }
        /* pick up node passengers (if their flag is set in selection array) */
        for(i=0, ptBool1=*pptBool, pptPass=ptAntNode->curNode->passengers; 
            i<ptAntNode->curNode->nPassengers; i++, pptPass++, ptBool1++)
        {
            if(currentAnt->passPicked[(*pptPass)->index])
            {
                *ptUchar=(*ptUchar)+1;
                (*pptBool)[(*pptPass)->index]=true;
            }
        }
        share = prData.edgeWeightMatrices[ptAntNode->carOut->index][ptAntNode->curNode->index][ptAntNode->nextNode->index]/(1.0+(*ptUchar));
        /* decrease pass budgets */
        for(i=0, ptBool1=*pptBool, ptFloat1=pManager->budgets; i<prData.nPass; i++, ptBool1++, ptFloat1++)
        {
            if(*ptBool1)
                (*ptFloat1) -= share; //decrease transitions passengers' budget
        }

        ptPathNodePass->nPassengers = *ptUchar;
        for(i=0, pptPass=ptPathNodePass->passengers, ptPass=passengers, ptBool1=*pptBool,
            ptFloat1=pManager->budgets, ptFloat2=ptPathNodePass->budgets; 
            i<prData.nPass; i++, ptPass++, ptBool1++, ptFloat1++)
                if(*ptBool1)
                {
                    *(pptPass++) = ptPass;
                    *(ptFloat2++) = *ptFloat1;
                }
    } //end of node traversal
}


/* test harness - chile6e */
void makeTestAnt(ant *currentAnt)
{
    int i;
    antNode *ptAntNode;
    if(currentAnt->nodeCounter < 6 || prData.nCars < 3)
        return;
    pManager->nodeCounter = 6;
    for(i=0, ptAntNode=pManager->nodes; i<6; i++, ptAntNode++)
    {
        ptAntNode->prevNode = &nodes[testNodes[i][0]];
        ptAntNode->curNode = &nodes[testNodes[i][1]];
        ptAntNode->nextNode = &nodes[testNodes[i][2]];
        ptAntNode->carIn = &cars[testCars[i][0]];
        ptAntNode->carOut = &cars[testCars[i][1]];
    }
    pManager->nodes->prevNode = nullptr;
    pManager->nodes->carIn = nullptr;
    
}

