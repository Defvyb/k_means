#include <k_means.h>
#include <parser.hpp>
#include <math.h>
#include <almost_equal_gtest.hpp>
#include <unordered_set>
bool KMeans::clustering(CentroidsType & centroids) noexcept
{
    if(!inspectFile()) return false;
    if(!obtainStartCentroids(centroids)) return false;
    if(!doClustering(centroids)) return false;

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

//TODO: obtainStartCentroids without inspecting
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
            int val = rand()%(m_lineCount+1);
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
                           std::vector<std::pair<std::vector<double>, double>> & centroidsSum) noexcept
{
    m_options.fstream.clear();
    m_options.fstream.seekg(0, m_options.fstream.beg);
    int lineNum = 0;

    initCentroids(centroidsSum, centroids);

    while (m_options.fstream.getline(lineBuf, MAX_LINE_LENGTH))
    {
        lineNum++;
        curPointBuf.clear();
        if(parsePoint(lineBuf, curPointBuf))
        {
            double minDist = 10000000000.0;
            double curDist = 0;
            int foundCentroid = 0;


            for(int i=0; i < centroids.size(); ++i)
            {
                if(compute(curPointBuf, centroids[i], curDist))
                {
                    if (curDist < minDist)
                    {
                        minDist = curDist;
                        foundCentroid = i;
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

    std::vector<std::pair<std::vector<double>, double>> centroidsSum;
    centroidsSum.resize(centroids.size());
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
