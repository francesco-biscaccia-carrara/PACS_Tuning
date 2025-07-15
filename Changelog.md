# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.2.11] - 2025-07-15  
### Added
- Constraint matrix is now shared as a bipartite graph among instances of MIP by the `MIP::MIPVarToConstr` adn `MIP::MIPVarToConstr` vector ([MIP.hpp](code/include/MIP.hpp)).  

### Changed 
- Enanched `FixPolicy::walkMIPMT` diversifactin by applying a 'kick' with a given probability ([FixPolicy.cpp](code/source/FixPolicy.cpp)).  


## [1.2.10] - 2025-07-09  
### Fixed  
- Optimized `FixPolicy::walkMIPMT` by reducing redundant computations through in-place recomputation of violated constraints during execution ([FixPolicy.cpp](code/source/FixPolicy.cpp)).  
- Improved performance of `MIP::getViolatedConstrIndex` by operating on references instead of returning a `std::vector<int>` ([MIP.hpp](code/include/MIP.hpp)).  


## [1.2.9] - 2025-07-06  
### Fixed  
- Optimized `FixPolicy::walkMIPMT` to reduce computational overhead while enhancing solution diversification ([FixPolicy.cpp](code/source/FixPolicy.cpp)).  
- Fixed potential race conditions in `FixPolicy::randomRhoFixMT` ([FixPolicy.cpp](code/source/FixPolicy.cpp)). 


## [1.2.8] - 2025-07-04  
### Added  
- Introduced new fixing strategy `FixPolicy::walkMIPMT` inspired by the WalkSAT algorithm ([FixPolicy.cpp](code/source/FixPolicy.cpp)).  
- Enabled sparse matrix retrieval, mimicking CPLEX’s structure, using a static vector to share the model across multiple MIP instances ([MIP.cpp](code/source/MIP.cpp)).  

### Changed  
- Renamed `Args::theta` to `Args::walkProb` to reflect the removal of the unused theta parameter ([Utils.hpp](code/include/Utils.hpp)).  
- Made `FMIP::ogObjFun` static to allow sharing the original objective function across FMIP instances ([FMIP.hpp](code/include/FMIP.hpp)).  
 

## [1.2.7] - 2025-06-27  
### Added  
- Introduced error codes based on `ExceptionType`, enabling streamlined error handling and integration with ACS pipelines ([ACSException.hpp](code/include/ACSException.hpp)).  

### Changed  
- Removed `OMIP::updateBudgetConstr`, as its role is now handled by `FixPolicy::fixSlackUpperBound` ([OMIP.cpp](code/source/OMIP.cpp)).  
- Enhanced CLI support: `-h/--help` now provides a more detailed help message ([Utils.cpp](code/source/Utils.cpp)).  


## [1.2.6] - 2025-06-23  
### Fixed  
- Resolved potential infeasibility caused by incorrect signed comparison of slack sums in `MTContext::setBestACSIncumbent` ([MTContext.cpp](code/source/MTContext.cpp)).  

### Changed  
- `MergeOMIP` is now executed before exiting on feasible solution discovery, potentially improving final solution quality ([ACS.cpp](code/source/ACS.cpp)). 


## [1.2.5] - 2025-06-16  
### Fixed  
- Corrected bug in `MIP::checkIntegrality` related to the computation of the maximum integrality violation ([MIP.cpp](code/source/MIP.cpp)).  

### Changed  
- Objective value comparison now considers relative error, improving robustness ([ACS.cpp](code/source/ACS.cpp)).  


## [1.2.4] - 2025-06-13  
### Added  
- Implemented `MIP::checkIntegrality` and `MIP::checkObjValue` without relying on CPLEX, enabling lightweight validation ([MIP.cpp](code/source/MIP.cpp)).  

### Fixed  
- Resolved inconsistency between actual and saved objective values due to incorrect slack sum comparison in `MTContext::setBestACSIncumbent` ([MTContext.cpp](code/source/MTContext.cpp)).  


## [1.2.3] - 2025-06-11
### Addded  
- Implemented `MIP::checkFeasibility` without relying on CPLEX, enabling lightweight feasibility checks ([MIP.cpp](code/source/MIP.cpp)).


## [1.2.1] - 2025-05-23  
### Added  
- Introduced `FixPolicy::fixSlackUpperBound`, a function that updates variable upper bounds prior to OMIP and FMIP optimization, based on the current ACS incumbent ([FixPolicy.cpp](code/source/FixPolicy.cpp)).


## [1.2.0] - 2025-05-23 
### Added  
- Introduced `FixPolicy::startSolMaxFeas`, a method to set the initial solution based on variable bounds and objective value ([FixPolicy.cpp](code/source/FixPolicy.cpp)).

### Fixed
- Suppressed all compiler warnings labeled as "WARNING" using pedantic flags and stricter suppression methods.  

### Changed  
- Centralized exception handling by unifying all exceptions into a single `ACSException` class ([ACSException.hpp](code/include/ACSException.hpp)).  
- Automated plot generation using GitHub Workflows ([plot.yml](.github/workflows/plot.yml)).  
- Updated performance testing to use a JSON file for result tracking.  
- Redesigned output formatting for message types in `Utils::Logger::print` ([Utils.cpp](code/source/Utils.cpp)).  
- Refactored timer handling logic in `Utils::Clock` for improved clarity and maintainability ([Utils.cpp](code/source/Utils.cpp)).  


## [1.1.0] - 2025-04-08
### Added
- Early stop when a max number of solutions is found via `MIP::setNumSols` ([MIP.cpp](code/source/MIP.cpp)).
- Early stop when a feasible solution is found by the ACS algorithm ([ACS.cpp](code/source/ACS.cpp)).
- Support for dynamic adjustment of the `Args::rho` parameter in multi-threaded scenarios via `FixPolicy::dynamicAdjustRhoMT` ([FixPolicy.cpp](code/source/FixPolicy.cpp)).
- Support for dynamic adjustment of the `Args::rho` parameter during recombination phases via `FixPolicy::dynamicAdjustRho` ([FixPolicy.cpp](code/source/FixPolicy.cpp)).
- Static `MIP::isINForUNBD` function to check for infeasibility or unboundedness of a MIP problem ([MIP.hpp](code/include/MIP.hpp)).

### Fixed
- Addressed bug where the `UNBOUNDED` case was not covered in the `MIP::isINForUNBD` function ([MIP.hpp](code/include/MIP.hpp)).
- Resolved potential `MIPException` caused by negligible time-limit in `MTContext::FMIPInstanceJob` and `MTContext::OMIPInstanceJob` functions ([MTContext.cpp](code/source/MTContext.cpp)).

### Changed
- Removed all tolerance settings from CPLEX execution to ensure solver consistency.
- Refined documentation in header files for improved clarity and maintainability.


## [1.0.6] - 2025-04-04  
### Fixed  
- Removed Version 1.0.5 due to a value cleanup error.  
- Reduced overhead in `MTContext::FMIPInstanceJob` and `MTContext::OMIPInstanceJob` functions ([MTContext.cpp](code/source/MTContext.cpp)).


## [1.0.5] - 2025-04-01 [YANKED]
### Fixed
- Adjusted `Utils::EPSILON` zero-threshold for better numerical tolerance ([Utils.hpp](code/source/Utils.cpp)).
- Discretized negligible values in `OMIP::updateBudgetConstr` function ([OMIP.cpp](code/source/OMIP.cpp)).
- Discretized negligible values returned by `MIP::getObjFunction` function ([MIP.cpp](code/source/MIP.cpp)).
- Discretized negligible values returned by `MIP::getSol` function ([MIP.cpp](code/source/MIP.cpp)).


## [1.0.4] - 2025-03-31
### Fixed
- Fixed issue with `MIP::MIP_INT_TOL` (MIP Integrality Tolerance) in CPLEX optimization ([MIP.hpp](code/include/MIP.hpp)).
- Fixed issue with `MIP::MIP_SIMPLEX_FEAS_TOL` (MIP Simplex Feasibility Tolerance) in CPLEX optimization ([MIP.hpp](code/include/MIP.hpp)).


## [1.0.3] - 2025-03-30
### Fixed
- Resolved unexpected behavior in `Logger::print` function ([Utils.cpp](code/source/Utils.cpp))
- Resolved error related to MIP tolerance in CPLEX optimization (`MIP::MIP_GAP_TOL`) ([MIP.hpp](code/include/MIP.hpp))


## [1.0.2] - 2025-03-29
### Added
- Documentation for header files.

### Fixed
- Resolved unexpected behavior in `OMIP::getSlackSum` function ([OMIP.cpp](code/source/OMIP.cpp)).


## [1.0.1] - 2025-03-26
### Fixed
- Resolved issue in `FixPolicy::startSolTheta` function
- Optimized ACS Incumbent overhead in ACS


## [1.0.0] - 2025-03-20
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
