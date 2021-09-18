#include "imagefilter.h"

int imagefiltering (int child_ref, int y_start, int row, int column, int colourdeep, int laufzahl, int N, double *kernel)
{
    int zeilen = row - y_start;
    int shmcreat;
    int a = - (N - 1) / 2; //Schrittweite für Multiplikationen
    int b = - (N - 1) / 2;
    int img_lauf = 0;
    int child_shmcreat[child_ref+1];
    unsigned char *buf;
    unsigned char *buf_child;

#ifdef ORIGINAL
    unsigned char img[zeilen*column*20];
#else
    unsigned char *img = malloc(zeilen*column*20);
#endif

    size_t PixelSize = sizeof (unsigned char);
    size_t Bufsize = PixelSize * laufzahl * 3;
    size_t Bufsize_child = PixelSize * zeilen * column;

#ifdef ORIGINAL
    double output[zeilen*column*20];
#else
    double *output = malloc(zeilen*column*20);
#endif

    shmcreat = shmget (key, Bufsize, 0600);
    if (shmcreat == -1) {
       printf("%s at [%s:%d]\n",strerror(errno),__FILE__,__LINE__);
    }

    if (shmcreat >= 0) {
        buf = shmat (shmcreat, 0, 0) ;
        if ( (char*) buf == (char*) - 1) {
            perror (" shmat ") ;
        } else {
            /*******Imagefilter with Kernel*************/
            for (int y = y_start; y < row; y++) {
                for (int x = 0; x < column; x++) {
                    a = - (N - 1) / 2;

#ifdef ORIGINAL
                    int yy = y;
#else
                    int yy = y - y_start;
#endif
                    output[yy * column + x] = 0;
                    int i = 0;
                    for (int k = 0; k < N; k++) {
                        b = - (N - 1) / 2;
                        for (int j = 0; j < N; j++) {
                            int sy = (y * column) + (a * column);
                            int sx = x + (b * 3);
                            if (sy < 0) {
                                do {
                                    sy += column;
                                } while (sy < 0);
                            }
                            if (sy > (column * (row - 1))) {
                                do {
                                    sy -= column;
                                } while (sy > (column * (row - 1)));
                            }
                            if (sx < 0) {
                                do {
                                    sx += 3;
                                } while (sx < 0);
                            }
                            if (sx > (column - 1)) {
                                do {
                                    sx -= 3;
                                } while (sx > (column - 1));
                            }
                            
                            output[ (yy * (column)) + x] += (buf[sy + sx] * kernel[i]) / 255;
                            i++;
                            b++;
                        }
                        a++;
                    }

                    output[ (yy * column) + x] *= 255;
                    if (output[ (yy * (column)) + x] > 255) {
                        output[ (yy * (column)) + x] = 255;
                    } else if (output[ (yy * (column)) + x] < 0) {
                        output[ (yy * (column)) + x] = 0;
                    } else
                        output[ (yy * (column)) + x] = round (output[ (yy * (column)) + x]);
                    img[img_lauf] = output[ (yy * (column)) + x];
                    img_lauf++;
                }
            }
            shmdt (buf);
        }
    } else {
        perror (" shmget ");
    }
    /********Ergebnis in eine eigene shared memory**************/

    key_child[child_ref] = ftok ("/etc/hostname", child_ref);
    if (key == -1) {
       printf("%s at [%s:%d]\n",strerror(errno),__FILE__,__LINE__);
    }
    //Child Shared Memory chreate
    child_shmcreat[child_ref] = shmget (key_child[child_ref], Bufsize_child, IPC_CREAT | 0600);
    if (child_shmcreat[child_ref] == -1) {
       printf("%s at [%s:%d]\n",strerror(errno),__FILE__,__LINE__);
    } else if (child_shmcreat[child_ref] >= 0) {
        buf_child = shmat (child_shmcreat[child_ref], 0, 0);
        if ( (char*) buf_child == (char*) - 1) {
            perror ("shmat");
            shmctl (child_shmcreat[child_ref], IPC_RMID, 0);	//Hier wird die Sharedmem zerstört
        } else {

            for (int a = 0; a < img_lauf; a++) {
                buf_child[a] = img[a];
            }
        }
    }
#ifndef ORIGINAL
    free(img);
    free(output);
#endif

    return 0;
}
