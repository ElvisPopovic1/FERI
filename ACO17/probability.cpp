#include "CreateSolution.h"

/* fills probability array, for sake of speed, no bound tests */
void AddProbability(std::vector<ProbabilityCell> &cells, int cellIndex, int itemIndex, double value)
{
    int i1;
    if(cellIndex == 0)
        cells[cellIndex].cumulative = 0.0;
    else
    {
        i1 = cellIndex-1;
        cells[cellIndex].cumulative = cells[i1].cumulative+cells[i1].value;
    }   
    cells[cellIndex].index = itemIndex;     
    cells[cellIndex].value = value;
}

/* adds border element, probability is 0 */
/* PickSegment needs it as well as sum of all elements */
void AddProbability(std::vector<ProbabilityCell> &cells, int cellIndex)
{
    AddProbability(cells, cellIndex, 0, 0.0);
}

//divide and conquer method of picking segment
int PickSegment(std::vector<ProbabilityCell> &cells, int size, std::mt19937 &mersenneGenerator)
{
    int i1, l, r, m; //indice, left, right and middle
    double e, p; //range (end cumulative) and pick
    if(size<2)
        return -1; //error
    if(size==2) //must have at least one element and end - returns 0th index
        return 0;
    i1 = size-1; //last index
    e = nextafter(cells[i1].cumulative, std::numeric_limits<double>::max());
    std::uniform_real_distribution<double> distribution((double)0.0,e);
    p = distribution(mersenneGenerator);
    //loop
    l=0;
    r=i1;
    while((r-l)>1)
    {
        m=(l+r)/2;
        if(p >= cells[m].cumulative)
            l=m;
        else
            r=m;
    }
    return l;
}
