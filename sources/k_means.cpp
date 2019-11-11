#include <k_means.h>
#include <parser.hpp>
#include <math.h>
#include <almost_equal_gtest.hpp>
#include <chrono>
bool KMeans::clustering(CentroidsType & centroids) noexcept
{
    auto t1 = std::chrono::high_resolution_clock::now();
    if(!inspectFile()) return false;
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    std::cout << "inspect duration: " << duration << std::endl;

    t1 = std::chrono::high_resolution_clock::now();
    if(!initCentroids(centroids)) return false;
    t2 = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    std::cout << "initCentroids duration: " << duration << std::endl;


    t1 = std::chrono::high_resolution_clock::now();
    if(!doClustering(centroids)) return false;
    t2 = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    std::cout << "doClustering duration: " << duration << std::endl;


    return true;
}

//TODO: remove inspecting
bool KMeans::inspectFile() noexcept
{
    m_options.fstream.seekg(0, std::ios::beg);
    m_lineCount = 0;

    char line[MAX_LINE_LENGTH];

    while (m_options.fstream.getline(line, MAX_LINE_LENGTH))
    {
        m_lineCount++;
        if(m_lineCount > 1000000000)
        {
            std::cerr << "ERROR: there are more than 1 billion points in file \n" ;
            return false;
        }
    }
    return true;
}

//TODO: initCentroids without inspecting
bool KMeans::initCentroids(CentroidsType & centroids) noexcept
{
    srand(static_cast<unsigned>(time(nullptr)));

    int iterations = m_options.maxIterations;

    for(uint16_t i=0; i < m_options.klusterCentroidsCount; i++)
    {
        iterations = m_options.maxIterations;
        while(iterations)
        {
            int val = rand()%(m_lineCount+1);
            if(centroids.find(val) == centroids.end())
            {
                centroids[val] = std::vector<double>();
                break;
            }
            iterations--;
        }

    }

    m_options.fstream.clear();
    m_options.fstream.seekg(0, m_options.fstream.beg);
    int i=0;
    char line[MAX_LINE_LENGTH];
    while (m_options.fstream.getline(line, MAX_LINE_LENGTH))
    {
        i++;
        auto curCentroid = centroids.find(i);
        if(centroids.end() != curCentroid)
        {
            if(!parsePoint(line, curCentroid->second))
            {
                std::cerr << "ERROR: failed to parse dimensions, line " << i << " text: " << line << "\n";
                return false;
            }
        }
    }
    return true;
}

static bool compute(std::vector<double> & pointDimensions, std::vector<double> & centerDimentions, double & result ) noexcept
{
    auto pointDim = pointDimensions.begin();
    auto centerDim = centerDimentions.begin();

    double sumOfPow = 0;
    errno = 0;
    for(;pointDim != pointDimensions.end() && centerDim != centerDimentions.end();
        ++pointDim, ++centerDim)
    {
        sumOfPow += pow((*pointDim - *centerDim),2.0);
    }
    result = sqrt(sumOfPow);

    if(errno == EDOM) //compute error
    {
        return false;
    }

    if(pointDim != pointDimensions.end() || centerDim != centerDimentions.end())//file format error
    {
        return false;
    }

    return true;
}

void KMeans::initCentroids(std::unordered_map<int, std::pair<std::vector<double>, double>> & centroidsSum,
                              const CentroidsType & centroids) noexcept
{
    for(auto centroid: centroids)
    {
        centroidsSum[centroid.first].first = centroid.second;
        centroidsSum[centroid.first].second = 1;
    }
}

void KMeans::moveCentroids(std::unordered_map<int, std::pair<std::vector<double>, double>> & centroidsSum,
                      CentroidsType & centroids) noexcept
{
    for(auto & centroid: centroids)
    {
        auto centroidSumDimension = centroidsSum[centroid.first].first.cbegin();
        auto centroidDimension = centroid.second.begin();

        for(; centroidSumDimension != centroidsSum[centroid.first].first.cend() &&
              centroidDimension != centroid.second.end();
            ++centroidSumDimension, ++centroidDimension)
        {
            *centroidDimension = *centroidSumDimension / centroidsSum[centroid.first].second;
        }

    }
}

bool KMeans::calcCentroids(char * lineBuf,
                           std::vector<double> & curPointBuf,
                           CentroidsType & centroids,
                           std::unordered_map<int, std::pair<std::vector<double>, double>> & centroidsSum) noexcept
{
    m_options.fstream.clear();
    m_options.fstream.seekg(0, m_options.fstream.beg);
    int lineNum = 0;

    auto t1 = std::chrono::high_resolution_clock::now();
    initCentroids(centroidsSum, centroids);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    std::cout << "initCentroids duration: " << duration << std::endl;


    while (m_options.fstream.getline(lineBuf, MAX_LINE_LENGTH))
    {
        lineNum++;
        curPointBuf.clear();
        t1 = std::chrono::high_resolution_clock::now();
        if(parsePoint(lineBuf, curPointBuf))
        {
            double minDist = 10000000000.0;
            double curDist = 0;
            int foundCentroid = centroids.begin()->first;


            for(auto centroid = centroids.begin(); centroid != centroids.end(); ++centroid)
            {
                if(compute(curPointBuf, centroid->second, curDist))
                {
                    if (curDist < minDist)
                    {
                        minDist = curDist;
                        foundCentroid = centroid->first;
                    }
                }
                else
                {
                    std::cerr << "failed to compute centroids line: " << lineNum << " text: " << lineBuf <<"\n";
                    return false;
                }
            }

            auto centrDim = centroidsSum[foundCentroid].first.begin();
            auto pointDim = curPointBuf.begin();

            for(;centrDim != centroidsSum[foundCentroid].first.end() &&
                pointDim != curPointBuf.end();
                ++centrDim, ++pointDim)
            {
                *centrDim += *pointDim;
            }
            centroidsSum[foundCentroid].second++;
        }
        else
        {
            std::cerr << "failed to parse point, line: " << lineNum << " text: " << lineBuf <<"\n";
            return false;
        }
        t2 = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
        std::cout << "point process duration: " << duration << std::endl;
    }
    moveCentroids(centroidsSum, centroids);
    return true;
}


bool KMeans::doClustering(CentroidsType & centroids) noexcept
{
    char lineBuf[MAX_LINE_LENGTH];

    std::vector<double> curPoint;
    curPoint.reserve(1000);

    CentroidsType centroidsNext;
    centroidsNext.reserve(centroids.size());

    int iterations = m_options.maxIterations;

    std::unordered_map<int, std::pair<std::vector<double>, double>> centroidsSum;
    while(true)
    {
        if(!calcCentroids(lineBuf, curPoint, centroids, centroidsSum)) return false;
        centroidsNext = centroids;
        if(!calcCentroids(lineBuf, curPoint, centroidsNext, centroidsSum)) return false;

        iterations--;
        if(!iterations) break;
        if(centroidsEqual(centroids, centroidsNext))
        {
            centroids = centroidsNext;
            break;
        }
        else
        {
            centroids = centroidsNext;
        }
    }
    return true;
}

bool KMeans::centroidsEqual(const CentroidsType & centroidsObjects,const CentroidsType & centroidObjectsNext) noexcept
{
    auto centroid = centroidsObjects.begin();
    auto centroidNext = centroidObjectsNext.begin();

    for(; centroid != centroidsObjects.end() && centroidNext != centroidObjectsNext.end(); ++centroid, ++centroidNext)
    {
        if(!std::equal(centroid->second.begin(), centroid->second.end(), centroidNext->second.begin(), [](double cDoubleVal, double cNextDoubleVal)
        {
          const FloatingPoint<double> lhs(cDoubleVal), rhs(cNextDoubleVal);
          return lhs.AlmostEquals(rhs);
         }))
        {
            return false;
        }
    }
    return true;
}
