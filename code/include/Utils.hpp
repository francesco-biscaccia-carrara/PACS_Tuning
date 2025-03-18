#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <array>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string.h>
#include <string>
#include <sys/time.h>
#include <vector>

#define NO_VER -1
#define DEFAULT 0
#define VERBOSE 1
#define DEBUG 10

#define EPSILON 1e-7

#define PRINT_ERR(format, ...) Logger::print(Logger::LogLevel::ERROR, format, ##__VA_ARGS__)
#define PRINT_WARN(format, ...) Logger::print(Logger::LogLevel::WARN, format, ##__VA_ARGS__)
#define PRINT_INFO(format, ...) Logger::print(Logger::LogLevel::INFO, format, ##__VA_ARGS__)
#define PRINT_OUT(format, ...) Logger::print(Logger::LogLevel::OUT, format, ##__VA_ARGS__)
#define PRINT_BEST(format, ...) Logger::print(Logger::LogLevel::BEST, format, ##__VA_ARGS__)

namespace Utils {

	struct VarBounds {
		double lowerBound;
		double upperBound;
	};

	struct Args {
		std::string		   fileName;
		double			   timeLimit, theta, rho, LNSDtimeLimit;
		unsigned long	   numsubMIPs;
		unsigned long long seed;
	};

	struct Solution {
		std::vector<double> sol;
		double				slackSum, oMIPCost;
	};

	class Random {
	public:
		Random(unsigned long long newSeed);

		int Int(int min, int max);
#if ACS_VERBOSE >= VERBOSE
		unsigned long long getSeed();
#endif
		~Random() = default;

	private:
		std::mt19937_64	   rng;
		unsigned long long seed;

	}; // namespace Random

	namespace Logger {
#if LOG
		inline FILE* logFile{ nullptr };
		void		 setFileLogName(Args CLIArgs);
		inline void	 closeFileLog() {
			 if (Logger::logFile != nullptr)
				 fclose(Logger::logFile);
		}
#endif

		enum class LogLevel { ERROR,
							  WARN,
							  INFO,
							  OUT,
							  BEST };

		void print(LogLevel typeMsg, const char* msg, ...);

		constexpr const char* ANSI_COLOR_RED{ "\x1b[31m" };
		constexpr const char* ANSI_COLOR_GREEN{ "\x1b[32m" };
		constexpr const char* ANSI_COLOR_YELLOW{ "\x1b[33m" };
		constexpr const char* ANSI_COLOR_BLUE{ "\x1b[34m" };
		constexpr const char* ANSI_COLOR_MAGENTA{ "\x1b[35m" };
		constexpr const char* ANSI_COLOR_CYAN{ "\x1b[36m" };
		constexpr const char* ANSI_COLOR_RESET{ "\x1b[0m" };

	}; // namespace Logger

	namespace Clock {
		inline double initTime{ 0.0 };
		double		  getTime();
		double		  timeElapsed(const double newInitTime = initTime);
		double		  timeRemaining(const double timeLimit);
	}; // namespace Clock

	class ArgsParserException : public std::runtime_error {

	public:
		explicit ArgsParserException(const std::string& message) : std::runtime_error("ArgsParserException: " + message){};
	};

	class CLIParser {

	public:
		CLIParser(int argc = 0, char* argv[] = nullptr);
		CLIParser(const CLIParser&) = delete;
		CLIParser& operator=(const CLIParser&) = delete;
		CLIParser(const CLIParser&&) = delete;
		CLIParser& operator=(const CLIParser&&) = delete;

		inline Args getArgs() { return args; }

		~CLIParser() = default;

	private:
		Args args;
	};

} // namespace Utils

#endif