#ifndef _PARSER_HPP_H
#define _PARSER_HPP_H
#include <vector>
#include <string>
#include <iostream>
#include <types.h>
static bool parsePoint(const char * string, std::vector<double> & pointDimensions)
{
    pointDimensions.clear();

    const char* p = string;
    char* end;

    for (double f = std::strtod(p, &end); p != end; f = std::strtod(p, &end))
       {
           p = end;
           if (errno == ERANGE){
               std::cerr << "ERROR: failed to parse value from string" << string << "\n";
               errno = 0;
               return false;
           }
           pointDimensions.push_back(f);

           if(pointDimensions.size() > MAX_ELEMENT_COUNT)
           {
               std::cerr << "ERROR: there are more than "<< MAX_ELEMENT_COUNT<< " elements in string: " << string << "\n";
               return false;
           }

       }
    return true;
}

#endif
