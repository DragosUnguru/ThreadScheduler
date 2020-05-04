#include "utils.h"
#include "unistd.h"

struct scheduler_t *scheduler;

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

void try_preempt(void)
{
	struct thread_t *next_thread;
	struct thread_t *self;

	self = scheduler->running_thread;
	next_thread = priq_toll(scheduler->priq);

	self->time_quantum =
		DEC(self->time_quantum);

	/* If the time quantum expired and theere is no other thread
	 * to replace the current running one, rerun this thread
	 */
	if (!self->time_quantum && (next_thread == NULL ||
		next_thread->priority < self->priority) &&
		self->state != TERMINATED) {

		self->time_quantum = scheduler->quantum;
		return;
	}

	/* If highest prioritized READY thread has
	 * a bigger priority than current running
	 * thread or if the current running thread's
	 * time quantum expired and there is no other READY
	 * thread with a higher or equal priority, preempt
	 */
	if (next_thread != NULL && ((self->state == TERMINATED) ||
		(next_thread->priority > self->priority) ||
		(!self->time_quantum &&
		next_thread->priority >= self->priority))) {

		self->time_quantum = scheduler->quantum;
		next_thread->state = RUNNING;
		scheduler->running_thread = next_thread;

		/* Switch context */
		if (self->state != TERMINATED) {
			self->state = READY;
			priq_reschedule(scheduler->priq, self);

			sem_post(&next_thread->semaphore);
			sem_wait(&self->semaphore);
		} else {
			sem_post(&next_thread->semaphore);
		}
	}
}

void force_preempt(unsigned int io)
{
	struct thread_t *next_thread;
	struct thread_t *self;
	int rc;

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

		rc = pthread_mutex_lock(&scheduler->io_locks[io]);
		DIE(rc != 0, "pthread_mutex_lock");

		while (!scheduler->io_is_set[io]) {
			rc = pthread_cond_wait(
				&scheduler->io_conds[io],
				&scheduler->io_locks[io]);
			DIE(rc != 0, "pthread_cond_wait");
		}

		rc = pthread_mutex_unlock(&scheduler->io_locks[io]);
		DIE(rc != 0, "pthread_mutex_unlock");

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
			node->data->waiting_io = SO_MAX_NUM_EVENTS + 1;
		}
		node = node->next;
	}

	return count;
}

void wait_for_threads(void)
{
	int rc, i;

	for (i = 0; i < scheduler->no_threads; ++i) {
		rc = pthread_join(scheduler->unique_tids[i], NULL);
		DIE(rc != 0, "pthread_join");
	}
}
