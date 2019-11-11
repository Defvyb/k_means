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

    bool clustering(std::unordered_map<int, std::vector<double>> & centroids) noexcept;

private:
    ProgramOptions & m_options;
    bool inspectFile() noexcept;
    bool initCentroids(std::unordered_map<int, std::vector<double>> & centroids) noexcept;
    bool doClustering(std::unordered_map<int, std::vector<double>> & centroids) noexcept;
    int m_lineCount;
    const int MAX_LINE_LENGTH = 32000;
};

#endif
