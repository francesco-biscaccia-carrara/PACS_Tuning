#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <string.h>
#include <sys/time.h>
#include <vector>
#include <set>
#include <random>
#include <algorithm>
#include <iostream>
#include <cstdarg>
#include <cstdlib>

#ifndef MH_VERBOSE
#define MH_VERBOSE 1
#endif

#define EPSILON 1e-7

class RandNumGen{
    public: 
        static void setSeed(unsigned long long seed);
        static int randInt(int min, int max); 

    private:
        static std::mt19937_64 rng;
};


enum LogLevel {ERROR, WARN, INFO };

class Logger{

    public:
        
        static void print(LogLevel typeMsg, const char* msg, ...);

    private:
        static const std::string ANSI_COLOR_RED;
        static const std::string ANSI_COLOR_GREEN;
        static const std::string ANSI_COLOR_YELLOW;  
        static const std::string ANSI_COLOR_BLUE;    
        static const std::string ANSI_COLOR_MAGENTA; 
        static const std::string ANSI_COLOR_CYAN;    
        static const std::string ANSI_COLOR_RESET;   
    };

class Clock{

    public:
        static double  getTime();
        static double  timeElapsed(const double initTime);
};

#endif