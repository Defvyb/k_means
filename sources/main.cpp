#include <string>
#include <iostream>
#include <utils.hpp>
#include <types.h>
#include <k_means.h>



int main(int argc, char *argv[])
{

    std::set_terminate([](){ std::cout << "Unhandled exception\n"; std::abort();});

    ProgramOptions options;
    if(!getProgramOptions(argc, argv, options)) return -1;

    KMeans means(options);
    std::unordered_map<int, std::vector<double>> centroids;
    means.clustering(centroids);

	return 0;
}
