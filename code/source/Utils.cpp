#include "../include/Utils.hpp"

using namespace Utils;

std::mt19937_64 RandNumGen::rng(std::random_device{}());

void RandNumGen::setSeed(unsigned long long seed){
    RandNumGen::rng.seed(seed);
}

//MIN and MAX are included
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


void ArgsParser::help(){
  Logger::print(Logger::ERROR,"To set the parameters properly you have to execute ./main and add: \
        \n '-file / -in / -f <filename>' to specity the input file;\
        \n '-time_limit / -tl / -time <time_dbl>' to specity the max execution time (double value);\
        \n '-theta_fix / -th / -theta <theta_dbl>' to specity the \\% of vars to fix in the initial vector (double value within (0,1));\
        \n '-seed / -sd / -rnd_seed <seed>' to specity the random seed (int value);");
}


ArgsParser::ArgsParser(int argc, char* argv[]): fileName {""}, timeLimit {0.0}, thetaFix {0.0}, seed {0} {

    std::set<std::string> fileNameInp {"-file","-in","-f"};
    std::set<std::string> timeLimitInp {"-time_limit","-tl","-time"};
    std::set<std::string> thetaFixInp {"-theta_fix","-th","-theta"};
    std::set<std::string> seedInp {"-seed","-sd","-rnd_seed"};

    for (size_t i {1}; i < argc; i++){
        if(fileNameInp.count(argv[i])) fileName = std::string(argv[++i]);
        if(timeLimitInp.count(argv[i])) timeLimit = std::atof(argv[++i]);
        if(thetaFixInp.count(argv[i])) thetaFix = std::atof(argv[++i]);
        if(seedInp.count(argv[i])) seed = std::atoi(argv[++i]);
    }

    if(fileName == "" || !timeLimit || !thetaFix || !seed) help();

    #if ACS_VERBOSE == 1
        Logger::print(Logger::INFO,"ENV contains \
                        \n\t - filename:\t%s \
                        \n\t - time-limit:\t%f\
                        \n\t - theta:\t%f\
                        \n\t - seed:\t%d",
                        fileName.c_str(),timeLimit,thetaFix,seed);
    #endif
}