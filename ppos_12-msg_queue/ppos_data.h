// Eduardo Gobbo Willi Vasconcellos Gonçalves GRR20203892

// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.4 -- Janeiro de 2022

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto

// possible states inside the core, used by {tasks, semaphores, mqueues}
enum States {NEW = 1, CREATED, READY, RUNNING, SUSPENDED, TERMINATED, DESTROYED };

// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
  struct task_t *prev, *next ;		// ponteiros para usar em filas
  int id ;				                // identificador da tarefa
  ucontext_t context ;			      // contexto armazenado da tarefa
  short status ;			            // pronta, rodando, suspensa, ...
  short preemptable ;			        // pode ser preemptada?
  short static_prio ;             // prioridade estatica da tarefa
  short dynamic_prio ;            // prioridade dinamica da tarefa
  unsigned int proc_time;         // tempo gasto no processador
  unsigned int live_time;         // tempo de vida da tarefa
  unsigned int num_quant;         // numero de quantum recebido
  struct task_t *QueueHeritage;   // fila de tarefas esperando esta morrer
  unsigned int sleep_until;       // marca horario q task suspensa deve acordar
  int exit_code;                  // codigo de saida da tarefa
  // ... (outros campos serão adicionados mais tarde)
} task_t ;

// estrutura que define um semáforo
typedef struct semaphore_t
{
  short int status;               // could be new, created or destroyed
  int value;                      // semaphore value
  struct semaphore_t *sem_queue;  // list of tasks waiting semaphore
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct {
    void **msgs;        // pointers to void pointers that holds the message
    int status;         // status of mqueue
    int ofsize;         // sizeof(mensagem)
    int max_len;        // msgs[max_len]
    int append;         // index to append new message 
    int remove;         // index to remove message
    int total;          // number of current messages
    semaphore_t vaga;   // semaforo com numero de vagas
    semaphore_t item;   // semaforo com numero de itens
    semaphore_t buff;   // mutex que permite um acesso ao buffer
} mqueue_t;


#endif
