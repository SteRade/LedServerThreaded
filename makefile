prog: LedSteuernThreaded.o
	gcc -o prog LedSteuernThreaded.o -lpthread -lwiringPi

LedSteuernThreaded.o: LedSteuernThreaded.c
	gcc -c LedSteuernThreaded.c 