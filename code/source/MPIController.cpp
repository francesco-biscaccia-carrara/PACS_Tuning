#include "../include/MPIController.hpp"

MPIController::MPIController(int& argc, char**& argv) {
	MPI_Init(&argc, &argv);
	int r, s;
	MPI_Comm_rank(MPI_COMM_WORLD, &r);
	MPI_Comm_size(MPI_COMM_WORLD, &s);
	rank = static_cast<unsigned int>(r);
	size = static_cast<unsigned int>(s);
#if ACS_VERBOSE >= VERBOSE
	if (rank == MASTER)
		Logger::print(Logger::LogLevel::INFO, "MPI_Env: Initialized --- Num. process: %u", size);
#endif
}

MPIController::~MPIController(){
#if ACS_VERBOSE >= VERBOSE
	if (rank == MASTER)
		Logger::print(Logger::LogLevel::INFO, "MPI_Env: Closed");
#endif
	MPI_Finalize();
}