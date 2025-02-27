#ifndef MPI_CTR_H
#define MPI_CTR_H

#include <iostream>
#include <mpi.h>

#include "Utils.hpp"

#define MASTER 0

using namespace Utils;

class MPIController {

public:
	MPIController(int& argc, char**& argv);
	MPIController(const MPIController&) = delete;
	MPIController& operator=(const MPIController&) = delete;

	~MPIController();

	MPIController& addMPIBarrier() {
		MPI_Barrier(MPI_COMM_WORLD);
		return *this;
	};

	MPIController& abort() {
		MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
		return *this;
	};

	[[nodiscard]]
	const bool isMasterProcess() { return rank == MASTER; };
	[[nodiscard]]
	const unsigned int getRank() { return rank; };
	[[nodiscard]]
	const unsigned int getSize() { return size; };

private:
	unsigned int rank;
	unsigned int size;
};

#endif