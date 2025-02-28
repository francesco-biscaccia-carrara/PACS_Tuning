#include "../include/MPIContext.hpp"

MPIContext::MPIContext(int& argc, char**& argv) {
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

MPIContext& MPIContext::broadcast(std::string& value) {
	if (rank == MASTER) {
		int length = value.size();
		MPI_Bcast(&length, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
		MPI_Bcast(&value[0], length, MPI_CHAR, MASTER, MPI_COMM_WORLD);
	} else {
		int length;
		MPI_Bcast(&length, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
		value.resize(length);
		MPI_Bcast(&value[0], length, MPI_CHAR, MASTER, MPI_COMM_WORLD);
	}
	return *this;
}

MPIContext& MPIContext::broadcast(int& value) {
	MPI_Bcast(&value, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
	return *this;
}

MPIContext& MPIContext::broadcast(double& value) {
	MPI_Bcast(&value, 1, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
	return *this;
}

MPIContext::~MPIContext() {
#if ACS_VERBOSE >= VERBOSE
	if (rank == MASTER)
		Logger::print(Logger::LogLevel::INFO, "MPI_Env: Closed");
#endif
	MPI_Finalize();
}