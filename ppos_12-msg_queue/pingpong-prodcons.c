// Eduardo Gobbo Willi Vasconcellos Gonçalves GRR20203892

// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.4 -- Janeiro de 2022

// Teste de semáforos (light)

#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"
#include "queue.h"

typedef struct {
    void *next;
    void *prev;
    int item;
} buffer_queue;

task_t      p1, p2, p3, c1, c2;
semaphore_t s_buffer, s_item, s_vaga;
buffer_queue *BUFFER;
int NUMPROD = 5;

// corpo do produtor
void produtor (void * arg)
{
    // int item;
    buffer_queue *it;
    for(int i = 0; i < NUMPROD; i++){
        task_sleep(1000);

        it = malloc(sizeof(buffer_queue));
        it->item = rand() % 100;
        it->prev = it->next = NULL;

        sem_down(&s_vaga);
        sem_down(&s_buffer);
        // insere item no buffer
        queue_append((queue_t**) &BUFFER, (queue_t*) it);

        sem_up(&s_buffer);
        sem_up(&s_item);

        printf("%s produziu %d (tamanho buffer: %d)\n", (char*)arg, it->item, queue_size((queue_t*) BUFFER));
    }

   task_exit (0) ;
}

// corpo da thread B
void consumidor (void * arg)
{
    // int item;
    buffer_queue *it;
    task_sleep(1000); // wait for item to be produced 

    // for(int i = 0; i < NUMPROD; i++){        // precisa calcular proporcao prod/cons
    while(queue_size((queue_t*) BUFFER) > 0){   // consome tudo na fila
        sem_down(&s_item);
        sem_down(&s_buffer);

        // remove item no buffer
        it = BUFFER;
        queue_remove((queue_t**) &BUFFER, (queue_t*) it);

        sem_up(&s_buffer);
        sem_up(&s_vaga);

        printf("%s consumiu %d (tamanho buffer %d)\n", (char*)arg, it->item, queue_size((queue_t*) BUFFER));
        free(it);
        task_sleep(1000);
    }

   task_exit (0) ;
}

int main (int argc, char *argv[])
{
   printf ("main: inicio\n") ;

   ppos_init () ;

   // cria semaforos
   sem_create (&s_buffer, 1);
   sem_create (&s_item, 0);
   sem_create (&s_vaga, 5);

   // cria tarefas
   task_create (&p1, produtor, "P1 produziu") ;
   task_create (&p2, produtor, "P2 produziu") ;
   task_create (&p3, produtor, "P3 produziu") ;

   task_create (&c1, consumidor, "\t\t\tC1 consumiu") ;
   task_create (&c2, consumidor, "\t\t\tC2 consumiu") ;


   task_join (&p1);
   task_join (&p2);
   task_join (&p3);

   task_join (&c1) ;
   task_join (&c2) ;
   
   // destroi semaforos
   sem_destroy (&s_buffer) ;
   sem_destroy (&s_item) ;
   sem_destroy (&s_vaga) ;


   printf ("main: fim\n") ;
   task_exit (0) ;

   exit (0) ;
}
