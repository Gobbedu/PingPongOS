// Eduardo Gobbo Willi Vasconcellos Gonçalves GRR20203892

#include "ppos.h"
#include "ppos_data.h"
#include <stdio.h>
#include <stdlib.h>

// #define DEBUG
#define STACKSIZE 64*1024	/* tamanho de pilha das threads */

task_t TaskMain;
task_t *CurrentTask;    
int ID = 0;         

// Inicializa o sistema operacional; deve ser chamada no inicio do main()
void ppos_init () 
{
    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0) ;

    TaskMain.id = ID++;     
    // TaskMain.next = NULL;   
    // TaskMain.prev = NULL;
    getcontext(&(TaskMain.context));

    CurrentTask = &TaskMain;

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
    
    // task->next = NULL;
    // task->prev = NULL;
    task->id = ID++;
    
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

    // printf("creating task no %d\n", task->id);
    makecontext(&(task->context), (void *)start_func, 1, arg);

    #ifdef DEBUG
    printf("PPOS: task %d created by task %d (body function %p)\n", task->id, CurrentTask->id, start_func);
    #endif

    return task->id;
}

// Termina a tarefa corrente, indicando um valor de status encerramento
void task_exit (int exit_code) 
{
    #ifdef DEBUG
    printf("PPOS: task %d exited\n", CurrentTask->id);
    #endif

    task_switch(&TaskMain);

    // exit(exit_code);
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
    ucontext_t *current_context;
    current_context = &(CurrentTask->context);
    
    #ifdef DEBUG
    printf("PPOS: switch task %d -> task %d\n", CurrentTask->id, task->id);
    #endif

    CurrentTask = task;
    swapcontext(current_context, &(task->context));

    return 0;
}   

// retorna o identificador da tarefa corrente (main deve ser 0)
int task_id () 
{
    return CurrentTask->id;
}
