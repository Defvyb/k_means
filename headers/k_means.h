#ifndef _K_MEANS_H_
#define _K_MEANS_H_
#include <types.h>
#include <vector>
#include <thread_pool.hpp>

class KMeans final
{
  public:
    KMeans(ProgramOptions & options):m_options(options), pool(m_options.threadPoolSize)
    {
    }

    bool clustering(CentroidsType & centroids) noexcept;

private:
    ProgramOptions & m_options;
    bool inspectFile() noexcept;
    bool obtainStartCentroids(CentroidsType & centroids) noexcept;
    bool doClustering(CentroidsType & centroids) noexcept;

    bool calcCentroids(char * lineBuf, std::vector<double> & curPointBuf, CentroidsType & centroids,
                       std::vector<std::pair<std::vector<double>, double>> & centroidsSum,
                        std::vector<double> & dists) noexcept;
    bool centroidsEqual(const CentroidsType & centroidsObjects,const CentroidsType & centroidObjectsNext) noexcept;
    void initCentroids(std::vector<std::pair<std::vector<double>, double>> & centroidsSum,
                          const CentroidsType & centroids) noexcept;
    void moveCentroids(std::vector<std::pair<std::vector<double>, double>> & centroidsSum,
                          CentroidsType & centroids) noexcept;

    const int MAX_LINE_LENGTH = 32000;
    int m_lineCount;
    ThreadPool pool;
};

#endif
