// Eduardo Gobbo Willi Vasconcellos Gonçalves GRR20203892

// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.4 -- Janeiro de 2022

/*
 *   INTER PROCCESS COMMUNICATIONS file
 */

#include <stdio.h>
#include "ppos.h"
#include "queue.h"


int BIG_LOCK = 0;    // [0, 1] for atomic operation lock
enum SEM_STATE {NEW = 1, CREATED, DESTROYED};


void enter_cs(int *lock){
    // atomic OR
    while(__sync_fetch_and_or(lock, 1));    // busy waiting
}
void leave_cs(int *lock){
    (*lock) = 0;
}

// cria um semáforo com valor inicial "value"
int sem_create (semaphore_t *s, int value){
    // VALIDATE //
    if(!s){
        // semaforo deve existir
        fprintf(stderr, "tried to create a semaphore that points to NULL\n");
        return -1;
    }

    if(s->status == CREATED){
        // semaforo deve ser novo
        fprintf(stderr, "tried to created an already existing semaphore\n");
        return -1;
    }

    // CREATE //
    s->status = NEW;        // new semaphore 
    s->value = value;       // valor inicial
    s->sem_queue = NULL;    // fila vazia
    s->status = CREATED;    // semaphore now exists (created)


    return 0;   // criado com sucesso
}

// requisita o semáforo
int sem_down (semaphore_t *s){
    // VALIDATE //
    if(!s){
        // semaforo deve existir
        fprintf(stderr, "tried to down a semaphore that points to NULL\n");
        return -1;
    }
    
    if(s->status == DESTROYED){
        // semaforo nao deve estar destruido
        fprintf(stderr, "tried to down a destroyed semaphore\n");
        return -1;
    }

    // DOWN //
    // enter_cs(&BIG_LOCK);
    s->value--;
    if(s->value < 0)
        task_suspend((task_t**) &(s->sem_queue));
    // leave_cs(&BIG_LOCK);

    return 0;   // DOWN success
}

// libera o semáforo
int sem_up (semaphore_t *s){
    // VALIDATE //
    if(!s){
        // semaforo deve existir
        fprintf(stderr, "tried to up a semaphore that points to NULL\n");
        return -1;
    }
    
    if(s->status == DESTROYED){
        // semaforo nao deve estar destruido
        fprintf(stderr, "tried to up a destroyed semaphore\n");
        return -1;
    }

    // UP //
    // enter_cs(&BIG_LOCK);
    s->value++;
    if(s->value <= 0)
        task_resume((task_t *) s->sem_queue, (task_t**) &(s->sem_queue));
    // leave_cs(&BIG_LOCK);

    return 0;   // UP success
}

// destroi o semáforo, liberando as tarefas bloqueadas
int sem_destroy (semaphore_t *s){
    // VALIDATE //
    if(!s){
        // semaforo deve existir
        fprintf(stderr, "tried to down a semaphore that points to NULL\n");
        return -1;
    }

    if(s->status == DESTROYED){
        // semaforo nao deve estar destruido
        fprintf(stderr, "tried to destroy a destroyed semaphore\n");
        return -1;
    }

    // DESTROY //
    int num_sus;
    task_t *aux, *unblock;
    unblock = (task_t*) s->sem_queue;
    num_sus = queue_size((queue_t*) s->sem_queue);

    for(int i = 0; i < num_sus; i++){
        aux = unblock->next;
        task_resume(unblock, (task_t **) &(s->sem_queue));
        unblock = aux;
    }

    
    s->status = DESTROYED;

    return 0; // DESTROY success
}