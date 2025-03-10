#ifndef MPI_CTR_H
#define MPI_CTR_H

#include <iostream>
#include <mpi.h>

#include "Utils.hpp"

#define MASTER 0

using namespace Utils;

class MPIContext {

public:
	MPIContext(int& argc, char**& argv);
	MPIContext(const MPIContext&) = delete;
	MPIContext& operator=(const MPIContext&) = delete;

	~MPIContext();

	MPIContext& barrier() {
		MPI_Barrier(MPI_COMM_WORLD);
		return *this;
	};

	MPIContext& abort() {
		MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
		return *this;
	};

	MPIContext& broadcast(std::string& value);
	MPIContext& broadcast(int& value);
	MPIContext& broadcast(double& value);
	MPIContext& broadcast(unsigned long& value);
	MPIContext& broadcast(unsigned long long& value);
	MPIContext& broadcast(Args& value);
	MPIContext& broadcast(std::vector<double>& value);
	MPIContext& broadcast(Solution& value);
	MPIContext& gather(std::vector<double>& source, std::vector<double>& dest);

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