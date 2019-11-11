#ifndef _TYPES_H_
#define _TYPES_H_

#include <fstream>

static const int MAX_ELEMENT_COUNT = 1000;

struct ProgramOptions final
{
    ProgramOptions(): threadPoolSize(1), klusterCentroidsCount(10), maxIterations(10000)
    {}

    std::ifstream fstream;
    int threadPoolSize;
    uint16_t klusterCentroidsCount;
    int maxIterations;

};



#endif
