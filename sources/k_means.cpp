#include <k_means.h>
#include <parser.hpp>
#include <math.h>
bool KMeans::clustering(std::unordered_map<int, std::vector<double>> & centroids) noexcept
{
    if(!inspectFile()) return false;
    if(!initCentroids(centroids)) return false;
    if(!doClustering(centroids)) return false;
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

bool KMeans::initCentroids(std::unordered_map<int, std::vector<double>> & centroids) noexcept
{
    srand(static_cast<unsigned>(time(nullptr)));

    for(uint16_t i=0; i < m_options.klusterCentroidsCount; i++)
    {
        centroids[rand()%(m_lineCount+1)] = std::vector<double>();
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
        sumOfPow += pow((pointDim - centerDim),2.0);
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


bool KMeans::doClustering(std::unordered_map<int, std::vector<double>> & centroids) noexcept
{
    m_options.fstream.clear();
    m_options.fstream.seekg(0, m_options.fstream.beg);
    char line[MAX_LINE_LENGTH];

    std::vector<double> curPoint;
    curPoint.reserve(1000);

    while (m_options.fstream.getline(line, MAX_LINE_LENGTH))
    {
        curPoint.clear();
        if(parsePoint(line, curPoint))
        {
            double minDist = pow(10,10);
            double curDist = 0;
            int foundCentroid = centroids.begin()->first;

            for(auto centroid = centroids.begin(); centroid != centroids.end(); ++centroid)
            {
                if(compute(curPoint, centroid->second, curDist))
                {
                    if (curDist < minDist)
                    {
                        minDist = curDist;
                        foundCentroid = centroid->first;
                    }
                }
                else
                {
                    std::cerr << "failed to compute centroids, line: " << line << "\n";
                    return false;
                }
            }

            auto centrDim = centroids[foundCentroid].begin();
            auto pointDim = curPoint.begin();

            for(;centrDim != centroids[foundCentroid].end() &&
                pointDim != curPoint.end();
                ++centrDim, ++pointDim)
            {
                *centrDim = ((*centrDim+*pointDim) / 2.0);
            }
        }
        else
        {
             std::cerr << "failed to parse point, line: " << line << "\n";
             return false;
        }
    }

    m_options.fstream.clear();
    m_options.fstream.seekg(0, m_options.fstream.beg);

    while (m_options.fstream.getline(line, MAX_LINE_LENGTH))
    {
        curPoint.clear();
        if(parsePoint(line, curPoint))
        {
            double minDist = pow(10,10);
            double curDist = 0;
            int foundCentroid = centroids.begin()->first;

            for(auto centroid = centroids.begin(); centroid != centroids.end(); ++centroid)
            {
                if(compute(curPoint, centroid->second, curDist))
                {
                    if (curDist < minDist)
                    {
                        minDist = curDist;
                        foundCentroid = centroid->first;
                    }
                }
                else
                {
                    std::cerr << "failed to compute centroids, line: " << line << "\n";
                    return false;
                }
            }

            auto centrDim = centroids[foundCentroid].begin();
            auto pointDim = curPoint.begin();

            for(;centrDim != centroids[foundCentroid].end() &&
                pointDim != curPoint.end();
                ++centrDim, ++pointDim)
            {
                *centrDim = ((*centrDim+*pointDim) / 2.0);
            }
        }
        else
        {
             std::cerr << "failed to parse point, line: " << line << "\n";
             return false;
        }
    }
    return true;
}
