#include "utils.h"

struct scheduler_t *scheduler;

int update_running_thread()
{
    struct thread_t *next_thread;

    next_thread = priq_toll(scheduler->priq);

    /* If highest prioritized READY thread has
     * a bigger priority than current running
     * thread, preempt
     */
    if (next_thread != NULL && next_thread->priority > scheduler->running_thread->priority) {
        scheduler->running_thread->state = READY;
        scheduler->running_thread->time_quantum = scheduler->quantum;
        scheduler->running_thread = next_thread;

        return 1;
    }

    return 0;
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
        rc = pthread_join(node->data->thread, NULL);
        DIE(rc != 0, "pthread_join");
    }
}
