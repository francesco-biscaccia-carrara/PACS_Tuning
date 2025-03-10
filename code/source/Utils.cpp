#include "../include/Utils.hpp"

#define ARGS_CONV_BASE 10

using namespace Utils;

#pragma region STATIC

static std::mt19937_64	  rng{ std::random_device{}() };
static unsigned long long storedSeed{ 0 };
static bool				  isSeedSet{ false };

static std::string printHelp() {
	return "Usage: ./main [OPTIONS]\
        \n '-h  / --help'\t\t\t Show help message\
        \n '-f  / --filename <string>'\t Input file\
        \n '-tl / --timelimit <double>'\t Max execution time;\
        \n '-th / --theta <double (0,1)>'\t %% of vars to fix in the initially;\
        \n '-rh / --rho <double (0,1)>'\t %% of vars to fix per ACS iteration;\
        \n '-sd / --seed <u long long>'\t Random seed;\
		\n '-cpus/ --CPLEXCpus <u long>'\t Number of CPUs for an instance of CPLEX;";
}

#pragma endregion

void Random::setSeed(unsigned long long newSeed) {
	rng.seed(newSeed);
	storedSeed = newSeed;
	isSeedSet = true;
}

#if ACS_VERBOSE >= VERBOSE
unsigned long long Random::getSeed() {
	return storedSeed;
}
#endif

// MIN and MAX are included
int Random::Int(int min, int max) {
	if (!isSeedSet)
		Logger::print(Logger::LogLevel::WARN, "Using OS generated seed!");

	if (min > max)
		return Random::Int(max, min);
	std::uniform_int_distribution get{ min, max };
	return get(rng);
}

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
		default:
			msgClr = ANSI_COLOR_RESET;
			msgPref = "";
			break;
	}

	printf("%s\033[1m\033[4m%s%s%s: ", msgClr, msgPref, ANSI_COLOR_RESET, msgClr);

	va_list args;
	va_start(args, format);
	vfprintf(stdout, format, args);
	va_end(args);

	printf("%s\n", ANSI_COLOR_RESET);
}

double Clock::getTime() {
	return MPI_Wtime();
}

double Clock::timeElapsed(const double initTime) {
	return MPI_Wtime() - initTime;
}

CLIParser::CLIParser(int argc, char* argv[]) : args{ .fileName{ "" }, .timeLimit{ 0.0 }, .theta{ 0.0 }, .rho{ 0.0 }, .CPLEXCpus{ 0 }, .seed{ 0 } } {
	if (argc > 0 && argv != nullptr) {

		constexpr std::array<std::pair<const char*, std::string Args::*>, 2> stringArgs{ {
			{ "-f", &Args::fileName },
			{ "--filename", &Args::fileName },
		} };

		constexpr std::array<std::pair<const char*, unsigned long Args::*>, 2> uLongArgs{ {
			{ "-cpus", &Args::CPLEXCpus },
			{ "--CPLEXCpus", &Args::CPLEXCpus },
		} };

		constexpr std::array<std::pair<const char*, double Args::*>, 6> doubleArgs{ {
			{ "-tl", &Args::timeLimit },
			{ "--timelimit", &Args::timeLimit },
			{ "-th", &Args::theta },
			{ "--theta", &Args::theta },
			{ "-rh", &Args::rho },
			{ "--rho", &Args::rho },
		} };

		constexpr std::array<std::pair<const char*, unsigned long long Args::*>, 2> ullongArgs{ {
			{ "-sd", &Args::seed },
			{ "--seed", &Args::seed },
		} };

		for (int i = 1; i < argc; ++i) {
			if (std::string(argv[i]) == "-h" || std::string(argv[i]) == "-help")
				throw ArgsParserException(printHelp());
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

		if (args.fileName.empty() || !args.timeLimit || !args.theta || !args.rho || !args.seed || !args.CPLEXCpus)
			throw ArgsParserException(printHelp());

#if ACS_VERBOSE >= VERBOSE
		Logger::print(Logger::LogLevel::INFO, "Parsed Arguments:\
                            \n\t - File Name :  \t%s \
                            \n\t - Time Limit : \t%f\
                            \n\t - Theta : \t\t%f\
                            \n\t - Rho : \t\t%f\
							\n\t - Seed : \t\t%d\
							\n\t - CPLEX CPUs : \t%d",
					  args.fileName.c_str(), args.timeLimit, args.theta, args.rho, args.seed, args.CPLEXCpus);
#endif
	} else {
		throw ArgsParserException(printHelp());
	}
}