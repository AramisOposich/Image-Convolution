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
/*
 * ### FB_CF: Warum *20?
 * ### FB_CF: Warum erzeugen Sie hier ein eigenes Array? Sie könnten sich
 *            den Speicher zuerst von shmget()/shmat() holen, dann könnten
 *            Sie direkt in das Output-Shared-Memory schreiben uns sparen
 *            sich das Kopieren am Schluss.
 * ### FB_CF: Ein Array dieser Größe sollten Sie mittels malloc() und nicht
 *            als Variable Sized Array allokieren. In Ihrer Implementierung
 *            verursachen Sie dadurch einen Stack Overflow (zu wenig Platz
 *            am Stack)
 */
#ifdef ORIGINAL
    unsigned char img[zeilen*column*20];
#else
    unsigned char *img = malloc(zeilen*column*20);
#endif

    size_t PixelSize = sizeof (unsigned char);
    size_t Bufsize = PixelSize * laufzahl * 3;
    size_t Bufsize_child = PixelSize * zeilen * column;
/*
 * ### FB_CF: Warum *20?
 * ### FB_CF: Selbes Problem wie bei img[]
 */
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
/*
 * ### FB_CF: output[] haben Sie mit folgender Größe erstellt:
 * 
 *            zeilen * column * 20
 *
 *            zeilen = row - y_start
 *
 *            Größe = (row - y_start) * column * 20
 *
 *            Wenn das Bild zB 512*256 groß ist, und auf 2 Prozesse aufgeteilt wird:
 *
 *            1. Prozess: row = 256/2     = 128, y_start = 0, column = 512
 *            2. Prozess: row = 256/2 * 2 = 256, y_start = 128, column = 512
 *
 *            In jedem dieser Prozesse ist zeilen = row - y_start = 128.
 *
 *            Sie lassen das y von y_start bis row laufen, d.h.:
 *
 *            1. Prozess: y = 0   ... 127
 *            2. Prozess: y = 128 ... 255
 *
 *            Das Problem ist jetzt, dass Sie die Größe von output[] zwar
 *            richtig berechnet haben, aber hier output[] bzw später img[] 
 *            trotzdem falsch beschreiben.
 *
 *            Da output[] nur den Bildausschnitt, den dieser Kindprozess
 *            bearbeitet, abspeichern kann, müssen Sie hier die
 *            y-Koordinaten so verschieben, dass die erste Zeile, die
 *            dieser Kindprozess bearbeitet, bei y=0 in output[] abgelegt wird,
 *            also:
 */
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
/*
 * ### FB_CF: Warum schreiben Sie hier zuerst in einen Buffer für das ganze Bild und
 *            kopieren dann in einen anderen Buffer?
 *
 *            Sie könnten genauso eine Schleifenlokale Variable statt output[]
 *            verwenden, da Sie nie wieder auf die einzelnen Stellen von output[] zugreifen, 
 *            nachdem Sie schon in img[] kopiert haben.
 */
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
/*
 * ### FB_CF: Der Parameter "proj_id" darf bei ftok() nicht 0 sein, siehe Manpage:
 *              "The  ftok() function uses the identity of the file named
 *               by the given pathname (which must refer to an existing,
 *               accessible file) and the least significant 8 bits of
 *               proj_id (which must be nonzero) to generate a key_t type System V IPC key."
 */
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
/*
 * ### FB_CF: memcpy() verwenden.
 */
            for (int a = 0; a < img_lauf; a++) {
                buf_child[a] = img[a];
            }
        }
    }
#ifndef ORIGINAL
    free(img);
    free(output);
#endif
/*
 * ### FB_CF: Wenn der Kindprozess fertig ist, sollte er eine Nachricht
 *            in eine Message Queue stellen, das ist essentiell für die
 *            Synchronisation.
 */
    return 0;
}
