#include "../include/Utils.hpp"

const std::string Logger::ANSI_COLOR_RED            ="\x1b[31m";
const std::string Logger::ANSI_COLOR_GREEN          ="\x1b[32m";
const std::string Logger::ANSI_COLOR_YELLOW         ="\x1b[33m";
const std::string Logger::ANSI_COLOR_BLUE           ="\x1b[34m";
const std::string Logger::ANSI_COLOR_MAGENTA        ="\x1b[35m";
const std::string Logger::ANSI_COLOR_CYAN           ="\x1b[36m";
const std::string Logger::ANSI_COLOR_RESET          ="\x1b[0m";
std::mt19937_64 RandNumGen::rng(std::random_device{}());

void RandNumGen::setSeed(unsigned long long seed){
    RandNumGen::rng.seed(seed);
}

int RandNumGen::randInt(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(rng);
}


void Logger::print(LogLevel typeMsg, const char* msg, ...){
    std::string msgClr;
    std::string msgPref;

    switch (typeMsg) {
        case ERROR:
            msgClr = Logger::ANSI_COLOR_RED; // ANSI_COLOR_RED
            msgPref = "ERROR";
            break;

        case WARN:
            msgClr = Logger::ANSI_COLOR_YELLOW ; // ANSI_COLOR_YELLOW
            msgPref = "WARNING";
            break;

        case INFO:
            msgClr = Logger::ANSI_COLOR_BLUE; // ANSI_COLOR_BLUE
            msgPref = "INFO";
            break;

        default:
            msgClr = Logger::ANSI_COLOR_RESET; // ANSI_COLOR_RESET
            msgPref = "";
            break;
    }

    std::cout << msgClr << "\033[1m\033[4m" << msgPref << Logger::ANSI_COLOR_RESET << msgClr << ": ";

    va_list ap;
    va_start(ap, msg);
    vprintf(msg,ap);
    va_end(ap);

    std::cout << Logger::ANSI_COLOR_RESET << std::endl;

    if (typeMsg == ERROR) std::exit(1);
}


double Clock::getTime(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    return ((double)tv.tv_sec)+((double)tv.tv_usec/1e+6);
}


double Clock::timeElapsed(const double initTime) {
    
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return ((double)tv.tv_sec)+((double)tv.tv_usec/1e+6) - initTime;
}