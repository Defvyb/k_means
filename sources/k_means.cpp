#include <k_means.h>
#include <parser.hpp>
#include <algorithm>
#include <unordered_map>
#include <chrono>
#include <random>


bool KMeans::defaultKMeansStartCentroidsObtainer(CentroidsType & centroids, ProgramOptions & options, int lineCount) noexcept
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, lineCount-1);

    int iterations;

    std::unordered_map<int, std::vector<double>> selectedCentroids;
    centroids.reserve(options.centroidsCount);
    selectedCentroids.reserve(options.centroidsCount);

    for(uint16_t i=0; i < options.centroidsCount; i++)
    {
        iterations = options.maxIterations;
        while(iterations)
        {
            int val = dis(gen);
            if(selectedCentroids.find(val) == selectedCentroids.cend())
            {
                selectedCentroids[val] = std::vector<double>();
                break;
            }
            iterations--;
        }

    }

    options.fstream.clear();
    options.fstream.seekg(0, std::ios_base::beg);
    int i=0;
    char line[MAX_LINE_LENGTH];
    while (options.fstream.getline(line, MAX_LINE_LENGTH))
    {
        auto curCentroid = selectedCentroids.find(i);
        if(selectedCentroids.cend() != curCentroid)
        {
            if(!parsePoint(line, selectedCentroids[i]))
            {
                std::cerr << "ERROR: failed to parse dimensions, line " << i << " text: " << line << "\n";
                return false;
            }
        }
        i++;
    }

    for(auto & centroid: selectedCentroids)
    {
        centroids.push_back(centroid.second);
    }
    return true;
}

bool KMeans::clustering(CentroidsType & centroids) noexcept
{
    if(!inspectFile()) return false;
    if(!m_startCentroidsObtainer(centroids, m_options, m_lineCount)) return false;

    auto t1 = std::chrono::high_resolution_clock::now();
    if(!doClustering(centroids)) return false;
    auto t2 = std::chrono::high_resolution_clock::now();
    m_stat.m_duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();


    return true;
}

bool KMeans::inspectFile() noexcept
{
    m_options.fstream.seekg(0, std::ios::beg);
    m_lineCount = 0;

    char line[MAX_LINE_LENGTH];

    int elementsCount = 0;
    std::vector<double> pointDimensions;
    pointDimensions.reserve(1000);

    auto parsePointFunc = m_options.checkFile ? &parsePointWithChecking : &parsePoint;
    while (m_options.fstream.getline(line, MAX_LINE_LENGTH))
    {

        m_lineCount++;

        if(parsePointFunc(line, pointDimensions))
        {
            if(!elementsCount)
            {
                elementsCount = pointDimensions.size();
            }
            else
            {
                if(elementsCount != pointDimensions.size())
                {
                    std::cerr << "ERROR: file format error, line: " <<m_lineCount << " text: " << line<< "\n" ;
                    return false;
                }
            }
        }
        else
        {
            std::cerr << "ERROR: file format error, line: " <<m_lineCount << " text: " << line<< "\n" ;
            return false;
        }

        if(m_lineCount > 1000000000)
        {
            std::cerr << "ERROR: there are more than 1 billion points in file \n" ;
            return false;
        }
    }

    if(m_lineCount < m_options.centroidsCount)
    {
        std::cerr << "ERROR: there are too much centroids, for file with " << m_lineCount <<" lines\n";
        return false;
    }
    return true;
}


Stat KMeans::getStat() const noexcept
{
    return m_stat;
}


void KMeans::initCentroids(CentroidsSum & centroidsSum,
                           CentroidsSumCount & centroidsSumCount,
                           const CentroidsType & centroids) noexcept
{
    std::copy(centroids.cbegin(), centroids.cend(), centroidsSum.begin());
    centroidsSumCount.assign(centroids.size(), 1.0);
}

void KMeans::moveCentroids(CentroidsSum & centroidsSum,
                           CentroidsSumCount & centroidsSumCount,
                      CentroidsType & centroids) noexcept
{
    for(int i=0; i < centroids.size(); ++i)
    {        
        std::transform(centroidsSum[i].cbegin(),
                       centroidsSum[i].cend(),
                       centroids[i].begin(),
                       [&centroidsSumCount, i](double centroidSumDimension)
        {
            return centroidSumDimension /  centroidsSumCount[i];
        });

    }
}

bool KMeans::calcCentroids(char * lineBuf,
                           std::vector<double> & curPointBuf,
                           CentroidsType & centroids,
                           CentroidsSum & centroidsSum,
                           CentroidsSumCount & centroidsSumCount) noexcept
{
    m_options.fstream.clear();
    m_options.fstream.seekg(0, std::ios_base::beg);

    if(m_options.threadPoolSize == 1)
    {
        initCentroids(centroidsSum, centroidsSumCount, centroids);
    }


    bool isFirstLine = true;
    while (m_options.fstream.getline(lineBuf, MAX_LINE_LENGTH))
    {
        if(parsePoint(lineBuf, curPointBuf))
        {

            int foundCentroid = 0;
            double minDistance = std::numeric_limits<double>::max();
            double curDistance = 0;
            if(m_options.threadPoolSize == 1)
            {
                for(int i = 0; i < centroids.size(); ++i )
                {
                    curDistance = tpCompute(&curPointBuf, centroids[i]);
                    if(minDistance > curDistance)
                    {
                        minDistance = curDistance;
                        foundCentroid = i;
                    }
                }
                std::transform(centroidsSum[foundCentroid].cbegin(),
                               centroidsSum[foundCentroid].cend(),
                               curPointBuf.cbegin(),
                               centroidsSum[foundCentroid].begin(),
                               std::plus<double>());


                centroidsSumCount[foundCentroid]++;
            }
            else
            {
                m_pool->startCompute(curPointBuf, isFirstLine);
                while(!m_pool->ready());
            }

        }
        else
        {
            std::cerr << "failed to parse point, text: " << lineBuf <<"\n";
            return false;
        }
        isFirstLine = false;
    }

    if(m_options.threadPoolSize == 1)
    {
        moveCentroids(centroidsSum, centroidsSumCount, centroids);
    }

    return true;
}


bool KMeans::doClustering(CentroidsType & centroids) noexcept
{
    static char lineBuf[MAX_LINE_LENGTH] ={0};

    std::vector<double> curPoint;
    curPoint.reserve(1000);

    CentroidsType centroidsNext;
    centroidsNext = centroids;

    m_iterations = 0;

    CentroidsSum centroidsSum;
    CentroidsSumCount centroidsSumCount;

    centroidsSum.resize(centroids.size());
    centroidsSumCount.resize(centroids.size());

    initCentroids(centroidsSum, centroidsSumCount, centroids);


    if(m_options.threadPoolSize > 1 )
    {
        if(m_pool)
        {
            delete m_pool;
            m_pool = nullptr;
        }
        m_pool = new ThreadPool(m_options.threadPoolSize, centroids, centroidsSum, centroidsSumCount);
    }

    while(true)
    {
        if(!calcCentroids(lineBuf, curPoint, centroids, centroidsSum, centroidsSumCount)) return false;

        m_iterations++;
        if(m_iterations >= m_options.maxIterations) break;
        if(centroids == centroidsNext && m_iterations > 2)
        {
            break;
        }
        else
        {
            centroidsNext = centroids;
        }
    }
    m_stat.m_iterations = m_iterations;
    return true;
}

