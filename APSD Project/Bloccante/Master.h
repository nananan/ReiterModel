// #pragma once

#include <memory>
using namespace std;

#include "AbstractProcess.h"

class Master: public AbstractProcess{
    private:
        enum MESSAGE {SEND_DATA = 0, SEND_BLOCK = 1, SEND_ROW = 2, SEND_U = 3, SEND_V = 4,
            RECV_DATA = 5, RECV_DIM= 6, RECV_U = 7, RECV_V = 8};
        float beta;
        float **grid_init;

        tuple<subblock> *partition;
    public:
        Master(int rank, float beta, float alpha, float gamma, int numrow, int numcol, int numproc):
         AbstractProcess(numrow, numcol, alpha, gamma, numproc){
            this->rank = rank;
            this->beta = beta;

            grid_init = new float*[numrow];
            for (int i = 0; i < numrow; i++) {
                grid_init[i] = new float[numcol];
                for (int j = 0; j < numcol; j++)
                    grid_init[i][j] = this->beta;
            }
            grid_init[(numrow-1)/2][(numcol-1)/2] = 1.0;
            partition = new tuple<subblock>[numproc-1];

        }
        
        // This is done from MASTER only ONCE
        void PartitioningGrid(int numproc) {
            int dim = numrow*numcol;
            data = new float[dim];
            data_U = new float[dim];
            data_V = new float[dim];
            for (int i = 0; i < dim; i++) {
                // data[i] = i;
                data[i] = this->beta;
                data_U[i] = 0;
                data_V[i] = 0;
            }

            data[(numrow-1)/2*numcol+(numcol-1)/2] = 1.0;

            initGrid();
            int contEven = 0, contOdd = 0;
            for (int id = 0; id < numproc-1; id++) {
                if (id % 2 == 0) {
                    if (contEven == 0) {
                        block.initI = 0;
                        block.initJ = 0;

                        block.row = numrow / (numproc /2);
                        block.column = numcol / (numproc /2);
                    }
                    else {
                        block.initI = numrow / (numproc /2);
                        block.row = numrow - (numrow / (numproc /2));

                        block.initJ = 0;
                        block.column = numcol / (numproc /2);
                    }
                    contEven++;
                }
                else {
                    if (contOdd == 0) {
                        block.initI = 0;
                        block.row = (numrow / (numproc /2));
                        
                        block.initJ = numcol / (numproc /2);
                        block.column = numcol - (numcol / (numproc /2));
                    }
                    else {
                        block.initI = numrow / (numproc /2);
                        block.row = numrow - (numrow / (numproc /2));
                        
                        block.initJ = numcol / (numproc /2);
                        block.column = numcol - (numcol / (numproc /2));
                    }
                    contOdd++;
                }

                // printf("RANK: %d NUM_ROW: %d NUM_COL: %d\n", id, block.row, block.column);
                MPI_Datatype columntype;
                MPI_Type_vector(block.row, block.column, numcol, MPI_FLOAT, &columntype);
                MPI_Type_commit(&columntype);


                MPI_Send(&block, 1, mpi_block_type, id, SEND_BLOCK, MPI_COMM_WORLD);
                MPI_Send(&data[block.initI*numcol+block.initJ], 1, columntype, id, SEND_DATA, MPI_COMM_WORLD);
                MPI_Send(&data_U[block.initI*numcol+block.initJ], 1, columntype, id, SEND_U, MPI_COMM_WORLD);
                MPI_Send(&data_V[block.initI*numcol+block.initJ], 1, columntype, id, SEND_V, MPI_COMM_WORLD);

                MPI_Type_free(&columntype);
                
                partition[id] = make_tuple(block);
            }
        }

        void receiveFromSlave(MPI_Status status, int step) {
            int dim;
            if (step >= STEP)
                return;
            
            for (int id = 0; id < numproc-1; id++) {
                int index = 0;
                subblock block = get<0>(partition[id]);
                int dim = block.row*block.column;
                float* tmp = new float[dim];
                MPI_Irecv(tmp, dim, MPI_FLOAT, id, RECV_DATA, MPI_COMM_WORLD, &recv_request);
                MPI_Wait(&recv_request, &status);
                
                for (int i = block.initI; i < block.initI+block.row; i++)
                    for (int j = block.initJ; j < block.initJ+block.column; j++)
                        grid_init[i][j] = tmp[index++];
                
                delete [] tmp;
            }
        }

        void initGrid() {
            
            for (int i = 0; i < numrow; i++) {
                for (int j = 0; j < numcol; j++) {
                    Point selectionCenter = {i,j};
                    bool recep = false;
                    if (data[i*numcol+j] >= 1)
                        recep = true;
                    else {
                        for ( int z = 0; z < 6 ; z++ )
                        {
                            int jx = i + neighb[z][0];
                            int jy = j + neighb[z][1];
                            if (isInGrid(jx,jy)) {
                                if (data[jx*numcol+jy] >= 1) {
                                    recep = true;
                                    break;
                                }
                            }
                        }
                    }

                    if (recep) {
                        data_U[i*numcol+j] = 0;
                        data_V[i*numcol+j] = data[i*numcol+j];
                    }
                    else {
                        data_U[i*numcol+j] = data[i*numcol+j];
                        data_V[i*numcol+j] = 0;
                    }
                }
            }
        }

        float** getGrid() {
            return grid_init;
        }

        void free_memory() {
            for (int i = 0; i < numrow; i++) 
                delete [] grid_init[i];
            delete [] grid_init;

            delete partition;

            delete [] data;
            delete [] data_U;
            delete [] data_V;
        }

};

