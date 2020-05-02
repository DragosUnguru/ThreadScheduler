#include "utils.h"

struct scheduler_t *scheduler;

struct thread_t *get_this_thread()
{
    pthread_t self;
    struct node_t *node;

    self = pthread_self();
    node = scheduler->priq->front;

    while (node != NULL && node->data->thread_id != self)
        node = node->next;
    
    return (node == NULL) ? NULL : node->data;
}

int queue_thread(struct thread_t *thread)
{
    priq_insert(scheduler->priq, thread);

    /* First thread scheduled */
    if (scheduler->running_thread == NULL) {
        scheduler->running_thread = thread;
        thread->state = RUNNING;
        sem_post(&thread->semaphore);

        return 1;
    }
    return 0;
}

void try_preempt()
{
    struct thread_t *next_thread;
    struct thread_t *self;

    next_thread = priq_toll(scheduler->priq);
    self = scheduler->running_thread;

    /* If highest prioritized READY thread has
     * a bigger priority than current running
     * thread or if the current running thread's
     * time quantum expired, preempt
     */
    if ((next_thread != NULL) &&
        (next_thread->priority > self->priority || self->time_quantum == 0 || self->state == TERMINATED)) {

        self->time_quantum = scheduler->quantum;

        next_thread->state = RUNNING;
        scheduler->running_thread = next_thread;

        /* Switch context */
        sem_post(&next_thread->semaphore);
        if (self->state != TERMINATED) {
            self->state = READY;
            sem_wait(&self->semaphore);
        }
        return;
    }

    self->time_quantum = DEC(self->time_quantum);
}

void force_preempt(unsigned int io)
{
    struct thread_t *next_thread;
    struct thread_t *self;

    next_thread = priq_toll(scheduler->priq);
    self = scheduler->running_thread;

    if (next_thread != NULL) {
        self->state = WAITING;
        self->time_quantum = scheduler->quantum;
        self->waiting_io = io;

        next_thread->state = RUNNING;
        scheduler->running_thread = next_thread;

        /* Switch context */
        sem_post(&next_thread->semaphore);

        pthread_mutex_lock(&scheduler->io_locks[io]);

        while (!scheduler->io_is_set[io])
            pthread_cond_wait(&scheduler->io_conds[io], &scheduler->io_locks[io]);

        pthread_mutex_unlock(&scheduler->io_locks[io]);

        self->state = READY;
        sem_wait(&self->semaphore);
    }
}

int ready_threads(unsigned int io)
{
    struct node_t *node;
    unsigned int count;

    node = scheduler->priq->front;
    count = 0;

    while (node != NULL) {
        if (node->data->waiting_io == io) {
            ++count;
            node->data->state = READY;
        }
        node = node->next;
    }

    return count;
}

void wait_for_threads()
{
    struct node_t *node;
    int rc;

    node = scheduler->priq->front;

    while (node != NULL) {
        rc = pthread_join(node->data->thread_id, NULL);
        DIE(rc != 0, "pthread_join");

        node = node->next;
    }
}
