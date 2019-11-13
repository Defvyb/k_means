#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <deque>
#include <vector>
#include <thread>
#include <atomic>
#include <math.h>
#include <iostream>
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
    ThreadPool(size_t threads,
               CentroidsType  & centerDimentions,
               std::vector<double> & centroidsDistances)
        :m_stop(false),
          m_pointDimensions(nullptr),
          m_centerDimentions(centerDimentions),
          m_centroidsDistances(centroidsDistances),
          m_threads(threads)
    {
        for(int i=0; i< m_threads; i++)
        {
            act[i].store(false);
        }

        for(size_t i = 0; i<threads ;++i)
            workers.emplace_back(
                [this, i, threads]
                {
                    for(;;)
                    {
                        if( act[i].load())
                        {
                            int size = m_centerDimentions.size();
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
                                (m_centroidsDistances)[j] = tpCompute(m_pointDimensions, (m_centerDimentions)[j]);
                            }


                            act[i].store(false);
                        }
                        if(m_stop.load()) return;
                    }

                }
            );
    }

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
        for(int i=0; i< m_threads; i++)
        {
            act[i].store(true);
        }
    }

    bool ready() const
    {

        for(int i=0; i< m_threads; i++)
        {
            if(act[i].load() == true) return false;
        }
        return true;
    }



private:
    std::vector<std::thread> workers;

    std::atomic<bool> m_stop;
    std::vector<double> * m_pointDimensions;
    std::vector<std::vector<double>> & m_centerDimentions;
    std::vector<double> & m_centroidsDistances;
    int m_threads;
    std::array<std::atomic<bool>, 100> act;
};




#endif
