#ifndef _K_MEANS_H_
#define _K_MEANS_H_
#include <types.h>
#include <vector>
#include <thread_pool.hpp>
struct Stat
{
    int m_iterations;
    int m_duration;
    int getDurationPerIteration()
    {
        return m_duration/m_iterations;
    }
};




class KMeans final
{
  public:
    explicit KMeans(ProgramOptions & options, StartCentroidsObtainer startCentroidsObtainer = KMeans::defaultKMeansStartCentroidsObtainer):m_options(options), m_pool(nullptr),
        m_startCentroidsObtainer(startCentroidsObtainer)
    {
    }
    ~KMeans()
    {
        if(m_pool) delete m_pool;
    }
    KMeans(KMeans const&) = delete;
    KMeans& operator=(KMeans const&) = delete;



    bool clustering(CentroidsType & centroids) noexcept;
    Stat getStat() const noexcept;

private:
    ProgramOptions & m_options;
    bool inspectFile() noexcept;
    bool doClustering(CentroidsType & centroids) noexcept;

    bool calcCentroids(char * lineBuf, std::vector<double> & curPointBuf, CentroidsType & centroids,
                        CentroidsSum & centroidsSum,
                        std::vector<double> & dists) noexcept;
    bool centroidsEqual(const CentroidsType & centroidsObjects,const CentroidsType & centroidObjectsNext) noexcept;
    void initCentroids(CentroidsSum & centroidsSum,
                          const CentroidsType & centroids) noexcept;
    void moveCentroids(CentroidsSum & centroidsSum,
                          CentroidsType & centroids) noexcept;

    static bool defaultKMeansStartCentroidsObtainer(CentroidsType & centroids, ProgramOptions & options, int lineCount) noexcept;

    StartCentroidsObtainer m_startCentroidsObtainer;

    int m_lineCount;
    int m_iterations;

    Stat m_stat;

    ThreadPool * m_pool;
};

#endif
