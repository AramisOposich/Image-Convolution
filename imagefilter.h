#ifndef _imagefilter
#define _imagefilter

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>

struct timeval t1, t2;

key_t key;
key_t key_child[5000];

int imagefiltering(int child_ref, int y_start, int row, int column, int colourdeep,int laufzahl, int N, double *kernel);

#endif
