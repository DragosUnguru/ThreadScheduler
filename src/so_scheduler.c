#include <math.h>
#include "so_scheduler.h"
#include "utils.h"

int so_init(unsigned int time_quantum, unsigned int io)
{
	int i;

	if (io > SO_MAX_NUM_EVENTS || time_quantum == 0 || scheduler != NULL)
		return -1;

	/* Alloc and initialize scheduler */
	scheduler = (struct scheduler_t *)
			malloc(sizeof(struct scheduler_t));
	scheduler->io_locks = (pthread_mutex_t *)
			malloc(io * sizeof(pthread_mutex_t));
	scheduler->io_conds = (pthread_cond_t *)
			malloc(io * sizeof(pthread_cond_t));
	scheduler->io_is_set = (unsigned char *)
			calloc(io, sizeof(unsigned char));
	scheduler->unique_tids = (tid_t *)
			calloc(MAX_THREADS, sizeof(tid_t));

	if (scheduler == NULL ||
		scheduler->io_conds == NULL ||
		scheduler->io_locks == NULL ||
		scheduler->io_is_set == NULL ||
		scheduler->unique_tids == NULL)
		return -1;

	priq_init(&(scheduler->priq));
	scheduler->supported_io = io;
	scheduler->quantum = time_quantum;
	scheduler->running_thread = NULL;
	scheduler->no_threads = 0;

	for (i = 0; i < io; ++i) {
		pthread_mutex_init(&scheduler->io_locks[i], NULL);
		pthread_cond_init(&scheduler->io_conds[i], NULL);
	}

	return OK;
}

void so_exec(void)
{
	static float sexy_prime = 38431.0f;

	sqrt(sqrt(sqrt(sexy_prime)));
	try_preempt();
}

static void *fork_util(void *args)
{
	struct thread_t *self;

	self = (struct thread_t *) args;

	/* Wait to be planned on processor */
	sem_wait(&self->semaphore);

	/* Call handler and exit*/
	self->func(self->priority);

	self->state = TERMINATED;
	try_preempt();

	return NULL;
}

tid_t so_fork(so_handler *func, unsigned int priority)
{
	struct thread_t *new_thread;
	int rc, first;

	if (priority > SO_MAX_PRIO || func == NULL)
		return INVALID_TID;

	/* Initialize new thread struture */
	new_thread = (struct thread_t *) malloc(sizeof(struct thread_t));
	DIE(new_thread == NULL, "malloc");

	sem_init(&new_thread->semaphore, 0, 0);
	new_thread->priority = priority;
	new_thread->state = READY;
	new_thread->time_quantum = scheduler->quantum;
	new_thread->waiting_io = SO_MAX_NUM_EVENTS + 1;
	new_thread->func = func;

	/* Launch thread */
	rc = pthread_create(&new_thread->thread_id,
					NULL,
					fork_util,
					(void *) new_thread);
	DIE(rc != 0, "pthread_create");

	/* Master thread needs all unique thread IDs to join */
	scheduler->unique_tids[scheduler->no_threads++] = new_thread->thread_id;

	/* Queue and schedule thread */
	first = queue_thread(new_thread);

	/* If this is not the initiating thread
	 * (the first fork of the system isn't taken
	 * into account as a system's thread)
	 */
	if (!first)
		try_preempt();

	return new_thread->thread_id;
}

int so_wait(unsigned int io)
{
	if (io >= scheduler->supported_io)
		return FAILURE;

	/* Switch context with the next thread
	 * in queue and wait for I/O operation
	 */
	force_preempt(io);

	return OK;
}

int so_signal(unsigned int io)
{
	int count;

	/* Invalid I/O operation */
	if (io >= scheduler->supported_io)
		return -1;

	/* Count the threads and set their states to READY */
	count = ready_threads(io);

	/* Signal waiting threads */
	pthread_mutex_lock(&scheduler->io_locks[io]);
	scheduler->io_is_set[io] = READY;
	pthread_cond_broadcast(&scheduler->io_conds[io]);
	pthread_mutex_unlock(&scheduler->io_locks[io]);

	try_preempt();

	return count;
}

void so_end(void)
{
	int i;

	if (scheduler == NULL)
		return;

	/* Wait for all procs to finish */
	wait_for_threads();

	/* Uninitialize synchronization tools */
	for (i = 0; i < scheduler->supported_io; ++i) {
		pthread_mutex_destroy(&scheduler->io_locks[i]);
		pthread_cond_destroy(&scheduler->io_conds[i]);
	}

	scheduler->running_thread = NULL;
	scheduler->no_threads = 0;

	/* Free scheduler's memory */
	priq_destroy(scheduler->priq);
	free(scheduler->io_is_set);
	free(scheduler->io_locks);
	free(scheduler->io_conds);
	free(scheduler->unique_tids);
	free(scheduler);

	scheduler = NULL;
}
