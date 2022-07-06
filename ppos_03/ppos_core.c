// Eduardo Gobbo Willi Vasconcellos Gonçalves GRR20203892

#include <stdlib.h>
#include <stdio.h>
#include "queue.h"
#include "ppos.h"
#include "ppos_data.h"

// #define DEBUG
#define STACKSIZE 64*1024	/* tamanho de pilha das threads */

/* core local function headers */
void dispatcher_body();
task_t *scheduler_body();
void print_task(void *ptr);


/* core global variables */
enum TaskStates {NEW = 1, READY, RUNNING, SUSPENDED, TERMINATED };
task_t TaskMain, TaskDispatcher;
task_t *CurrentTask, *QueueReady;
int TID, UserTasks;

// Inicializa o sistema operacional; deve ser chamada no inicio do main()
void ppos_init () 
{
    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0) ;

    /* initialize core global values */
    TID = 0;
    UserTasks = 0;

    /* create main task */
    // task_create(&TaskMain, NULL, NULL);
    // queue_remove((queue_t **) &QueueReady, (queue_t *) &TaskMain);
    // UserTasks--;
    TaskMain.id = TID++;
    CurrentTask = &TaskMain;

    /* create dispatcher task */
    task_create(&TaskDispatcher, dispatcher_body, NULL);
    queue_remove((queue_t **) &QueueReady, (queue_t *) &TaskDispatcher);
    UserTasks--;

    #ifdef DEBUG
    printf("PPOS: system initialized\n");
    #endif
}

// gerência de tarefas =========================================================

// Cria uma nova tarefa. Retorna um ID> 0 ou erro.
int task_create (task_t *task,			        // descritor da nova tarefa
                 void (*start_func)(void *),	// funcao corpo da tarefa
                 void *arg) 			        // argumentos para a tarefa
{
    // VALIDADE //
    if(!task)
    {   // task must exist
        fprintf(stderr, "### ERROR: tried to create a task with an empty descriptor\n");
        return -9;
    }

    if(!start_func)
    {   // function must exist
        fprintf(stderr, "### ERROR: tried to create a task with an empty body function\n");
        return -10;
    }

    // CREATE TASK //
    char* stack;
    
    // set task initial state & identifiers
    task->status = NEW;
    task->id = TID++;
    UserTasks++;
    
    getcontext(&(task->context));

    stack = malloc (STACKSIZE) ;
    if (stack)
    {
        (task->context).uc_stack.ss_sp = stack ;
        (task->context).uc_stack.ss_size = STACKSIZE ;
        (task->context).uc_stack.ss_flags = 0 ;
        (task->context).uc_link = 0 ;
    }
    else
    {
        perror ("Erro na criação da pilha: ") ;
        exit (1) ;
    }

    // creating task body function
    makecontext(&(task->context), (void *)start_func, 1, arg);

    // prepare to append task 
    task->next = NULL;
    task->prev = NULL;

    // append task to global queue
    queue_append((queue_t **) &QueueReady, (queue_t *) task);
    

    #ifdef DEBUG
    printf("PPOS: task %d created by task %d (body function %p)\n", task->id, CurrentTask->id, start_func);
    #endif

    // finished creating task
    task->status = READY;
    return task->id;
}

// Termina a tarefa corrente, indicando um valor de status encerramento
void task_exit (int exit_code) 
{
    #ifdef DEBUG
    printf("PPOS: task %d exited\n", CurrentTask->id);
    #endif  

    CurrentTask->status = TERMINATED;

    // return control to dispatcher (except if dispatcher exit)
    (CurrentTask != &TaskDispatcher) ? task_switch(&TaskDispatcher) : task_switch(&TaskMain);
}

// alterna a execução para a tarefa indicada
int task_switch (task_t *task) 
{
    // VALIDATE //
    if(!task)
    {   // task must exist
        fprintf(stderr, "### ERROR: tried to switch to a task with an empty descriptor\n");
        return -11;
    }

    // SWITCH //
    #ifdef DEBUG
    printf("PPOS: switch task %d -> task %d\n", CurrentTask->id, task->id);
    // queue_print("Ready ", (queue_t *) QueueReady, print_task);
    #endif

    // save old context
    ucontext_t *current_context = &(CurrentTask->context);
    
    CurrentTask = task;                             // update current task pointer
    CurrentTask->status = RUNNING;                  // new task is running
    swapcontext(current_context, &(task->context)); // swap old cpu register data with new task data

    return 0;
}   

// retorna o identificador da tarefa corrente (main deve ser 0)
int task_id () 
{   
    return CurrentTask->id;
}

// libera o processador para a próxima tarefa, retornando à fila de tarefas
// prontas ("ready queue")
void task_yield ()
{
    #ifdef DEBUG
    printf("PPOS: task %d yields the CPU\n", CurrentTask->id);
    #endif

    // yielding the cpu, change task state
    CurrentTask->status = READY;
    task_switch(&TaskDispatcher);
}

// function to print tasks with queue print
void print_task(void *ptr)
{
    task_t *task = ptr;

    if(!task)   // task must exist to be printed
        return;

    printf("%d", task->id);
}

// 
void dispatcher_body()
{
    #ifdef DEBUG
    printf("PPOS: task dispatcher launched\n");
    queue_print("Ready ", (queue_t *) QueueReady, print_task);
    #endif

    task_t *next_task;

    while(UserTasks)
    {   // enquanto houverem tarefas de usuário
        // escolhe a próxima tarefa a executar
        next_task = scheduler_body();

        if(next_task)
        {   // next task must exist (!NULL)
            task_switch(next_task);

            switch (next_task->status)
            {
                case NEW:
                    break;
                
                case READY:
                    break;
                
                case RUNNING:
                    break;
                
                case SUSPENDED:
                    break;
                
                case TERMINATED:
                    // printf("trying to remove task %d\n", next_task->id);
                    queue_remove((queue_t **) &QueueReady, (queue_t *) next_task);
                    UserTasks--;
                    break;
                
                default:
                    break;
            }

            #ifdef DEBUG
            queue_print("Ready ", (queue_t *) QueueReady, print_task);
            #endif
        }
    }

    task_exit(0);
}

// FCFS - First Come First Serve (FIFO 4 OS)
task_t *scheduler_body()
{
    task_t *next = QueueReady;
    QueueReady = QueueReady->next;

    return next;
}