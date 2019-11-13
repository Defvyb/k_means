#include <string>
#include <iostream>
#include <utils.hpp>
#include <types.h>
#include <k_means.h>
#include <chrono>
#include <algorithm>


int main(int argc, char *argv[])
{

    std::set_terminate([](){ std::cout << "Unhandled exception\n"; std::abort();});

    ProgramOptions options;
    if(!getProgramOptions(argc, argv, options)) return -1;

    KMeans means(options);
    CentroidsType centroids;
    auto t1 = std::chrono::high_resolution_clock::now();
    means.clustering(centroids);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    std::cout << "clasterisation duration: " << duration << " microseconds\n";

    std::sort(centroids.begin(), centroids.end());

    for(auto centroid: centroids)
    {
        for(auto val: centroid)
        {
            std::cout << val << " ";
        }
        std::cout << "\n";
    }

	return 0;
}
