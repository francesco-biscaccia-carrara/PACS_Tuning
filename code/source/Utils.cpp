#include "../include/Utils.hpp"

using namespace Utils;

#pragma region STATIC

static std::mt19937_64 rng{ std::random_device{}() };

static bool isSeedSet{ false };

static void printHelp() {
	Logger::print(Logger::LogLevel::ERROR, "Usage: ./main [OPTIONS]\
        \n '-h  / --help'\t\t\t Show help message\
        \n '-f  / --filename <string>'\t Input file\
        \n '-tl / --timelimit <double>'\t Max execution time;\
        \n '-th / --theta <double (0,1)>'\t %% of vars to fix in the initially;\
        \n '-rh / --rho <double (0,1)>'\t %% of vars to fix per ACS iteration;\
        \n '-sd / --seed <u long long>'\t Random seed;");
}

#pragma endregion

void Random::setSeed(unsigned long long newSeed) {
	rng.seed(newSeed);
	isSeedSet = true;
}

// MIN and MAX are included
int Random::Int(int min, int max) {
	if (!isSeedSet)
		Logger::print(Logger::LogLevel::WARN, "Using OS generated seed!");

    if(min > max)
		Random::Int(max, min);
	std::uniform_int_distribution get{ min, max };
	return get(rng);
}

void Logger::print(LogLevel typeMsg, const char* msg, ...) {
	std::string msgClr;
	std::string msgPref;

	switch (typeMsg) {
		case Logger::LogLevel::ERROR:
			msgClr = Logger::ANSI_COLOR_RED; // ANSI_COLOR_RED
			msgPref = "ERROR";
			break;

		case Logger::LogLevel::WARN:
			msgClr = Logger::ANSI_COLOR_YELLOW; // ANSI_COLOR_YELLOW
			msgPref = "WARNING";
			break;

		case Logger::LogLevel::INFO:
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
	vprintf(msg, ap);
	va_end(ap);

	std::cout << Logger::ANSI_COLOR_RESET << std::endl;

	if (typeMsg == Logger::LogLevel::ERROR)
		std::exit(1);
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

ArgsParser::ArgsParser(int argc, char* argv[]) : fileName{ "" }, timeLimit{ 0.0 }, theta{ 0.0 }, rho{ 0.0 }, seed{ 0 } {

	constexpr std::array<std::pair<const char*, std::string ArgsParser::*>, 2> stringArgs{ {
		{ "-f", &ArgsParser::fileName },
		{ "--filename", &ArgsParser::fileName },
	} };

	constexpr std::array<std::pair<const char*, double ArgsParser::*>, 6> doubleArgs{ {
		{ "-tl", &ArgsParser::timeLimit },
		{ "--timelimit", &ArgsParser::timeLimit },
		{ "-th", &ArgsParser::theta },
		{ "--theta", &ArgsParser::theta },
		{ "-rh", &ArgsParser::rho },
		{ "--rho", &ArgsParser::rho },
	} };

	constexpr std::array<std::pair<const char*, unsigned long long ArgsParser::*>, 2> ullongArgs{ {
		{ "-sd", &ArgsParser::seed },
		{ "--seed", &ArgsParser::seed },
	} };

	for (int i = 1; i < argc; ++i) {
		if (std::string(argv[i]) == "-h" || std::string(argv[i]) == "-help")
			printHelp();
	}

	for (int i = 1; i < argc - 1; i++) {
		std::string key = argv[i];

		for (const auto& [flag, member] : stringArgs) {
			if (key == flag) {
				this->*member = argv[++i];
				break;
			}
		}

		for (const auto& [flag, member] : doubleArgs) {
			if (key == flag) {
				this->*member = std::strtod(argv[++i], nullptr);
				break;
			}
		}

		for (const auto& [flag, member] : ullongArgs) {
			if (key == flag) {
				this->*member = std::strtoull(argv[++i], nullptr, 10);
				break;
			}
		}
	}

	if (fileName.empty() || !timeLimit || !theta || !rho || !seed)
		printHelp();

#if ACS_VERBOSE == DEBUG
	Logger::print(Logger::LogLevel::INFO, "Parsed Arguments:\
                            \n\t - File Name :  \t%s \
                            \n\t - Time Limit : \t%f\
                            \n\t - Theta : \t\t%f\
                            \n\t - Rho : \t\t%f\
                            \n\t - Seed : \t\t%d",
				  fileName.c_str(), timeLimit, theta, rho, seed);
#endif
}