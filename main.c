// ./a.out -p 5 -k "0,0,0,0,1,0,0,0,0" -i airplane.p6.ppm -o airplane.ppm
#include "imagefilter.h"

int main (int argc, char *argv[])
{
    FILE *fp;
    FILE *fOutput;
    int N, i, Np, shmcreat, opt, rv, start_y, y, row, width, zeilen, colourdeep;
    double kernel[1000];
    char buffer[100];
    char *end = NULL;
    unsigned char *buf;
    size_t result;
    size_t PixelSize = sizeof (unsigned char);
    /******Eingabeargumente einlesen******/
/*
 * ### FB_CF: Statt o müsste auch o: stehen, da o ja auch ein Argument
 *            benötigt.
 */
#ifdef ORIGINAL
    while ( (opt = getopt (argc, argv, "p:k:i:o")) != -1) {
#else
    while ( (opt = getopt (argc, argv, "p:k:i:o:")) != -1) {
#endif
        switch (opt) {
        case 'p':
            Np = strtol (optarg, &end, 100);
/*
 * ### FB_CF: Sie müssen auch checken, ob *end == '\0', dann waren nur gültige Zeichen im String.
 */
            if (errno == ERANGE || Np == 0) {
               printf("%s at [%s:%d]\n",strerror(errno),__FILE__,__LINE__);
                exit (EXIT_FAILURE);
            }
           printf("%s at [%s:%d]\n",strerror(errno),__FILE__,__LINE__);
            break;
        case 'k':
            end = NULL;
            char *pker = optarg;
/*
 * ### FB_CF: i wird uninitialisiert verwendet, dadurch Zugriff auf ungültigen Speicher => segfault
 */
#ifndef ORIGINAL
            i = 0;
#endif
            for (kernel[0] = strtod (pker, &end); pker != end ; kernel[i] = strtod (pker, &end)) {
/*
 * ### FB_CF: Überprüfen, ob *pker ein gültiges Zeichen ist (also ',' oder '\0')
 * ### FB_CF: Sie benötigen auch eine Überprüfung, ob i < 5000, da kernel[] ja nur 5000 Elemente
 *            aufnehmen kann. Sinnvoller wäre es allerdings als Maximalgröße 5041, also 71x71,
 *            zo wählen, da 5000 keine Quadratzahl ist.
 */
                pker = end;
                pker++;
                i++;
            }
            if (i < 8 || (i % 2) == 0 || i == (int) sqrt (i)) {
               printf("%s at [%s:%d]\n",strerror(errno),__FILE__,__LINE__);
                exit (EXIT_FAILURE);
            } else {
                N = sqrt (i);
               printf("%s at [%s:%d]\n",strerror(errno),__FILE__,__LINE__);
                for (int a = 0; a < i; a++) {
                   printf("%s at [%s:%d]\n",strerror(errno),__FILE__,__LINE__);
                }
            }
            break;
        case 'i':
            fp = fopen (optarg,"rb");
            if (fp == NULL) {
               printf("%s at [%s:%d]\n",strerror(errno),__FILE__,__LINE__);
                exit (EXIT_FAILURE);
            }
            break;
        case 'o':
            errno = 0;
            fOutput = fopen (optarg, "wb");
/*
 * ### FB_CF: Hier müssen Sie fOutput und nicht fp checken.
 */
#ifdef ORIGINAL
            if (fp == NULL) {
#else
            if (fOutput == NULL) {
                perror("fopen()");
#endif
               printf("%s at [%s:%d]\n",strerror(errno),__FILE__,__LINE__);
                exit (EXIT_FAILURE);
            }
            break;
        default:
           printf("%s at [%s:%d]\n",strerror(errno),__FILE__,__LINE__);
           printf("%s at [%s:%d]\n",strerror(errno),__FILE__,__LINE__);
            exit (EXIT_FAILURE);
        }
    }

/*
 * ### FB_CF: Was, wenn -i oder -o vergessen wird? => Sie müssen hier
 *            prüfen, ob fp und fOutput gültig sind.
 * ### FB_CF: Allgemein greifen Sie auf uninitialisierten Speicher zu,
 *            wenn bestimmte Argumente fehlen (auch -p und -k)
 */

    /*******READ THE HEADER FROM P6 PIC*****/
    if (fgets (buffer, 100, (FILE*) fp)  == NULL) {
        fclose (fp);
        exit (EXIT_FAILURE);
    }
    if (strncmp (buffer, "P6", 2) != 0) {
       printf("%s at [%s:%d]\n",strerror(errno),__FILE__,__LINE__);
        fclose (fp);
        exit (EXIT_FAILURE);
    }
    int b = fscanf (fp, "%d", &width);
    if (b < 1) {
        fclose (fp);
        exit (EXIT_FAILURE);
    }
    b = fscanf (fp, "%d", &zeilen);
    if (b < 1) {
        fclose (fp);
        exit (EXIT_FAILURE);
    }
    b = fscanf (fp, "%d", &colourdeep);
    if (b < 1) {
        fclose (fp);
        exit (EXIT_FAILURE);
    }
    if (fgets (buffer, 100, (FILE*) fp)  == NULL) {
        fclose (fp);
        exit (EXIT_FAILURE);
    }
    if (strncmp (buffer, "\n", 2) != 0) {
       printf("%s at [%s:%d]\n",strerror(errno),__FILE__,__LINE__);
        fclose (fp);
        exit (EXIT_FAILURE);
    }
    if (Np > zeilen) {
       printf("%s at [%s:%d]\n",strerror(errno),__FILE__,__LINE__);
        exit (EXIT_FAILURE);
    }
    /****Filling the Inputpic in the MAIN-sharedmemory**************/
    int laufzahl = zeilen * width;
    size_t Bufsize = PixelSize * laufzahl * 3;

    key = ftok ("/etc/hostname", 'b');
    if (key == -1) {
/*
 * ### FB_CF: Das reicht leider nicht zur Fehlerbehandlung. Sie müssen auch
 *            dafür sorgen, dass key (das ja hier -1 ist) nicht weiter verwendet wird.
 */
       printf("%s at [%s:%d]\n",strerror(errno),__FILE__,__LINE__);
    }
    //Shared Memory erstellt
    shmcreat = shmget (key, Bufsize, IPC_CREAT | 0600);
    if (shmcreat == -1) {
       printf("%s at [%s:%d]\n",strerror(errno),__FILE__,__LINE__);
    } else if (shmcreat >= 0) {
        buf = shmat (shmcreat, 0, 0);
        if ( (char*) buf == (char*) - 1) {		//Wenn nichts an die Shared Mem hinzugefügt werden kann
            perror ("shmat");
            shmctl (shmcreat, IPC_RMID, 0);		//Hier wird die Sharedmem zerstört
        } else {
            for (int a = 0; a < laufzahl * 3; a++) {
                result = fread (&buf[a], PixelSize, 1, fp);
                if (result != 1) {
                   printf("%s at [%s:%d]\n",strerror(errno),__FILE__,__LINE__);
                    exit (EXIT_FAILURE);
                }
            }
        }
    }
    /*************Parallel operation***************/
    int aufgeteilt = zeilen / Np;
    int rest_aufgeteilt = zeilen - aufgeteilt * (Np - 1);
    int column = width * 3;
    int child_shmcreat[Np];

/*
 * ### FB_CF: start_y wird uninitialisiert verwendet.
 */
#ifndef ORIGINAL
    start_y = 0;
#endif
    for (int child_ref = 0 ; child_ref < Np; child_ref++, start_y += (aufgeteilt)) {
        // Variablen für den Kindprozess initialisieren (z.B. welchen Teil des Bildes er bearbeiten soll)

        if (child_ref == (Np - 1)) {    //Letzter Prozess
            row = start_y + rest_aufgeteilt;
        } else {
            row = start_y + aufgeteilt;
        }
        y = start_y;
        rv = fork();
        if (rv == -1) {	//Falls kein Kind erstellt werden konnte
           printf("%s at [%s:%d]\n",strerror(errno),__FILE__,__LINE__);
            exit (EXIT_FAILURE);
        }
        if (rv == 0) {		//Kindprozess
/*
 * ### FB_CF: Da der Kindprozess alle FILEs dupliziert bekommt, müssen Sie
 *            fOutput und fp hier schließen (Im Hauptprozess bleiben sie
 *            trotzdem offen, da die zwei Prozesse ab dem fork()
 *            im Bezug auf Schreibezugriffe auf Variablen komplett
 *            getrennt sind.
 */
            gettimeofday(&t1, NULL);
            imagefiltering (child_ref, y, row, column, colourdeep, laufzahl, N, kernel);
            gettimeofday(&t2, NULL);
            //double diff = (t2.tv_sec+t2.tv_usec*0.000001)-(t1.tv_sec+t1.tv_usec*0.000001);
            exit (rv);
        }
        else {
            if (child_ref == (Np - 1)) {
/*
 * ### FB_CF: Woher wissen Sie, dass zu diesem Zeitpunkt alle Kindprozesse fertig sind?
 *            => Eigentlich (lt. Angabe) bräuchten Sie hier eine Synchronisation mittels
 *            Message Queue. In Ihrer Implementierung tritt eine sog. Race Condition auf,
 *            d.h. man kann nicht voraussagen, ob alle Kindprozesse schon terminiert sind
 *            wenn der Elternprozess hier ankommt und den Speicher, den die Kinder beschreiben
 *            schon auslesen möchte.
 */
                start_y = 0;
/*
 * ### FB_CF: Rückgabewert prüfen.
 */
                fprintf(fOutput, "P6\n# Kernel filter was applied\n%d %d\n%d\n",width, zeilen, colourdeep);
                for (int child_ref = 0 ; child_ref < Np; child_ref++, start_y += (aufgeteilt)) {
                    if (child_ref == (Np - 1)) {    //Letzter Prozess
                        row = start_y + rest_aufgeteilt;
                    } else
                        row = start_y + aufgeteilt;
                    unsigned char *buf_child;
                    size_t Bufsize_child = PixelSize * (row-start_y) * column;
                    int img_lauf = column * (row - start_y);
/*
 * ### FB_CF: Der Parameter "proj_id" darf bei ftok() nicht 0 sein, siehe Manpage:
 *              "The  ftok() function uses the identity of the file named
 *               by the given pathname (which must refer to an existing,
 *               accessible file) and the least significant 8 bits of
 *               proj_id (which must be nonzero) to generate a key_t type System V IPC key."
 */
                    key_child[child_ref] = ftok ("/etc/hostname", child_ref);
                    if (key_child[child_ref] == -1) {
                       printf("%s at [%s:%d]\n",strerror(errno),__FILE__,__LINE__);
/*
 * ### FB_CF: Hier müssen Sie irgendwie reagieren, nicht nur eine Fehlermeldung ausgeben.
 */
                    }

/*
 * ### FB_CF: Wenn die Kindprozesse hier das jeweilige Shared Memory noch nicht erstellt haben,
 *            geht das shmget() schief. Hier brauchen Sie eine Synchronisation (eben zB über die Message Queue).
 *            Damit ich Ihr Programm ausprobieren kann, habe ich hier eine Quick&Dirty Synchronisiation eingefügt
 *            die so lange wartet, bis das shared memory da ist.
 */

#ifdef ORIGINAL
                    child_shmcreat[child_ref] = shmget (key_child[child_ref], Bufsize_child, 0600);
#else
                    do {
                        errno = 0;
                        child_shmcreat[child_ref] = shmget (key_child[child_ref], Bufsize_child, 0600);
                        usleep(1000); // Sleep 1 ms
                    } while (errno == ENOENT);
#endif
                    if (child_shmcreat[child_ref] == -1) {
                       printf("%s at [%s:%d]\n",strerror(errno),__FILE__,__LINE__);
                    } else if (child_shmcreat[child_ref] >= 0) {
                        buf_child = shmat (child_shmcreat[child_ref], NULL, 0);
                        if ( (char*) buf_child == (char*) - 1) {
                            perror ("shmat");
                            shmctl (child_shmcreat[child_ref], IPC_RMID, 0);		//Hier wird die Sharedmem zerstört
                        } else {
                            for (int a = 0; a < img_lauf; a++) {
/*
 * ### FB_CF: Hier sollten Sie auch in das Output File schreiben.
 */
                                printf (" %d ", buf_child[a]);          //Hier kannst du mit der Shared Memory operieren
#ifndef ORIGINAL
                                fputc ((unsigned char)buf_child[a],fOutput);
#endif
                            }
                            shmctl (child_shmcreat[child_ref], IPC_RMID, 0);
                        }
                    }
                }
                shmctl (shmcreat, IPC_RMID, 0);     //Main Shared Memory löschen
                fclose (fp);
                //fclose (fOutput);
                return 0;
                exit(EXIT_FAILURE);
            }
        }
    }
/*
 * ### FB_CF: Warum ist das hier auskommentiert?
 */
//fclose (fOutput);
    fclose (fp);
    return 0;
}
