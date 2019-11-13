#include <k_means.h>
#include <parser.hpp>
#include <math.h>
#include <algorithm>
#include <unordered_map>
#include <chrono>

bool KMeans::clustering(CentroidsType & centroids) noexcept
{
    if(!inspectFile()) return false;
    if(!obtainStartCentroids(centroids)) return false;
    if(!doClustering(centroids)) return false;

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
    while (m_options.fstream.getline(line, MAX_LINE_LENGTH))
    {

        m_lineCount++;

        if(parsePoint(line, pointDimensions))
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
    return true;
}

bool KMeans::obtainStartCentroids(CentroidsType & centroids) noexcept
{
    srand(static_cast<unsigned>(time(nullptr)));

    int iterations = m_options.maxIterations;

    std::unordered_map<int, std::vector<double>> selectedCentroids;
    centroids.reserve(m_options.centroidsCount);
    selectedCentroids.reserve(m_options.centroidsCount);

    for(uint16_t i=0; i < m_options.centroidsCount; i++)
    {
        iterations = m_options.maxIterations;
        while(iterations)
        {
            int val = rand()%(m_lineCount);
            if(selectedCentroids.find(val) == selectedCentroids.cend())
            {
                selectedCentroids[val] = std::vector<double>();
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


void KMeans::initCentroids(CentroidsSum & centroidsSum,
                              const CentroidsType & centroids) noexcept
{

    for(int i=0; i < centroids.size(); ++i)
    {
        centroidsSum[i].first = centroids[i];
        centroidsSum[i].second = 1;
    }
}

void KMeans::moveCentroids(CentroidsSum & centroidsSum,
                      CentroidsType & centroids) noexcept
{
    for(int i=0; i < centroids.size(); ++i)
    {
        auto centroidSumDimension = centroidsSum[i].first.cbegin();
        auto centroidDimension = centroids[i].begin();

        for(; centroidSumDimension != centroidsSum[i].first.cend() &&
              centroidDimension != centroids[i].end();
            ++centroidSumDimension, ++centroidDimension)
        {
            *centroidDimension = *centroidSumDimension / centroidsSum[i].second;
        }

    }
}

bool KMeans::calcCentroids(char * lineBuf,
                           std::vector<double> & curPointBuf,
                           CentroidsType & centroids,
                           CentroidsSum & centroidsSum,
                           std::vector<double> & centroidsDistances) noexcept
{
    m_options.fstream.clear();
    m_options.fstream.seekg(0, m_options.fstream.beg);

    initCentroids(centroidsSum, centroids);

    while (m_options.fstream.getline(lineBuf, MAX_LINE_LENGTH))
    {
        //auto t1 = std::chrono::high_resolution_clock::now();
        if(parsePoint(lineBuf, curPointBuf))
        {
            if(!pool->start(curPointBuf)) return false;
            while(!pool->ready());

            int foundCentroid = std::distance(centroidsDistances.cbegin(),
                                std::min_element(centroidsDistances.cbegin(), centroidsDistances.cend()));

            std::transform(centroidsSum[foundCentroid].first.cbegin(),
                           centroidsSum[foundCentroid].first.cend(),
                           curPointBuf.cbegin(),
                           centroidsSum[foundCentroid].first.begin(),
                           std::plus<double>());


            centroidsSum[foundCentroid].second++;
        }
        else
        {
            std::cerr << "failed to parse point, text: " << lineBuf <<"\n";
            return false;
        }
       // auto t2 = std::chrono::high_resolution_clock::now();
        //auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>( t2 - t1 ).count();
       // std::cout << duration << std::endl;

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


    std::vector<double> centroidsDistances;
    centroidsDistances.resize(m_options.centroidsCount);
    CentroidsSum centroidsSum;
    centroidsSum.resize(centroids.size());

    pool = new ThreadPool(m_options.threadPoolSize, centroids, centroidsDistances);
    while(true)
    {
        if(!calcCentroids(lineBuf, curPoint, centroids, centroidsSum, centroidsDistances)) return false;
        centroidsNext = centroids;
        if(!calcCentroids(lineBuf, curPoint, centroidsNext, centroidsSum, centroidsDistances)) return false;

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
    auto centroid = centroidsObjects.cbegin();
    auto centroidNext = centroidObjectsNext.cbegin();

    for(; centroid != centroidsObjects.cend() && centroidNext != centroidObjectsNext.cend(); ++centroid, ++centroidNext)
    {
        if(!std::equal(centroid->cbegin(), centroid->cend(), centroidNext->cbegin()))
        {
            return false;
        }
    }
    return true;
}
