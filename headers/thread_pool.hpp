#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <thread>
#include <atomic>
#include <math.h>
#include <types.h>

static inline double tpCompute(const std::vector<double> * pointDimensions, const std::vector<double> & centerDimentions ) noexcept
{
    auto pointDim = pointDimensions->cbegin();
    auto centerDim = centerDimentions.cbegin();

    double sumOfPow = 0;
    for(;pointDim != pointDimensions->cend();
        ++pointDim, ++centerDim)
    {
        sumOfPow += pow((*pointDim - *centerDim),2.0);
    }
    return sqrt(sumOfPow);

}



class ThreadPool final
{
public:

    enum TaskType
    {
      TASK_TYPE_COMPUTE,
      TASK_TYPE_MOVE
    };
    explicit ThreadPool(size_t threads,
               CentroidsType  & centroids,
               std::vector<double> & centroidsDistances,
                CentroidsSum & centroidsSum,
                CentroidsSumCount & centroidsSumCount)
        :m_stop(false),
          m_pointDimensions(nullptr),
          m_centroids(centroids),
          m_centroidsDistances(centroidsDistances),
          m_centroidsSum(centroidsSum),
          m_centroidsSumCount(centroidsSumCount),
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
                            static int size = m_centroids.size();
                            static int numOperations = size/m_threads;
                            int maxOperations;
                            if(i+1 == m_threads)
                            {
                                maxOperations = size;
                            }
                            else
                            {
                                maxOperations = ((i+1)*numOperations);
                            }

                            if(m_taskType == TASK_TYPE_COMPUTE)
                            {

                                for(int j = i*numOperations; j < maxOperations; ++j )
                                {
                                    (m_centroidsDistances)[j] = tpCompute(m_pointDimensions, (m_centroids)[j]);
                                }
                            }
                            else
                            {
                                for(int j = i*numOperations; j < maxOperations; ++j )
                                {
                                    auto centroidSumDimension = m_centroidsSum[j].cbegin();
                                    auto centroidDimension = m_centroids[j].begin();

                                    for(; centroidSumDimension != m_centroidsSum[j].cend();
                                        ++centroidSumDimension, ++centroidDimension)
                                    {
                                        *centroidDimension = *centroidSumDimension / m_centroidsSumCount[j];
                                    }
                                }
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


    void startCompute(std::vector<double> & pointDimensions)
    {
        m_taskType = TASK_TYPE_COMPUTE;
        m_pointDimensions = &pointDimensions;
        act = readyMask;
    }

    void startMove()
    {
        m_taskType = TASK_TYPE_MOVE;
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
    CentroidsSum & m_centroidsSum;
    CentroidsSumCount & m_centroidsSumCount;

    int m_threads;
    uint32_t readyMask;
    std::atomic<uint32_t> act;
    TaskType m_taskType;
};




#endif
