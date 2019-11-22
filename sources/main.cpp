#include <string>
#include <iostream>
#include <utils.hpp>
#include <types.h>
#include <k_means.h>
#include <chrono>
#include <algorithm>
#include <fstream>

int main(int argc, char *argv[])
{

    std::set_terminate([](){ std::cout << "Unhandled exception\n"; std::abort();});

    ProgramOptions options;
    if(!getProgramOptions(argc, argv, options)) return -1;

    KMeans means(options);
    CentroidsType centroids;
    if(!means.clustering(centroids))
    {
        std::cerr << "clustering failed\n";
        return -1;
    }

    std::sort(centroids.begin(), centroids.end());
    std::ofstream outFile(options.outputFileName);
    if(outFile.is_open())
    {
        for(auto centroid: centroids)
        {
            for(auto val: centroid)
            {
                outFile << val << " ";
            }
            outFile << "\n";
        }
    }
    else
    {
        std::cerr << "failed to open " << options.outputFileName << "\n";
        return -1;
    }


    std::cout << "clustering iterations: " << means.getStat().m_iterations << "\n";
    std::cout << "clustering duration: " << means.getStat().m_duration << " microseconds\n";
    std::cout << "clustering duration per iteration: " << means.getStat().getDurationPerIteration() << " microseconds\n";
	return 0;
}
