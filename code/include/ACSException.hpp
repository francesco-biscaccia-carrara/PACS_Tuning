/**
 * @file ACSException.hpp
 * @brief Exception class for handling ACS-related errors
 *
 * @author Francesco Biscaccia Carrara
 * @version v1.2.8
 * @since 07/04/2025
 */

#ifndef ACS_EXC_H
#define ACS_EXC_H

#define CODE_ERROR_SAFE_OFFSET 200

#include <stdexcept>
#include <array>
#include <string>

class ACSException : public std::runtime_error {

public:
	/**
     * @enum ExceptionType
     * @brief Enumeration of possible ACS-related exception types
     */
	enum class ExceptionType : size_t{
        General,
		CheckFeasibilityFailed,
		CheckIntegralityFailed,
		CheckObectiveFailed,
		ModelCreation,
        GetFunction,
        SetFunction,
		OutOfBound,
		MIP_OptimizationError,
		LP_OptimizationError,
		WrongTimeLimit,
		FileNotFound,
		InputSizeError,
        WrongArgsValue,
		_count // Helper for array size
	};

	  /**
     * @brief Constructs an ACSException with a specific type and message
     * @param type The type of exception
     * @param message Detailed error message
     * @param className Name of the class that threw the exception
     */
	explicit ACSException(ExceptionType type, const std::string& message, const std::string className="-") : std::runtime_error(formatMessage(type, message,className)), errorCode(static_cast<size_t>(type)) {};
	size_t getErrorCode() const { return  CODE_ERROR_SAFE_OFFSET + errorCode; }

private:
	size_t errorCode;

	/// Static array of exception type names
	static constexpr std::array<const char*, static_cast<size_t>(ExceptionType::_count)> typeNames = {
		"_general-ex_",
		"CheckFeasibilityFailed",
		"CheckIntegralityFailed",
		"CheckObectiveFailed",
		"ModelCreation",
        "GetFunction",
        "SetFunction",
		"OutOfBounds",
		"MIP_OptimizationError",
		"LP_OptimizationError",
		"WrongTimeLimit",
		"FileNotFound",
		"InputSizeError",
        "WrongArgsValue"
    };

	/**
     * @brief Formats the exception message with type and details
     * @param type The type of exception
     * @param message Detailed error message
     * @param className Name of the class that threw the exception
     * @return Formatted error message string
     */
	static std::string formatMessage(ExceptionType type, const std::string& message, const std::string className="-") {
		return "ACSException: ["+ className +"] {" + std::string(typeNames[static_cast<size_t>(type)]) + "} - " + std::string(message);
	}
};

#endif