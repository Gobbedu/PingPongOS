# GRR20203892 Eduardo Gobbo Willi Vasconellos Gonçalves

testafila: queue.h queue.c testafila.c
	gcc -Wall testafila.c queue.c -o testafila

clean:
	rm *.o

purge: clean
	rm testafila