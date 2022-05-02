#include "ACO.h"
#include "supervisor.h"

using namespace std;

/* chrono timer stuff */
chrono::duration<double> elapsed_time;
chrono::system_clock::time_point start_time, current_time;

int main(int argc, char* argv[])
{
    bool result, stall;
    uint iter, startNode;
    ant *bestAnt;
    if(argc < 2)
    {
        cout << "No filename parameter." << endl;
        cout << "Usage: " << argv[0] << " " << "[-d] filename (--startNode | -sn) startNode (--rhoNode | -r1) rhoNode (--rhoCar | -r2) rhoCar " << 
        "(--alphaNode | -a1) alphaNode  (--betaNode | -b1) betaNode (--alphaCar | -a2) alphaCar  (--betaCar | -b2) betaCar " <<
        "(--ants | -na) nAnts (--iter | -ni) nIterations (--favorits | -nfav) nFavorits (--stall | -st) stall (--passAnts | -npa) nPassengersAnts (--rhoPass | -r3) " << 
        "rhoPassengers (--alphaPass | -a3) alphaPassengers (--betaPass | -b3) betaPassengers (--passTries | -npt) nPassengersTries " << 
        "[--opt] opt [--pass] passengers [--write] write" << endl;
        return 0;
    }
    setParameters(argc, argv);
    if(!loadData(parData.fileName))
    {
        cout << "Data file error." << endl;
        return 0;
    }
    cout << "Data loaded." << endl;

    beginSupervisor("./outputData/stat"); //for write (to file) purposes
    // here for multiple execution
    for(uint multExecution=0; multExecution < parData.nExecutions; multExecution++)
    {
        parData.seed = -1;

        init();
        
        displayProblemData(); //write to screen
        writeProblemData(); //write to file
        writeInitData();
        cout << "Problem and init data written." << endl;
        cout << "Ant simulation in exec " << multExecution << " will commence..." << endl;

        if(parData.writeData == true)
            writeHeaders();

        uniform_int_distribution<int> distribution(0,prData.dim-1);
        start_time = chrono::system_clock::now();
        /* fix start node parameter if necessary */

        if(parData.startNode >= (int)(prData.dim))
            parData.startNode = prData.dim-1;
        
        if(parData.startNode == -1)
            startNode = (uint)distribution(*mersenneGenerator);
        else
            startNode = parData.startNode;

        for(iter=0; iter < parData.nIter; iter++)
        {
            PheromoneEvaporation(); 
            Solution(0, &nodes[startNode]);
            opt2_5(); //passengers call added inside opt2_5 (for each optimized)
            bestAnt = findBestAnt();
            current_time = chrono::system_clock::now();

            if(bestAnt != nullptr) //???
            {
                if((stall = checkSolutionsStall(bestAnt)) == true)
                {
                    resetPheromones();
                    if(parData.startNode == -1)
                        startNode = (uint)distribution(*mersenneGenerator);
                    if(parData.writeData == true)
                        writeBestData(iter, current_time-start_time, startNode, bestAnt, stall); 
                    writeResult("./outputData/stat");
                    continue;
                }
                updatePheromones(bestAnt);
                limitPheromoneTraces();

                if(iter > 0) //do not use phero reset in starting iterations
                {
                    result = updateBestPathSincePheroReset(bestAnt);
                    if(result)
                    {
                        calculateMaxMin();
                        limitPheromoneTraces();
                    }
                }

                result = updateBestPath(iter, bestAnt, current_time-start_time);
                if(result)
                {
                    displayBestPath(current_time-start_time); //display in display.cpp
                }
    

                if(parData.writeData == true)
                    writeBestData(iter, current_time-start_time, startNode, bestAnt, stall);  
                writeResult("./outputData/stat");    
            }
            
        }

     writeFinalPrice();

        cout << "Ant simulation ended." << endl;
        cleanup();
        cout << "Init cleanup done." << endl;

    displayFinalPrice();
    //end of multiple execution
    }

        /* cleans parser memory space */
        freeData();
        cout << "Loaded data freeing done." << endl;
        

   

    endSupervisor();
    return 0;
}







