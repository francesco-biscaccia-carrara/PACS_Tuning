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

constexpr const char* HELP_ACS = "Usage: ./ACS <PARS>\
        \n '-h  / --help'\t\t\t\t Show help message\
        \n '-f  / --filename <string>'\t\t Input file\
        \n '-tl / --timelimit <double>'\t\t Max execution time\
        \n '-th / --theta <double (0,1)>'\t\t %% of vars to fix in the initially vector\
        \n '-rh / --rho <double (0,1)>'\t\t %% of vars to fix per ACS iteration\
        \n '-sd / --seed <u long long>'\t\t Random seed\
		\n '-nSMIPs/ --numsubMIPs <u long>'\t Number of subMIP in the parallel phase\
		\n '-ag/ --algorithm <u long>'\t\t Type of algorithm for the initial vector (tbd value<->algo)";

constexpr const char* HELP_CPLEXRUN = "Usage: ./CPLEXRun <PARS>\
        \n '-h  / --help'\t\t\t\t Show help message\
        \n '-f  / --filename <string>'\t\t Input file\
        \n '-tl / --timelimit <double>'\t\t Max execution time";

CLIParser::CLIParser(int argc, char* argv[], bool CPLEXRun) : args{ .fileName = "", .timeLimit = 0.0, .theta = 0.0, .rho = 0.0, .numsubMIPs = 0, .seed = 0, .algo = 0 } {
	if (argc > 0 && argv != nullptr) {

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
