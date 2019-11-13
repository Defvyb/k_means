#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>
#include <iostream>
#include <types.h>

void printHelp()
{
    std::cout << "Usage: -f=<filename>(mandatory parameter) \n";
    std::cout << "       -t=<thread pool size>(Default: 1) \n";
    std::cout << "       -k=<kluster centroids count>(Default: 10) \n";
    std::cout << "       -m=<max iterations>(Default: 1000000) \n";
    std::cout << "       -o=<filename>(Default: output.file) \n";
    std::cout << "       -h this help  \n";
}

bool getProgramOptions(int argc, char *argv[],  ProgramOptions & options) noexcept
{

    std::string filename;
    for(int i=1; i< argc; i++)
    {
        std::string arg(argv[i]);
        auto findResult = arg.find("-h");
        if(findResult != std::string::npos)
        {
           printHelp();
           return false;
        }

        findResult = arg.find("-f=");
        if(findResult != std::string::npos)
        {
            filename = arg.substr(arg.find_first_of("=")+1);
            options.fstream = std::ifstream(filename);
            if(!options.fstream.good())
            {
                std::cerr << "ERROR: file " << filename << " is not exist\n";
                return false;
            }
        }

        findResult = arg.find("-t=");
        if(findResult != std::string::npos)
        {
            options.threadPoolSize = std::atoi(arg.substr(arg.find_first_of("=")+1).c_str());
            if(!options.threadPoolSize || options.threadPoolSize < 1 || options.threadPoolSize > 100)
            {
                std::cerr << "ERROR: failed setting thread pool size, it's value will be 1\n";
                options.threadPoolSize = 1;
            }
        }

        findResult = arg.find("-k=");
        if(findResult != std::string::npos)
        {
            int centroidsCount = std::atoi(arg.substr(arg.find_first_of("=")+1).c_str());
            if(centroidsCount > 1000)
            {
                centroidsCount = 1000;
                std::cerr << "ERROR: failed setting kluster centroids count, it's value will be 1000\n";
            }
            options.centroidsCount = centroidsCount;
            if(!options.centroidsCount || options.centroidsCount < 1)
            {
                std::cerr << "ERROR: failed setting kluster centroids count, it's value will be 3\n";
                options.centroidsCount = 3;
            }
        }

        findResult = arg.find("-m=");
        if(findResult != std::string::npos)
        {
            int maxIterations = std::atoi(arg.substr(arg.find_first_of("=")+1).c_str());
            if(maxIterations < 1)
            {
                maxIterations = 1000000;
                std::cerr << "ERROR: failed setting max iterations count, it's value will be 1000000\n";
            }
            options.maxIterations = maxIterations;
        }

        findResult = arg.find("-o=");
        if(findResult != std::string::npos)
        {
            options.outputFileName = arg.substr(arg.find_first_of("=")+1);
        }
    }
    if(filename.empty())
    {
        std::cerr << "ERROR: mandatory parameter -f is not presented \n";
        printHelp();
        return false;
    }
    if(options.threadPoolSize > options.centroidsCount)
    {
        std::cerr << "ERROR: thread pool size is bigger than klusters count \n";
        return false;
    }

    return true;
}
#endif
