#include <math.h>
#include "so_scheduler.h"
#include "utils.h"

int so_init(unsigned int time_quantum, unsigned int io)
{
    int i;

    if (io > MAX_NUM_EVENTS || time_quantum == 0 || scheduler != NULL)
        return -1;

    /* Alloc and initialize */
    scheduler = (struct scheduler_t *) malloc(sizeof(struct scheduler_t));
    scheduler->locks = (pthread_mutex_t *) malloc (io * sizeof(pthread_mutex_t));
    scheduler->conds = (pthread_cond_t *) malloc(io * sizeof(pthread_cond_t));

    if (scheduler == NULL || scheduler->conds == NULL || scheduler->locks == NULL)
        return -1;
    
    priq_init(&(scheduler->priq));
    scheduler->io = io;
    scheduler->quantum = time_quantum;

    for (i = 0; i < io; ++i) {
        scheduler->locks[i] = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
        scheduler->conds[i] = (pthread_cond_t) PTHREAD_COND_INITIALIZER;
    }

    return OK;
}

void so_exec(void)
{
    static float sexy_prime = 38431.0f;
    sqrt(sqrt(sqrt(sexy_prime)));
}

tid_t so_fork(so_handler *func, unsigned int priority)
{
    /* TODO:
     * Semafor pe fiecare thread.
     * Se pune semaforul pe rosu doar atunci cand e preemptat sau cand i-a expirat cuanta. NU are rost sa faci semafor cu N = quantum
     * exec fork signal si wait = o perioada din cuanta de timp
     * STARILE!! cum pui pe WAITING. UTILS.C sa vezi cum faci sa preemptezi ca lumea!!
     */
    return scheduler->priq->front->data->thread;
}

int so_wait(unsigned int io)
{
    if (io > scheduler->io)
        return FAILURE;

    pthread_cond_wait(&scheduler->conds[io], &scheduler->locks[io]);

    return OK;
}

int so_signal(unsigned int io)
{
    int count;

    if (io > scheduler->io)
        return -1;

    count = ready_threads(io);
    pthread_cond_broadcast(&scheduler->conds[io]);

    return count;
}

void so_end(void)
{
    int i;

    if (scheduler == NULL)
        return;

    /* TODO: wait for all procs to finish */
    wait_for_threads();

    for (i = 0; i < scheduler->io; ++i) {
        pthread_mutex_destroy(&scheduler->locks[i]);
        pthread_cond_destroy(&scheduler->conds[i]);
    }

    priq_destroy(scheduler->priq);
    free(scheduler->locks);
    free(scheduler->conds);
    free(scheduler);

    scheduler = NULL;
}