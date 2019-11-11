#ifndef _K_MEANS_H_
#define _K_MEANS_H_
#include <types.h>
#include <vector>
#include <unordered_map>

class KMeans final
{
  public:
    KMeans(ProgramOptions & options):m_options(options)
    {
    }

    bool clustering(CentroidsType & centroids) noexcept;

private:
    ProgramOptions & m_options;
    bool inspectFile() noexcept;
    bool initCentroids(CentroidsType & centroids) noexcept;
    bool doClustering(CentroidsType & centroids) noexcept;

    bool calcCentroids(char * lineBuf, std::vector<double> & curPointBuf, CentroidsType & centroids,
                       std::unordered_map<int, std::pair<std::vector<double>, double>> & centroidsSum) noexcept;
    bool centroidsEqual(const CentroidsType & centroidsObjects,const CentroidsType & centroidObjectsNext) noexcept;
    void initCentroids(std::unordered_map<int, std::pair<std::vector<double>, double>> & centroidsSum,
                          const CentroidsType & centroids) noexcept;
    void moveCentroids(std::unordered_map<int, std::pair<std::vector<double>, double>> & centroidsSum,
                          CentroidsType & centroids) noexcept;
    int m_lineCount;
    const int MAX_LINE_LENGTH = 32000;
};

#endif
