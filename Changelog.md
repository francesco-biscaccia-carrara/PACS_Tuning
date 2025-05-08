# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).


### [Unreleased]  
- Implemented `MIP::checkFeasibility` without relying on CPLEX, enabling lightweight feasibility checks.  
- Enforced FMIP resolution in the ACS workflow prior to transitioning to the OMIP phase.

## [0.0.7] - 2025-05-08  
### Changed  
- Performance testing involves the use of a JSON file for result tracking.  

## InitSol [0.0.6] - 2025-05-07  
### Added  
- Enabled parallel execution of multiple RENS (Relaxation Enforced Neighborhood Search) instances to generate the initial vector in `MTContext::parallelInitSolMerge` ([MTContext.cpp](code/source/MTContext.cpp)). 
- Added methods to set lower and upper bounds for specific variables ([MIP.cpp](code/source/MIP.cpp)).  

### Changed  
- Unified CPLEX callback logic into a single consolidated function ([MTContext.cpp](code/source/MTContext.cpp)).  


## InitSol [0.0.5] - 2025-05-05
### Added  
- Enabled CPLEX callback functionality to share candidate solutions between FMIP and OMIP phases ([MTContext.cpp](code/source/MTContext.cpp)).  


## InitSol [0.0.4] - 2025-04-24 [YANKED]
### Changed 
- Modified ` MTContext::parallelInitSolMerge`, a method to set the initial solution based on relaxed solutions and common values mergin ([MTContext.cpp](code/source/MTContext.cpp)).  


## InitSol [0.0.3] - 2025-04-19  
### Added  
- Introduced `FixPolicy::startSolMin`, a method to set the initial solution based on variable bounds and objective value ([FixPolicy.cpp](code/source/FixPolicy.cpp)).  

### Changed  
- Redesigned output formatting for message types in `Utils::Logger::print` ([Utils.cpp](code/source/Utils.cpp)).  
- Refactored timer handling logic in `Utils::Clock` for improved clarity and maintainability ([Utils.cpp](code/source/Utils.cpp)).  


## InitSol[0.0.2] - 2025-04-16
### Added
- ACK signature at the end to execution to track unexpected execution behaviours both on ACS and CPLEX.

### Changed
- Updated recombination phase guided by the MIP objective funciton in function `MTContext::parallelInitSolMerge` ([MTContext.cpp](code/source/MTContext.cpp)).


## InitSol[0.0.1] - 2025-04-08 [YANKED]
### Added
- Merge-recombination of random solutions
- Tested against single random vector and standard fixing: no effective results


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
