#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <deque>
#include <vector>
#include <thread>
#include <atomic>
#include <math.h>

#include <iostream>
static double tpCompute(std::vector<double> & pointDimensions, std::vector<double> & centerDimentions ) noexcept
{
    auto pointDim = pointDimensions.begin();
    auto centerDim = centerDimentions.begin();

    double sumOfPow = 0;
    for(;pointDim != pointDimensions.end() && centerDim != centerDimentions.end();
        ++pointDim, ++centerDim)
    {
        sumOfPow += pow((*pointDim - *centerDim),2.0);
    }
    return sqrt(sumOfPow);

}

class ThreadPool {
public:
    ThreadPool(size_t threads)
        :stop(false)
    {
        act.resize(threads);
        for(auto & val: act)
        {
            val = false;
        }

        for(size_t i = 0; i<threads ;++i)
            workers.emplace_back(
                [this, i, threads]
                {
                    for(;;)
                    {
                        if(act[i])
                        {
                            int size = m_centerDimentions->size();
                            int numOperations = size/threads;
                            if(i+1 == act.size())
                            {
                                for(int j = i*numOperations; j < size; ++j )
                                {
                                    (*m_results)[j] = tpCompute(*m_pointDimensions, (*m_centerDimentions)[j]);
                                }
                            }
                            else
                            {
                                for(int j = i*numOperations; j < ((i+1)*numOperations); ++j )
                                {
                                    (*m_results)[j] = tpCompute(*m_pointDimensions, (*m_centerDimentions)[j]);
                                }
                            }

                              //

                            act[i] = false;
                        }
                        if(stop) return;
                    }

                }
            );
    }

    inline ~ThreadPool()
    {
        stop = true;

        for(std::thread &worker: workers)
        {
            worker.join();
        }
    }


    void start()
    {
        for(auto & val: act)
        {
            val = true;
        }
    }

    bool ready()
    {
        for(auto & val: act)
        {
            if(val == true) return false;
        }
        return true;
    }


    void setParams(std::vector<double> * pointDimensions, std::vector<std::vector<double>> * centerDimentions, std::vector<double> * results)
    {
        m_pointDimensions = pointDimensions;
        m_centerDimentions = centerDimentions;
        m_results = results;
    }

private:
    std::vector<std::thread> workers;

    std::vector<double> * m_pointDimensions;
    std::vector<std::vector<double>> * m_centerDimentions;
    std::vector<double> * m_results;

    std::atomic<bool> stop;
    std::deque<std::atomic<bool>> act;
};




#endif
