#pragma once

#include <mpi.h>
#include <memory>
using namespace std;

#include "Constant.h"

// #define SIZE 4
struct Point {
  int x;
  int y;

  bool operator==(const Point &p){ return p.x == x && p.y == y; }
};

class AbstractProcess {
    protected:
        enum MESSAGE_BORDER {BORDER = 20, BORDER_U = 21};
        float *data;
        float *data_U;
        float *data_V;
        int x;
        int chunk;
        int *value;
        int row = 2;

        float **grid;
        float **U;
        float **V;
        int rank;

        int size;
        int numproc;

        unsigned int* neighbX = new unsigned int[6]; // = {-1, 0, -1, 1, 0, 1};
        unsigned int* neighbY = new unsigned int[6]; // = [-1, -1, 0, 0, 1, 1] 

        float gamma;
        float alpha;

        int start;
        int end;
        int new_len;

        float** grid_new;
        float** U_new;
        float** V_new;

        int neighbour[2] = {0,0};
        
        float *bord_up;
        float *bord_up_U;
        float *bord_down;
        float *bord_down_U;


        bool isInGrid(int i, int j) {
            if (i < 0 || i >= row)
                return false;
            if (j < 0 || j >= size)
                return false;
            return true;
        }

    public:
        AbstractProcess(int size, float alpha, float gamma, int numproc) {
            this->size = size;
            // vector for index to send 
            this->value = new int[2];
            for (int i = 0; i < 2; i++)
                this->value[i] = 0;

            this->numproc = numproc;

            neighbX[0] = -1;
            neighbX[1] = -1;
            neighbX[2] = 0;
            neighbX[3] = 0;
            neighbX[4] = 1;
            neighbX[5] = 1;

            neighbY[0] = 0;
            neighbY[1] = 1;
            neighbY[2] = 1;
            neighbY[3] = -1;
            neighbY[4] = -1;
            neighbY[5] = 0;

            this->alpha = alpha;
            this->gamma = gamma;

            grid_new = new float*[row];
            U_new = new float*[row];
            V_new = new float*[row];
            
            for (int i = 0; i < row; i++) {
                grid_new[i] = new float[size];
                U_new[i] = new float[size];
                V_new[i] = new float[size];
                for (int j = 0; j < size; j++) {
                    grid_new[i][j] = 0.0;
                }
            }

            bord_up = new float[size];
            bord_up_U = new float[size];
            bord_down = new float[size];
            bord_down_U = new float[size];


        }
        
        int getRank() {
            return this->rank;
        }

        // every process initialize own grid
        void createGrid() {
            grid = new float*[row];
            U = new float*[row];
            V = new float*[row];
            for (int i = 0; i < row; i++) {
                grid[i] = new float[size];
                U[i] = new float[size];
                V[i] = new float[size];
            }

            int column = 0;
            int ii = 0;
            for (int i = 0; i < chunk*row; i++) {
                if (i % size == 0 && i != 0) {
                    column++;
                    ii = 0;
                }
                grid[column][ii] = data[i];
                U[column][ii] = data_U[i];
                V[column][ii] = data_V[i];
                ii++;
            }
        }

        virtual void PartitioningGrid(int numproc) {};
        virtual int receive(MPI_Status status, int numproc) {};
        virtual int send_new() {};

        void update(int numproc) {
            
            start = 1;
            if (rank == 0)
                start = 0;

            end = row-1;
            if (rank == numproc -1)
                end = row;

            for (int i = start; i < end; i++) {
                for (int j = 0; j < size; j++) {
                    bool recep = false;
                    if (grid[i][j] >= 1)
                        recep = true;
                    else {
                        for ( int z = 0; z < 6 ; z++ )
                        {
                            int jx = i + neighbX[z];
                            int jy = j + neighbY[z];
                            if (isInGrid(jx,jy)) {
                                if (grid[jx][jy] >= 1) {
                                recep = true;
                                break;
                                }
                            }
                        }
                    }

                    if (recep) {
                        data_U[i*size+j] = 0;
                        data_V[i*size+j] = grid[i][j] + gamma;
                    }
                    else {
                        data_U[i*size+j] = grid[i][j];
                        data_V[i*size+j] = 0;
                    }
                }
            }


            for (int i=start; i < end; i++) {
                for (int j=0; j < size; j++) {
                    bool recep = false;
                    int neigh = 0;
                    float sum = U[i][j] * (1.0 - alpha * 6.0 /12.0);
                    for (int z = 0; z < 6 ; z++ ) 
                    {
                        int jx = i + neighbX[z];
                        int jy = j + neighbY[z];
                        if (isInGrid(jx,jy)) {
                            neigh++;
                            sum += U[jx][jy] * alpha / 12.0;
                        }
                    }
                    
                    data[i*size+j] = V[i][j] + sum;
                    // data[i*size+j] = grid[i][j] + 1.0;
                    // if (rank == 0) {
                    //     printf("VEDIAMO: %d %f %f\n", i, grid[i][j], data[i*size+j], grid);
                    // }
                }
            }

            if (rank == 0 || rank == numproc-1)
                new_len = chunk*(row-1);
            else
                new_len = chunk*(row-2);
        }

        float* convertToArray() {
            float* grid_new = new float[new_len];
            int z = 0;
            int sot = 1;
            if (rank == 0)
                sot = 0;
            
            for (int i = start; i < end; i++) {
                for (int j = 0; j < size; j++) {
                    grid_new[(i-sot)*size+j] = grid[i][j];
                }
            }
            return grid_new;
        }

        
        virtual float** getGrid() {};

        virtual void sendNeighbourd() {};
        virtual void receiveNeighbourd(MPI_Status status) {};

        void sendReceiveBorder(MPI_Status status, bool firstTime) {
            int num_col = size;
            int init = 0;

            if (neighbour[UP] != -1) {
                float* tmp = new float[num_col];
                float* tmp_U = new float[num_col];
                for (int j = init; j < num_col; j++) {
                    tmp[j-init] = data[1*num_col+(j-init)];
                    tmp_U[j-init] = data_U[1*num_col+(j-init)];
                }
                MPI_Send(tmp, num_col, MPI_FLOAT, neighbour[0], BORDER, MPI_COMM_WORLD);
                MPI_Recv(bord_up, num_col, MPI_FLOAT, neighbour[0], BORDER, MPI_COMM_WORLD, &status);
                MPI_Send(tmp_U, num_col, MPI_FLOAT, neighbour[0], BORDER_U, MPI_COMM_WORLD);
                MPI_Recv(bord_up_U, num_col, MPI_FLOAT, neighbour[0], BORDER_U, MPI_COMM_WORLD, &status);
                delete [] tmp;
                delete [] tmp_U;
            }
            if (neighbour[DOWN] != -1) {
                float* tmp = new float[num_col];
                float* tmp_U = new float[num_col];
                for (int j = init; j < num_col; j++) {
                    tmp[j-init] = data[(row-2)*num_col+(j-init)];
                    tmp_U[j-init] = data_U[(row-2)*num_col+(j-init)];
                }
                MPI_Recv(bord_down, num_col, MPI_FLOAT, neighbour[1], BORDER, MPI_COMM_WORLD, &status);
                MPI_Send(tmp, num_col, MPI_FLOAT, neighbour[1], BORDER, MPI_COMM_WORLD);
                MPI_Recv(bord_down_U, num_col, MPI_FLOAT, neighbour[1], BORDER_U, MPI_COMM_WORLD, &status);
                MPI_Send(tmp_U, num_col, MPI_FLOAT, neighbour[1], BORDER_U, MPI_COMM_WORLD);
                delete [] tmp;
                delete [] tmp_U;
            }
        }

        void updateGrid() {
            
            if (neighbour[UP] != -1) {
                for (int j = 0; j < size; j++) {
                    data[0*size+j] = bord_up[j];
                    data_U[0*size+j] = bord_up_U[j];
                }
            }
            if (neighbour[DOWN] != -1) {
                for (int j = 0; j < size; j++) {
                    data[(row-1)*size+j] = bord_down[j];
                    data_U[(row-1)*size+j] = bord_down_U[j];
                }
            }
            
            int column = 0;
            int ii = 0;
            for (int i = 0; i < chunk*row; i++) {
                if (i % size == 0 && i != 0) {
                    column++;
                    ii = 0;
                }
                grid[column][ii] = data[i];
                U[column][ii] = data_U[i];
                V[column][ii] = data_V[i];
                ii++;
            }
            
        }

};
