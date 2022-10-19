#ifndef PASSENGERS_H
#define PASSENGERS_H

#include "ACO.h"

struct passAnt
{
    int nIt;
    float price;
    uchar thrownOutPassNumber;
    pass **thrownOutPassengers; //array of thrown passengers ie. their pointers
};

/* passengers' manager */
struct passManager //arrays of cars and passengers are global
{
    uint nodeCounter; //number of nodes on tour
    antNode *nodes; //tour - array of nodes, basic or opt

    passAnt* ants;
    passAnt bestAnt;

    uchar *transPassNumber; //number of passengers in transitions
    bool **transPassengers; //passengers (their pointers) in all transitions

    bool *pickedPassengers; //flags for picked passengers: true=picked

    bool penalisation;
    ushort initThrownOutPassNumber;
    pass **initThrownOutPassengers;

    float max, min;
    float *pheromones;
    float *budgets;

    float upperBound; //upper bound of price (all transitions are max price, and all returns are max price)
 
    float bestPrice;
    bool passWithOpt; //whether is passengers on opt nodes
};

float calculatePassengers(ant *currentAnt, bool useOpt);

void checkPassengers();
void resetPassPheromones();
bool passNodeTraversal();
void thrownOutPassenger();
float calculateTourPrice();
void updatePheromones();
void calculatePassMaxMin();
void limitPassPheromones();
void updateUpperAnt(ant *currentAnt);

/* testiranje */
void makeTestAnt(ant *currentAnt);

#endif