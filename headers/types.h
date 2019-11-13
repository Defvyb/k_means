#ifndef _TYPES_H_
#define _TYPES_H_

#include <fstream>
#include <vector>
static const int MAX_ELEMENT_COUNT = 1000;

struct ProgramOptions final
{
    ProgramOptions(): threadPoolSize(1), centroidsCount(10), maxIterations(1000000)
    {}

    std::ifstream fstream;
    int threadPoolSize;
    uint16_t centroidsCount;
    int maxIterations;

};

typedef std::vector<std::vector<double>> CentroidsType ;
typedef std::vector<std::pair<std::vector<double>, double>> CentroidsSum;


#endif
