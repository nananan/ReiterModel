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

int imgx = 500; int imgy = 500;
int imgx1 = imgx - 1; int imgy1 = imgy - 1;

void initAllegro(int &blu, int &bianco, int &nero);

int main(int argc, char* argv[])
{
    int numproc, rank, dest, source, tag=0, err;
    
    int outbuf;
    int inbuf[4] = {MPI_PROC_NULL, MPI_PROC_NULL, MPI_PROC_NULL, MPI_PROC_NULL};
    int dims[2] = {2,2};
    int periods[2] = {0,0}, reorder = 0;

    MPI_Comm cartcomm;
    MPI_Status status;
    

    double an45 = - M_PI / 4.0;
    double sn45 = sin(an45); 
    double cs45 = cos(an45);
    double scale = sqrt(3.0); 
    double ox = imgx1 / 2.0; 
    double oy = imgy1 / 2.0;

    int blu;
    int bianco;
    int nero;

    float** gridDraw;

    int count = 0;
    err = MPI_Init(&argc, &argv);
    err = MPI_Comm_size(MPI_COMM_WORLD, &numproc);
    err = MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    shared_ptr<AbstractProcess> process;

    if (rank == MASTER) {
        process = make_shared<Master>(0, BETA, ALPHA, GAMMA, NUMROW, NUMCOL, numproc);
        process->PartitioningGrid(numproc);
    }
    else {
        process = make_shared<Slave>(rank, ALPHA, GAMMA, NUMROW, NUMCOL, numproc);
        process->receive(status, numproc);
    }

    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, reorder, &cartcomm);
    
    process->setTopology(cartcomm);
    process->sendReceiveBorder(status, true);
    process->createGrid(status);

    double start, end;
    process->update();
    int step = 0;
    MPI_Barrier(MPI_COMM_WORLD); /* IMPORTANT */
    start = MPI_Wtime();
    while(step < STEP) {
        if (rank != MASTER) {
            if (step % SEND_MASTER == 0) {
                if (rank == 0)
                    MPI_Send(&step, 1, MPI_INT, MASTER, 12, MPI_COMM_WORLD);
                process->sendToMaster(status);
            }

            process->sendReceiveBorder(status, false);
            process->updateGrid(status);
            process->update();
            step++;
        }
        else  {
            if (count == 0) {
                initAllegro(blu, bianco, nero);
                gridDraw = process->getGrid();
            }
            for (int i=0; i < NUMROW; i++) {
                for (int j=0; j < NUMCOL; j++) {
                    double tx = (i - ox)* (scale); 
                    double ty = (j - oy);
                    double tx0 = tx * cs45 - ty * sn45 + ox;
                    ty = tx * sn45 + ty * cs45 + oy; 
                    tx = tx0;
                    double c;
                    if (tx >= 0 and tx <= imgx1 and ty >= 0 and ty <= imgy1)
                        c = gridDraw[int((NUMROW - 1) * ty / imgy1)][int((NUMCOL - 1) * tx / imgx1)];

                    if (c >= 1)
                        putpixel(buffer, i, j, blu);
                    else if (c >= 0.9 and c < 1)
                        putpixel(buffer, i, j, bianco);
                }
            }
            char str [] = "Alpha: %f Beta: %f Gamma: %f";
            textprintf_ex(buffer,font,0,SIZE_WIN-10,bianco,nero, str,ALPHA, BETA, GAMMA);
            blit(buffer,screen,0,0,0,0,NUMROW,SIZE_WIN);
            clear_bitmap(buffer);
            
            MPI_Recv(&step, 1, MPI_INT, 0, 12, MPI_COMM_WORLD, &status);
            process->receiveFromSlave(status, step);

            count++;
        }
    }

    
    if (rank == 0)
        MPI_Send(&step, 1, MPI_INT, MASTER, 12, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD); /* IMPORTANT */
    end = MPI_Wtime();

    if (rank == 0) { /* use time on master node */
        printf("Runtime = %f\n", end-start);
    }
    if (rank == MASTER) {
        for (int i = 0; i < NUMROW; i++)
            delete [] gridDraw[i];
        delete [] gridDraw;
    }

    process->freeMemory();
    err = MPI_Finalize();
	return 0;
}


void initAllegro(int &blu, int &bianco, int &nero) {
    allegro_init();
    install_keyboard();
    install_mouse();
    set_close_button_callback(close_button_handler);
    //Indica al computer la modalità grafica che intendiamo usare
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, NUMROW, SIZE_WIN, 0, 0);
    //Mostra il mouse sullo schermo
    show_mouse(screen);
    // Funzione che crea il buffer dove disegniamo gli elementi
    buffer = create_bitmap(NUMROW, SIZE_WIN);

    blu = makecol(0, 128, 255);
    bianco = makecol(255,255,255);
    nero = makecol(0, 0, 0);

}
