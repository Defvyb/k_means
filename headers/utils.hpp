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
            if(!options.threadPoolSize)
            {
                std::cerr << "ERROR: failed setting thread pool size, it's value will be 1\n";
                options.threadPoolSize = 1;
            }
        }

        findResult = arg.find("-k=");
        if(findResult != std::string::npos)
        {
            int klusterCentroidsCount = std::atoi(arg.substr(arg.find_first_of("=")+1).c_str());
            if(klusterCentroidsCount > 1000)
            {
                klusterCentroidsCount = 1000;
                std::cerr << "ERROR: failed setting kluster centroids count, it's value will be 1000\n";
            }
            options.klusterCentroidsCount = klusterCentroidsCount;
            if(!options.klusterCentroidsCount)
            {
                std::cerr << "ERROR: failed setting kluster centroids count, it's value will be 10\n";
                options.klusterCentroidsCount = 10;
            }
        }
    }
    if(filename.empty())
    {
        std::cerr << "ERROR: mandatory parameter -f is not presented \n";
        printHelp();
        return false;
    }

    return true;
}
#endif
