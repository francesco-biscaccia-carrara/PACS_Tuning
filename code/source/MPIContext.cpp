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
		MPI_Bcast(value.data(), length, MPI_CHAR, MASTER, MPI_COMM_WORLD);
	} else {
		int length;
		MPI_Bcast(&length, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
		value.resize(length);
		MPI_Bcast(value.data(), length, MPI_CHAR, MASTER, MPI_COMM_WORLD);
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

MPIContext& MPIContext::broadcast(unsigned long& value) {
	MPI_Bcast(&value, 1, MPI_UNSIGNED_LONG, MASTER, MPI_COMM_WORLD);
	return *this;
}

MPIContext& MPIContext::broadcast(unsigned long long& value) {
	MPI_Bcast(&value, 1, MPI_UNSIGNED_LONG_LONG, MASTER, MPI_COMM_WORLD);
	return *this;
}

MPIContext& MPIContext::broadcast(Args& value) {
	broadcast(value.fileName);
	broadcast(value.timeLimit);
	broadcast(value.theta);
	broadcast(value.rho);
	broadcast(value.seed);
	broadcast(value.CPLEXCpus);
	return *this;
}

MPIContext& MPIContext::broadcast(Solution& value) {
	broadcast(value.sol);
	broadcast(value.slackSum);
	return *this;
}

MPIContext& MPIContext::broadcast(std::vector<double>& value) {
	if (rank == MASTER) {
		int length = value.size();
		MPI_Bcast(&length, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
		MPI_Bcast(value.data(), length, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
	} else {
		int length;
		MPI_Bcast(&length, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
		value.resize(length);
		MPI_Bcast(value.data(), length, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
	}
	return *this;
}

MPIContext& MPIContext::gather(std::vector<double>& source, std::vector<double>& dest) {
	int length = source.size();

	if (rank == MASTER) {
		dest.resize(length * size);
	}

	MPI_Gather(source.data(), length, MPI_DOUBLE, rank == MASTER ? dest.data() : nullptr, length, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
	return *this;
}

/* Gather on root std:.vector of double of differnt size
MPIContext& MPIContext::gatherV(std::vector<double>& source, std::vector<double>& dest) {
	int				 length = source.size();
	std::vector<int> lengths(rank == MASTER ? size : 0);
	MPI_Gather(&length, 1, MPI_INT, lengths.data(), 1, MPI_INT, MASTER, MPI_COMM_WORLD);

	std::vector<int> displs;
	int				 totalSize = 0;
	if (rank == MASTER) {
		displs.resize(size);
		displs[0] = 0;

		for (int i = 0; i < size; ++i) {
			displs[i] = displs[i - 1] + lengths[i - 1];
			totalSize += lengths[i];
		}
		dest.resize(totalSize);
	}

	MPI_Gatherv(source.data(), length, MPI_DOUBLE, rank == MASTER ? dest.data() : nullptr, lengths.data(), displs.data(), MPI_DOUBLE, MASTER, MPI_COMM_WORLD);
	return *this;
}*/

MPIContext::~MPIContext() {
#if ACS_VERBOSE >= VERBOSE
	if (rank == MASTER)
		Logger::print(Logger::LogLevel::INFO, "MPI_Env: Closed");
#endif
	MPI_Finalize();
}