all: main.c shm_auslesen.c
	gcc --pedantic -g main.c shm_auslesen.c -lm
clean:
	rm -rf *~*.o a.out
