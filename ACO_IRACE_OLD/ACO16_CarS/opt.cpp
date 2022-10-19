#include "ACO.h"
#include "passengers.h"

using namespace std;

void createOptPath(uint n0, uint n1, ant *ptAnt, short variant);
void updateAntOptPass(ant *currentAnt, float currentOptPrice, float currentPassPrice);

void opt2_5()
{
    uint i, j, k, v;
    ant *ptAnt;
    float optPrice, passPrice;
    antNode *ptAntNode1, *ptAntNode2;

    for(k=0, ptAnt=ants; k<parData.nAnts; k++, ptAnt++)
    {
        ptAnt->haveOpt = false;
        ptAnt->havePassengers = false;
        ptAnt->bestOptPrice = numeric_limits<float>::max();
        /* not completed tour, jump to next ant */
        if(ptAnt->nodeCounter < prData.dim || ptAnt->closedPath == false)
            continue;
        // no passengers, either not in data, or not in switch
        if(prData.nPass == 0 || parData.pass == false)
        {
            ptAnt->havePassengers = false;
            //no opt, no pass, curent ant remains as it was, have opt and have pass flags are false by default
            if(prData.dim <= 5 || parData.opt == false)
            {
                continue;
            }
            else //we have opt 
            {
                ptAnt->haveOpt = true;
                /* first, we try without swap */
                for(i=0, ptAntNode1=ptAnt->nodes, ptAntNode2=ptAnt->optNodes; 
                    i<ptAnt->nodeCounter; i++, ptAntNode1++, ptAntNode2++)
                        memcpy(ptAntNode2, ptAntNode1, sizeof(antNode));
                optPrice = calculatePathCost(ptAnt->optNodes, ptAnt->nodeCounter);
                if(optPrice < ptAnt->bestOptPrice) //put non-opt as the first opt
                {
                    updateAntOptPass(ptAnt, optPrice, numeric_limits<float>::max());
                }
                /* than swap nodes to opt */
                for(j=0; j<ptAnt->nodeCounter-3; j++)
                {
                    for(i=j+3; i<(j==0?ptAnt->nodeCounter-1:ptAnt->nodeCounter); i++)
                    {
                        for(v=0; v<2; v++) //both opt variants
                        {
                            createOptPath(j,i, ptAnt, v);
                            optPrice = calculatePathCost(ptAnt->optNodes, ptAnt->nodeCounter);
                            if(optPrice < ptAnt->bestOptPrice)
                            {
                                updateAntOptPass(ptAnt, optPrice, numeric_limits<float>::max());
                            }
                        }
                    }
                }
            }
        }
        else // we have pass, both data and switch
        {
            ptAnt->havePassengers = true;
            if(prData.dim <= 5 || parData.opt == false) // condition for opt, if no opt
            {
                passPrice = calculatePassengers(ptAnt, false); //basic (no opt) calculation
                if(passPrice < ptAnt->passPrice)
                {
                    for(i=0, ptAntNode1=ptAnt->nodes, ptAntNode2=ptAnt->bestOptNodes; 
                        i<ptAnt->nodeCounter; i++, ptAntNode1++, ptAntNode2++)
                            memcpy(ptAntNode2, ptAntNode1, sizeof(antNode)); // memcpy, we copy pointers to external structures, so ptAntNode can't have internal array!!!
                    updateAntOptPass(ptAnt, numeric_limits<float>::max(), passPrice);
                }
            }
            else //we have opt
            {
                ptAnt->haveOpt = true;
                /* first we try no swap */
                for(i=0, ptAntNode1=ptAnt->nodes, ptAntNode2=ptAnt->optNodes; 
                    i<ptAnt->nodeCounter; i++, ptAntNode1++, ptAntNode2++)
                    memcpy(ptAntNode2, ptAntNode1, sizeof(antNode));
                optPrice = calculatePathCost(ptAnt->optNodes, ptAnt->nodeCounter);
                passPrice = calculatePassengers(ptAnt, true); //use opt

                //optPrice = ptAnt->priceNoPassengers;
                //passPrice = calculatePassengers(ptAnt, false); //use non-opt
                if(passPrice < ptAnt->passPrice)
                {
                    updateAntOptPass(ptAnt, optPrice, passPrice);
                }
                for(j=0; j<ptAnt->nodeCounter-3; j++) //now go through all opts
                {
                    for(i=j+3; i<(j==0?ptAnt->nodeCounter-1:ptAnt->nodeCounter); i++)
                    {
                        for(v=0; v<2; v++)
                        {
                            createOptPath(j,i, ptAnt, v);
                            optPrice = calculatePathCost(ptAnt->optNodes, ptAnt->nodeCounter);
                            passPrice = calculatePassengers(ptAnt, true); //use opt for passengers
                            if(passPrice < ptAnt->passPrice)
                            {
                                updateAntOptPass(ptAnt, optPrice, passPrice);
                            }
                        }
                    }
                }
            }
        }
    }
}

void createOptPath(uint n0, uint n1, ant *ptAnt, short variant)
{
    uint i;
    antNode *ptAntNode1, *ptAntNode2;
    /* nodes bypass */
    for(i=0, ptAntNode1=ptAnt->nodes, ptAntNode2=ptAnt->optNodes; i<=n0; i++, ptAntNode1++, ptAntNode2++)
        memcpy(ptAntNode2, ptAntNode1, sizeof(antNode));
    ptAntNode1++;
    i++;
    for(;i<=n1; i++, ptAntNode1++, ptAntNode2++)
        memcpy(ptAntNode2, ptAntNode1, sizeof(antNode));
    memcpy(ptAntNode2++, (ptAnt->nodes)+n0+1, sizeof(antNode));
    for(;i<ptAnt->nodeCounter; i++, ptAntNode1++, ptAntNode2++)
        memcpy(ptAntNode2, ptAntNode1, sizeof(antNode));
    /* update links */
    for(i=0, ptAntNode1=ptAnt->optNodes, ptAntNode2=ptAnt->optNodes+1; i<ptAnt->nodeCounter-1; i++, ptAntNode1++, ptAntNode2++)
    {
        ptAntNode1->nextNode = ptAntNode2->curNode;
        ptAntNode2->prevNode = ptAntNode1->curNode;
    }
    ptAntNode1->nextNode = ptAnt->optNodes->curNode;
    /* cars bypass */
    ptAnt->optNodes[n0+1].carIn = ptAnt->optNodes[n0].carOut;
    /* shifted one left, n1 becomes n1-1, n1+1 becomes n1 */
    if(variant==0)
    {
        ptAnt->optNodes[n1].carIn=ptAnt->optNodes[n1].carOut=ptAnt->optNodes[n1-1].carOut; 
        if(n1+1<(ptAnt->nodeCounter))
            ptAnt->optNodes[n1+1].carIn=ptAnt->optNodes[n1].carOut;
    }
    else
    {
        ptAnt->optNodes[n1].carIn=ptAnt->optNodes[n1-1].carOut;
        if(n1+1<(ptAnt->nodeCounter))
            ptAnt->optNodes[n1].carOut=ptAnt->optNodes[n1+1].carIn;
        else
            ptAnt->optNodes[n1].carOut=ptAnt->nodes[ptAnt->nodeCounter-1].carOut;
    }
}

void updateAntOptPass(ant *currentAnt, float currentOptPrice, float currentPassPrice)
{
    uint i;
    antNode *ptAntNode1, *ptAntNode2;
    /* copy opt nodes */
    /* copying opt nodes to best opt nodes */
    if(currentOptPrice < numeric_limits<float>::max()) //if we don't have opt it doesn't care
    {
        for(i=0, ptAntNode1=currentAnt->optNodes, ptAntNode2=currentAnt->bestOptNodes; 
            i<currentAnt->nodeCounter; i++, ptAntNode1++, ptAntNode2++)
            memcpy(ptAntNode2, ptAntNode1, sizeof(antNode)); // memcpy, we copy pointers to external structures, so ptAntNode can't have internal array!!!
    }
 
    /* copy new prices to ant */
    currentAnt->bestOptPrice = currentOptPrice; 
    currentAnt->passPrice = currentPassPrice; 
}

