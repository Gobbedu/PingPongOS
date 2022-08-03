// Eduardo Gobbo Willi Vasconcellos Gonçalves GRR20203892

// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.4 -- Janeiro de 2022

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto

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
  int exit_code;                  // codigo de saida da tarefa
  // ... (outros campos serão adicionados mais tarde)
} task_t ;

// estrutura que define um semáforo
typedef struct
{
  // preencher quando necessário
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
typedef struct
{
  // preencher quando necessário
} mqueue_t ;

#endif
