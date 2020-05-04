#ifndef _UTILS_H
#define _UTILS_H

#include <pthread.h>
#include <semaphore.h>
#include "so_scheduler.h"
#include "priq.h"

/* Useful macro for handling error codes */
#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while (0)

#define DEC(x) ((x == 0) ? 0 : x - 1)

#define MAX_THREADS    1024
#define FAILURE        1
#define OK             0

enum thread_state {
	TERMINATED,
	RUNNING,
	NEW,
	WAITING,
	READY
};

struct thread_t {
	/* Thread's specific semaphore for context switching */
	sem_t semaphore;
	/* Indicates thread's current state */
	enum thread_state state;
	/* Handled to be called */
	so_handler *func;
	/* I/O operation expected. MAX_IO + 1 if not */
	unsigned int waiting_io;
	/* Current remaining time quantum on the processor */
	unsigned int time_quantum;
	/* Thread's priority */
	unsigned int priority;
	/* Thread's unique ID */
	tid_t thread_id;
};

struct scheduler_t {
	/* Priority queue of threads */
	struct qhead_t *priq;
	/* Current thread running */
	struct thread_t *running_thread;
	/* Default maximum time quantum */
	unsigned int quantum;
	/* No. of supported I/O operations */
	unsigned int supported_io;
	/* Used for conditional waiting */
	unsigned char *io_is_set;
	/* Unique locks for every I/O */
	pthread_mutex_t *io_locks;
	/* Synchronization tool to wait for I/O */
	pthread_cond_t *io_conds;
	/* Set of unique thread IDs */
	tid_t *unique_tids;
	/* Current no. of threads */
	unsigned int no_threads;
};

extern struct scheduler_t *scheduler;

/* The scheduler receives a new thread to schedule
 *  +thread = thread to be inserted into the queue
 * returns: 1 if this is the first scheduled thread, 0 otherwise
 */
int queue_thread(struct thread_t *thread);

/* Manages context switching. Should be
 * called at the end of every operation.
 */
void try_preempt(void);

/* Preempts the current running thread
 * regardless of the priority. This
 * function is used when the current
 * running thread is waiting for an
 * I/O operation.
 *  + io = sets the thread's waiting I/O operation
 */
void force_preempt(unsigned int io);

/* Sets the state as READY for the threads
 * waiting for a specific I/O operation
 *  +io = the specific I/O operation to count
 * returns the total number of threads waiting for io
 */
int ready_threads(unsigned int io);

/* Waits for all the threads to terminate
 * their execution
 */
void wait_for_threads(void);

#endif /* _UTILS_H */
