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

namespace Utils {

	struct VarBounds {
		double lowerBound;
		double upperBound;
	};

	namespace Random {
		void setSeed(unsigned long long newSeed);
		int	 Int(int min, int max);
	}; // namespace Random

	namespace Logger {
		enum class LogLevel { ERROR,
							  WARN,
							  INFO,
							  OUT };

		void print(LogLevel typeMsg, const char* msg, ...);

		constexpr const char* ANSI_COLOR_RED{ "\x1b[31m" };
		constexpr const char* ANSI_COLOR_GREEN{ "\x1b[32m" };
		constexpr const char* ANSI_COLOR_YELLOW{ "\x1b[33m" };
		constexpr const char* ANSI_COLOR_BLUE{ "\x1b[34m" };
		constexpr const char* ANSI_COLOR_MAGENTA{ "\x1b[35m" };
		constexpr const char* ANSI_COLOR_CYAN{ "\x1b[36m" };
		constexpr const char* ANSI_COLOR_RESET{ "\x1b[0m" };

	}; // namespace Logger

	class Clock {

	public:
		static double getTime();
		static double timeElapsed(const double initTime);
	};

	class ArgsParserException : public std::runtime_error {

	public:
		explicit ArgsParserException(const std::string& message) : std::runtime_error("ArgsParserException: " + message){};
	};

	class CLIParser {

	public:
		static CLIParser& getInstance(int argc = 0, char* argv[] = nullptr);
		CLIParser(const CLIParser&) = delete;
		CLIParser& operator=(const CLIParser&) = delete;

		inline std::string		  getFileName() noexcept { return fileName; };
		inline double			  getTimeLimit() noexcept { return timeLimit; };
		inline double			  getTheta() noexcept { return theta; };
		inline double			  getRho() noexcept { return rho; };
		inline unsigned long long getSeed() noexcept { return seed; };

		~CLIParser() = default;

	private:
		CLIParser(int argc = 0, char* argv[] = nullptr);
		std::string		   fileName;
		double			   timeLimit;
		double			   theta;
		double			   rho;
		unsigned long long seed;
		static bool		   init; // Singleton initilization
	};

} // namespace Utils

#endif