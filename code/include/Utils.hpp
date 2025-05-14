/**
 * @file Utils.hpp
 * @brief Utility header file providing various helper classes and macros for the project.
 *
 * @author Francesco Biscaccia Carrara
 * @version v1.1.0 - InitSol v0.0.8
 * @since 05/14/2025
 */

#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <numeric>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>

#include <string.h>
#include <sys/time.h>

#include "ACSException.hpp"

#pragma region UTILS_DEFINTION

/** Current version of the code */
#define ACS_VERSION "v1.1.0 - InitSol v0.0.8"
/** Last update date */
#define LAST_UPDATE "05/14/2025"

/** Verbosity level constants */
#define NO_VER -1
#define DEFAULT 0
#define VERBOSE 1
#define DEBUG 10

/** Mathematical and computational constants */
#define EPSILON 1e-5
#define ARGS_CONV_BASE 10

/** Time limit regulation constants */
#define MAX_DET_TL 1e7
#define MIN_DET_TL 1e3
#define SCALE 1e2

#pragma endregion

#pragma region UTILS_MACRO

/**
 * Compute the deterministic-timelimit based on non-zeros entries in the matrix (nnz)
 * @param nnz Number of non-zero entries
 * @return Calculated deterministic time limit
 */
#define DET_TL(nnz) std::max(MIN_DET_TL, std::min((double)(nnz / SCALE), MAX_DET_TL))

/** Macro for printing error messages */
#define PRINT_ERR(...) Logger::print(Logger::LogLevel::ERROR, __VA_ARGS__)
/** Macro for printing warning messages */
#define PRINT_WARN(...) Logger::print(Logger::LogLevel::WARN, __VA_ARGS__)
/** Macro for printing informational messages */
#define PRINT_INFO(...) Logger::print(Logger::LogLevel::INFO, __VA_ARGS__)
/** Macro for printing standard output messages */
#define PRINT_OUT(...) Logger::print(Logger::LogLevel::OUT,  __VA_ARGS__)
/** Macro for printing best result messages */
#define PRINT_BEST(...) Logger::print(Logger::LogLevel::BEST, __VA_ARGS__)

#pragma endregion

namespace Utils {

	/**
	 * @brief Configuration parameters for the Alternating Criteria Search (ACS) algorithm
	 *
	 * This struct captures the command-line interface (CLI) arguments
	 * used to configure the ACS algorithm execution.
	 */
	struct Args {
		std::string		   fileName;   ///< Input file name
		double			   timeLimit;  ///< Time limit for execution
		double			   theta;	   ///< Theta parameter
		double			   rho;		   ///< Rho parameter
		unsigned long	   numsubMIPs; ///< Number of sub-MIPs
		unsigned long long seed;	   ///< Random number generator see
		unsigned long	   algo;	   ///< Flag to set the type of algorithm (0,1,2,3,4)
	};

	/**
	 * @brief Extracts the corresponding JSON filename from an environment file path.
	 *
	 * @param envFilePath The full path to the environment file.
	 * @return A string representing the JSON filename derived from the input path.
	 */
	std::string getJSONFilename(const std::string& envFilePath);

	/**
	 * Provides random number generation functionality.
	 *
	 * Uses a 64-bit Mersenne Twister random number generator.
	 */
	class Random {
	public:
		/**
		 * Constructs a random number generator with a specific seed.
		 *
		 * @param newSeed The seed value for the random number generator
		 */
		Random(unsigned long long newSeed);

		/**
		 * Generates a random integer within a specified range.
		 *
		 * @param min Minimum value of the range (inclusive)
		 * @param max Maximum value of the range (inclusive)
		 * @return A random integer within the specified range
		 */
		int Int(int min, int max);

		/**
		 * Generates a random double within a specified range.
		 *
		 * @param min Minimum value of the range (inclusive)
		 * @param max Maximum value of the range (inclusive)
		 * @return A random double within the specified range
		 */
		double Double(double min, double max);

		/**
		 * Gets the current seed of the random number generator.
		 * Only available when verbose mode is on.
		 *
		 * @return The current seed value
		 */
#if ACS_VERBOSE >= VERBOSE
		unsigned long long getSeed();
#endif
		/**
		 * Destructor for the Random class.
		 */
		~Random() = default;

	private:
		std::mt19937_64	   rng;	 ///< Mersenne Twister random number generator
		unsigned long long seed; ///< Seed value for the random number generator

	}; // namespace Random

	/**
	 * Logging utility with support for different log levels and color-coded output.
	 */
	namespace Logger {

		/**
		 * Enumeration of log levels.
		 */
		enum class LogLevel { ERROR,
							  WARN,
							  INFO,
							  OUT,
							  BEST,
							  _NO };

		/**
		 * Prints a formatted log message with a specified log level.
		 *
		 * @param typeMsg The log level of the message
		 * @param msg Formatting string for the message
		 * @param ... Variable arguments to be formatted
		 */
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

		/** Initial time value */
		inline double initTime{ 0.0 };

		/**
		 * Retrieves the current time.
		 *
		 * @return Current time as a double
		 */
		double getTime();

		/**
		 * Calculates the time elapsed since a given initial time.
		 *
		 * @param newInitTime Initial time to calculate elapsed time from (defaults to initTime)
		 * @return Time elapsed
		 */
		double timeElapsed(const double newInitTime = initTime);

		/**
		 * Calculates the remaining time based on a time limit.
		 *
		 * @param timeLimit Total allowed time
		 * @return Remaining time
		 */
		double timeRemaining(const double timeLimit);
	}; // namespace Clock

	/**
	 * Custom exception for command-line argument parsing errors.
	 */
	class ArgsParserException : public ACSException{
		public :
			ArgsParserException(ExceptionType type,const std::string& message) : ACSException(type, message, "ArgsParser") {}
	};

	/**
	 * Command-line argument parser utility.
	 *
	 * Provides functionality to parse and manage command-line arguments.
	 * Implements copy and move constructors/assignment operators as deleted
	 * to prevent unintended copying.
	 */
	class CLIParser {

	public:
		/**
		 * Constructs a CLIParser with optional command-line arguments.
		 *
		 * @param argc Number of command-line arguments (default 0)
		 * @param argv Array of command-line argument strings (default nullptr)
		 * @param CPLEXRun Flag indicating if this is a CPLEX run (default false)
		 */
		CLIParser(int argc = 0, char* argv[] = nullptr, bool CPLEXRun = false);

		// Prevent copying
		CLIParser(const CLIParser&) = delete;
		CLIParser& operator=(const CLIParser&) = delete;

		// Prevent moving
		CLIParser(const CLIParser&&) = delete;
		CLIParser& operator=(const CLIParser&&) = delete;

		/**
		 * Retrieves the parsed arguments.
		 *
		 * @return Parsed command-line arguments
		 */
		inline Args getArgs() { return args; }

		/**
		 * Destructor for the CLIParser class.
		 */
		~CLIParser() = default;

	private:
		Args args; ///< Stored parsed arguments
	};

}; // namespace Utils

#endif