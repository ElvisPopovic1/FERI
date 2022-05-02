#include "ACO.h"

using namespace std;

char headerTokens[][48] =
{
    "NAME","TYPE","COMMENT","DIMENSION","CARS_NUMBER","PASSENGERS_NUMBER","EDGE_WEIGHT",
    "RETURN_RATE","EDGE_WEIGHT_TYPE","EDGE_WEIGHT_FORMAT","EDGE_WEIGHT_SECTION",
    "RETURN_RATE_SECTION","PASSENGERS_LIMIT","ORIGINS_DESTINATIONS_AND_FINANCIAL_LIMITS",
    "EOF"
};

parameters parData;
problemData prData;

int parseHeaderToken(const char* name);
bool parseMatrix1(ifstream& dataFile, float*** matrix, int nMatrix, int dim);
bool parseMatrix2(ifstream& dataFile, float*** matrix, int dim);
void allocateMatrices(int nCars, int dim);
void fillReturnRates();

bool setParameters(int argc, char** argv)
{
    int i;
    float p1;
    uint p2;
    int p3;
    parData.argc = argc;
    cout << "Arg: " << argv[1] << endl;
    if(argv[1][0]!='-')
        strncpy(parData.fileName, argv[1], 127);
    else 
        for(i=1; i<argc; i++)
            if(strcmp(argv[i],"-d")==0 && i<argc-1)
                strncpy(parData.fileName, argv[i+1], 127);
    /* default values */
    parData.seed = SEED;
    parData.startNode = START_NODE;
    parData.rho1 =  RHO1;
    parData.rho2 =  RHO2;
    parData.rho3 =  RHO3;
    parData.alpha1 = ALPHA1;
    parData.beta1 =  BETA1;
    parData.alpha2 = ALPHA2;
    parData.beta2 =  BETA2;
    parData.alpha3 = ALPHA3;
    parData.beta3 =  BETA3;
    parData.nAnts = N_ANTS;
    parData.nIter = N_ITER;
    parData.stall = STALL;
    parData.nPassAnts = N_PASS_ANTS;
    parData.nPassIters = N_PASS_ITER;
    parData.opt =   false;
    parData.pass =  false;
    parData.writeData = false;
    parData.nFavorits = N_FAVORITS;
    parData.nExecutions = N_EXEC;
    
    for(i=1; i<argc; i++)
    {
        if(strcmp(argv[i],"--write")==0)
            parData.writeData = true; 
        if(strcmp(argv[i],"--opt")==0)
            parData.opt = true;
        if(strcmp(argv[i],"--pass")==0)
            parData.pass = true;
        if((strcmp(argv[i],"--seed")==0 || strcmp(argv[i],"-s")==0) && i<argc-1)
        {
            p3 = atoi(argv[i+1]);
            if(p3 > -1)
                parData.seed = p3;           
        }
        if((strcmp(argv[i],"--startNode")==0 || strcmp(argv[i],"-sn")==0) && i<argc-1)
        {
            p3 = atoi(argv[i+1]);
            if(p3 < -1)
                parData.startNode = -1;
            else
                parData.startNode = p3;        
        }
        if((strcmp(argv[i],"--rhoNode")==0 || strcmp(argv[i],"-r1")==0) && i<argc-1)
        {
            p1 = atof(argv[i+1]);
            if(p1 > 0.0)
                parData.rho1 = p1;           
        }
        if((strcmp(argv[i],"--rhoCar")==0 || strcmp(argv[i],"-r2")==0) && i<argc-1)
        {
            p1 = atof(argv[i+1]);
            if(p1 > 0.0)
                parData.rho2 = p1;           
        }
        if((strcmp(argv[i],"--rhoPass")==0 || strcmp(argv[i],"-r3")==0) && i<argc-1)
        {
            p1 = atof(argv[i+1]);
            if(p1 > 0.0)
                parData.rho3 = p1;           
        }
        if((strcmp(argv[i],"--alphaNode")==0 || strcmp(argv[i],"-a1")==0) && i<argc-1)
        {
            p1 = atof(argv[i+1]);
            if(p1 > 0.0)
                parData.alpha1 = p1;           
        }
        if((strcmp(argv[i],"--betaNode")==0 || strcmp(argv[i],"-b1")==0) && i<argc-1)
        {
            p1 = atof(argv[i+1]);
            if(p1 > 0.0)
                parData.beta1 = p1;           
        }
        if((strcmp(argv[i],"--alphaCar")==0 || strcmp(argv[i],"-a2")==0) && i<argc-1)
        {
            p1 = atof(argv[i+1]);
            if(p1 > 0.0)
                parData.alpha2 = p1;           
        }
        if((strcmp(argv[i],"--betaCar")==0 || strcmp(argv[i],"-b2")==0) && i<argc-1)
        {
            p1 = atof(argv[i+1]);
            if(p1 > 0.0)
                parData.beta2 = p1;           
        }
        if((strcmp(argv[i],"--alphaPass")==0 || strcmp(argv[i],"-a3")==0) && i<argc-1)
        {
            p1 = atof(argv[i+1]);
            if(p1 > 0.0)
                parData.alpha3 = p1;           
        }
        if((strcmp(argv[i],"--betaPass")==0 || strcmp(argv[i],"-b3")==0) && i<argc-1)
        {
            p1 = atof(argv[i+1]);
            if(p1 > 0.0)
                parData.beta3 = p1;           
        }
        if((strcmp(argv[i],"--ants")==0 || strcmp(argv[i],"-na")==0) && i<argc-1)
        {
            p2 = (uint)atoi(argv[i+1]);
            if(p2 > 0)
                parData.nAnts = p2;           
        }

        if((strcmp(argv[i],"--iter")==0 || strcmp(argv[i],"-ni")==0) && i<argc-1)
        {
            p2 = (uint)atoi(argv[i+1]);
            if(p2 > 0)
                parData.nIter = p2;           
        }
        if((strcmp(argv[i],"--favorits")==0 || strcmp(argv[i],"-nfav")==0) && i<argc-1)
        {
            p2 = (uint)atoi(argv[i+1]);
            if(p2 > 0)
                parData.nFavorits = p2;           
        }
        if((strcmp(argv[i],"--executions")==0 || strcmp(argv[i],"-nexec")==0) && i<argc-1)
        {
            p2 = (uint)atoi(argv[i+1]);
            if(p2 > 0)
                parData.nExecutions = p2;           
        }
        if((strcmp(argv[i],"--stall")==0 || strcmp(argv[i],"-st")==0) && i<argc-1)
        {
            p2 = (uint)atoi(argv[i+1]);
            if(p2 > 0)
                parData.stall = p2;           
        }
        if((strcmp(argv[i],"--passAnts")==0 || strcmp(argv[i],"-npa")==0) && i<argc-1)
        {
            p2 = (uint)atoi(argv[i+1]);
            if(p2 > 0)
                parData.nPassAnts = p2;           
        }
        if((strcmp(argv[i],"--passIter")==0 || strcmp(argv[i],"-npi")==0) && i<argc-1)
        {
            p2 = (uint)atoi(argv[i+1]);
            if(p2 > 0)
                parData.nPassIters = p2;           
        }
    }

    return true;
};

bool loadData(const char* filename)
{
    uint i, j;
    string line, element;
    uchar* uchPt;
    dataPass* ptPass;
    int token, pos, pos2;
    ifstream dataFile(filename);
    if(!dataFile.is_open())
        return false;
    prData.nCars = 1;       // at least one car must exist (allocate matrices depends on at least one car)
    prData.nPass = 0;       // zero passengers by default
    while(getline(dataFile, line))
    {
        /* try to find token with border " : " */
        
        pos=line.find(" : ");
        if(pos < 0)
        {
            pos=line.find(": ");
            pos2 = pos+2;
        }
        else pos2=pos+3;
        token = parseHeaderToken(line.substr(0, pos).c_str());
        if(pos>0 && (int)line.size() > pos2)
        {
            element = line.substr(pos2);
            switch(token)
            {
            case 0: strncpy(prData.name, element.c_str(), element.size());
                    break;
            case 1: strncpy(prData.type, element.c_str(), element.size());
                    break;
            case 2: strncpy(prData.comment, element.c_str(), element.size());
                    break;
            case 3: prData.dim = stoi(element); // dimension
                    break;
            case 4: prData.nCars = stoi(element); // cars number
                    break;
            case 5: prData.nPass = stoi(element);
                    if(prData.nPass > 0)
                        prData.passengers = new dataPass[prData.nPass];
                    break;
            case 6: if(strncmp(element.c_str(), "SYMMETRIC", 10)==0)
                        prData.edgeWeight = SYMMETRIC;
                    else
                        prData.edgeWeight = ASYMMETRIC;
            case 7: if(strncmp(element.c_str(), "SYMMETRIC", 10)==0)
                        prData.returnRate = SYMMETRIC;
                    else
                        prData.returnRate = ASYMMETRIC;
            case 8: //edge weight type and edge weight format we don't use
            case 9: break;
            }
        }
        else if(pos < 0 && (int)line.find(" ")==-1)
        {
            switch(token)
            {
            case 10:    // edge weight section
                allocateMatrices(prData.nCars, prData.dim);
                if(strncmp(prData.type, "CaRSP", 6)==0 || strncmp(prData.type, "CaRS", 6)==0) // if CarSP or CaRS, we have cars
                    parseMatrix1(dataFile, prData.edgeWeightMatrices, prData.nCars, prData.dim);
                else if(strncmp(prData.type, "ATSP",5)==0)   // if not, use different matrix parser
                {
                    parseMatrix2(dataFile, prData.edgeWeightMatrices, prData.dim);
                    fillReturnRates();  // construct missing return rates
                }
                break;
            case 11:    // return rates section
                if(strncmp(prData.type, "CaRSP", 6)==0 || strncmp(prData.type, "CaRS", 6)==0)
                    parseMatrix1(dataFile, prData.returnRateMatrices, prData.nCars, prData.dim);
                break;
            case 12:    // passenger limit
                if(prData.dim > 0)
                    prData.carPassLimit = new uchar[prData.nCars];
                getline(dataFile, line);
                pos = 0;
                for(i=0, uchPt=prData.carPassLimit; i<prData.nCars; i++, uchPt++)
                {
                    pos2 = line.find(" ", pos);
                    *uchPt = (uchar)stoi(line.substr(pos,pos2-pos));
                    pos=pos2+1;
                }
                break;
            case 13: // origins destination and financial limits
                for(j=0, ptPass=prData.passengers; j<prData.nPass; j++, ptPass++)
                {
                    pos = 0;
                    getline(dataFile, line);
                    pos2 = line.find(" ",pos);
                    ptPass->startNode = (uint)stoi(line.substr(pos,pos2-pos));
                    pos = pos2+1;
                    pos2 = line.find(" ",pos);
                    ptPass->destinationNode = (uint)stoi(line.substr(pos,pos2-pos));
                    pos = pos2+1;
                    ptPass->budget = stof(line.substr(pos));
                }
                break;
            case 14: // eof
                if(prData.carPassLimit == nullptr) //not found passengers limits in file (no cars)
                {
                    if(prData.nCars == 0)
                    {
                        prData.carPassLimit = new uchar[1]; //at least one car must exist
                        prData.carPassLimit[0] = 1; // and set limit to 1
                    }
                    else
                    {
                        prData.carPassLimit = new uchar[prData.nCars];
                        for(i=0, uchPt=prData.carPassLimit; i<prData.nCars; i++, uchPt++)
                            *uchPt = 1;
                    }
                    
                }
                dataFile.close();
                return true;
            }
        }
        // if there are no cars, we set one from imagination
    };
    if(prData.carPassLimit == nullptr) //not found passengers limits in file, no EOF
    {
        if(prData.nCars == 0)
        {
            prData.carPassLimit = new uchar[1]; //at least one car must exist
            prData.carPassLimit[0] = 1; // and set limit to 1
        }
        else
        {
            prData.carPassLimit = new uchar[prData.nCars];
            for(i=0, uchPt=prData.carPassLimit; i<prData.nCars; i++, uchPt++)
                *uchPt = 1;
        }
    }
    dataFile.close();
    return false;
}

void freeData()
{
    uint i, j;
    if(prData.edgeWeightMatrices != nullptr)
    {
        for(j=0; j<prData.nCars; j++)
        {
            for(i=0; i<prData.dim; i++)
            {
                delete[] prData.edgeWeightMatrices[j][i];
                delete[] prData.returnRateMatrices[j][i];
            }
            delete[] prData.edgeWeightMatrices[j];
            delete[] prData.returnRateMatrices[j];
        }
        delete[] prData.edgeWeightMatrices;
        delete[] prData.returnRateMatrices;
    }
    if(prData.carPassLimit != nullptr)
        delete[] prData.carPassLimit;
    if(prData.passengers != nullptr)
        delete[] prData.passengers;
}

int parseHeaderToken(const char* name)
{
    uchar length, tokenLength, i;

    tokenLength = sizeof(headerTokens[0]);
    length = sizeof(headerTokens)/tokenLength;
    for(i=0; i<length; i++)
    {
        if(strncmp((char*)name, headerTokens[i], tokenLength)==0)
            return i;
    }
    return -1;
}

/* parsing matrices in CARSP formatu */
bool parseMatrix1(ifstream& dataFile, float*** matrix, int nMatrix, int dim)
{
    int i,j,k, res;
    float *pt1, **pt2, ***pt3;
    float matValue;
    string line;
    stringstream ss;
    for(k=0, pt3=matrix; k<nMatrix; k++, pt3++)
    {
        getline(dataFile, line);
        ss.clear();
        ss << line; //ucitava broj matrice (automobili)
        ss >> res;
        if(res < 0 || res >= nMatrix)   //mora biti manje od broja matrica
            return false;
        for(j=0, pt2=*pt3; j<dim; j++, pt2++)
        {
            getline(dataFile, line);
            ss.clear();
            ss << line;
           for(i=0, pt1=*pt2; i<dim; i++, pt1++)
           {
               ss >> matValue;
               *pt1 = matValue;
           }            
        }
    }
    return true;
}

/* parsiranje matrice u TSPLIB formatu */
/* dotjerati na isti nacin kao i parseMatrix1 koristeci stringstream */
bool parseMatrix2(ifstream& dataFile, float*** matrix, int dim)
{
    string line;
    int pos, pos2, n, n1, row, column;
    n1 = dim*dim;
    n = 0;
    row = 0;
    column = 0;
    do
    {
        getline(dataFile, line);
        pos = 0;
        do
        {
            while(line[pos]==' ') //leading spaces
                pos++;
            pos2 = line.find(" ", pos); //find space on other side
            if(pos2 ==-1) // if not found, try find end of line
                pos2 = line.find("\n", pos);
            matrix[0][row][column] = (float)stoi(line.substr(pos,pos2-pos));
            n++;
            column++;
            if(column == dim)
            {
                row++;
                column = 0;
            }
            if(pos2 >= (int)line.length()-1)
                break;
            pos=pos2+1;
        }while(pos > 0);
    }while(n<n1);
    return true;
}

/* TSPLIB has not return matrix so we construct one */
void fillReturnRates()
{
    uint i, j;
    for(j=0; j < prData.dim; j++)
        for(i=0; i < prData.dim; i++)
            prData.returnRateMatrices[0][j][i]=0.0;
}

void allocateMatrices(int nCars, int dim)
{
    uint i, j;
    prData.edgeWeightMatrices = new float**[nCars];
    prData.returnRateMatrices = new float**[nCars];
    for(j=0; j<prData.nCars; j++)
    {
        prData.edgeWeightMatrices[j] = new float*[dim];
        prData.returnRateMatrices[j] = new float*[dim];
        for(i=0; i<prData.dim; i++)
        {
            prData.edgeWeightMatrices[j][i] = new float[dim];
            prData.returnRateMatrices[j][i] = new float[dim];
        }
    }
}