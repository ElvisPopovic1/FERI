#include "CreateSolution.h"

using namespace std;

SolutionData solData;



bool InitSolver(ProblemData &problem, Parameters &params)
{
    Solution solution;
    mt19937::result_type seed;
    if(params.seed == -1)
        seed = time(0);
    else 
        seed = params.seed;
    solData.mersenneGenerator = mt19937(seed);

    resetSolver(); //reset when problem is changed (all vectors clears)
    solData.problemData = problem;
    solData.parameters = params;

    /* Euclidean problems are not currently supported. */
    if(solData.problemData.dimension < 2 || solData.problemData.cars_number < 1 || 
        solData.problemData.edge_weight_type != ET_EXPLICIT || solData.problemData.edge_weight_format != EF_FULL_MATRIX)
    return false;

    /* a node can have dim-1 neighbors */
    if(solData.parameters.nFavorits < 1 || solData.parameters.nFavorits >= solData.problemData.dimension)
        solData.parameters.nFavorits = solData.problemData.dimension-1;
    
    createTensors(); //build tensor
    calculateCarHeuristics(); //etas for cars
    calculateNodeHeuristics(); //etas for nodes

    std::cout << "Start display" << std::endl;
    return true;
}

double CalculateSolution(double (*calculateSolutionCost)(ProblemData &problem, Solution &solution), Solution &solution)
{
    int it, a;
    double result = 0.0;
    Solution antSolution, iterSolution, kappaMinSolution;
    std::deque<Solution>::iterator kappaIt;

    resetSolution(antSolution); //ant solution
    resetSolution(iterSolution); //best ant solution in one iteration
    resetSolution(solution); //best solution of all iterations


    solution.cost = numeric_limits<double>::max();
    for(it=0; it<solData.parameters.nIterations; it++)
    {
        iterSolution.cost = numeric_limits<double>::max();
        for(a=0; a<solData.parameters.nAnts; a++)
        {
            if(AntRun(antSolution))
            {
                result = calculateSolutionCost(solData.problemData, antSolution); 
                antSolution.iteration = it;
                if(result < iterSolution.cost) //best ant
                    CopySolution(antSolution, iterSolution);
            }
        }
        if(CheckSolutionStall(solData.parameters, iterSolution))
        {
            //std::cout << "Stall: " << iterSolution.cost << ", reset cost: " << solution.cost << ", global cost: " << solution.globalCost << std::endl;
            resetPheromones(iterSolution); //we use iterSolution as phero update so we reset it here
            solution.cost = std::numeric_limits<double>::max();
            continue; //jump over pheromone updating and limiting
        }
        if(it==0)
        {
            CopySolution(iterSolution, solution);
            solution.globalCost = iterSolution.cost;
            solution.iteration = it;
            CalculateMaxMin(solution.globalCost); //in first iteration it is global best also
        }
        UpdatePheromones(iterSolution); //best ant solution = iterSolution
        LimitPheromones();
        if(iterSolution.cost < solution.cost)
        {
            solution.cost = iterSolution.cost; 
            
            std::cout << "Iter: " << iterSolution.iteration << 
            ", best reset " << solution.cost << ", best solution " << solution.globalCost << 
            ", min car: " << solData.carPheroMin << ", max car: " << solData.carPheroMax << 
            ", min node: " << solData.nodePheroMin << ", max node: " << solData.nodePheroMax << std::endl;
            /*
            int i;
            for(i=0; i<iterSolution.nodeList.size(); i++)
                std::cout << iterSolution.nodeList[i] << " ";
            std::cout << std::endl;
            for(i=0; i<iterSolution.carList.size(); i++)
                std::cout << iterSolution.carList[i] << " ";
            std::cout << std::endl;
            */

            
        }
        // else print each iteration
        //else
        //    std::cout << "It: " << iterSolution.iteration << ", cost: " << iterSolution.cost << std::endl;

        if(iterSolution.cost < solution.globalCost)
        {
            CopySolution(iterSolution, solution);
            solution.globalCost = iterSolution.cost;
            solution.iteration = it;
            CalculateMaxMin(solution.globalCost); //global cost for pheromone range
        }
    }

    //this goes in iteration loop
    //we don't store starting node as the last element
    DisplayPheromones();
    return result;
}


/********************************************/
/*                                          */
/*           auxiliary functions            */
/*                                          */
/********************************************/
/* delete states - call it at each ant start */

/* reset solData, deleting all vectors during problem change (test problems can be different) */
void resetSolver()
{
    solData.tensor.clear();
    solData.pheromones.clear();
}
void resetSolution(Solution &solution)
{
    solution.cost = numeric_limits<double>::max();
    solution.globalCost = numeric_limits<double>::max();
    solution.carList.resize(solData.problemData.dimension, -1);
    solution.nodeList.resize(solData.problemData.dimension, -1);
}

bool compareNeighbors(TensorCell n1, TensorCell n2)
{
    return (n1.distance < n2.distance);
}

void createTensors()
{
    int i, j;
    std::vector< std::vector< std::vector<int> > >::iterator carIt;
    std::vector< std::vector<int> >::iterator nodeIt;
    std::vector<int>::iterator neighborIt;
    for(carIt=solData.problemData.nonEuclidSection.edgeWeight.begin(), j=0; 
        carIt!=solData.problemData.nonEuclidSection.edgeWeight.end(); 
        ++carIt, j++) //edge weight matrices by car
    {
        TensorCar tensorCar;
        if(static_cast<int>(solData.problemData.carLimits.size())==solData.problemData.cars_number) //if we have passengers and so car limits
            tensorCar.capacity = solData.problemData.carLimits[j];
        else
            tensorCar.capacity = 0; //we have no information about cars' limits
        for(nodeIt=carIt->begin(), i=0; nodeIt!=carIt->end(); ++nodeIt, i++)
        {
            std::vector<TensorCell> tensorNode;
            for(neighborIt=nodeIt->begin(), i=0; neighborIt!=nodeIt->end();++neighborIt, i++)
            {
                TensorCell tensorNeighbor;
                tensorNeighbor.index = i;
                tensorNeighbor.distance = *neighborIt;
                tensorNode.push_back(tensorNeighbor);
            }
            /* tensor cells (neighbors) in each tensor node are sorted */
            sort(tensorNode.begin(),tensorNode.end(), compareNeighbors);
            tensorCar.nodes.push_back(tensorNode);
        }
        solData.tensor.push_back(tensorCar);
    }
    //set pheromones in phero tensor to pheromax for node cars and node neighbors
    for(j=0; j<solData.problemData.dimension; j++)
    {
        PheroNode node;
        for(i=0; i<solData.problemData.cars_number; i++) //cars in node
        {
            PheroCell cell;
            cell.eta = 0.0; //will be set later in heuristics
            cell.tau = PHERO_MAX;
            node.carsPhero.push_back(cell);
        }
        for(i=0; i<solData.problemData.dimension; i++) //neighbors in node
        {
            PheroCell cell;
            cell.eta = 0.0; //will be set later in heuristics
            if(i!=j)
                cell.tau = PHERO_MAX;
            else
                cell.tau = 0.0;
            node.neighborsPhero.push_back(cell);
        }
        solData.pheromones.push_back(node);
    }
    Solution s;
    s.cost = numeric_limits<double>::max();

}


void calculateCarHeuristics()
{
    int c, i, j;
    int sum;
    double avg, d2;
    std::vector<double> carHeuristics;
    if(solData.problemData.edge_weight_type != ET_EXPLICIT || solData.problemData.edge_weight_format != EF_FULL_MATRIX)
        return;
    d2 = 2.0*(double)solData.problemData.dimension;

    /* average transition and return for this car, no max normalization in old algo */
    for(c=0; c<solData.problemData.cars_number; c++)
    {
        avg = 0.0;
        for(j=0; j<solData.problemData.dimension; j++)
        {
            sum = 0;
            for(i=0; i<solData.problemData.dimension; i++)
            {
                sum+=solData.problemData.nonEuclidSection.edgeWeight[c][j][i];
                sum+=solData.problemData.nonEuclidSection.returnRate[c][j][i];
            }
            avg += (double)sum/d2;
        }
        avg /= (double)solData.problemData.dimension;
        carHeuristics.push_back(1.0/avg);
    }
    for(j=0; j<solData.problemData.dimension; j++) //node
        for(i=0; i<solData.problemData.cars_number; i++) //next cars
            solData.pheromones[j].carsPhero[i].eta = carHeuristics[i];
}

/* we have sorted neighbor order */
void calculateNodeHeuristics()
{
    int c, i, j, index;
    //reset all heuriustics - order is not important
    for(j=0; j<solData.problemData.dimension; j++) //node
        for(i=0; i<solData.problemData.dimension; i++) //next nodes (neighbors)
            solData.pheromones[j].neighborsPhero[i].eta = 0.0;
    for(j=0; j<solData.problemData.dimension; j++) //node
    {
        for(i=0; i<solData.problemData.dimension; i++) //next nodes (neighbors)
        {
            for(c=0; c<solData.problemData.cars_number; c++) //average car tranisitions
            {
                index = solData.tensor[c].nodes[j][i].index; //phero nodes are in natural order (not sorted)
                if(j!=index)
                {
                    solData.pheromones[j].neighborsPhero[index].eta += 
                        (1.0/(double)(solData.tensor[c].nodes[j][i].distance));
                }
            }
        }
    }
    for(j=0; j<solData.problemData.dimension; j++)
        for(i=0; i<solData.problemData.dimension; i++)
            solData.pheromones[j].neighborsPhero[i].eta /= solData.problemData.cars_number;
}

void CopySolution(Solution &source, Solution &destination)
{
    destination.dimension = source.dimension;
    destination.cost = source.cost;
    destination.carList = source.carList;
    destination.nodeList = source.nodeList;
    destination.iteration = source.iteration;
}

bool CheckSolutionStall(Parameters &params, Solution &solution)
{
    static uint stallCounter = 0;
    static double lastSolutionPrice = numeric_limits<double>::max();
    if(params.stall<2)
        return false;
    if(solution.cost == lastSolutionPrice)
        stallCounter++;
    else
    {
        lastSolutionPrice = solution.cost;
        stallCounter = 0;
    }
    if(stallCounter > params.stall) //deadlock occured
    {
        stallCounter = 0;
        return true;
    }
    return false;
}


/***********************************************/
/*                                             */
/*       Display pheromone deposition          */
/*                                             */
/***********************************************/
void DisplayPheromones()
{
    int i, j;

    for(j=0; j<solData.pheromones.size(); j++)
    {
        cout << "N" << j << endl;
        for(i=0; i<solData.pheromones[j].carsPhero.size(); i++)
            cout << "C" << i << "(" << solData.pheromones[j].carsPhero[i].tau << ";" << 
            solData.pheromones[j].carsPhero[i].eta << ") ";
        cout << endl;
        for(i=0; i<solData.pheromones[j].neighborsPhero.size(); i++)
            cout << "N" << i << "(" << solData.pheromones[j].neighborsPhero[i].tau << ";" <<
            solData.pheromones[j].neighborsPhero[i].eta << ") ";
        cout << endl;
    }
}

/**********************************/
/*                                */
/*           test part            */
/*                                */
/**********************************/
bool InitSolverTest(ProblemData &problem, Solution &solution)
{
    static TensorCell ass_with_passengers[] = {{50,4362},{66,2705},{60,5085}};
    static TensorCell ass_without_passengers[] = {{372,651},{457,1155}};
    static double ass_pass_sum[] = {39, 31};
    static double ass_car_heuristics_with_passengers[] = {40.8787,0.177037}; //new and old algo
    static double ass_car_heuristics_without_passengers[] = {52.0341,1.37051};
    static double ass_node_heuristics_with_passengers[] = {57.0269, 0.214183};
    static double ass_node_heuristics_without_passengers[] = {305.092, 12.808};

    std::vector<TensorCar>::iterator carIt;
    std::vector< std::vector<TensorCell> >::iterator nodeIt;
    std::vector<TensorCell>::iterator neighborIt;
    std::vector<Budget>::iterator budgetIt;

    std::vector<PheroNode>::iterator pheroCarIt;
    std::vector<PheroNode>::iterator pheroNodeIt;
    std::vector<PheroCell>::iterator pheroCellIt;

    Parameters params;
    int i, j, sum1, sum2;
    double sum3, sum4;

    params.nFavorits = 4; //InitSolver needs it
    params.seed = 0;
    params.startNode = 0;
    if(!InitSolver(problem, params))
        return false;
    
    /* tensor test */
    for(carIt=solData.tensor.begin(), i=0; carIt!=solData.tensor.end(); ++carIt, i++)
    {
        sum1 = sum2 = 0;
        for(nodeIt=carIt->nodes.begin(); nodeIt!=carIt->nodes.end(); ++nodeIt)
        {
            if(static_cast<int>(nodeIt->size())!=problem.dimension)
                return false;
            for(neighborIt=nodeIt->begin()+1; neighborIt!=nodeIt->begin()+params.nFavorits+1; ++neighborIt)
            {
                sum1 += neighborIt->index;
                sum2 += neighborIt->distance;
            }
        }
        if(problem.pass_number > 0)
        {
            if(ass_with_passengers[i].index != sum1 || ass_with_passengers[i].distance != sum2)
                return false;
            sum1 = sum2 = 0;
            for(budgetIt=problem.financialLimits.begin(); budgetIt!=problem.financialLimits.end(); ++budgetIt)
            {
                sum1+=budgetIt->nodeStart;
                sum2+=budgetIt->nodeEnd;
            }
            if((sum1 != ass_pass_sum[0]) || (sum2 != ass_pass_sum[1]))
                return false;
        }
        else
            if(ass_without_passengers[i].index != sum1 || ass_without_passengers[i].distance != sum2)
                return false; 
    }
    /* pheromone tensor - heuristics test */
    sum3 = sum4 = 0.0;
    for(pheroNodeIt=solData.pheromones.begin(), j=0; pheroNodeIt!=solData.pheromones.end(); ++pheroNodeIt, j++)
    {
        for(pheroCellIt=pheroNodeIt->carsPhero.begin(); pheroCellIt!=pheroNodeIt->carsPhero.end(); ++pheroCellIt)
            sum3 += pheroCellIt->eta;
        for(pheroCellIt=pheroNodeIt->neighborsPhero.begin(), i=0; pheroCellIt!=pheroNodeIt->neighborsPhero.end(); ++pheroCellIt, i++)
            if(i!=j) //diagonal
                sum4 += pheroCellIt->eta;
    }
    
    //std::cout << "pass number: " << problem.pass_number << ", sum3: " << sum3 << ", sum4: " << sum4 << std::endl;
    if(problem.pass_number > 0)
    {
        if(std::abs(ass_car_heuristics_with_passengers[0]-sum3) > 0.001 && std::abs(ass_car_heuristics_with_passengers[1]-sum3) > 0.001)
                return false;
        if(std::abs(ass_node_heuristics_with_passengers[0]-sum4) > 0.001 && std::abs(ass_node_heuristics_with_passengers[1]-sum4) > 0.001)
            return false;
    }
    else
    {
        if(std::abs(ass_car_heuristics_without_passengers[0]-sum3) > 0.001 && std::abs(ass_car_heuristics_without_passengers[1]-sum3) > 0.001)
            return false;
        if(std::abs(ass_node_heuristics_without_passengers[0]-sum4) > 0.001 && std::abs(ass_node_heuristics_without_passengers[1]-sum4) > 0.001)
            return false;
    }
    return true;
}
/* main solver test function */
bool SolverTest(Solution &solution, double (*calculateSolutionCost)(ProblemData &problem, Solution &solution))
{
    double result;
    result = calculateSolutionCost(solData.problemData, solution); //use caller's function
    solution.problemName = solData.problemData.name;
    if(std::abs(result - solution.cost) > 0.001 || 
        (solData.problemData.name != "Chile6e" && solData.problemData.name != "BrasilRJ14n"))
        return false;
    if(!ProbabTest(solData.mersenneGenerator))
        return false;
    return true;
}

//for test purposes (statistical)
bool ProbabTest(mt19937 &mersenneGenerator)
{
    bool result;
    int i, p;
    int maxCount, maxCellIndex, maxItemIndex;
    vector<ProbabilityCell> v(7);
    vector<int>counter(6);

    //phase 1 - clean vector and 6 elements, sets minimal size
    AddProbability(v, 0, 3, 0.08);
    AddProbability(v, 1, 4, 0.19);
    AddProbability(v, 2, 6, 0.2);
    AddProbability(v, 3, 7, 0.18);
    AddProbability(v, 4, 8, 0.075);
    AddProbability(v, 5, 9, 0.08);
    AddProbability(v, 6); //end, probability is 0.0 and cumulative is a sum 
    
    for(i=0; i<6; i++)
        counter[i]=0;
    for(i=0; i<1000000; i++)
    {
        p = PickSegment(v, 7, mersenneGenerator);
        counter[p]++;
    }
    result = true;
    if(counter[5]<counter[4] || counter[4]>counter[0] || 
        counter[2]<counter[1] || counter[2]<counter[3] || 
        counter[1]<counter[3])
        result = false;
    
    maxCount = 0;
    maxCellIndex = 0;
    maxItemIndex=0;
    for(i=0; i<6; i++)
        if(counter[i]>maxCount)
        {
            maxCellIndex = i;
            maxCount = counter[i];
            maxItemIndex = v[i].index;
        }
    if(maxCellIndex != 2 || maxItemIndex != 6)
        result = false;

    //phase 2 - clear and use 2 elements only
    counter[0] = counter[1] = 0;
    AddProbability(v, 0, 1, 0.6);
    AddProbability(v, 1, 4, 0.4);
    AddProbability(v, 2);
    for(i=0; i<1000; i++)
    {
        p = PickSegment(v, 3, mersenneGenerator);
        counter[p]++;
    }
    if(counter[0]<counter[1])
        result = false;
    

    //phase 3 - one element only
    v.clear();
    AddProbability(v, 0, 25, 0.6);
    AddProbability(v, 1);
    p = PickSegment(v, 2, mersenneGenerator);
    if(p!=0 || v[p].index !=25)
        result = false;


    return result;
}
