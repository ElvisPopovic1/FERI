#include "ACO.h"

using namespace std;

mt19937* mersenneGenerator = nullptr;



/* pick from cumulative probability aaray */
uint selectFromFreqArray(float sum, uint n, float *probabilities)
{
    float l, r;
    uint left=0;
    uint right = n; //probabilities from 0 to n-1, right limit on n
    uint center =(left+right)>>1;
    l = nextafter(sum, numeric_limits<float>::max()); //upper closed interval limit
    uniform_real_distribution<float> distribution((float)0.0,l);
    r = distribution(*mersenneGenerator);
    while(left<center)
    {
        if(r >= *(probabilities+center))
            left = center;
        else
            right = center;
        center = (left+right)>>1;
    }
    return center;
}

uint PrepareNeighbours(ant *currentAnt, node *currentNode, car *currentCar, float &sum, uint favorits, uint start)
{
    uint i, nNeighbours;
    bool *ptBool;
    float eta, tau;
    float *ptFloat, *ptFloat2, *ptFloat3, *ptFloat4;
    uint *ptUint;
    float p=0.0;
    for(i=start, nNeighbours=0, 
        ptFloat4 = probArrays.probs,
        ptUint = probArrays.indices, 
        ptFloat = probArrays.cumulatives, sum=0.0;
        i<start+favorits && i<prData.dim;
        i++, ptFloat2++, ptFloat3++, ptBool++)
    {
        ptBool = currentAnt->nodesVisited+currentNode->neighbourSortIndice[currentCar->index][i];
        ptFloat2=&prData.edgeWeightMatrices[currentCar->index][currentNode->index][currentNode->neighbourSortIndice[currentCar->index][i]];

        if(*ptFloat2 != 0.0 && *ptFloat2 < 9999 && *ptBool == false) //can be diagonal but can be no link also
        {
            /*
            cout << nNeighbours << ":(" << currentNode->index << ";" << i << ";" << 
                currentNode->neighbourSortIndice[currentCar->index][i] << ";" << 
                prData.edgeWeightMatrices[currentCar->index][currentNode->index][currentNode->neighbourSortIndice[currentCar->index][i]] << ") ";
            */
            ptFloat3=currentNode->pheroNeighbours+currentNode->neighbourSortIndice[currentCar->index][i];
            *(ptUint++) = currentNode->neighbourSortIndice[currentCar->index][i];
            tau = *ptFloat3;
            eta = 1.0/(*ptFloat2);
            /* PROBABILITY */
            p = pow(tau, parData.alpha1) * pow(eta, parData.beta1);
            *(ptFloat++) = sum;     //freq array probability (cumulative)
            *(ptFloat4++) = p;
            sum += p;               //left shift, p0 to 0, sum after, last sum not needed
            nNeighbours++;
        }
        
    }
    /*
    if(nNeighbours>0)
        cout << endl;
    */    
    return nNeighbours;
}


int PickNode(ant *currentAnt, node *currentNode, car *currentCar)
{
    uint i, nNeighbours, favorits, start;
    float sum;
    /* edgeWeightMatrices[car][node][neighbour] */

    //calculate full neighbours for min pheromone value
    favorits = prData.dim;
    nNeighbours = PrepareNeighbours(currentAnt, currentNode, currentCar, sum, favorits, 0);
    probArrays.n1 = nNeighbours;
    probArrays.sum1 = sum;


    if(parData.nFavorits > 1 && parData.nFavorits < (int)prData.dim)
        favorits = parData.nFavorits;
    else
        favorits = prData.dim;
    
    start = 0;
    do
    {
        nNeighbours = PrepareNeighbours(currentAnt, currentNode, currentCar, sum, favorits, start);
        start+=favorits;
    }while(nNeighbours==0 && start<prData.dim-1 );

    probArrays.n = nNeighbours;
    probArrays.sum = sum;
    if(nNeighbours == 0)
    {
        probArrays.selected = -1;
        return -1;
    }
    else if(nNeighbours == 1)
    {
        probArrays.selected = 0;
        return probArrays.indices[0];
    }
    i = selectFromFreqArray(sum, nNeighbours, probArrays.cumulatives);
    probArrays.selected = i;
    return probArrays.indices[i];
}

int PickCar(ant *currentAnt, node *currentNode, car *currentCar)
{
    uint i, j, counter, nCars;
    float avgReturn, avgNeighbour, eta, tau, p, sum, ftemp;
    uint *ptUint;
    float *ptFloat, *ptFloat2, *ptFloat3, *ptFloat4;
    car *ptCar;
    int picked = 0;
    if(prData.nCars <= 1) //at least one car exists, car index 0
    {
        probArrays.n = 1;
        probArrays.sum = 1;
        probArrays.probs[0] = 1.0;
        probArrays.selected = 0;
        return picked;
    }
    for(j=0, nCars=0, ptCar=cars, 
        ptFloat4 = probArrays.probs,
        ptFloat3=currentNode->pheroCars,
        ptFloat = probArrays.cumulatives,
         ptUint = probArrays.indices, sum=0.0; 
        j<prData.nCars; 
        j++,ptCar++, ptFloat3++)
    {
        /* non-rented cars or current car */
        if(currentAnt->carsRented[ptCar->index]==false || 
            (currentCar != nullptr && ptCar->index == currentCar->index))
        {
            /* average return */
            if(currentCar != nullptr && ptCar->index == currentCar->index) //no return cost for current car
                avgReturn = 0.0;
            else
            {
                for(i=0, avgReturn=0.0, counter=0; i<prData.dim; i++)
                    if(currentNode->index != i)
                    {
                        avgReturn+=prData.returnRateMatrices[ptCar->index][i][currentNode->index];
                        counter++;
                    }
                if(counter>0)
                    avgReturn/=counter;
            }
            /* average forward */
            for(i=0, avgNeighbour=0.0, counter=0, 
                ptFloat2=prData.edgeWeightMatrices[ptCar->index][currentNode->index]; 
                i<prData.dim; i++, ptFloat2++)
                if(currentNode->index != i && *ptFloat2 != 0.0 && *ptFloat2 < 9999)
                {
                     avgNeighbour+=(*ptFloat2);
                     counter++;
                }
            if(counter>0)
                avgNeighbour/=counter;
            *(ptUint++)=j;
            tau = *ptFloat3;
            ftemp = avgNeighbour+avgReturn; // path to neighbour, new car average return in future
            if(currentAnt->carPickedNode != nullptr) //we need to return old car also
                ftemp += prData.returnRateMatrices[currentCar->index][currentNode->index][currentAnt->carPickedNode->index];
            eta = 1.0/ftemp;
            /* PROBABILITY */
            p = pow(tau, parData.alpha2) * pow(eta, parData.beta2);
            *(ptFloat++) = sum;
            *(ptFloat4++) = p;
            sum+=p;
            nCars++;
        }
    }
    probArrays.n = nCars;
    probArrays.sum = sum;
    if(nCars==0)
    {
        probArrays.selected = -1;
        return -1;
    }
    else if(nCars==1)
    {
        probArrays.selected = 0;
        return probArrays.indices[0];
    }
    i = selectFromFreqArray(sum, nCars, probArrays.cumulatives);
    probArrays.selected = i;
    return probArrays.indices[i];
}


void calculateMinRatio(antNode *nodes, uint nodeCounter, float& minRatio1, float& minRatio2)
{
    uint i;
    double p1, p2, avg1, avg2;
    int n1, n2;
    antNode *ptAntNode;
    p1 = p2 = 1.0;
    n1 = n2 = 0;
    for(i=0, ptAntNode=nodes; i<nodeCounter; i++, ptAntNode++)
    {
        p1 *= pow(ptAntNode->prob1,(1.0/nodeCounter));
        p2 *= pow(ptAntNode->prob2,(1.0/nodeCounter));
        n1 += ptAntNode->choices1;
        n2 += ptAntNode->choices2;
    }

    //p1 = pow(p1, (1.0/nodeCounter)); premjesteno gore zbog double overflowa
    //p2 = pow(p2, (1.0/nodeCounter));
    avg1 = (double)n1/nodeCounter;
    avg2 = (double)n2/nodeCounter;

    minRatio1 = (1.0-p1)/((avg1-1.0)*p1);
    if(minRatio1 > 1.0)
        minRatio1 = 1.0;
    minRatio2 = (1.0-p2)/((avg2-1.0)*p2);
    if(minRatio2 > 1.0)
        minRatio2 = 1.0;

    //cout << "Nodes p: " << p1 << ", neighbours: " << n1 << ", avg: " << avg1 << ", min ratio: " << minRatio1 << endl;
    //cout << "Cars  p: " << p2 << ", picks: " << n2 << ", avg: " << avg2 << ", min ration: " << minRatio2 << endl;

    pheroRatio1 = minRatio1;
    pheroRatio2 = minRatio2;
}

void calculateMaxMin()
{
    float minRatio1, minRatio2, price = 0.0;
    if(bPath.haveOpt && bPath.optPrice > 0.0)
    {
        price = bPath.optPrice;
        calculateMinRatio(bPath.optNodes, bPath.nodeCounter, minRatio1, minRatio2);
    }
    else if(bPath.price > 0.0)
    {
        price = bPath.price;
        calculateMinRatio(bPath.nodes, bPath.nodeCounter, minRatio1, minRatio2);
    }
    else
        return; //prices are zero so no phero calculation (starting)

    //nodes
    pheroMax1 = 1.0/(parData.rho1*price);
    pheroMin1 = minRatio1 * pheroMax1;
    //cars
    pheroMax2 = 1.0/(parData.rho2*price);
    pheroMin2 = minRatio2*pheroMax2;

}
