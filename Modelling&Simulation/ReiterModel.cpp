#include <allegro.h>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <iostream>
#include <time.h>
#include <algorithm>
#include "HelperFile.h"
using namespace std;

#define NUM_NEIGHBOUR 6

struct Point {
  int x;
  int y;

  bool operator==(const Point &p){ return p.x == x && p.y == y; }
};

volatile int close_button_pressed = FALSE;
BITMAP *buffer; //Dichiarazioni generali: buffer

bool isInGrid(int i, int j);
vector<vector<float>> update(vector<vector<float>> grid, vector<vector<float>>& U, 
  vector<vector<float>>& V, float gamma, float alpha, int percentual, int margolus);
void close_button_handler(void) {
  close_button_pressed = TRUE;
}

const int size_grid = 500;
const int size_windowY = 540;
int step = 0;
int imgx = 500; int imgy = 500;
int imgx1 = imgx - 1; int imgy1 = imgy - 1;

const int neighb[NUM_NEIGHBOUR][2] = { {-1,0}, {-1,1}, {0,1}, {0,-1}, {1,-1}, {1,0}};
const int neighbEven[NUM_NEIGHBOUR][2] = { {-1,0}, {-1,-1}, {0,-1}, {1,-1}, {1,0}, {0,1}};
const int neighbOdd[NUM_NEIGHBOUR][2] = { {-1,0}, {-1,1}, {0,1}, {1,1}, {1,0}, {0,-1}};

vector<Point> receptive_point(size_grid);
int main(int argc, char* argv[])
{
  srand(time (0));

  // float alpha = 1.6;
  // float beta = 0.7;
  // float gamma = 0.0002;

  float alpha = 0;
  float beta = 0.0;
  float gamma = 0.0;
  // float percentual = 0.4;
  int percentual = 0;
  int margolus = 0;

  HelperFile helperFile;
  helperFile.readFile(alpha, beta, gamma, percentual, margolus);

  vector<vector<float>> grid(size_grid, vector<float>(size_grid,beta));
  vector<vector<float>> U(size_grid, vector<float>(size_grid,0));
  vector<vector<float>> V(size_grid, vector<float>(size_grid,0));

  // int num = rand()%size;
  grid[(size_grid-1)/2][(size_grid-1)/2] = 1;

  for (int i = 0; i < size_grid; i++) {
    for (int j = 0; j < size_grid; j++) {
        Point selectionCenter = {i,j};
      bool recep = false;
      if (grid[i][j] >= 1)
        recep = true;
      else {
        for ( int z = 0; z < 6 ; z++ )
        {
            int jx = i + neighb[z][0];
            int jy = j + neighb[z][1];
            if (margolus == 1) {
              //EVEN
              if (j % 2 == 0) { 
                jx = i + neighbEven[z][0];
                jy = j + neighbEven[z][1];
              }
              else { //ODD
                jx = i + neighbOdd[z][0];
                jy = j + neighbOdd[z][1];
              }
            }

            if (isInGrid(jx,jy)) {
                if (grid[jx][jy] >= 1) {
                  recep = true;
                  break;
                }
            }
        }
      }

      if (recep) {
        U[i][j] = 0;
        V[i][j] = grid[i][j];
      }
      else {
        U[i][j] = grid[i][j];
        V[i][j] = 0;
      }
    }
  }

  allegro_init();
  install_keyboard();
  install_mouse();
  set_close_button_callback(close_button_handler);
   //Indica al computer la modalità grafica che intendiamo usare
  set_gfx_mode(GFX_AUTODETECT_WINDOWED, size_grid, size_windowY, 0, 0);
   //Mostra il mouse sullo schermo
  show_mouse(screen);
   // Funzione che crea il buffer dove disegniamo gli elementi
  buffer = create_bitmap(size_grid, size_windowY);

  int blu = makecol(0, 128, 255);
  int bianco = makecol(255,255,255);
  int nero = makecol(0, 0, 0);

  double an45 = - M_PI / 4.0;
  double sn45 = sin(an45); 
  double cs45 = cos(an45);
  double scale = sqrt(3.0); 
  double ox = imgx1 / 2.0; 
  double oy = imgy1 / 2.0;

  while (!(close_button_pressed || key[KEY_ESC])) {
    for (int i=0; i < size_grid; i++) {
      for (int j=0; j < size_grid; j++) {
        double tx = (i - ox)* (scale); 
        double ty = (j - oy);
        double tx0 = tx * cs45 - ty * sn45 + ox;
        ty = tx * sn45 + ty * cs45 + oy; 
        tx = tx0;
        double c;
        if (tx >= 0 and tx <= imgx1 and ty >= 0 and ty <= imgy1)
            c = grid[int((size_grid - 1) * ty / imgy1)][int((size_grid - 1) * tx / imgx1)];

        if (c >= 1)
          putpixel(buffer, i, j, blu);
        else if (c >= 0.9 and c < 1)
          putpixel(buffer, i, j, bianco);
      }
    }
    char str [] = "Alpha: %f Beta: %f Gamma: %f Perc: %i";
    textprintf_ex(buffer,font,0,size_windowY-25,bianco,nero, "Step: %i ",step);
    textprintf_ex(buffer,font,0,size_windowY-10,bianco,nero, str,alpha, beta, gamma, percentual);
    blit(buffer,screen,0,0,0,0,size_grid,size_windowY);
    clear_bitmap(buffer);

    grid = update(grid, U, V, gamma, alpha, percentual, margolus);
    step++;
  }

  return 0;
}

vector<vector<float>> update(vector<vector<float>> grid, vector<vector<float>>& U,
  vector<vector<float>>& V, float gamma, float alpha, int percentual, int margolus) {

  vector<vector<float>> grid_new(size_grid, vector<float>(size_grid,0.6));
  vector<vector<float>> U_new(size_grid, vector<float>(size_grid,0));
  vector<vector<float>> V_new(size_grid, vector<float>(size_grid,0));
  int range = 1;

  for (int i = 1; i < size_grid-1; i++) {
    for (int j = 1; j < size_grid-1; j++) {
       bool recep = false;
      if (grid[i][j] >= 1)
        recep = true;
      else {
        for ( int z = 0; z < 6 ; z++ )
        {
            int jx = i + neighb[z][0];
            int jy = j + neighb[z][1];
            if (margolus == 1) {
              //EVEN
              if (j % 2 == 0) { 
                jx = i + neighbEven[z][0];
                jy = j + neighbEven[z][1];
              }
              else { //ODD
                jx = i + neighbOdd[z][0];
                jy = j + neighbOdd[z][1];
              }
            }

            if (isInGrid(jx,jy)) {
                if (grid[jx][jy] >= 1) {
                  recep = true;
                  break;
                }
            }
        }
      }
      // float percent = (rand() % 100)  / (float)(100);
      int percent = rand() % 100;
      if (percent <= percentual) {
        if (recep) {
          U_new[i][j] = 0;
          V_new[i][j] = grid[i][j] + gamma;
        }
        else {
          U_new[i][j] = grid[i][j];
          V_new[i][j] = 0;
        }
      }
      else {
        U_new[i][j] = U[i][j];
        V_new[i][j] = V[i][j];
      }
    }
  }


  for (int i=1; i < size_grid-1; i++) {
    for (int j=1; j < size_grid-1; j++) {
        bool recep = false;
        int neigh = 0;
        float sum = U[i][j] * (1.0 - alpha * 6.0 /12.0);
        for (int z = 0; z < 6 ; z++ ) 
        {
            int jx = i + neighb[z][0];
            int jy = j + neighb[z][1];
            if (margolus == 1) {
              //EVEN
              if (j % 2 == 0) { 
                jx = i + neighbEven[z][0];
                jy = j + neighbEven[z][1];
              }
              else { //ODD
                jx = i + neighbOdd[z][0];
                jy = j + neighbOdd[z][1];
              }
            }

            if (isInGrid(jx,jy)) {
                neigh++;
                sum += U[jx][jy] * alpha / 12.0;
            }
        }
        grid_new[i][j] = V[i][j] + sum;
    }
  }

  U = U_new;
  V = V_new;
  return grid_new;
}

bool isInGrid(int i, int j) {
  if (i < 1 || i >= size_grid-1)
    return false;
  if (j < 1 || j >= size_grid-1)
    return false;
  return true;
}
