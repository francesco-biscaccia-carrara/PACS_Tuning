#ifndef MER_POL_H
#define MER_POL_H

#include "Utils.hpp"

using namespace Utils;

namespace MergePolicy {

	class MergePolicyException : public std::runtime_error {

	public:
		enum class ExceptionType {
			General,
			InputSizeError,
			_count // Helper for array size
		};

		explicit MergePolicyException(ExceptionType type, const std::string& message) : std::runtime_error(formatMessage(type, message)){};

	private:
		static constexpr std::array<const char*, static_cast<size_t>(ExceptionType::_count)> typeNames = {
			"_general-ex_",
			"InputSizeError"
		};

		static std::string formatMessage(ExceptionType type, const std::string& message) {
			return "MergePolicyException: [" + std::string(typeNames[static_cast<size_t>(type)]) + "] - " + std::string(message);
		}
	};

	std::vector<std::pair<int, double>> recombine(const std::vector<double>& x, int numProc, const char* phase);
}; // namespace MergePolicy

#endif