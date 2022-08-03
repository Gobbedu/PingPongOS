// Eduardo Gobbo Willi Vasconcellos Gonçalves GRR20203892

#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>

#include "queue.h"
#include "ppos.h"
#include "ppos_data.h"

// #define DEBUG            // Print operations
// #define CONTAB           // Print tasks time info
#define STACKSIZE 64*1024	/* tamanho de pilha das threads */

/* core local function headers */
task_t *(*scheduler_body)(), *scheduler_fcfs(), *scheduler_prio();
void print_task(void *ptr), tick_init(), tick_manager();
void dispatcher_body();


/* core global variables */
enum TaskStates {NEW = 1, READY, RUNNING, SUSPENDED, TERMINATED };
task_t TaskMain, TaskDispatcher, *CurrentTask, *QueueReady;
unsigned short QUANTUM;             // counts between 0 and 20
unsigned int GLOBAL_TICK;           // ticks since tick_init
struct sigaction tick_action;
struct itimerval timer;
int TID, UserTasks;                 // TID: Task ID, incrementa infinitamente; UserTasks: numero de tasks com 

// Inicializa o sistema operacional; deve ser chamada no inicio do main()
void ppos_init () 
{
    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0) ;

    /* initialize core global values */
    TID = 0;
    UserTasks = 0;
    scheduler_body = &scheduler_prio;

    /* task main's descriptor */
    TaskMain.id = TID++;
    TaskMain.next = NULL;
    TaskMain.prev = NULL;
    queue_append((queue_t **) &QueueReady, (queue_t *) &TaskMain);
    CurrentTask = &TaskMain;
    UserTasks++;

    /* create dispatcher task */
    task_create(&TaskDispatcher, dispatcher_body, NULL);
    queue_remove((queue_t **) &QueueReady, (queue_t *) &TaskDispatcher);
    UserTasks--;

    /* initialize ppos clock */
    tick_init();

    /* initialize dispatcher */
    task_yield();

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
    task->id = TID++;
    task->status = NEW;
    task->static_prio = 0;
    task->dynamic_prio = 0;
    task->proc_time = 0;
    task->live_time = systime();
    task->num_quant = 0;
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
    
    // finished creating task
    task->status = READY;

    #ifdef DEBUG
    printf("PPOS: task %d created by task %d (body function %p)\n", task->id, CurrentTask->id, start_func);
    #endif

    return task->id;
}

// Termina a tarefa corrente, indicando um valor de status encerramento
void task_exit (int exit_code) 
{
    // total time since task creation
    CurrentTask->live_time = systime() - CurrentTask->live_time;

    #ifdef DEBUG
    printf("PPOS: task %d exited\n", CurrentTask->id);
    #endif

    #ifdef CONTAB
    printf("PPOS: task %d exited: execution time %d ms, processor time: %d ms, %d activations\n",
            CurrentTask->id, CurrentTask->live_time, CurrentTask->proc_time, CurrentTask->num_quant);
    #endif  

    CurrentTask->status = TERMINATED;

    // return control to dispatcher (except if dispatcher exit)
    // (CurrentTask != &TaskDispatcher) ? task_switch(&TaskDispatcher) : task_switch(&TaskMain);

    if(CurrentTask != &TaskDispatcher)
        task_switch(&TaskDispatcher);
    else
    {
        free(TaskDispatcher.context.uc_stack.ss_sp);    // free stack
        task_switch(&TaskMain);                         // exit from core
    }

    
}

// alterna a execução para a tarefa indicada
// return < 0 if error, else return 0
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

// retorna o identificador da tarefa (>= 0) corrente (main deve ser 0)
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


// define a prioridade estática de uma tarefa (ou a tarefa atual)
// O escalonador deve usar prioridades no estilo UNIX (valores entre -20 e +20, com escala negativa).
void task_setprio (task_t *task, int prio)
{
    if( prio > 20 || prio < -20)
    {
        fprintf(stderr, "### ERROR: tried to set static priority value out of range [-20 .. +19]\n");
        return;
    }

    if(!task)
    {
        CurrentTask->static_prio = prio;
        CurrentTask->dynamic_prio = prio;
        #ifdef DEBUG
        printf("PPOS: set task %d with priority %d\n", CurrentTask->id, CurrentTask->static_prio);
        #endif
    }
    else
    {
        task->static_prio = prio;
        task->dynamic_prio = prio;
        #ifdef DEBUG
        printf("PPOS: set task %d with priority %d\n", task->id, task->static_prio);
        #endif
    }

}

// retorna a prioridade estática de uma tarefa (ou a tarefa atual)
int task_getprio (task_t *task)
{
    if(!task) // if task == NULL
        return CurrentTask->static_prio;

    return task->static_prio;
}

// 
void dispatcher_body()
{
    #ifdef DEBUG
    printf("PPOS: task dispatcher launched\n");
    queue_print("Ready ", (queue_t *) QueueReady, print_task);
    #endif

    int proc_time;
    task_t *next_task;
    TaskDispatcher.num_quant++;     // taskDispatcher contab info

    while(UserTasks)
    {   // enquanto houverem tarefas de usuário
        // incrementa numero de ativacoes do dispatcher
        TaskDispatcher.num_quant++; 

        next_task = scheduler_body();   // escolhe a próxima tarefa a executar
        next_task->num_quant++;         // number of quantums given for task

        if(next_task)
        {   // next task must exist (!NULL)
            QUANTUM = 20;                       // reseta quantum da proxima task
            proc_time = systime();              // processor time for task
            task_switch(next_task);
            next_task->proc_time += systime() - proc_time;

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
                    queue_remove((queue_t **) &QueueReady, (queue_t *) next_task);  // remove da lista de prontos
                    free((next_task->context).uc_stack.ss_sp);    // free stack
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
task_t *scheduler_fcfs()
{
    task_t *next = QueueReady;
    if(QueueReady)  // if list not empty (NULL)
        QueueReady = QueueReady->next;

    return next;
}

task_t *scheduler_prio()
{
    if(!QueueReady) // if list is empty
        return NULL;

    int alpha = -1;
    int max_prio = 20;
    task_t *hike, *next;
    hike = QueueReady;

    // set next task;
    do
    {
        if(hike->dynamic_prio < max_prio)
        {
            max_prio = hike->dynamic_prio;
            next = hike;
        }
        hike = hike->next;
    }   while(hike != QueueReady);

    // grow tasks beard
    do
    {
        if(hike != next)
            hike->dynamic_prio += alpha;

        hike = hike->next;
    }   while(hike != QueueReady);

    next->dynamic_prio = next->static_prio;

    return next;
}

void tick_init()
{
    // initialize global tick counter
    GLOBAL_TICK = 0;

    // registra a ação para o sinal de timer SIGALRM
    tick_action.sa_handler = tick_manager ;
    sigemptyset (&tick_action.sa_mask) ;
    tick_action.sa_flags = 0 ;
    if (sigaction (SIGALRM, &tick_action, 0) < 0)
    {
        perror ("Erro em sigaction: ") ;
        exit (1) ;
    }

    // ajusta valores do temporizador
    timer.it_value.tv_usec = 1000 ;      // primeiro disparo, em micro-segundos
    timer.it_value.tv_sec  = 0 ;      // primeiro disparo, em segundos
    timer.it_interval.tv_usec = 1000 ;   // disparos subsequentes, em micro-segundos
    timer.it_interval.tv_sec  = 0 ;   // disparos subsequentes, em segundos

    // arma o temporizador ITIMER_REAL (vide man setitimer)
    if (setitimer (ITIMER_REAL, &timer, 0) < 0)
    {
        perror ("Erro em setitimer: ") ;
        exit (1) ;
    }
}

void tick_manager()
{
    GLOBAL_TICK++; // increment tick every 1 ms

    if(CurrentTask->id != 1) 
    {   // user tasks
        --QUANTUM;
        if(QUANTUM == 0) task_yield();
    }
    
    // printf ("Recebi o sinal %d, quantum = %d\n", -1, quantum) ;
}

unsigned int systime()
{
    return GLOBAL_TICK;
}
// ============ AUXILIAR FUNCITONS ============ // 

// function to print tasks with queue print
void print_task(void *ptr)
{
    task_t *task = ptr;

    if(!task)   // task must exist to be printed
        return;

    printf("%d", task->id);
}
