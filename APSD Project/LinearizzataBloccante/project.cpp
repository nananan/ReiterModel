#include <allegro.h>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <iostream>
#include <time.h>
#include <algorithm>
#include <mpi.h>
#include "Master.h"
#include "Slave.h"
using namespace std;


volatile int close_button_pressed = FALSE;
BITMAP *buffer;//Dichiarazioni generali: buffer

void close_button_handler(void) {
  close_button_pressed = TRUE;
}

#define MASTER 0
#define SIZE 500
#define SIZE_D 540

#define ALPHA 1
#define BETA 0.6
#define GAMMA 0.0001

#define STEP 1000

int imgx = 500; int imgy = 500;
int imgx1 = imgx - 1; int imgy1 = imgy - 1;

int main(int argc, char* argv[])
{
    int numproc, rank, dest, source, tag=0, err;
    MPI_Status status;
    
    // int end = 0;
    double start = 0, end = 0;
    int count = 0;

    double an45 = - M_PI / 4.0;
    double sn45 = sin(an45); 
    double cs45 = cos(an45);
    double scale = sqrt(3.0); 
    double ox = imgx1 / 2.0; 
    double oy = imgy1 / 2.0;

    int blu;
    int bianco;
    int nero;



    err = MPI_Init(&argc, &argv);
    err = MPI_Comm_size(MPI_COMM_WORLD, &numproc);
    err = MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    shared_ptr<AbstractProcess> process;

    if (rank == MASTER) {
        process = make_shared<Master>(0, BETA, ALPHA, GAMMA, SIZE, numproc);
        process->PartitioningGrid(numproc);
        process->sendNeighbourd();
    }
    else {
        process = make_shared<Slave>(rank, ALPHA, GAMMA, SIZE, numproc);
        process->receive(status, numproc);
        process->receiveNeighbourd(status);
    }

    MPI_Barrier(MPI_COMM_WORLD); /* IMPORTANT */
    start = MPI_Wtime();

    process->createGrid();
    process->update(numproc);
    int step = 0;

    while(step < STEP) {
        if (rank != MASTER) {
            if (step % SEND_MASTER == 0)
              process->send_new();
            process->sendReceiveBorder(status, false);

            process->updateGrid();
            process->update(numproc);
            step++;
        }
        else  {
            if (step % SEND_MASTER == 0 && numproc > 1)
                process->receive(status, numproc);

            if (count == 0) {
                allegro_init();
                install_keyboard();
                install_mouse();
                set_close_button_callback(close_button_handler);
                //Indica al computer la modalità grafica che intendiamo usare
                set_gfx_mode(GFX_AUTODETECT_WINDOWED, SIZE_D, SIZE_D, 0, 0);
                //Mostra il mouse sullo schermo
                show_mouse(screen);
                // Funzione che crea il buffer dove disegniamo gli elementi
                buffer = create_bitmap(SIZE_D, SIZE_D);
            }
            count ++;
            int blu = makecol(0, 128, 255);
            int bianco = makecol(255,255,255);
            int nero = makecol(0, 0, 0);

            // int step = 0;
            float** grid = process->getGrid();
            // while (!(close_button_pressed || key[KEY_ESC])) {
                // MPI_Barrier(MPI_COMM_WORLD);
            for (int i=0; i < SIZE; i++) {
                for (int j=0; j < SIZE; j++) {
                    double tx = (i - ox)* (scale); 
                    double ty = (j - oy);
                    double tx0 = tx * cs45 - ty * sn45 + ox;
                    ty = tx * sn45 + ty * cs45 + oy; 
                    tx = tx0;
                    double c;
                    if (tx >= 0 and tx <= imgx1 and ty >= 0 and ty <= imgy1)
                        c = grid[int((SIZE - 1) * ty / imgy1)][int((SIZE - 1) * tx / imgx1)];

                    if (c >= 1)
                        putpixel(buffer, i, j, blu);
                    else if (c >= 0.9 and c < 1)
                        putpixel(buffer, i, j, bianco);
                }
            }
            char str [] = "Alpha: %f Beta: %f Gamma: %f";
            // textprintf_ex(buffer,font,0,SIZE_WIN-25,bianco,nero, "Step: %i ",step);
            textprintf_ex(buffer,font,0,SIZE_D-10,bianco,nero, str,ALPHA, BETA, GAMMA);
            blit(buffer,screen,0,0,0,0,SIZE,SIZE_D);
            clear_bitmap(buffer);

            step++;
            if (numproc > 1)
                process->sendReceiveBorder(status, false);
            process->updateGrid();
            process->update(numproc);
        }
    }

    MPI_Barrier(MPI_COMM_WORLD); /* IMPORTANT */
    end = MPI_Wtime();

    if (rank == 0) { /* use time on master node */
        printf("Runtime = %f\n", end-start);
    }


    err = MPI_Finalize();
	return 0;
}
