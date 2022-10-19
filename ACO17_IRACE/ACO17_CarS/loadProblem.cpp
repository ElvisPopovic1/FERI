#include "ACO.h"

void ClearProblem(ProblemData &problem);
bool ParseMatrice(std::vector< std::vector< std::vector<int> > > &matrice, std::ifstream &dataFile, int dim, int cars);
bool ParseCoordinates(std::vector< std::vector<float> > &coordinates, std::ifstream &dataFile, int dim);
bool ParseVectors(std::vector< std::vector<int> >  &eucVector, std::ifstream &dataFile, int dim, int cars);
bool ParseVector(std::vector<int> &limVector, std::ifstream &dataFile, int cars);
bool ParseFinancialLimits(std::vector<Budget> &financLimits, std::ifstream &dataFile, int passengers);

/*********************************************************************/
/*                                                                   */
/*                   problem parser main function                    */
/*                                                                   */
/*********************************************************************/
bool LoadProblem(std::string filename, ProblemData &problem)
{
    std::string line, token, svalue;
    std::stringstream ss;
    std::ifstream dataFile(filename);

    problem.edge_weight_type = ET_NONE;
    problem.edge_weight_format = EF_NONE;
    if(!dataFile.is_open())
    {
        std::cerr << filename << ": " << strFileNotOpened << std::endl;
        return false;
    }
    ClearProblem(problem); //reset
    while(std::getline(dataFile, line))
    {
        clearStringStream(ss,line);
        token = "";
        ss >> token;
        if((token == "NAME") && parseColon(token, ss))
            ss >> problem.name;
        else if((token == "TYPE") && parseColon(token, ss))
            ss >> problem.type;
        else if((token == "COMMENT") && parseColon(token, ss))
            getline(ss, problem.comment);
        else if((token == "DIMENSION") && parseColon(token, ss))
            ss >> problem.dimension;
        else if((token == "CARS_NUMBER") && parseColon(token, ss))
            ss >> problem.cars_number;
        else if((token == "PASSENGERS_NUMBER") && parseColon(token, ss))
            ss >> problem.pass_number;
        else if((token == "EDGE_WEIGHT") && parseColon(token, ss))
        {
            ss >> svalue;
            if(svalue == "SYMMETRIC")
                problem.edge_weight = SM_SYMMETRIC;
            else if(svalue == "ASYMMETRIC")
                problem.edge_weight = SM_ASIMMETRIC;
        }
        else if((token == "RETURN_RATE") && parseColon(token, ss))
        {
            ss >> svalue;
            if(svalue == "SYMMETRIC")
                problem.return_rate = SM_SYMMETRIC;
            else if(svalue == "ASYMMETRIC")
                problem.return_rate = SM_ASIMMETRIC;
        }   
        else if((token == "EDGE_WEIGHT_TYPE") && parseColon(token, ss))
        {
            ss >> svalue;
            if(svalue == "EXPLICIT")
                problem.edge_weight_type = ET_EXPLICIT;
            else if(svalue == "EUC_2D")
                problem.edge_weight_type = ET_EUC_2D;
        }
        else if((token == "EDGE_WEIGHT_FORMAT") && parseColon(token, ss))
        {
            ss >> svalue;
            if(svalue == "FULL_MATRIX")
                problem.edge_weight_format = EF_FULL_MATRIX;
            else if(svalue == "VECTOR")
                problem.edge_weight_format = EF_VECTOR;
        }
        else if(token == "PASSENGERS_LIMIT")
        {
            if(ParseVector(problem.carLimits, dataFile, problem.cars_number)==false)
            {
                dataFile.close();;
                return false;
            }
        }
        else if(token == "ORIGINS_DESTINATIONS_AND_FINANCIAL_LIMITS")
        {
            if(ParseFinancialLimits(problem.financialLimits, dataFile, problem.pass_number)==false)
            {
                dataFile.close();
                return false;
            }
        }
        else if(problem.edge_weight_type==ET_EXPLICIT && problem.edge_weight_format==EF_FULL_MATRIX)
        {
            bool res = true;
            if(token == "EDGE_WEIGHT_SECTION")
            {
                if(ParseMatrice(problem.nonEuclidSection.edgeWeight, dataFile, problem.dimension, problem.cars_number)==false)
                {
                    res = false;
                    std::cerr << "Edge weight: " << strMatrixReadError << std::endl;  
                }
            }
            else if(token == "RETURN_RATE_SECTION")
            {
                if(ParseMatrice(problem.nonEuclidSection.returnRate, dataFile, problem.dimension, problem.cars_number)==false)
                {
                    res = false;
                    std::cerr << "Return rate: " << strMatrixReadError << std::endl; 
                }
            }
            if(!res)
            {
                dataFile.close();
                return false;
            }
        }
        else if(problem.edge_weight_type==ET_EUC_2D && problem.edge_weight_format==EF_VECTOR)
        {
            bool res = true;
            if(token == "NODE_COORD_SECTION")
            {
                if(ParseCoordinates(problem.euclidSection.nodeCoordSection, dataFile, problem.dimension)==false)
                {
                    res = false;
                    std::cerr << "Node coord: " << strCoordReadError << std::endl; 
                }
            }
            else if(token == "EDGE_WEIGHT_SECTION")
            {
                if(ParseVectors(problem.euclidSection.edgeWeight, dataFile, problem.dimension, problem.cars_number)==false)
                {
                    res = false;
                    std::cerr << "Edge weight: " << strVectorReadError << std::endl;
                }
            }
            else if(token == "RETURN_RATE_SECTION")
            {
                if(ParseVectors(problem.euclidSection.returnRate, dataFile, problem.dimension, problem.cars_number)==false)
                {
                    res = false;
                    std::cerr << "Return rate: " << strVectorReadError << std::endl;
                }
            }
            if(!res)
            {
                dataFile.close();
                return false;
            }
        }
    }
    dataFile.close();
    return true;
}

/*********************************************************************/
/*                                                                   */
/*   auxiliary functions required by the loadProblem funcion         */
/*                                                                   */
/*********************************************************************/
bool parseColon(std::string &token, std::stringstream &ss)
{
    std::string drop;
    ss >> drop;
    return (drop == ":");
}

void ClearProblem(ProblemData &problem)
{
    problem.name = "";
    problem.comment = "";
    problem.dimension = 0;
    problem.edge_weight = SM_NONE;
    problem.edge_weight_format = EF_NONE;
    problem.edge_weight_type = ET_NONE;
    problem.return_rate = SM_NONE;
    problem.pass_number = 0;
    problem.type = "";
    problem.carLimits.clear();
    problem.euclidSection.edgeWeight.clear();
    problem.euclidSection.nodeCoordSection.clear();
    problem.euclidSection.returnRate.clear();
    problem.nonEuclidSection.edgeWeight.clear();
    problem.nonEuclidSection.returnRate.clear();
    problem.financialLimits.clear();
}

void clearStringStream(std::stringstream &ss, std::string lineString)
{
    ss.str(std::string()); //clears content
    ss.clear(); //clears error state
    ss << lineString;
}

bool ParseMatrice(std::vector< std::vector< std::vector<int> > > &matrice, std::ifstream &dataFile, int dim, int cars)
{
    int i, j, k, value;
    std::string lineString;
    std::stringstream ss;
    for(k=0; k<cars; k++)
    {
        std::vector< std::vector<int> > matrix;
        if(!getline(dataFile, lineString))
            return false; //fatal failure
        clearStringStream(ss,lineString);
        ss >> value;
        if(value != k)
            return false;
        for(j=0; j<dim; j++)
        {
            std::vector<int> row;
            if(!getline(dataFile, lineString))
                return false;
            clearStringStream(ss,lineString);
            for(i=0; i<dim; i++)
            {
                if(ss >> value)
                    row.push_back(value);
            }
            matrix.push_back(row);
        }  
        matrice.push_back(matrix);
    }
    return true;
}

bool ParseCoordinates(std::vector< std::vector<float> > &coordinates, std::ifstream &dataFile, int dim)
{
    int i, j;
    float value;
    std::string lineString;
    std::stringstream ss;
    for(j=0; j<dim; j++)
    {
        std::vector<float> row;
        if(!getline(dataFile, lineString))
            return false; //fatal failure
        clearStringStream(ss,lineString);
        ss >> value;
        if(value != j)
            return false;
        for(i=0; i<2; i++)
        {
            if(ss >> value)
                row.push_back(value);
        }
        coordinates.push_back(row);
    }
    return true;
}

bool ParseVectors(std::vector< std::vector<int> >  &eucVector, std::ifstream &dataFile, int dim, int cars)
{
    int i, j;
    int value;
    std::string lineString;
    std::stringstream ss;
    for(j=0; j<cars; j++)
    {
        std::vector<int> v;
        if(!getline(dataFile, lineString))
            return false; //fatal failure
        clearStringStream(ss,lineString);
        ss >> value;
        if(value != j)
            return false;
        if(!getline(dataFile, lineString))
            return false; //fatal failure
        clearStringStream(ss,lineString);
        for(i=0; i<dim; i++)
        {
            if(ss >> value)
                v.push_back(value);
        }
        eucVector.push_back(v);
    }
    return true;
}

bool ParseVector(std::vector<int> &limVector, std::ifstream &dataFile, int cars)
{
    int i, value;
    std::string lineString;
    std::stringstream ss;
    if(!getline(dataFile, lineString))
        return false; //fatal failure
    ss << lineString;
    for(i=0; i<cars; i++)
    {
        if(ss >> value)
            limVector.push_back(value);
    }
    return true;
}

bool ParseFinancialLimits(std::vector<Budget> &financLimits, std::ifstream &dataFile, int passengers)
{
    int i;
    std::string lineString;
    std::stringstream ss;
    for(i=0; i<passengers; i++)
    {
        Budget budget;
        if(!getline(dataFile, lineString))
            return false; //fatal failure
        clearStringStream(ss,lineString);
        if((ss >> budget.nodeStart) && (ss >> budget.nodeEnd) && (ss >> budget.budget))
            financLimits.push_back(budget);
    }
    return true;
}

/**********************************/
/*                                */
/*           test part            */
/*                                */
/**********************************/
bool LoadProblemTest()
{
    int i, j, k, sum, count;
    float fsum;
    ProblemData problem;
    /* Afe4na.car part */
    if(!LoadProblem("testFiles/Afe4na.car", problem))
        return false;
    if(problem.name != "Afeganistao4n" || problem.type != "CaRSP" || 
        problem.edge_weight != SM_ASIMMETRIC || problem.return_rate != SM_ASIMMETRIC ||
        problem.edge_weight_type != ET_EXPLICIT || problem.edge_weight_format != EF_FULL_MATRIX)
        return false;
    
    if(problem.dimension != 4 || problem.cars_number != 2 || problem.pass_number != 10)
        return false;
    sum = count = 0;
    for(k=0; k<problem.cars_number; k++)
    {
        for(j=0; j<problem.dimension; j++)
        {
            for(i=0; i<problem.dimension; i++)
            {
                sum += problem.nonEuclidSection.edgeWeight[k][j][i];
                sum += problem.nonEuclidSection.returnRate[k][j][i];
                count++;
            }
        }
    }
    if(count != 32 || sum != 6860)
        return false;
    sum = 0;
    for(i=0; i<problem.cars_number; i++)
        sum+=problem.carLimits[i];
    if(sum != 9)
        return false;
    sum = 0; fsum = 0.0;
    for(i=0; i<problem.pass_number; i++)
    {
        sum += (problem.financialLimits[i].nodeStart + problem.financialLimits[i].nodeEnd);
        fsum += problem.financialLimits[i].budget;
    }
    if(sum != 23 || std::abs(fsum-368.503) > 0.001)
        return false;


    /* Egito9e.car part */
    if(!LoadProblem("testFiles/Egito9e.car", problem))
        return false;
    if(problem.name != "Egito9e" || problem.type != "CaRS" || 
        problem.edge_weight_type != ET_EUC_2D || problem.edge_weight_format != EF_VECTOR)
        return false;
    if(problem.dimension != 9 || problem.cars_number != 4)
        return false;
    
    sum = count = 0;
    for(j=0; j<problem.dimension; j++)
        for(i=0; i<2; i++)
        {
            count ++;
            sum += problem.euclidSection.nodeCoordSection[j][i];
        }
    if(count != 18 || sum != 935)
        return false;
    sum = count = 0;
    for(j=0; j<problem.cars_number; j++)
    {
        for(i=0; i<problem.dimension; i++)
        {
            count ++;
            sum += problem.euclidSection.edgeWeight[j][i];
            sum += problem.euclidSection.returnRate[j][i];
        }
    }
    if(count != 36 || sum != 2672)
        return false;

    /* Egito9n.car part */
    problem.nonEuclidSection.edgeWeight.clear(); //clear dirty vectors from Afe4na.car part
    problem.nonEuclidSection.returnRate.clear();
    if(!LoadProblem("testFiles/Egito9n.car", problem))
        return false;
    if(problem.name != "Egito9n" || problem.type != "CaRS" )
        return false;
    if(problem.dimension != 9 || problem.cars_number != 4 || 
        problem.edge_weight_type != ET_EXPLICIT || problem.edge_weight_format != EF_FULL_MATRIX)
        return false;
    sum = count = 0;

    for(k=0; k<problem.cars_number; k++)
    {
        for(j=0; j<problem.dimension; j++)
        {
            for(i=0; i<problem.dimension; i++)
            {
                sum += problem.nonEuclidSection.edgeWeight[k][j][i];
                sum += problem.nonEuclidSection.returnRate[k][j][i];
                count++;
            }
        }
    }
     if(count != 324 || sum != 93381)
        return false;
    
    return true;
}
