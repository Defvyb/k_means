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

    static double sumOfPow;
    sumOfPow=0;
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
               std::vector<double> & centroidsDistances)
        :m_stop(false),
          m_pointDimensions(nullptr),
          m_centroids(centroids),
          m_centroidsDistances(centroidsDistances),
          readyMask(0),
          act(0),
          m_taskType(TASK_TYPE_COMPUTE)
    {
        for(int i=0; i<threads; ++i)
        {
            readyMask |= 1<<i;
        }

        int size = m_centroids.size();
        int numOperations = size/threads;

        for(size_t i = 0; i<threads ;++i)
            workers.emplace_back(
                [this, i, threads, size, numOperations]
                {
                    for(;;)
                    {
                        if( act.load(std::memory_order_relaxed) & (1U << i))
                        {
                            int maxOperations = 0;
                            if(i+1 == threads)
                            {
                                maxOperations = size;
                            }
                            else
                            {
                                maxOperations = ((i+1)*numOperations);
                            }

                            for(int j = i*numOperations; j < maxOperations; ++j )
                            {
                                m_centroidsDistances[j] = tpCompute(m_pointDimensions, m_centroids[j]);
                            }

                            std::atomic_fetch_xor_explicit(&act, (1U << i), std::memory_order_relaxed );
                        }
                        if(m_stop.load(std::memory_order_relaxed)) return;
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
        act.store(readyMask, std::memory_order_relaxed);
    }

    void startMove()
    {
        m_taskType = TASK_TYPE_MOVE;
        act.store(readyMask, std::memory_order_relaxed);
    }

    bool ready()
    {
       return act.load(std::memory_order_relaxed) == 0;
    }



private:
    std::vector<std::thread> workers;
    std::atomic<bool> m_stop;
    std::vector<double> * m_pointDimensions;
    CentroidsType & m_centroids;
    std::vector<double> & m_centroidsDistances;

    uint32_t readyMask;
    std::atomic<uint32_t> act;
    TaskType m_taskType;
};




#endif
