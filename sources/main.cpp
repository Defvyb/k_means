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
    auto t1 = std::chrono::high_resolution_clock::now();
    if(!means.clustering(centroids)) return -1;
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();


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


    std::cout << "clasterisation duration: " << duration << " microseconds\n";
	return 0;
}
