// #pragma once

#include <memory>
using namespace std;

#include "AbstractProcess.h"

class Master: public AbstractProcess{
    private:
        enum MESSAGE {SEND_DATA = 0, SEND_CHUNK = 1, SEND_ROW = 2, SEND_U = 3, SEND_V = 4,
            RECV_DATA = 5, RECV_CHUNK = 6, RECV_U = 7, RECV_V = 8, SEND_NEIGH = 12};
        float beta;
        float **grid_init;
        float **U_init;
        float **V_init;

        float *data_init;
        int len;

        tuple<int, int, int> *partition;
    public:
        Master(int rank, float beta, float alpha, float gamma, int size, int numproc):
         AbstractProcess(size, alpha, gamma, numproc){
            this->rank = rank;
            this->beta = beta;

            grid_init = new float*[size];
            U_init = new float*[size];
            V_init = new float*[size];
            for (int i = 0; i < size; i++) {
                grid_init[i] = new float[size];
                U_init[i] = new float[size];
                V_init[i] = new float[size];
            }

            partition = new tuple<int,int,int>[numproc];
        }
        
        // This is done from MASTER only ONCE
        void PartitioningGrid(int numproc) {
            
            data = new float[size*size];
            data_U = new float[size*size];
            data_V = new float[size*size];
            for (int i = 0; i < size*size; i++) {
                data[i] = this->beta;
            }

            data[(size-1)/2*size+(size-1)/2] = 1.0;

            initGrid();
            int j = 0; int z = 0;
            for (int i = 0; i < size*size; i++) {
                if (i % size == 0 && i != 0) {
                    j++;
                    z = 0;
                }
                grid_init[j][z] = data[i];
                z++;

            }

            x = ( (int) (size/numproc)) * size;
            chunk = size;

            for (int source = 1; source < numproc; source++) {
                value[0] += x;
                value[1] = value[0] + x;
                if (source == numproc -1)
                    value[1] = size*size;
                
                int init = value[0] - chunk;
                int finish = (value[1] + chunk > size*size)?value[1]:value[1] + chunk;
                row = (finish - init);
                int rows = row / chunk;
                partition[source] = make_tuple(row, chunk, init);
                
                MPI_Send(&rows, 1, MPI_INT, source, SEND_ROW, MPI_COMM_WORLD);
                MPI_Send(&chunk, 1, MPI_INT, source, SEND_CHUNK, MPI_COMM_WORLD);
                MPI_Send(data+init, row, MPI_FLOAT, source, SEND_DATA, MPI_COMM_WORLD);
                MPI_Send(data_U+init, row, MPI_FLOAT, source, SEND_U, MPI_COMM_WORLD);
                MPI_Send(data_V+init, row, MPI_FLOAT, source, SEND_V, MPI_COMM_WORLD);
            }
            value[0] = 0;
            row = (x + size) /size;
            if (numproc == 1)
            partition[0] = make_tuple(row, chunk, value[0]);
        }

        void sendNeighbourd() {
            for (int id = 1; id < numproc; id++) {
                if (id == numproc-1) {
                    neighbour[UP] = id-1;
                    neighbour[DOWN] = -1;
                }
                else {
                    neighbour[UP] = id-1;
                    neighbour[DOWN] = id+1;
                }
                MPI_Send(neighbour, 2, MPI_INT, id, SEND_NEIGH, MPI_COMM_WORLD);
            }
            neighbour[UP] = -1;
            neighbour[DOWN] = rank+1;
        }

        void initGrid() {
            for (int i = 0; i < size; i++) {
                for (int j = 0; j < size; j++) {
                    Point selectionCenter = {i,j};
                    bool recep = false;
                    if (data[i*size+j] >= 1)
                        recep = true;
                    else {
                        for ( int z = 0; z < 6 ; z++ )
                        {
                            int jx = i + neighbX[z];
                            int jy = j + neighbY[z];
                            if (isInGrid(jx,jy)) {
                                if (data[jx*size+jy] >= 1) {
                                    recep = true;
                                    break;
                                }
                            }
                        }
                    }

                    if (recep) {
                        data_U[i*size+j] = 0;
                        data_V[i*size+j] = data[i*size+j];
                    }
                    else {
                        data_U[i*size+j] = data[i*size+j];
                        data_V[i*size+j] = 0;
                    }
                }
            }
        }


        int receive(MPI_Status status, int numproc) {
            float* grid_convert = convertToArray();
            int z = 0;
            int j = 0;
            int ind_data = 0;
            for (int i = 0; i < new_len; i++) {
                if (i % size == 0 && i != 0) {
                    j++;
                    z = 0;
                }
                grid_init[j][z] = grid_convert[i];
                z++;

                ind_data++;
            }

            delete [] grid_convert;
            
            for (int source = 1; source < numproc; source++) {
                j++;
                z = 0;
                MPI_Recv(&len, 1, MPI_INT, source, RECV_CHUNK, MPI_COMM_WORLD, &status);

                float* data_proc = new float[len];
                MPI_Recv(data_proc, len, MPI_FLOAT, source, RECV_DATA, MPI_COMM_WORLD, &status);

                for (int i = 0; i < len; i++) {
                    if (i % size == 0 && i != 0) {
                        j++;
                        z = 0;
                    }
                    grid_init[j][z] = data_proc[i];
                    z++;

                    ind_data++;
                }

                delete [] data_proc;
            }
        }

        float** getGrid() {
            return grid_init;
        }

};

