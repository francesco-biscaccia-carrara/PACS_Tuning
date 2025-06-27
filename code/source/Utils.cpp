#include "../include/Utils.hpp"

using namespace Utils;
using ExType = ACSException::ExceptionType;

Random::Random(unsigned long long newSeed) : seed{ newSeed } {
	rng.seed(newSeed);
}

#if ACS_VERBOSE >= VERBOSE
unsigned long long Random::getSeed() {
	return seed;
}
#endif

// MIN and MAX are included
int Random::Int(int min, int max) {
	if (min > max)
		return Random::Int(max, min);

	std::uniform_int_distribution get{ min, max };
	return get(rng);
}

// MIN and MAX are included
double Random::Double(double min, double max) {
	if (min > max)
		return Random::Double(max, min);

	std::uniform_real_distribution get{ min, max };
	return get(rng);
}

void Logger::print(LogLevel typeMsg, const char* format, ...) {
#if !ACS_TEST
	const char* msgClr;
#endif
	const char* msgPref;

	switch (typeMsg) {
		case LogLevel::ERROR:
#if !ACS_TEST
			msgClr = ANSI_COLOR_RED;
#endif
			msgPref = "[  ERR  ]";
			break;
		case LogLevel::WARN:
#if !ACS_TEST
			msgClr = ANSI_COLOR_YELLOW;
#endif
			msgPref = "[  WAR  ]";
			break;
		case LogLevel::INFO:
#if !ACS_TEST
			msgClr = ANSI_COLOR_BLUE;
#endif
			msgPref = "[  DEB  ]";
			break;
		case LogLevel::OUT:
#if !ACS_TEST
			msgClr = ANSI_COLOR_RESET;
#endif
			msgPref = "[  OUT  ]";
			break;
		case LogLevel::BEST:
#if !ACS_TEST
			msgClr = ANSI_COLOR_GREEN;
#endif
			msgPref = "[  OUT* ]";
			break;
		default:
#if !ACS_TEST
			msgClr = ANSI_COLOR_RESET;
#endif
			msgPref = "";
			break;
	}

	/// FIXED: Bug #5860f1916463f69833a7cb9170845d492fabee8f –- Segmentation fault caused by overlapping printf calls.
	
flockfile(stdout);
#if ACS_TEST
	printf("%s|%8.2f|", msgPref, Clock::timeElapsed());
#else
	printf("%s\033[1m\033[4m%s%s%s|%8.2f|", msgClr, msgPref, ANSI_COLOR_RESET, msgClr, Clock::timeElapsed());
#endif

	va_list args;
	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);

#if ACS_TEST
	printf("\n");
#else
	printf("%s\n", ANSI_COLOR_RESET);
#endif

	fflush(stdout);

	funlockfile(stdout);
}

double Clock::getTime() {
	auto now = std::chrono::steady_clock::now();
	return std::chrono::duration<double>(now.time_since_epoch()).count();
}

double Clock::timeElapsed(const double initTime) {
	return getTime() - initTime;
}

double Clock::timeRemaining(const double timeLimit) {
	return timeLimit - timeElapsed();
}

constexpr const char* HELP_ACS = R"(
ACS - Alternating Criteria Search Optimizer
Version: )" ACS_VERSION R"(| Last Update: )" LAST_UPDATE R"(

DESCRIPTION:
    Advanced implementation of the Alternating Criteria Search algorithm for 
    Mixed Integer Programming problems with parallel subMIP solving capabilities.

USAGE:
    ./ACS -f <filename> -tl <seconds> [OPTIONS]

OPTIONS:
    General:
      -h, --help                    Display this help message and exit
      
    Input/Output:
      -f, --filename <name>         Input problem filename (required)
                                    File must be located in /data/ directory
                                    Extension .mps.gz will be added automatically
                                    Example: -f "problem" loads ./data/problem.mps.gz
      
    Algorithm Parameters:
      -tl, --timelimit <seconds>    Maximum execution time in seconds (required)

      -rh, --rho <ratio>            Iteration variable fixing ratio (0.0-1.0)  
                                    Percentage of variables to fix per ACS iteration
                                    (default: 0.1, optimal for most problems)

      -th, --theta <ratio>          Initial variable fixing ratio (0.0-1.0)
                                    Percentage of variables to fix in initial vector
                                    (default: 0.5)	
			
      -ag, --algorithm <id>         ONLY FOR DEBUG PURPOSE, WILL BE REMOVED
                                    
                                    
    Parallelization:
      -nSMIPs, --numsubMIPs <num>   Number of parallel subMIPs
                                    (default: 4, optimal for most systems)
                                    Note: Higher values may not improve performance
                                    
    Miscellaneous:
      -sd, --seed <value>           Random seed for reproducible results
                                    (default: random system generated value)

EXAMPLES:
    ./ACS -f problem -tl 1800 -rh 0.6
    ./ACS --filename instance01 --timelimit 3600 --numsubMIPs 8 --seed 12345

EXIT CODES:
    Success:
      0      Success - Heuristic found
      
    General Errors:
      1      General program error (unexpected behavior - report it)

    ACS Algorithm Errors:
      201    Check feasibility failed - Solution violates constraints
      202    Check integrality failed - Solution has non-integer values
      203    Check objective failed - Objective function evaluation error
      
    Input/File Errors:
      210    Wrong time limit - Invalid time limit parameter specified
      211    File not found - Verify filename and data/ directory
      212    Input size error - Problem dimensions exceed limits
      213    Wrong argument values - Invalid parameter values provided
      
    MIP Solver Errors:
      204    Model creation error - Problem with MIP model setup
      205    Get function error - Failed to retrieve MIP solver data
      206    Set function error - Failed to set MIP solver parameters
      207    Out of bounds error - MIP array/vector index violation
      208    MIP optimization error - Mixed Integer Programming solver failed
      209    LP optimization error - Linear Programming solver failed

For detailed error descriptions and handling, see ACSException.hpp

For more information, visit: https://github.com/francesco-biscaccia-carrara/MIP_Heuristic
Report bugs to: francesco.biscaccia.carrara@gmail.com
)";

constexpr const char* HELP_CPLEXRUN = "Usage: ./CPLEXRun <PARS>\
        \n '-h  / --help'\t\t\t\t Show help message\
        \n '-f  / --filename <string>'\t\t Input file\
        \n '-tl / --timelimit <double>'\t\t Max execution time";

CLIParser::CLIParser(int argc, char* argv[], bool CPLEXRun) : 
	args{.fileName = "", 
			.timeLimit = 0.0, .theta = 0.0, 
			.rho = DEF_RHO, 
			.numsubMIPs = DEF_SUBMIPS, 
			.seed = 0, 
			.algo = 0 } 
			
	{ if (argc > 0 && argv != nullptr) {
		for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "-h" || arg == "--help") {
                if (CPLEXRun) {
                    printf("%s\n", HELP_CPLEXRUN);
                } else {
                    printf("%s\n", HELP_ACS);
                }
                std::exit(EXIT_SUCCESS);
            }
        }

		constexpr std::array<std::pair<const char*, std::string Args::*>, 2> stringArgs{ {
			{ "-f", &Args::fileName },
			{ "--filename", &Args::fileName },
		} };

		constexpr std::array<std::pair<const char*, unsigned long Args::*>, 4> uLongArgs{ {
			{ "-nSMIPs", &Args::numsubMIPs },
			{ "--numsubMIPs", &Args::numsubMIPs },
			{ "-ag", &Args::algo },
			{ "--algorithm", &Args::algo },
		} };

		constexpr std::array<std::pair<const char*, double Args::*>, 6> doubleArgs{ { { "-tl", &Args::timeLimit },
																					  { "--timelimit", &Args::timeLimit },
																					  { "-th", &Args::theta },
																					  { "--theta", &Args::theta },
																					  { "-rh", &Args::rho },
																					  { "--rho", &Args::rho } } };

		constexpr std::array<std::pair<const char*, unsigned long long Args::*>, 2> ullongArgs{ {
			{ "-sd", &Args::seed },
			{ "--seed", &Args::seed },
		} };

		for (int i = 1; i < argc - 1; i++) {
			std::string key = argv[i];

			for (const auto& [flag, member] : stringArgs) {
				if (key == flag) {
					args.*member = argv[++i];
					break;
				}
			}

			for (const auto& [flag, member] : doubleArgs) {
				if (key == flag) {
					args.*member = std::strtod(argv[++i], nullptr);
					break;
				}
			}

			for (const auto& [flag, member] : uLongArgs) {
				if (key == flag) {
					args.*member = std::strtoul(argv[++i], nullptr, ARGS_CONV_BASE);
					break;
				}
			}

			for (const auto& [flag, member] : ullongArgs) {
				if (key == flag) {
					args.*member = std::strtoull(argv[++i], nullptr, ARGS_CONV_BASE);
					break;
				}
			}
		}

		if (CPLEXRun) {
			if (args.fileName.empty() || !args.timeLimit){
				printf("%s\n", HELP_CPLEXRUN);
				throw ArgsParserException(ExType::WrongArgsValue,"Wrong values passed as CLI args");
			}	
		} else {
			if (args.fileName.empty() || !args.timeLimit || !args.theta || !args.rho || !args.seed || !args.numsubMIPs){
				printf("%s\n", HELP_ACS);
				throw ArgsParserException(ExType::WrongArgsValue,"Wrong values passed as CLI args");
			}
		}

		printf("%s\t%s -- %s\n", argv[0] + 2, ACS_VERSION, LAST_UPDATE);
		std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		printf("Date:\t%s", std::ctime(&time));

#if ACS_VERBOSE >= VERBOSE
		if (CPLEXRun) {
			PRINT_INFO("Parsed Arguments:\
							\n\t - File Name :  \t%s \
							\n\t - Time Limit : \t%f",
					   args.fileName.c_str(), args.timeLimit);
		} else {
			PRINT_INFO("Parsed Arguments:\
							\n\t - Algorithm :  \t%d \
                            \n\t - File Name :  \t%s \
                            \n\t - Time Limit : \t%f\
                            \n\t - Theta : \t\t%f\
                            \n\t - Rho : \t\t%f\
							\n\t - Seed : \t\t%d\
							\n\t - Num sub-MIP : \t%d",
					   args.algo, args.fileName.c_str(), args.timeLimit, args.theta, args.rho, args.seed, args.numsubMIPs);
		}

#endif
	} else {
		if (CPLEXRun) {
			printf("%s\n", HELP_CPLEXRUN);
		} else {
			printf("%s\n", HELP_ACS);
		}
		throw ArgsParserException(ExType::InputSizeError, "No input are passed to CLIParser::CLIParser()");
	}
}
