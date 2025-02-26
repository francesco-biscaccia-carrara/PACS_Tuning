#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <array>
#include <cstdarg>
#include <cstdlib>
#include <iostream>
#include <random>
#include <set>
#include <sstream>
#include <string.h>
#include <string>
#include <sys/time.h>
#include <vector>

#ifndef ACS_VERBOSE
	#define ACS_VERBOSE 10
#endif

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
							  INFO };

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

	class ArgsParser {

	public:
		ArgsParser(int argc, char* argv[]);
		ArgsParser(const ArgsParser&) = delete;
		ArgsParser& operator=(const ArgsParser&) = delete;

		inline std::string		  getFileName() { return fileName; };
		inline double			  getTimeLimit() { return timeLimit; };
		inline double			  getTheta() { return theta; };
		inline double			  getRho() { return rho; };
		inline unsigned long long getSeed() { return seed; };

		~ArgsParser() = default;

	private:
		std::string		   fileName;
		double			   timeLimit;
		double			   theta;
		double			   rho;
		unsigned long long seed;
	};

} // namespace Utils

#endif