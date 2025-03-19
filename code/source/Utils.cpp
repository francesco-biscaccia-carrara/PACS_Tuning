#include "../include/Utils.hpp"



using namespace Utils;

#pragma region STATIC

constexpr const char* HELP_ACS="Usage: ./ACS <PARS>\
        \n '-h  / --help'\t\t\t\t Show help message\
        \n '-f  / --filename <string>'\t\t Input file\
        \n '-tl / --timelimit <double>'\t\t Max execution time\
        \n '-th / --theta <double (0,1)>'\t\t %% of vars to fix in the initially vector\
        \n '-rh / --rho <double (0,1)>'\t\t %% of vars to fix per ACS iteration\
        \n '-sd / --seed <u long long>'\t\t Random seed\
		\n '-nSMIPs/ --numsubMIPs <u long>'\t Number of subMIP in the parallel phase";

constexpr const char* HELP_CPLEXRUN="Usage: ./CPLEXRun <PARS>\
        \n '-h  / --help'\t\t\t\t Show help message\
        \n '-f  / --filename <string>'\t\t Input file\
        \n '-tl / --timelimit <double>'\t\t Max execution time";

#pragma endregion

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

#if LOG
void Logger::setFileLogName(Args args) {
	std::ostringstream oss;
	oss << "../log/" << args.fileName
		<< "_tl-" << args.timeLimit
		<< "_th-" << args.theta
		<< "_rh-" << args.rho
		<< "_nMIP-" << args.numsubMIPs
		<< "_sd" << args.seed
		<< ".log";
	Logger::logFile = fopen(oss.str().c_str(), "w+");
}
#endif

void Logger::print(LogLevel typeMsg, const char* format, ...) {
	const char* msgClr;
	const char* msgPref;

	switch (typeMsg) {
		case LogLevel::ERROR:
			msgClr = ANSI_COLOR_RED;
			msgPref = "[ ERR  ]";
			break;
		case LogLevel::WARN:
			msgClr = ANSI_COLOR_YELLOW;
			msgPref = "[ WARN ]";
			break;
		case LogLevel::INFO:
			msgClr = ANSI_COLOR_BLUE;
			msgPref = "[ INFO ]";
			break;
		case LogLevel::OUT:
			msgClr = ANSI_COLOR_RESET;
			msgPref = "[ OUT  ]";
			break;
		case LogLevel::BEST:
			msgClr = ANSI_COLOR_GREEN;
			msgPref = "[ OUT* ]";
			break;
		default:
			msgClr = ANSI_COLOR_RESET;
			msgPref = "";
			break;
	}
#if LOG
	fprintf(Logger::logFile, "%s|%8.2f|", msgPref, Clock::timeElapsed());
#else
	printf("%s\033[1m\033[4m%s%s%s|%8.2f|", msgClr, msgPref, ANSI_COLOR_RESET, msgClr, Clock::timeElapsed());
#endif

	va_list args;
	va_start(args, format);
#if LOG
	vfprintf(Logger::logFile, format, args);
#else
	vfprintf(stdout, format, args);
#endif

	va_end(args);

#if LOG
	fprintf(Logger::logFile, "\n");
#else
	printf("%s\n", ANSI_COLOR_RESET);
#endif
}

double Clock::getTime() {
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return ((double)tv.tv_sec) + ((double)tv.tv_usec / 1e+6);
}

double Clock::timeElapsed(const double initTime) {
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return ((double)tv.tv_sec) + ((double)tv.tv_usec / 1e+6) - initTime;
}

double Clock::timeRemaining(const double timeLimit) {
	return timeLimit - timeElapsed();
}

CLIParser::CLIParser(int argc, char* argv[], bool CPLEXRun) : args{ .fileName = "", .timeLimit = 0.0, .theta = 0.0, .rho = 0.0, .numsubMIPs = 0, .seed = 0 } {
	if (argc > 0 && argv != nullptr) {

		constexpr std::array<std::pair<const char*, std::string Args::*>, 2> stringArgs{ {
			{ "-f", &Args::fileName },
			{ "--filename", &Args::fileName },
		} };

		constexpr std::array<std::pair<const char*, unsigned long Args::*>, 2> uLongArgs{ {
			{ "-nSMIPs", &Args::numsubMIPs },
			{ "--numsubMIPs", &Args::numsubMIPs },
		} };

		constexpr std::array<std::pair<const char*, double Args::*>, 6> doubleArgs{ { { "-tl", &Args::timeLimit },
																					  { "--timelimit", &Args::timeLimit },
																					  { "-th", &Args::theta },
																					  { "--theta", &Args::theta },
																					  { "-rh", &Args::rho },
																					  { "--rho", &Args::rho }} };

		constexpr std::array<std::pair<const char*, unsigned long long Args::*>, 2> ullongArgs{ {
			{ "-sd", &Args::seed },
			{ "--seed", &Args::seed },
		} };

		for (int i = 1; i < argc; ++i) {
			if (std::string(argv[i]) == "-h" || std::string(argv[i]) == "-help")
				if(CPLEXRun)
					throw ArgsParserException(HELP_CPLEXRUN);
				else
					throw ArgsParserException(HELP_ACS);
		}

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

#if LOG
		Logger::setFileLogName(args);
#endif
		if(CPLEXRun){
			if (args.fileName.empty() || !args.timeLimit)
				throw ArgsParserException(HELP_CPLEXRUN);
		}else{
			if (args.fileName.empty() || !args.timeLimit || !args.theta || !args.rho || !args.seed || !args.numsubMIPs)
				throw ArgsParserException(HELP_ACS);
		}
		

#if ACS_VERBOSE >= VERBOSE
	if(CPLEXRun)
		PRINT_INFO("Parsed Arguments:\
			\n\t - File Name :  \t%s \
			\n\t - Time Limit : \t%f",
			args.fileName.c_str(), args.timeLimit);
	else
		PRINT_INFO("Parsed Arguments:\
                            \n\t - File Name :  \t%s \
                            \n\t - Time Limit : \t%f\
                            \n\t - Theta : \t\t%f\
                            \n\t - Rho : \t\t%f\
							\n\t - Seed : \t\t%d\
							\n\t - Num sub-MIP : \t%d",
				   			args.fileName.c_str(), args.timeLimit, args.theta, args.rho, args.seed, args.numsubMIPs);
#endif
	} else {
		if(CPLEXRun)
			throw ArgsParserException(HELP_CPLEXRUN);
		else
			throw ArgsParserException(HELP_ACS);
	}
}
