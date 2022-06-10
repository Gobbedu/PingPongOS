testafila: queue.h queue.c testafila2.c
	gcc -Wall testafila2.c queue.c -o testafila

clean:
	rm *.o

purge: clean
	rm testafila