#include <k_means.h>
#include <parser.hpp>
#include <algorithm>
#include <unordered_map>
#include <chrono>


bool KMeans::defaultKMeansStartCentroidsObtainer(CentroidsType & centroids, ProgramOptions & options, int lineCount) noexcept
{
    srand(static_cast<unsigned>(time(nullptr)));

    int iterations = options.maxIterations;

    std::unordered_map<int, std::vector<double>> selectedCentroids;
    centroids.reserve(options.centroidsCount);
    selectedCentroids.reserve(options.centroidsCount);

    for(uint16_t i=0; i < options.centroidsCount; i++)
    {
        iterations = options.maxIterations;
        while(iterations)
        {
            int val = rand()%(lineCount);
            if(selectedCentroids.find(val) == selectedCentroids.cend())
            {
                selectedCentroids[val] = std::vector<double>();
                break;
            }
            iterations--;
        }

    }

    options.fstream.clear();
    options.fstream.seekg(0, options.fstream.beg);
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
    auto t1 = std::chrono::high_resolution_clock::now();

    if(!inspectFile()) return false;
    if(!m_startCentroidsObtainer(centroids, m_options, m_lineCount)) return false;
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
    while (m_options.fstream.getline(line, MAX_LINE_LENGTH))
    {

        m_lineCount++;

        if(parsePointWithChecking(line, pointDimensions))
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
        if(parsePoint(lineBuf, curPointBuf))
        {
            m_pool->start(curPointBuf);
            while(!m_pool->ready())
            {
            }

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

    m_iterations = 0;

    std::vector<double> centroidsDistances;
    centroidsDistances.resize(m_options.centroidsCount);
    CentroidsSum centroidsSum;
    centroidsSum.resize(centroids.size());

    if(m_pool)
    {
        delete m_pool;
        m_pool = nullptr;
    }
    m_pool = new ThreadPool(m_options.threadPoolSize, centroids, centroidsDistances);
    while(true)
    {
        if(!calcCentroids(lineBuf, curPoint, centroids, centroidsSum, centroidsDistances)) return false;
        centroidsNext = centroids;
        if(!calcCentroids(lineBuf, curPoint, centroidsNext, centroidsSum, centroidsDistances)) return false;

        m_iterations++;
        if(m_iterations >= m_options.maxIterations) break;
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
    m_stat.m_iterations = m_iterations;
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
