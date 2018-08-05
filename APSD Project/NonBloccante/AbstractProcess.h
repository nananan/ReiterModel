#pragma once

#include <mpi.h>
#include <memory>
#include "Constant.h"
#include "Convertitor.h"
using namespace std;

struct Point {
  int x;
  int y;

  bool operator==(const Point &p){ return p.x == x && p.y == y; }
};

typedef struct info_block {
    int initI, initJ;
    int row, column;
} subblock;

class AbstractProcess {
    protected:
        float *data;
        float *data_U;
        float *data_V;

        int rank;

        int size;
        int numrow, numcol;
        int numproc;

        const int neighb[NUM_NEIGHBOUR][2] = { {-1,0}, {-1,1}, {0,1}, {0,-1}, {1,-1}, {1,0}};

        float gamma;
        float alpha;

        MPI_Datatype mpi_block_type;
        MPI_Datatype columntype;
        MPI_Datatype columntypeBorder;
        MPI_Datatype rowtype;

        subblock block;
        shared_ptr<Convertitor> convertitor;

        MPI_Request send_request,recv_request;

        bool isInGrid(int i, int j) {
            if (i < 0 || i > block.row)
                return false;
            if (j < 0 || j > block.column)
                return false;
            return true;
        }

    public:
        AbstractProcess(int numrow, int numcol, float alpha, float gamma, int numproc) {
            this->numrow = numrow;
            this->numcol = numcol;

            this->numproc = numproc;

            this->alpha = alpha;
            this->gamma = gamma;

            this->convertitor = make_shared<Convertitor>();

            /* create a type for struct piece */
            const int    nitems=4;
            int          blocklengths[4] = {1,1,1,1};
            MPI_Datatype types[4] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT};
            MPI_Aint     offsets[4];

            offsets[0] = offsetof(subblock, initI);
            offsets[1] = offsetof(subblock, initJ);
            offsets[2] = offsetof(subblock, row);
            offsets[3] = offsetof(subblock, column);

            MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_block_type);
            MPI_Type_commit(&mpi_block_type);
        }
        
        int getRank() {
            return this->rank;
        }

        virtual void PartitioningGrid(int numproc) {};
        virtual int receive(MPI_Status status, int numproc) {};

        virtual float** getGrid() {};
        virtual void send_partition() {};
        virtual void setTopology(MPI_Comm cartcomm) {};
        virtual void sendReceiveBorder(MPI_Status status, bool firstTime) {};

        virtual void createGrid(MPI_Status status) {};
        virtual void updateGrid(MPI_Status status) {};
        virtual void update(int step) {};
        virtual void updateNeighbour(bool horizontal) {};
        virtual void waitNeighbour() {};
        virtual void convert() {};
        virtual void sendReceiveBorderFirst(MPI_Status status, bool firstTime) {};


        virtual void sendToMaster(MPI_Status status) {};
        virtual void receiveFromSlave(MPI_Status status, int step) {};

        virtual void freeMemory() {};
};
