#include <k_means.h>
#include <parser.hpp>
#include <math.h>
#include <almost_equal_gtest.hpp>
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

bool KMeans::obtainStartCentroids(CentroidsType & centroids) noexcept
{
    srand(static_cast<unsigned>(time(nullptr)));

    int iterations = m_options.maxIterations;

    std::unordered_map<int, std::vector<double>> selectedCentroids;
    centroids.reserve(m_options.klusterCentroidsCount);
    selectedCentroids.reserve(m_options.klusterCentroidsCount);

    for(uint16_t i=0; i < m_options.klusterCentroidsCount; i++)
    {
        iterations = m_options.maxIterations;
        while(iterations)
        {
            int val = rand()%(m_lineCount);
            if(selectedCentroids.find(val) == selectedCentroids.end())
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
        if(selectedCentroids.end() != curCentroid)
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


void KMeans::initCentroids(std::vector<std::pair<std::vector<double>, double>> & centroidsSum,
                              const CentroidsType & centroids) noexcept
{

    for(int i=0; i < centroids.size(); ++i)
    {
        centroidsSum[i].first = centroids[i];
        centroidsSum[i].second = 1;
    }
}

void KMeans::moveCentroids(std::vector<std::pair<std::vector<double>, double>> & centroidsSum,
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
                           std::vector<std::pair<std::vector<double>, double>> & centroidsSum,
                           std::vector<double> & dists) noexcept
{
    m_options.fstream.clear();
    m_options.fstream.seekg(0, m_options.fstream.beg);

    initCentroids(centroidsSum, centroids);

    while (m_options.fstream.getline(lineBuf, MAX_LINE_LENGTH))
    {
       // auto t1 = std::chrono::high_resolution_clock::now();
        if(parsePoint(lineBuf, curPointBuf))
        {
            pool.setParams(&curPointBuf, &centroids, &dists);
            pool.start();
            while(!pool.ready());

            int foundCentroid = std::distance(dists.begin(), std::min_element(dists.begin(), dists.end()));

           std::transform(centroidsSum[foundCentroid].first.begin(),
                           centroidsSum[foundCentroid].first.end(),
                           curPointBuf.begin(),
                           centroidsSum[foundCentroid].first.begin(),
                           std::plus<double>());


            centroidsSum[foundCentroid].second++;
        }
        else
        {
            std::cerr << "failed to parse point, text: " << lineBuf <<"\n";
            return false;
        }
      /*  auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
        std::cout << duration << std::endl;*/

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
    centroidsDistances.resize(m_options.klusterCentroidsCount);
    std::vector<std::pair<std::vector<double>, double>> centroidsSum;
    centroidsSum.resize(centroids.size());
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
    auto centroid = centroidsObjects.begin();
    auto centroidNext = centroidObjectsNext.begin();

    for(; centroid != centroidsObjects.end() && centroidNext != centroidObjectsNext.end(); ++centroid, ++centroidNext)
    {
        if(!std::equal(centroid->begin(), centroid->end(), centroidNext->begin(), [](double cDoubleVal, double cNextDoubleVal)
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
