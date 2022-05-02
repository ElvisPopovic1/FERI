#include "supervisor.h"
#define BEST_HEADER_LENGTH 24

char bestHeader[][BEST_HEADER_LENGTH]={"Iter","time[s]", "StartNode", "BestAntTrav", "BestAntCalc","BestAntOpt","BestAntPass",
"BestPathIt", "BestPathTime", "BestPathTrav",
"BestPathCalc","BestPathOpt","BestPathPass","PassNumber","NodePheroMin", "NodePheroMax", "NodePheroRatio", 
"CarPheroMin", "CarPheroMax", "CarPheroRatio", "Stall"};

using namespace std;

char nodeHeader[][8]={"Index","Name","nPass"};
char nodePassHeader[][8]={"Node","Index","Name","Start","End"};


ofstream problemDataStr, initDataStr, bestFileStream, resultStream;
ofstream multExecStr;

void writeHeaders();
void displayMatrix(float*** matrix, int nMatrix, int dim);


/* starts all supervisor stuff e.g. opening files */
void beginSupervisor(const char* filename)
{
    char name[64];

    sprintf(name,"%s_multiple.txt", filename);
    multExecStr.open(name, ios::out);

    if(parData.writeData == false)
        return;
    

    sprintf(name,"%s_input_data_%d.txt", filename,parData.seed);
    problemDataStr.open(name, ios::out);
    sprintf(name,"%s_init_data_%d.txt", filename,parData.seed);
    initDataStr.open(name, ios::out);
    sprintf(name,"%s_best_%d.txt", filename, parData.seed);
    bestFileStream.open(name, ios::out);
}

/* ends supervisor */
void endSupervisor()
{   

    multExecStr.close();

    if(parData.writeData == false)
        return;
    bestFileStream.close();
    
}

/* supervisor functions */

/* supervisor functions */
void writeProblemData()
{
    uint i;
    uchar *pt1;
    dataPass *ptPass;
    problemDataStr << "Parameters (argc=" << parData.argc << "):" << endl;
    problemDataStr << "Data file: " << parData.fileName << endl;
    problemDataStr << "start node: " << parData.startNode << endl;
    problemDataStr << "node rho: " << parData.rho1 << endl;
    problemDataStr << "car rho: " << parData.rho2 << endl;
    problemDataStr << "node alpha: " << parData.alpha1 << endl;
    problemDataStr << "node beta: " << parData.beta1 << endl;
    problemDataStr << "car alpha: " << parData.alpha2 << endl;
    problemDataStr << "car beta: " << parData.beta2 << endl;
    problemDataStr << "nIter: " << parData.nIter << endl;
    problemDataStr << "nAnts: " << parData.nAnts << endl;
    problemDataStr << "stall: " << parData.stall << endl;
    problemDataStr << "opt: " << (parData.opt?"yes":"no") << endl;
    problemDataStr << "pass rho: " << parData.rho3 << endl;
    problemDataStr << "pass alpha: " << parData.alpha3 << endl;
    problemDataStr << "pass beta: " << parData.beta3 << endl;
    problemDataStr << "nPassAnts: " << parData.nPassAnts << endl;
    problemDataStr << "nPassTries: " << parData.nPassIters << endl;
    problemDataStr << "pass: " << (parData.pass?"yes":"no") << endl;
    problemDataStr << "write to file: " << (parData.writeData?"yes":"no") << endl;
    problemDataStr << "Seed: " << parData.seed << endl;
    problemDataStr << "------------------------\nProblem data parameters:" << endl;
    problemDataStr << "Name: " << prData.name << endl;
    problemDataStr << "Type: " << prData.type << endl;
    problemDataStr << "Comment: " << prData.comment << endl;
    problemDataStr << "Dimension: " << (int)prData.dim << endl;
    problemDataStr << "Cars: " << (int)prData.nCars << ", passengers: " << (int)prData.nPass << endl;
    problemDataStr << "Edge Weight Matrices: " << endl;

    for(i=0; i<prData.nCars; i++)
        displayMatrix(prData.edgeWeightMatrices, i, prData.dim);

    problemDataStr << "Return rate Matrices: " << endl;
    for(i=0; i<prData.nCars; i++)
        displayMatrix(prData.returnRateMatrices, i, prData.dim);
    problemDataStr << "Average nodes available (navg): " << prData.nodeNAverage << endl;
    if(prData.nPass > 0) // we can not write passengers if there are no any
    {
        problemDataStr << "Passenger's limits (car capacities):" << endl;
        for(i=0, pt1=prData.carPassLimit; i<prData.nCars; i++, pt1++)
            problemDataStr << "(car:" << i << ", cap:" << (int)*pt1 << ") ";

        problemDataStr << endl << "Passengers: " << endl;
        for(i=0, ptPass=prData.passengers; i<prData.nPass; i++, ptPass++)
            problemDataStr << ptPass->startNode << " " << ptPass->destinationNode << " " << ptPass->budget << endl;
    }
    problemDataStr << "END" << endl;
    problemDataStr.close(); //writes once and closes file
}

void writeInitData()
{
    uint i,j;
    unsigned long strSize;
    node* ptNode;
    pass** pptPass;
    strSize = sizeof(nodeHeader)/(8*sizeof(char));
    initDataStr << "Nodes:" << endl;
    for(i=0; i<strSize-1; i++)
        initDataStr << nodeHeader[i] << " ";
    initDataStr << nodeHeader[i] << endl;
    for(i=0, ptNode=nodes; i<prData.dim; i++, ptNode++)
    {
        initDataStr << ptNode->index << " " << 
        ptNode->name << " " << 
        ptNode->nPassengers << endl;
    }
    strSize = sizeof(nodePassHeader)/(8*sizeof(char));
    initDataStr << "Node passengers:" << endl;
    for(i=0; i<strSize-1; i++)
        initDataStr << nodePassHeader[i] << " ";
    initDataStr << nodePassHeader[i] << endl;
    for(j=0, ptNode=nodes; j<prData.dim; j++, ptNode++)
    {
        for(i=0, pptPass=ptNode->passengers; i<ptNode->nPassengers; i++, pptPass++)
        {
            initDataStr << ptNode->name << " " << 
            (*pptPass)->index << " " <<
            (*pptPass)->name << " " <<
            (*pptPass)->startNode << " " <<
            (*pptPass)->endNode << endl;
        }
    }
    initDataStr.close(); //writes once and closes file
}

/* help functions */
void writeHeaders()
{
    unsigned long i, strSize;
    if(parData.writeData == false)
        return;
    strSize = sizeof(bestHeader)/(BEST_HEADER_LENGTH*sizeof(char));

    for(i=0; i<strSize-1; i++)
        bestFileStream << bestHeader[i] << " ";
    bestFileStream << bestHeader[i] << endl;

}


void writeResult(const char* filename)
{
    uint i, j;
    float *ptFloat;
    antNode *ptAntNode;
    car *ptCar;
    node *ptNode, *carPickNode;
    pathNodePass *ptPathNodePass;
    pass *ptPass, **pptPass;
    bool *ptBool;
    char name[64];
    if(parData.writeData == false)
        return;
    sprintf(name,"%s_results_%d.txt", filename, parData.seed);
        resultStream.open(name, ios::out);
    if(bPath.nodeCounter == 0)
    {
        resultStream << "No best cost tour." << endl;
        resultStream.close();
        return;
    }
    resultStream << "Best: iteration: " << bPath.iteration << ", time: " << bPath.solution_time.count() << 
    ", start node: " << bPath.nodes->curNode->name << ", cost non-opt: " << bPath.price << 
    ", opt: " << (bPath.haveOpt?bPath.optPrice:-1) << endl;
    resultStream << "Nodes (name, car in, car out, arc cost, car return cost, passengers, passPrice): " << endl;
    for(i=0, ptAntNode=bPath.nodes, ptPathNodePass = bPath.tourPassengers, carPickNode=bPath.nodes->curNode; 
        i<bPath.nodeCounter; i++, ptAntNode++, ptPathNodePass++)
    {
        resultStream << ptAntNode->curNode->name << " " << 
        (ptAntNode->carIn==nullptr?"C-":ptAntNode->carIn->name) << " " << ptAntNode->carOut->name << " " <<
        prData.edgeWeightMatrices[ptAntNode->carOut->index][ptAntNode->curNode->index][ptAntNode->nextNode->index] << " ";
        if(i==0)
            resultStream << 0 << " ";
        else if(ptAntNode->carIn->index == ptAntNode->carOut->index)
        {
            resultStream << 0 << " ";
        }
        else
        {
            resultStream << prData.returnRateMatrices[ptAntNode->carIn->index][ptAntNode->curNode->index][carPickNode->index] << " ";
            carPickNode = ptAntNode->curNode;
        }
        if(bPath.havePass && !bPath.passWithOpt)
            resultStream << (int)(ptPathNodePass->nPassengers) << " " <<  
                prData.edgeWeightMatrices[ptAntNode->carOut->index][ptAntNode->curNode->index][ptAntNode->nextNode->index]/((ptPathNodePass->nPassengers)+1) << endl;
        else resultStream << endl;
    }
    resultStream << bPath.nodes->curNode->name << " " <<  (ptAntNode-1)->carOut->name << " - " << " - ";
    resultStream << prData.returnRateMatrices[(ptAntNode-1)->carOut->index][(ptAntNode-1)->nextNode->index][carPickNode->index];
    if(bPath.havePass && !bPath.passWithOpt)
        resultStream << " " << 0 << " " << 0 << endl;
    else
        resultStream << endl;

    if(bPath.haveOpt)
    {
        resultStream << "Opt:" << endl << "Nodes (name, car in, car out, arc cost, car return cost, passengers, passPrice): " << endl;

        for(i=0, ptAntNode=bPath.optNodes, ptPathNodePass = bPath.tourPassengers, carPickNode=bPath.optNodes->curNode; 
            i<bPath.nodeCounter; i++, ptAntNode++, (bPath.havePass?ptPathNodePass++:ptPathNodePass+=0))
        {
            
            resultStream << ptAntNode->curNode->name << " " <<
            (ptAntNode->carIn==nullptr?"C-":ptAntNode->carIn->name) << " " << ptAntNode->carOut->name << " " <<
            prData.edgeWeightMatrices[ptAntNode->carOut->index][ptAntNode->curNode->index][ptAntNode->nextNode->index] << " ";
            if(i==0)
                resultStream << 0;
            else if(ptAntNode->carIn->index == ptAntNode->carOut->index)
            {
                resultStream << 0;
            }
            else
            {
                resultStream << prData.returnRateMatrices[ptAntNode->carIn->index][ptAntNode->curNode->index][carPickNode->index];
                carPickNode = ptAntNode->curNode;
            }
            if(bPath.havePass)
                resultStream << " " << (int)(ptPathNodePass->nPassengers)  << " " <<
                prData.edgeWeightMatrices[ptAntNode->carOut->index][ptAntNode->curNode->index][ptAntNode->nextNode->index]/((ptPathNodePass->nPassengers)+1);
            resultStream << endl;
            
        }
        resultStream << bPath.optNodes->curNode->name << " " <<  (ptAntNode-1)->carOut->name << " - " << " - ";
        resultStream << prData.returnRateMatrices[(ptAntNode-1)->carOut->index][(ptAntNode-1)->nextNode->index][carPickNode->index];
        if(bPath.havePass) 
            resultStream << " " << 0 << " " << 0;
        resultStream << endl;
    }

    resultStream << endl << "Pheromones: neighbours (max = " << bPath.pheroMax1 << ", min = " << bPath.pheroMin1 << "):" << endl;
    for(j=0, ptAntNode = (bPath.haveOpt?bPath.optNodes:bPath.nodes); j<bPath.nodeCounter; j++, ptAntNode++)
    {
        resultStream << (ptAntNode->prevNode==nullptr?"V-":ptAntNode->prevNode->name) << " -> " << 
        ptAntNode->curNode->name << " -> " << ptAntNode->nextNode->name << endl;
        for(i=0, ptNode=nodes, ptFloat=ptAntNode->curNode->pheroNeighbours; i<prData.dim; i++, ptNode++, ptFloat++)
            resultStream << "(" << ptNode->name << ";" << *ptFloat << ") ";
        resultStream << endl;
    }
    resultStream << endl << "Pheromones: cars (max = " << bPath.pheroMax2 << ", min= " << bPath.pheroMin2 << "):" << endl;
    for(j=0, ptAntNode = (bPath.haveOpt?bPath.optNodes:bPath.nodes); j<bPath.nodeCounter; j++, ptAntNode++)
    {
        for(i=0, ptCar=cars, ptFloat=ptAntNode->curNode->pheroCars; i<prData.nCars; i++, ptCar++, ptFloat++)
            resultStream << "(" << ptCar->name << ";" << *ptFloat << ") ";
        resultStream << endl;
        resultStream << ptAntNode->curNode->name << ", " << 
        (ptAntNode->carIn==nullptr?"C-":ptAntNode->carIn->name) << " -> " << ptAntNode->carOut->name << endl;
    }
    if(bPath.havePass > 0)
    {
        resultStream << endl << "Pheromones and budgets: passengers (" << bPath.nPickedPassengers << " of " << prData.nPass << 
            (bPath.passWithOpt?", with opt):":", without opt):") << endl;
        for(j=0, ptPathNodePass = bPath.tourPassengers, ptAntNode=(bPath.passWithOpt?bPath.optNodes:bPath.nodes); 
            j<prData.dim; j++, ptPathNodePass++, ptAntNode++)
        {
            resultStream << ptAntNode->curNode->name << endl;
            if(ptPathNodePass->nPassengers > 0)
            {
                for(i=0, pptPass = ptPathNodePass->passengers, ptFloat = ptPathNodePass->budgets; 
                    i<ptPathNodePass->nPassengers; i++, pptPass++, ptFloat++)
                    resultStream << "(" << (*pptPass)->name << ";V" << (*pptPass)->startNode << "-V" << 
                    (*pptPass)->endNode << ";" << bPath.passPheromones[(*pptPass)->index] << ", " << 
                    (*ptFloat) << ") ";
            }
            resultStream << endl;
        }
    }

    resultStream << endl << "Choices and probabilities:" << endl;
    resultStream << " -: \tn = " << bPath.nodes->choices1 << 
    ", nCar = " << (bPath.passWithOpt?bPath.optNodes:bPath.nodes)->choices2 << 
    ", pNode = " << (bPath.passWithOpt?bPath.optNodes:bPath.nodes)->prob1 << 
    ", pCar = " << (bPath.passWithOpt?bPath.optNodes:bPath.nodes)->prob2 << endl;
    for(j=0, ptAntNode = (bPath.passWithOpt?bPath.optNodes:bPath.nodes)+1; j<bPath.nodeCounter-1; j++, ptAntNode++)
        resultStream <<ptAntNode->prevNode->name << ": \tn = " << ptAntNode->choices1 << 
        ", nCar = " << ptAntNode->choices2 << 
        ", pNode = " << ptAntNode->prob1 << ", pCar = " << ptAntNode->prob2 << endl; 

    resultStream << endl << "Summary:" << endl;
    for(j=0, ptAntNode = bPath.nodes; j<bPath.nodeCounter-1; j++, ptAntNode++)
        resultStream << ptAntNode->curNode->name << ", ";
    resultStream << ptAntNode->curNode->name << " ";
    resultStream << "(basic nodes)" << endl;
    if(bPath.haveOpt)
    {
        for(j=0, ptAntNode = bPath.optNodes; j<bPath.nodeCounter-1; j++, ptAntNode++)
            resultStream << ptAntNode->curNode->name << ", ";
        resultStream << ptAntNode->curNode->name << " ";
        resultStream << "(opt nodes)" << "" << endl;
    }

    resultStream << (bPath.passWithOpt?bPath.optNodes:bPath.nodes)->carOut->name << (bPath.nodeCounter<=1?"\n":" ");
    for(j=1, ptAntNode = (bPath.passWithOpt?bPath.optNodes:bPath.nodes)+1; j<bPath.nodeCounter; j++, ptAntNode++)
        if(ptAntNode->carIn->index != ptAntNode->carOut->index)
            resultStream << ptAntNode->carOut->name << " ";
    resultStream << endl;

    if(bPath.havePass > 0)
    {
    for(i=0, ptBool=bPath.pickedPassengers, ptPass=passengers; i<prData.nPass; i++, ptBool++, ptPass++)
        if(*ptBool)
            resultStream << ptPass->name << " ";
    resultStream << endl;
    }
    resultStream << "Non-opt price: " << bPath.price << 
    ", opt price: " << (bPath.haveOpt?bPath.optPrice:-1) <<
    ", pass price: " << (bPath.havePass?bPath.passPrice:-1) << 
    ", time: " << bPath.solution_time.count() << " s"
    << endl;
    resultStream.close();
}

void writeBestData(uint iteration, chrono::duration<double> cur_time, uint startNode, ant *bestAnt, bool solutionStall)
{
    if(parData.writeData == false)
        return;
    bestFileStream << iteration << " " << 
    cur_time.count() << " " << startNode << " " <<
    bestAnt->priceNoPassengers << " " << calculatePathCost(bestAnt->nodes, bestAnt->nodeCounter) << " " <<
    (bestAnt->haveOpt?bestAnt->bestOptPrice:-1) << " " << 
    (bestAnt->havePassengers?bestAnt->passPrice:-1) << " " << 
    bPath.iteration << " " << bPath.solution_time.count() << " " <<
    bPath.price << " " << calculatePathCost(bPath.nodes, bPath.nodeCounter) << " " << 
    (bPath.haveOpt?bPath.optPrice:-1) << " " <<
    (bPath.havePass?bPath.passPrice:-1) << " " <<
    bPath.nPickedPassengers << " " <<
    bPath.pheroMin1 << " " << bPath.pheroMax1 << " " << bPath.pheroRatio1 << " " <<
    bPath.pheroMin2 << " " << bPath.pheroMax2 << " " << bPath.pheroRatio2 << " " <<
    (solutionStall?" *":"") << endl;
}

// for multiple solution
void writeFinalPrice()
{
    multExecStr << parData.seed << " ";
    if(bPath.havePass)
        multExecStr << bPath.passPrice;
    else if(bPath.haveOpt)
        multExecStr << bPath.optPrice;
    else
        multExecStr << bPath.price;
    multExecStr << " " << bPath.iteration << endl;
}

void displayMatrix(float*** matrix, int nMatrix, int dim)
{
    int i;
    int j;
    float *pt1;
    float **pt2;
    if(matrix == nullptr)
        return;
    problemDataStr << "Car: " << nMatrix << endl;
    for(j=0, pt2=matrix[nMatrix]; j<dim; j++,pt2++)
    {
        for(i=0, pt1=*pt2; i<dim; i++,pt1++)
            problemDataStr << *pt1 << " ";
        problemDataStr << endl;
    }
}

