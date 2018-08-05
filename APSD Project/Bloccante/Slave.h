// #pragma once

#include <memory>
using namespace std;

#include "AbstractProcess.h"

class Slave: public AbstractProcess{
    private:
        enum MESSAGE {RECV_DATA = 0, RECV_BLOCK = 1, BORDER = 2, RECV_U = 3, RECV_V = 4,
            SEND_DATA = 5, SEND_DIM = 6, SEND_U = 7, DIAG = 9, BORDER_U = 10, DIAG_U = 11};
        
        float **grid;
        float **U;
        float **V;       
        float **grid_new;
        float **U_new;
        float **V_new;       
        
        float *bord_horizontal;
        float *bord_vertical;

        float *bord_horizontal_U;
        float *bord_vertical_U;

        int nbrs[4], coords[2];

        int neighbour[2];

        MPI_Comm cartcomm;
        MPI_Datatype rowtype;
        MPI_Datatype columntypeBorder;

    public:
        Slave(int rank, float alpha, float gamma, int numrow, int numcol, int numproc): 
            AbstractProcess(numrow, numcol, alpha, gamma, numproc){
            this->rank = rank;

            this->grid = NULL;
        }

        int receive(MPI_Status status, int numproc) {
            MPI_Recv(&block, 1, mpi_block_type, MASTER, RECV_BLOCK, MPI_COMM_WORLD, &status);
            int num_col = block.column;
            int num_row = block.row;
            data = new float[num_col*num_row];
            data_U = new float[num_col*num_row];
            data_V = new float[num_col*num_row];
            bord_horizontal = new float[num_col];
            bord_vertical = new float[num_row];
            bord_horizontal_U = new float[num_col];
            bord_vertical_U = new float[num_row];
            MPI_Recv(data, num_col*num_row, MPI_FLOAT, MASTER, RECV_DATA, MPI_COMM_WORLD, &status);
            MPI_Recv(data_U, num_col*num_row, MPI_FLOAT, MASTER, RECV_U, MPI_COMM_WORLD, &status);
            MPI_Recv(data_V, num_col*num_row, MPI_FLOAT, MASTER, RECV_V, MPI_COMM_WORLD, &status);
        }

        void setTopology(MPI_Comm cartcomm) {
            // MPI_Datatype rowtype;
            MPI_Type_contiguous(block.column, MPI_FLOAT, &rowtype);
            MPI_Type_commit(&rowtype);
   
            // MPI_Datatype columntypeBorder;
            MPI_Type_vector(block.row, 1, block.column, MPI_FLOAT, &columntypeBorder);
            MPI_Type_commit(&columntypeBorder);

            MPI_Comm_rank(cartcomm, &rank);
            MPI_Cart_coords(cartcomm, rank, 2, coords);
            MPI_Cart_shift(cartcomm, 0, 1, &nbrs[UP], &nbrs[DOWN]);
            MPI_Cart_shift(cartcomm, 1, 1, &nbrs[LEFT], &nbrs[RIGHT]);

            if (nbrs[UP] != -2)
                neighbour[0] = nbrs[UP];
            if (nbrs[DOWN] != -2)
                neighbour[0] = nbrs[DOWN];
            if (nbrs[LEFT] != -2)
                neighbour[1] = nbrs[LEFT];
            if (nbrs[RIGHT] != -2)
                neighbour[1] = nbrs[RIGHT];

            this->cartcomm = cartcomm;
        }

        void sendReceiveBorder(MPI_Status status, bool firstTime) {
            int num_col = block.column;
            int num_row = block.row;
            int init = 0;
            if (!firstTime) {
                init = 1;
            }

            if (nbrs[UP] != -2) {
                float* tmp = new float[num_col+1];
                float* tmp_U = new float[num_col+1];
                for (int j = init; j < num_col+1; j++) {
                    tmp[j-init] = data[0*num_col+(j-init)];
                    tmp_U[j-init] = data_U[0*num_col+(j-init)];
                }
                MPI_Send(&(tmp[0]), 1, rowtype, neighbour[0], BORDER, cartcomm);
                MPI_Recv(bord_horizontal, num_col, MPI_FLOAT, neighbour[0], BORDER, cartcomm, &status);
                MPI_Send(&(tmp_U[0]), 1, rowtype, neighbour[0], BORDER_U, cartcomm);
                MPI_Recv(bord_horizontal_U, num_col, MPI_FLOAT, neighbour[0], BORDER_U, cartcomm, &status);
                delete [] tmp;
                delete [] tmp_U;
            }
            if (nbrs[DOWN] != -2) {
                float* tmp = new float[num_col+1];
                float* tmp_U = new float[num_col+1];
                for (int j = init; j < num_col+1; j++) {
                    tmp[j-init] = data[(num_row-1)*num_col+(j-init)];
                    tmp_U[j-init] = data_U[(num_row-1)*num_col+(j-init)];
                }
                MPI_Recv(bord_horizontal, num_col, MPI_FLOAT, neighbour[0], BORDER, cartcomm, &status);
                MPI_Send(&(tmp[0]), 1, rowtype, neighbour[0], BORDER, cartcomm);
                MPI_Recv(bord_horizontal_U, num_col, MPI_FLOAT, neighbour[0], BORDER_U, cartcomm, &status);
                MPI_Send(&(tmp_U[0]), 1, rowtype, neighbour[0], BORDER_U, cartcomm);
                delete [] tmp;
                delete [] tmp_U;
            }
            if (nbrs[LEFT] != -2) {
                MPI_Send(&data[0*num_col+0], 1, columntypeBorder, neighbour[1], BORDER, cartcomm);
                MPI_Recv(bord_vertical, num_row, MPI_FLOAT, neighbour[1], BORDER, cartcomm, &status);
                MPI_Send(&data_U[0*num_col+0], 1, columntypeBorder, neighbour[1], BORDER_U, cartcomm);
                MPI_Recv(bord_vertical_U, num_row, MPI_FLOAT, neighbour[1], BORDER_U, cartcomm, &status);
            }
            if (nbrs[RIGHT] != -2) {
                MPI_Recv(bord_vertical, num_row, MPI_FLOAT, neighbour[1], BORDER, cartcomm, &status);
                MPI_Send(&data[0*num_col+(num_row-1)], 1, columntypeBorder, neighbour[1], BORDER, cartcomm);
                MPI_Recv(bord_vertical_U, num_row, MPI_FLOAT, neighbour[1], BORDER_U, cartcomm, &status);
                MPI_Send(&data_U[0*num_col+(num_row-1)], 1, columntypeBorder, neighbour[1], BORDER_U, cartcomm);
            }
        }

        // Perchè i vicini dei vicini sono sempre tuoi vicini! lol
        void createGrid(MPI_Status status) {
            int dI, dJ;
            if (grid != NULL) {
                for (int i = 0; i < block.row; i++) {
                    delete [] grid;
                    delete [] U;
                    delete [] V;
                }

                delete grid;
                delete U;
                delete V;
            }
            grid = convertitor->unifyArray(data, bord_horizontal, bord_vertical, nbrs, block.row, block.column, dI, dJ);
            U = convertitor->unifyArray(data_U, bord_horizontal_U, bord_vertical_U, nbrs, block.row, block.column, dI, dJ);
            V = convertitor->unifyArray(data_V, bord_horizontal, bord_vertical, nbrs, block.row, block.column, dI, dJ);

            receiveDiag(dI, dJ, status);
        }

        // The neighbour are exagonal, so I need an element in diagonal
        void receiveDiag(int dI, int dJ, MPI_Status status) {
             if (nbrs[UP] != -2 && nbrs[RIGHT] != -2) {
                //RECEIVE 2
                MPI_Recv(&grid[dI][dJ], 1, MPI_FLOAT, neighbour[1], DIAG, cartcomm, &status);
                MPI_Recv(&U[dI][dJ], 1, MPI_FLOAT, neighbour[1], DIAG_U, cartcomm, &status);
                // convertitor->printGrid(grid, block.row, block.column);
            }
            else if (nbrs[DOWN] != -2 && nbrs[RIGHT] != -2) {
                //MANDA ULTIMO ELEM NEIGH DOWN      1
                MPI_Send(&bord_horizontal[block.column-1], 1, MPI_FLOAT, neighbour[1], DIAG, cartcomm);
                MPI_Send(&bord_horizontal_U[block.column-1], 1, MPI_FLOAT, neighbour[1], DIAG_U, cartcomm);
            }
            else if (nbrs[LEFT] != -2 && nbrs[UP] != -2) {
                //MANDA PRIMO ELEM NEIGH UP         2
                MPI_Send(&bord_horizontal[0], 1, MPI_FLOAT, neighbour[1], DIAG, cartcomm);
                MPI_Send(&bord_horizontal_U[0], 1, MPI_FLOAT, neighbour[1], DIAG_U, cartcomm);
            }
            else if (nbrs[LEFT] != -2 && nbrs[DOWN] != -2) {
                //RECEIVE 1
                MPI_Recv(&grid[dI][dJ], 1, MPI_FLOAT, neighbour[1], DIAG, cartcomm, &status);
                MPI_Recv(&U[dI][dJ], 1, MPI_FLOAT, neighbour[1], DIAG_U, cartcomm, &status);
            }
        }

        void updateGrid(MPI_Status status) {
            int dI, dJ;
            convertitor->unifyUdatedGrid(grid, bord_horizontal, bord_vertical, nbrs, block.row, block.column, dI, dJ);
            convertitor->unifyUdatedGrid(U, bord_horizontal_U, bord_vertical_U, nbrs, block.row, block.column, dI, dJ);
            convertitor->unifyUdatedGrid(V, bord_horizontal, bord_vertical, nbrs, block.row, block.column, dI, dJ);

            receiveDiag(dI, dJ, status);
        }

        void update() {
            int row = block.row;
            int column = block.column;

            grid_new = grid;
            U_new = U;
            V_new = V;

            for (int i = 0; i < row+1; i++) {
                for (int j = 0; j < column+1; j++) {
                    bool recep = false;
                    if (grid[i][j] >= 1)
                        recep = true;
                    else {
                        for ( int z = 0; z < 6 ; z++ ) {
                            int jx = i + neighb[z][0];
                            int jy = j + neighb[z][1];
                            if (isInGrid(jx,jy)) {
                                if (grid[jx][jy] >= 1) {
                                    recep = true;
                                    break;
                                }
                            }
                        }
                    }

                    if (recep) {
                        U_new[i][j] = 0;
                        V_new[i][j] = grid[i][j] + gamma;
                    }
                    else {
                        U_new[i][j] = grid[i][j];
                        V_new[i][j] = 0;
                    }
                }
            }


            for (int i=0; i < row+1; i++) {
                for (int j=0; j < column+1; j++) {
                    bool recep = false;
                    int neigh = 0;
                    float sum = U[i][j] * (1.0 - alpha * 6.0 /12.0);
                    for (int z = 0; z < 6 ; z++ ) {
                        int jx = i + neighb[z][0];
                        int jy = j + neighb[z][1];
                        if (isInGrid(jx,jy)) {
                            neigh++;
                            sum += U[jx][jy] * alpha / 12.0;
                        }
                    }
                    
                    grid_new[i][j] = V[i][j] + sum;
                }
            }
            
            grid = grid_new;
            U = U_new;
            V = V_new;

            convertitor->convertToArray(data, grid, nbrs, row, column);
            convertitor->convertToArray(data_U, U, nbrs, row, column);
            convertitor->convertToArray(data_V, V, nbrs, row, column);
        }


        void sendToMaster(MPI_Status status) {
            int dim = block.row * block.column;
            MPI_Isend(&data[0], dim, MPI_FLOAT, MASTER, SEND_DATA, MPI_COMM_WORLD, &send_request);
            MPI_Wait(&send_request,&status);
        }


        void freeMemory() {
            MPI_Type_free(&rowtype);
            MPI_Type_free(&columntypeBorder);
            
            for (int i = 0; i < block.row; i++) {
                delete [] grid[i];
                delete [] U[i];
                delete [] V[i];
            }
            delete [] grid;
            delete [] U;
            delete [] V;

            delete [] data;
            delete [] data_U;
            delete [] data_V;
        }
};

