# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

### [Unreleased]
- Enhanced performance and efficiency  
- Reduced computational overhead  


# [1.0.5] - 2025-04-01
### Fixed
- Adjusted `Utils::EPSILON` zero-threshold for better numerical tolerance ([Utils.hpp](code/source/Utils.cpp)).
- Discretized negligible values in `OMIP::updateBudgetConstr` function ([OMIP.cpp](code/source/OMIP.cpp)).
- Discretized negligible values returned by `MIP::getObjFunction` function ([MIP.cpp](code/source/MIP.cpp)).
- Discretized negligible values returned by `MIP::getSol` function ([MIP.cpp](code/source/MIP.cpp)).


# [1.0.4] - 2025-03-31
### Fixed
- Fixed issue with `MIP::MIP_INT_TOL` (MIP Integrality Tolerance) in CPLEX optimization ([MIP.hpp](code/include/MIP.hpp)).
- Fixed issue with `MIP::MIP_SIMPLEX_FEAS_TOL` (MIP Simplex Feasibility Tolerance) in CPLEX optimization ([MIP.hpp](code/include/MIP.hpp)).


# [1.0.3] - 2025-03-30
### Fixed
- Resolved unexpected behavior in `Logger::print` function ([Utils.cpp](code/source/Utils.cpp))
- Resolved error related to MIP tolerance in CPLEX optimization (`MIP::MIP_GAP_TOL`) ([MIP.hpp](code/include/MIP.hpp))


# [1.0.2] - 2025-03-29
### Added
- Documentation for header files.

### Fixed
- Resolved unexpected behavior in `OMIP::getSlackSum` function ([OMIP.cpp](code/source/OMIP.cpp)).


# [1.0.1] - 2025-03-26
### Fixed
- Resolved issue in `FixPolicy::firstThetaFixing` function
- Optimized ACS Incumbent overhead in ACS


# [1.0.0] - 2025-03-20
### Added
- Initial implementation of the ACS algorithm  

### Fixed
- Ensured hardware-independent parallel computation  
- Enforced deterministic time limit for sub-MIP instance execution  
- Properly set up the CPLEX benchmark  

<!--
## [Unreleased]
- Add new changes here before the next release.

## [1.0.0] - YYYY-MM-DD
### Added
- Initial release of the project.

<!-- Add future versions below -->

<!--
## [0.1.0] - YYYY-MM-DD
### Added
- Project setup and initial development.-->
