// Eduardo Gobbo Willi Vasconcellos Gonçalves GRR20203892

// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.4 -- Janeiro de 2022

/*
 *   INTER PROCCESS COMMUNICATIONS file (IPC) 
 *      Contains functions to manage:
 *          - semaphore_t operations
 *          - mqueue_t operations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ppos.h"
#include "queue.h"


int BIG_LOCK = 0;    // [0, 1] for atomic operation lock

// ========= semaforos

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

// ========= filas de mensagens

// cria uma fila para até max mensagens de size bytes cada, retorna 1 se ok, 0 c.c.
int mqueue_create (mqueue_t *queue, int max, int size){
    queue->status = NEW;
    queue->max_len = max;
    queue->ofsize = size;
    queue->append = 0;
    queue->remove = 0;
    queue->total = 0;

    // init semaphore
    if( sem_create( &(queue->buff), 1) ){
        fprintf(stderr, "could not create buffer semaphore for mqueue\n");
        return 0;
    }
    if( sem_create( &(queue->item), 0) ){
        fprintf(stderr, "could not create items semaphore for mqueue\n");
        return 0;
    }
    if( sem_create( &(queue->vaga), max) ){
        fprintf(stderr, "could not create vagas semaphore for mqueue\n");
        return 0;
    }

    queue->msgs = calloc(1, max*sizeof(void*));
    if(!queue->msgs){
        fprintf(stderr, "could not allocate memory for message queue\n");
        return 0;
    }
    for(int i = 0; i < max; i++){
        queue->msgs[i] = calloc(1, max*sizeof(void*));
        if(!queue->msgs[i]){
            fprintf(stderr, "could not allocate memory for message queue\n");
            return 0;
        }
    }

    queue->status = CREATED;
    return 1;
}

// envia uma mensagem para a fila
int mqueue_send (mqueue_t *queue, void *msg){
    // VALIDATE //
    if(!queue){
        fprintf(stderr, "tentou enviar msg pra queue NULL\n");
        return -1;
    }
    if(!msg){
        fprintf(stderr, "tentou enviar msg NULL pra queue\n");
        return -1;
    }
    if(queue->status == DESTROYED){
        fprintf(stderr, "tentou operar em um mqueue destruido\n");
        return -1;
    }

    if(sem_down(&(queue->vaga)) != 0) return -1;
    if(sem_down(&(queue->buff)) != 0) return -1;

    // APPEND //
    memcpy(queue->msgs[queue->append], msg, queue->ofsize);
    queue->append = (queue->append+1)%queue->max_len;
    queue->total++;

    if(sem_up(&(queue->buff)) != 0) return -1;
    if(sem_up(&(queue->item)) != 0) return -1;

    return 0;
}

// recebe uma mensagem da fila
int mqueue_recv (mqueue_t *queue, void *msg){
    // VALIDATE //
    if(!queue){
        fprintf(stderr, "tentou receber msg pra queue NULL\n");
        return -1;
    }
    if(queue->status == DESTROYED){
        fprintf(stderr, "tentou operar em um mqueue destruido\n");
        return -1;
    }

    if(sem_down(&(queue->item)) != 0) return -1;
    if(sem_down(&(queue->buff)) != 0) return -1;

    // RECEIVE //
    memcpy(msg, queue->msgs[queue->remove], queue->ofsize);
    queue->remove = (queue->remove+1)%queue->max_len;
    queue->total--;
    
    if(sem_up(&(queue->buff)) != 0) return -1;
    if(sem_up(&(queue->vaga)) != 0) return -1;


    return 0;
}

// destroi a fila, liberando as tarefas bloqueadas
int mqueue_destroy (mqueue_t *queue){
    // VALIDATE //
    if(!queue){
        fprintf(stderr, "tentou destruir queue NULL\n");
        return -1;
    }
    if(queue->status == DESTROYED){
        fprintf(stderr, "tentou destruir queue destruida\n");
        return -1;
    }

    queue->status = DESTROYED;
    free(queue->msgs);
    // destroi semaforo

    if(sem_destroy(&(queue->vaga)) != 0){
        fprintf(stderr, "could not destroy vagas semaphore for mqueue\n");
        return -1;
    }
    if(sem_destroy(&(queue->item)) != 0){
        fprintf(stderr, "could not destroy items semaphore for mqueue\n");
        return -1;
    }
    if(sem_destroy(&(queue->buff)) != 0){
        fprintf(stderr, "could not destroy buffer semaphore for mqueue\n");
        return -1;
    }
    
    return 0;
}

// informa o número de mensagens atualmente na fila
int mqueue_msgs (mqueue_t *queue){
    // VALIDATE //
    if(!queue){
        fprintf(stderr, "tentou ler queue NULL\n");
        return -1;
    }
    if(queue->status == DESTROYED){
        fprintf(stderr, "could not read size of destroyed mqueue\n");
        return 0;
    }
    return queue->total;
}
