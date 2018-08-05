// #pragma once

#include <memory>
using namespace std;

#include "AbstractProcess.h"
#define MASTER 0    

class Slave: public AbstractProcess{
    private:
        enum MESSAGE {RECV_DATA = 0, RECV_CHUNK = 1, RECV_ROW = 2, RECV_U = 3, RECV_V = 4,
            SEND_DATA = 5, SEND_CHUNK = 6, SEND_U = 7, SEND_V = 8, SEND_RANK = 9, RECV_NEIGH = 12};
    public:
        Slave(int rank, float alpha, float gamma, int size, int numproc): AbstractProcess(size, alpha, gamma, numproc){
            this-> rank = rank;
        }

        int receive(MPI_Status status, int numproc) {
            MPI_Recv(&row, 1, MPI_INT, MASTER, RECV_ROW, MPI_COMM_WORLD, &status);
            MPI_Recv(&chunk, 1, MPI_INT, MASTER, RECV_CHUNK, MPI_COMM_WORLD, &status);
            data = new float[chunk*row];
            data_U = new float[chunk*row];
            data_V = new float[chunk*row];
            MPI_Recv(data, chunk*row, MPI_FLOAT, MASTER, RECV_DATA, MPI_COMM_WORLD, &status);
            MPI_Recv(data_U, chunk*row, MPI_FLOAT, MASTER, RECV_U, MPI_COMM_WORLD, &status);
            MPI_Recv(data_V, chunk*row, MPI_FLOAT, MASTER, RECV_V, MPI_COMM_WORLD, &status);

        }

        int send_new() {
            start = 1;
            if (rank == 0)
                start = 0;

            end = row-1;
            if (rank == numproc -1)
                end = row;

            MPI_Send(&new_len, 1, MPI_INT, MASTER, SEND_CHUNK, MPI_COMM_WORLD);
            MPI_Send(&data[start*size+end], new_len, MPI_FLOAT, MASTER, SEND_DATA, MPI_COMM_WORLD);

        }

        void receiveNeighbourd(MPI_Status status) {
            MPI_Recv(neighbour, 2, MPI_INT, MASTER, RECV_NEIGH, MPI_COMM_WORLD, &status);
        }

        
};


