#ifndef _PARSER_HPP_H
#define _PARSER_HPP_H
#include <vector>
#include <string>
#include <iostream>
#include <types.h>
#include <math.h>

static inline double getEachDouble(const char *& p, int num, int count, bool & noResult)
{
    noResult = true;
    count -=num;
    while(num)
    {
        if(*p != ' ')
        {
            num--;
            while(*p != ' ' && *p != '\0') p++;
        }
        else
        {
            p++;
        }
    }
    while(*p == ' ' ) p++;

    double r;
    r=0.0;
    bool neg = false;
    if (*p == '-') {
        neg = true;
        ++p;
        if(noResult) noResult = false;
    }
    while (*p >= '0' && *p <= '9') {
        r = (r*10.0) + (*p - '0');
        ++p;
        if(noResult) noResult = false;
    }
    if (*p == '.') {
        double f = 0.0;
        int n = 0;
        ++p;
        while (*p >= '0' && *p <= '9') {
            f = (f*10.0) + (*p - '0');
            ++p;
            ++n;
        }
        r += f / std::pow(10.0, n);
    }
    if (neg) {
        r = -r;
    }

    while(count)
    {
        if(*p != ' ')
        {
            count--;
            while(*p != ' ' && *p != '\0') p++;
        }
        else
        {
            p++;
        }
    }
    return r;
}

static inline double getDouble(const char *& p)
{
    while(*p == ' ') p++;
    static double r;
    r=0.0;
    bool neg = false;
    if (*p == '-') {
        neg = true;
        ++p;
    }
    while (*p >= '0' && *p <= '9') {
        r = (r*10.0) + (*p - '0');
        ++p;
    }
    if (*p == '.') {
        static double f;
        f = 0.0;
        int n = 0;
        ++p;
        while (*p >= '0' && *p <= '9') {
            f = (f*10.0) + (*p - '0');
            ++p;
            ++n;
        }
        r += f / std::pow(10.0, n);
    }
    if (neg) {
        r = -r;
    }
    return r;
}
static inline bool parsePoint(const char * string, std::vector<double> & pointDimensions)
{
    pointDimensions.clear();

    const char* p = string;
    while (*p != '\0')
    {
        pointDimensions.push_back(getDouble(p));
    }
    return !pointDimensions.empty();
}

static inline bool parsePointWithChecking(const char * string, std::vector<double> & pointDimensions)
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

    return !pointDimensions.empty();
}

#endif
