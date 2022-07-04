# GRR20203892 Eduardo Gobbo Willi Vasconellos Gonçalves

contexts: contexts.c 
	gcc -Wall contexts.c -o contexts

# testafila: queue.h queue.c testafila.c
# 	gcc -Wall testafila.c queue.c -o testafila

clean:
# 	rm testafila 
	rm contexts
	
