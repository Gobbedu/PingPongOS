#include "queue.h"
#include <stdio.h>


void queue_print (char *name, queue_t *queue, void print_elem (void*) )
{
    printf("%s: [", name);

    // if queue not empty
    if (queue) 
    {
        queue_t *aux = queue->next;     // print first element
        print_elem((void*) queue);
        
        while(aux != queue)             // iterate and print 
        {
            print_elem((void*) aux);    // cast to (void*) queue_t
            aux = aux->next;
        }
    }

    printf("]\n");
}


int queue_append (queue_t **queue, queue_t *elem)
{
    // VALIDATE //
    if((elem->next != NULL) || (elem->prev != NULL))
    {   // element must be disconnected
        fprintf(stderr, "### ERROR: tried to append element connected to another list\n");
        return -1;
    }

    if(!queue)
    {   // list must exist
        fprintf(stderr, "### ERROR: tried to append element to nonexistent list\n");
        return -2;
    }

    if(!elem)
    {   // element must exist
        fprintf(stderr, "### ERROR: tried to append nonexistent element to list\n");
        return -3;
    }

    // APPEND //
    if(!(*queue))
    {   // list is empty, create first element
        elem->next = elem;
        elem->prev = elem;
        *queue = elem;
        return 0;
    } 
    else 
    {   // list not empty, append element to ent
        elem->prev = (*queue)->prev;    // elem prev points to last
        elem->next = (*queue);          // elem next points to first

        (*queue)->prev->next = elem;    // set list's last
        (*queue)->prev= elem;           // set list's first
        return 0;
    }   

    fprintf(stderr, "something went wrong with append\n");
    return -1;
}


int queue_remove (queue_t **queue, queue_t *elem)
{
    // VALIDATE //
    if((elem->next == NULL) || (elem->prev == NULL))
    {   // element must be connected
        fprintf(stderr, "### ERROR: tried to remove element not connected to list\n");
        return -4;
    }

    if(!queue)
    {   // list must exist
        fprintf(stderr, "### ERROR: tried to remove element from nonexistent list\n");
        return -5;
    }

    if(!elem)
    {   // element must exist
        fprintf(stderr, "### ERROR: tried to remove nonexistent element from list\n");
        return -6;
    }

    if(!(*queue))
    {   // list must have at least one element
        fprintf(stderr, "### ERROR: tried to remove element from a empty list\n");
        return -7;
    }

    queue_t *aux = (*queue);
    queue_t *found = NULL;
    do
    {   // iterate over list to find element 
        if(elem == aux)
        {
            found = aux;
            break;
        }
        aux = aux->next;
    } while(aux != (*queue));

    if(!found)
    {   // element must belong to list
        fprintf(stderr, "### ERROR: tried to remove element from a different list\n");
        return -8;
    }

    // REMOVE //
    if((*queue)->next == (*queue) && (*queue)->prev == (*queue))
    {   // only one element on the list
        elem->next = NULL;
        elem->prev = NULL;
        (*queue) = NULL;
        return 0;
    }
    else
    {   // list has more than 1 element
        if(elem == (*queue))  // move head of list
            (*queue) = (*queue)->next;
        elem->prev->next = elem->next;  
        elem->next->prev = elem->prev;
        elem->next = NULL;
        elem->prev = NULL;
        return 0;
    }

    fprintf(stderr, "something went wrong with remove\n");
    return -1;
}


int queue_size (queue_t *queue)
{
    // if empty list
    if(!queue)
        return 0;

    // has at least one element
    int size = 1;

    // save pointer to next as reference to first
    queue_t *aux = queue->next;

    // iterates over everyone
    while(aux != queue)
    {
        size++;
        aux = aux->next;
    }

    return size;
}



