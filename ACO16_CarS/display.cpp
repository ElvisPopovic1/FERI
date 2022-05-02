#include "ACO.h"
#include "passengers.h"

using namespace std;

extern passManager* pManager;

void displayProblemData()
{
    cout << prData.name << ", type: " << prData.type << ", nodes: " << prData.dim << ", cars: " << prData.nCars << ", passengers: " << prData.nPass << endl;
    cout << "Seed: " << parData.seed << ", startNode: " << parData.startNode << ", iters: " << parData.nIter << 
    ", favorits: " << parData.nFavorits << ", ants: " << parData.nAnts <<
    ", stall: " << parData.stall << ", rhoNode: " << parData.rho1 << ", rhoCar: " << parData.rho2 << 
    ", alpha1: " << parData.alpha1 << ", beta1: " << parData.beta1 << 
    ", alpha2: " << parData.alpha2 << ", beta2: " << parData.beta2 << 
    ", rho3: " << parData.rho3 << 
    ", alpha3: " << parData.alpha3 << ", beta3: " << parData.beta3 << endl;
    cout << "Executions: " << parData.nExecutions << ", " << ", favorits: " << parData.nFavorits << endl;
    cout << "Switches: " << (parData.opt?"--opt ":"") << (parData.pass?"--pass ":"") << (parData.writeData?"--write ":"") << endl; 
    cout << "Average node available (navg): " << prData.nodeNAverage << endl;
}

void displayBestPath(std::chrono::duration<double> cur_time)
{
    cout << "it. " << bPath.iteration << ", time: " << cur_time.count() << " s" <<
    ", stNode: " << bPath.nodes->curNode->index << ", price: " << bPath.price << 
    ", opt. price: " << (bPath.haveOpt?bPath.optPrice:-1) << 
    ", pass. price: " << (bPath.havePass?bPath.passPrice:-1);
    if(bPath.havePass)
        cout << ", picked pass: " << bPath.nPickedPassengers;
    cout << ", nodes phero Min: " << bPath.pheroMin1 << ", nodes phero Max: " << bPath.pheroMax1 << 
    ", nodes phero ratio: " << bPath.pheroRatio1 <<
    ", cars phero Min: " << bPath.pheroMin2 << ", cars phero Max: " << bPath.pheroMax2 << 
     ", cars phero ratio: " << bPath.pheroRatio2 << endl;
}

/* passengers part */
/* write initial data, nodes, prices, passengers and budgets */
void displayInitData(passManager *pManager)
{
    uint i, j;
    float price;
    antNode *ptAntNode;
    car *ptCar;
    node *ptNode1, *ptNode2;
    pass **pptPass;
    for(j=0, ptAntNode=pManager->nodes; j<pManager->nodeCounter; j++, ptAntNode++)
    {
        ptCar = ptAntNode->carOut;
        ptNode1 = ptAntNode->curNode;
        ptNode2 = ptAntNode->nextNode;
        price = prData.edgeWeightMatrices[ptCar->index][ptNode1->index][ptNode2->index];
        cout << "Node " << ptNode1->index << ", transition price: " << price << 
        ", car: " << ptAntNode->carOut->name << 
        ", passengers (" << ptAntNode->curNode->nPassengers << "):" << endl;
        for(i=0, pptPass=ptAntNode->curNode->passengers; i<ptAntNode->curNode->nPassengers; i++,pptPass++)
            if((*pptPass) != nullptr)
                cout << " -passenger " << (*pptPass)->index << ", budget: " << (*pptPass)->budget << 
                ", destination: " << (*pptPass)->endNode << endl; 
            else if(i==0)
                cout << "No passenger." << endl;     
    }
    price = calculatePathCost(pManager->nodes, pManager->nodeCounter);
    cout << "Calculated path price: " << price << endl;
}

void displayPassengers(passManager *pManager)
{
    uint i;
    pass *ptPass;
    cout << "\tPassengers (rho=" << parData.rho1 << ", rho3=" << parData.rho3 << "):" << endl;
    for(i=0, ptPass=passengers; i<prData.nPass; i++, ptPass++)
    {
        cout << "P" << i << ": fero: " << ptPass->phero << ", radius: " << 
        ptPass->radius << ", budget: " << ptPass->budget << endl;
    }
}

/* final price for IRACE - can be executed after cleanup */
void displayFinalPrice()
{
    cout << "Final best price: ";
    if(bPath.havePass)
        cout << bPath.passPrice;
    else if(bPath.haveOpt)
        cout << bPath.optPrice;
    else
        cout << bPath.price;
    cout << ", found at iteration: " << bPath.iteration << endl;
}

