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

#ifndef ACS_VERBOSE
    #define ACS_VERBOSE 1
#endif

#define EPSILON 1e-7

namespace Utils{

    class RandNumGen{
        public: 
            static void setSeed(unsigned long long seed);
            static int randInt(int min, int max); 
    
        private:
            static std::mt19937_64 rng;
    };
    

    namespace Logger{
        enum LogLevel {ERROR, WARN, INFO};
        
        void print(LogLevel typeMsg, const char* msg, ...);
    
        constexpr const char*   ANSI_COLOR_RED          {"\x1b[31m"};
        constexpr const char*   ANSI_COLOR_GREEN        {"\x1b[32m"};
        constexpr const char*   ANSI_COLOR_YELLOW       {"\x1b[33m"};
        constexpr const char*   ANSI_COLOR_BLUE         {"\x1b[34m"};
        constexpr const char*   ANSI_COLOR_MAGENTA      {"\x1b[35m"};
        constexpr const char*   ANSI_COLOR_CYAN         {"\x1b[36m"};
        constexpr const char*   ANSI_COLOR_RESET        {"\x1b[0m"};         

    };
    
    class Clock{
    
        public:
            static double  getTime();
            static double  timeElapsed(const double initTime);
    };
    

    class ArgsParser{
    
        public:
            ArgsParser(int argc, char* argv[]);
            ArgsParser(const ArgsParser&) = delete;
            ArgsParser& operator=(const ArgsParser&) = delete;

            inline  std::string         getFileName(){return fileName;};
            inline  double              getTimeLimit(){return timeLimit;};
            inline  double              getThetaFix(){return thetaFix;};
            inline  unsigned long long  getSeed(){return seed;};
    
            ~ArgsParser()   =  default;
    
        private:
            void                help();
            std::string         fileName;
            double              timeLimit;
            double              thetaFix;
            unsigned long long  seed;
    };

}



#endif