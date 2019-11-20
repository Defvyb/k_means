#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <thread>
#include <atomic>
#include <math.h>
#include <types.h>

static double tpCompute(const std::vector<double> * pointDimensions, const std::vector<double> & centerDimentions ) noexcept
{
    auto pointDim = pointDimensions->cbegin();
    auto centerDim = centerDimentions.cbegin();

    double sumOfPow = 0;
    for(;pointDim != pointDimensions->cend() && centerDim != centerDimentions.cend();
        ++pointDim, ++centerDim)
    {
        sumOfPow += pow((*pointDim - *centerDim),2.0);
    }
    return sqrt(sumOfPow);

}

class ThreadPool final
{
public:
    explicit ThreadPool(size_t threads,
               CentroidsType  & centroids,
               std::vector<double> & centroidsDistances)
        :m_stop(false),
          m_pointDimensions(nullptr),
          m_centroids(centroids),
          m_centroidsDistances(centroidsDistances),
          m_threads(threads),
          readyMask(0),
          act(0)
    {
        for(int i=0; i<m_threads; ++i)
        {
            readyMask |= 1<<i;
        }

        for(size_t i = 0; i<threads ;++i)
            workers.emplace_back(
                [this, i, threads]
                {
                    for(;;)
                    {
                        if( act & (1 << i))
                        {
                            int size = m_centroids.size();
                            int numOperations = size/m_threads;
                            int maxOperations;
                            if(i+1 == m_threads)
                            {
                                maxOperations = size;
                            }
                            else
                            {
                                maxOperations = ((i+1)*numOperations);
                            }
                            for(int j = i*numOperations; j < maxOperations; ++j )
                            {
                                (m_centroidsDistances)[j] = tpCompute(m_pointDimensions, (m_centroids)[j]);
                            }


                            act ^= (1 << i);
                        }
                        if(m_stop.load()) return;
                    }

                }
            );
    }

    ThreadPool(ThreadPool const&) = delete;
    ThreadPool& operator=(ThreadPool const&) = delete;


    ~ThreadPool()
    {
        m_stop.store(true);

        for(std::thread &worker: workers)
        {
            worker.join();
        }
    }


    void start(std::vector<double> & pointDimensions)
    {
        m_pointDimensions = &pointDimensions;
        act = readyMask;
    }

    bool ready()
    {
       return act == 0;
    }



private:
    std::vector<std::thread> workers;
    std::atomic<bool> m_stop;
    std::vector<double> * m_pointDimensions;
    CentroidsType & m_centroids;
    std::vector<double> & m_centroidsDistances;

    int m_threads;
    uint32_t readyMask;
    std::atomic<uint32_t> act;
};




#endif
