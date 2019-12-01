#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <thread>
#include <atomic>
#include <math.h>
#include <types.h>
#include <algorithm>
#include <parser.hpp>
static inline double tpCompute(const std::vector<double> & pointDimensions, const std::vector<double> & centerDimentions ) noexcept
{
    auto pointDim = pointDimensions.cbegin();
    auto centerDim = centerDimentions.cbegin();

    double sumOfPow = 0;

    for(;pointDim != pointDimensions.cend();
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
                CentroidsSum & centroidsSum,
                CentroidsSumCount & centroidsSumCount,
                int dimensions)
        :m_stop(false),
          m_centroids(centroids),
          m_centroidsSum(centroidsSum),
          m_centroidsSumCount(centroidsSumCount),
          readyMask(0),
          act(0),
          parse(true)
    {
        int size = m_centroids.size();
        int numOperations = size/threads;
        m_pointDimensions.resize(dimensions);

        m_positions.resize(threads);
        m_minValues.resize(threads);
        m_isEq.resize(threads);

        for(size_t i = 0; i<threads ;++i)
        {
            readyMask |= 1<<i;

            int maxOperations = 0;
            if(i+1 == threads)
            {
                maxOperations = size;
            }
            else
            {
                maxOperations = ((i+1)*numOperations);
            }

            workers.emplace_back(
                [this, i, numOperations, maxOperations, threads]
                {
                    for(;;)
                    {
                        if(act.load(std::memory_order_relaxed) & (1U << i))
                        {
                            if(parse) // parse line concurently
                            {
                                int pointsCount = 0;
                                double result = 0;
                                bool noResult = true;
                                const char* linePtr = m_lineBuf;
                                while (*linePtr != '\0')
                                {
                                    result = getEachDouble(linePtr, i, threads-1, noResult);
                                    if(!noResult)
                                    {
                                        m_pointDimensions[i + pointsCount*threads] = result;
                                    }
                                    pointsCount++;
                                }
                                std::atomic_fetch_xor_explicit(&act, (1U << i), std::memory_order_relaxed);
                                continue;
                            }

                            if(m_isFirstPoint) // move centroids
                            {
                                if(!std::all_of(m_centroidsSumCount.begin(),
                                                m_centroidsSumCount.end(),
                                                [](double val) { return val==0; }))
                                {
                                    for(int j = i*numOperations; j < maxOperations; ++j )
                                    {
                                        std::transform(m_centroidsSum[j].cbegin(),
                                                       m_centroidsSum[j].cend(),
                                                       m_centroids[j].cbegin(),
                                                       m_centroids[j].begin(),
                                                       [this, i, j](double centroidSumDimension, double centroidDimension)
                                        {
                                            double newDim = centroidSumDimension /  m_centroidsSumCount[j];
                                            if(m_isEq[i] && newDim != centroidDimension)
                                            {
                                                m_isEq[i] = false;
                                            }
                                            return newDim;
                                        });

                                        m_centroidsSumCount[j] = 1.0;
                                        std::copy(m_centroids[j].begin(), m_centroids[j].end(), m_centroidsSum[j].begin());
                                    }

                                }
                            }

                            double curDistance = 0; // calculate distances for each centroid
                            m_minValues[i] = std::numeric_limits<double>::max();
                            for(int j = i*numOperations; j < maxOperations; ++j )
                            {
                                curDistance = tpCompute(m_pointDimensions, m_centroids[j]);
                                if(m_minValues[i] > curDistance)
                                {
                                    m_minValues[i] = curDistance;
                                    m_positions[i] = j;
                                }
                            }


                            std::atomic_fetch_xor_explicit(&act, (1U << i), std::memory_order_relaxed );
                        }
                        if(m_stop.load(std::memory_order_relaxed)) return;
                    }

                }
            );
        }
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


    void iterationInit()
    {
        m_isEq.assign(m_isEq.size(), true);
    }

    bool centroidsIsNotMoving()
    {
        return std::all_of(m_isEq.begin(),
                               m_isEq.end(),
                               [](bool val) { return val==true; });
    }

    void startCompute(char * lineBuf, bool isFirstPoint)
    {
        m_isFirstPoint = isFirstPoint;
        m_lineBuf = lineBuf;

        parse = true;
        act.store(readyMask, std::memory_order_relaxed);
        while(act.load(std::memory_order_relaxed) != 0);

        parse = false;
        act.store(readyMask, std::memory_order_relaxed);
    }


    bool ready()
    {
       if(act.load(std::memory_order_relaxed) != 0) return false;

       int foundCentroid = m_positions[std::min_element(m_minValues.cbegin(),m_minValues.cend()) - m_minValues.cbegin()];

       std::transform(m_centroidsSum[foundCentroid].cbegin(),
                      m_centroidsSum[foundCentroid].cend(),
                      m_pointDimensions.cbegin(),
                      m_centroidsSum[foundCentroid].begin(),
                      std::plus<double>());


       m_centroidsSumCount[foundCentroid]++;
       return true;
    }



private:
    std::vector<std::thread> workers;
    std::atomic<bool> m_stop;
    char * m_lineBuf;
    std::vector<double> m_pointDimensions;
    CentroidsType & m_centroids;
    std::vector<int> m_positions;
    std::vector<double> m_minValues;
    std::vector<bool> m_isEq;
    CentroidsSum & m_centroidsSum;
    CentroidsSumCount & m_centroidsSumCount;
    bool m_isFirstPoint;

    uint32_t readyMask;
    std::atomic<uint32_t> act;
    bool parse;
};




#endif
