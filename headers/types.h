#ifndef _TYPES_H_
#define _TYPES_H_

#include <vector>
#include <fstream>
#include <functional>
static const int MAX_ELEMENT_COUNT = 1000;
const int MAX_LINE_LENGTH = 32000;
struct ProgramOptions final
{
    ProgramOptions(): threadPoolSize(1), centroidsCount(10), maxIterations(1000000), outputFileName("output.file"), checkFile(true)
    {}

    std::ifstream fstream;
    int threadPoolSize;
    uint16_t centroidsCount;
    int maxIterations;
    std::string outputFileName;
    bool checkFile;

};

typedef std::vector<std::vector<double>> CentroidsType ;
typedef std::vector<std::vector<double>> CentroidsSum;
typedef std::vector<int> CentroidsSumCount;

typedef std::function<bool (CentroidsType & , ProgramOptions & , int ) > StartCentroidsObtainer;


#endif
