#ifndef FIX_POL_H
#define FIX_POL_H

#include <type_traits>
#include "RlxFMIP.hpp"


using namespace Utils;

namespace FixPolicy {

	class FixPolicyException : public std::runtime_error {

	public:
		enum class ExceptionType {
			General,
			InputSizeError,
			_count // Helper for array size
		};

		explicit FixPolicyException(ExceptionType type, const std::string& message) : std::runtime_error(formatMessage(type, message)){};

	private:
		static constexpr std::array<const char*, static_cast<size_t>(ExceptionType::_count)> typeNames = {
			"_general-ex_",
			"InputSizeError"
		};

		static std::string formatMessage(ExceptionType type, const std::string& message) {
			return "FixPolicyException: [" + std::string(typeNames[static_cast<size_t>(type)]) + "] - " + std::string(message);
		}
	};

	void firstThetaFixing(std::vector<double>& x,std::string fileName, double theta, Random rnd);
	void randomRhoFix(const std::vector<double>& sol, MIP& model, const size_t threadID, double rho,const char* type, Random& rnd);
}; // namespace FixPolicy

#endif