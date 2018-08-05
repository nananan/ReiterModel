// #pragma once

#include <memory>
#include "Constant.h"
using namespace std;


class Convertitor{
    private:

    public:
        Convertitor(){}

        float** unifyArray(float* array, float* hBorder, float* vBorder, int nbrs[4], int row, int column,
            int &dI, int &dJ) {

            float ** grid = new float*[row+1];
            for (int i = 0; i < row+1; i++) {
                grid[i] = new float[column+1];
            }

            if (nbrs[UP] != -2 && nbrs[RIGHT] != -2) {
                //UP-ARR-RIGHT
                for (int i = 0; i < column; i++){
                    grid[0][i] = hBorder[i];
                }
                for (int i = 0; i < row; i++) {
                    for (int j = 0; j < column; j++) {
                        grid[i+1][j] = array[i*column+j];
                    }
                }
                for (int i = 0; i < row; i++){
                    grid[i+1][column] = vBorder[i];
                }
                dI = 0;
                dJ = column;
            }
            else if (nbrs[DOWN] != -2 && nbrs[RIGHT] != -2) {
                for (int i = 0; i < row; i++) {
                    for (int j = 0; j < column; j++) {
                        grid[i][j] = array[i*column+j];
                    }
                }
                for (int i = 0; i < row; i++){
                    grid[i][column] = vBorder[i];
                }
                for (int i = 0; i < column; i++){
                    grid[row][i] = hBorder[i];
                }
                dI = row;
                dJ = column;
            }
            else if (nbrs[LEFT] != -2 && nbrs[UP] != -2) {
                //LEFT-UP-ARR
                for (int i = 0; i < row; i++){
                    grid[i+1][0] = vBorder[i];
                }
                for (int i = 0; i < column; i++){
                    grid[0][i+1] = hBorder[i];
                }
                for (int i = 0; i < row; i++) {
                    for (int j = 0; j < column; j++) {
                        grid[i+1][j+1] = array[i*column+j];
                    }
                }
                dI = 0;
                dJ = 0;
            }
            else if (nbrs[LEFT] != -2 && nbrs[DOWN] != -2) {
                //LEFT-ARR-DOWN
                for (int i = 0; i < row; i++){
                    grid[i][0] = vBorder[i];
                }
                for (int i = 0; i < row; i++) {
                    for (int j = 0; j < column; j++) {
                        grid[i][j+1] = array[i*column+j];
                    }
                }
                for (int i = 0; i < column; i++){
                    grid[row][i+1] = hBorder[i];
                }
                dI = row;
                dJ = 0;
            }
            return grid;
        }

        void unifyUdatedGrid(float** &grid, float* hBorder, float* vBorder, int nbrs[4], int row, int column,
            int &dI, int &dJ) {
            if (nbrs[UP] != -2 && nbrs[RIGHT] != -2) {
                //UP-ARR-RIGHT
                for (int i = 0; i < column; i++){
                    grid[0][i] = hBorder[i];
                }
                for (int i = 0; i < row; i++){
                    grid[i+1][column] = vBorder[i];
                }
                dI = 0;
                dJ = column;
            }
            else if (nbrs[DOWN] != -2 && nbrs[RIGHT] != -2) {
                for (int i = 0; i < row; i++){
                    grid[i][column] = vBorder[i];
                }
                for (int i = 0; i < column; i++){
                    grid[row][i] = hBorder[i];
                }
                dI = row;
                dJ = column;
            }
            else if (nbrs[LEFT] != -2 && nbrs[UP] != -2) {
                //LEFT-UP-ARR
                for (int i = 0; i < row; i++){
                    grid[i+1][0] = vBorder[i];
                }
                for (int i = 0; i < column; i++){
                    grid[0][i+1] = hBorder[i];
                }
                dI = 0;
                dJ = 0;
            }
            else if (nbrs[LEFT] != -2 && nbrs[DOWN] != -2) {
                //LEFT-ARR-DOWN
                for (int i = 0; i < row; i++){
                    grid[i][0] = vBorder[i];
                }
                for (int i = 0; i < column; i++){
                    grid[row][i+1] = hBorder[i];
                }
                dI = row;
                dJ = 0;
            }
        }

        void convertToArray(float* &array, float** grid, int nbrs[4], int row, int column) {
            
            if (nbrs[UP] != -2 && nbrs[RIGHT] != -2) {
                for (int i = 1; i < row+1; i++) {
                    for (int j = 0; j < column; j++) {
                        array[(i-1)*column+j] = grid[i][j];
                    }
                }


            }
            else if (nbrs[DOWN] != -2 && nbrs[RIGHT] != -2) {
                for (int i = 0; i < row; i++) {
                    for (int j = 0; j < column; j++) {
                         array[i*column+j] = grid[i][j];
                    }
                }
            }
            else if (nbrs[LEFT] != -2 && nbrs[UP] != -2) {
                for (int i = 1; i < row+1; i++) {
                    for (int j = 1; j < column+1; j++) {
                        array[(i-1)*column+(j-1)] = grid[i][j];
                    }
                }
            }
            else if (nbrs[LEFT] != -2 && nbrs[DOWN] != -2) {
                for (int i = 0; i < row; i++) {
                    for (int j = 1; j < column+1; j++) {
                         array[i*column+(j-1)] = grid[i][j];
                    }
                }
            }
        }


        void printGrid(float** grid, int row, int column, int rank,int step) {
            printf("PRINT FROM %d STEP: %d\n", rank, step);
            for (int i = 0; i < row+1; i++) {
                for (int j = 0; j < column+1; j++) 
                    printf("%f ",grid[i][j]);
                printf("\n");
            }
            printf("--------------------------------\n");
        }

        void printArray(float* data, int row, int column, int rank) {
            for(int i = 0; i < row*column; i++) {
                printf("FROM: %d ELEM %f\n", rank, data[i]);
            }
        }

};

