#ifndef _UTILS_H
#define _UTILS_H

#include <pthread.h>
#include <semaphore.h>
#include "so_scheduler.h"
#include "priq.h"

/* useful macro for handling error codes */
#define DIE(assertion, call_description)	\
	do {								    \
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}                                   \
	} while (0)

#define DEC(x) ((x == 0) ? 0 : x - 1)

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
    sem_t semaphore;
    enum thread_state state;
    so_handler *func;
    unsigned int waiting_io;
    unsigned int time_quantum;
    unsigned int priority;
    tid_t thread_id;
};

struct scheduler_t {
    struct qhead_t *priq;
    struct thread_t *running_thread;
    unsigned int quantum;
    unsigned int supported_io;
    unsigned char *io_is_set;
    pthread_mutex_t *io_locks;
    pthread_cond_t *io_conds;
};

extern struct scheduler_t *scheduler;

/* Returns all the information of the current appellant
 * thread fetched from the priority queue.
 */
struct thread_t *get_this_thread();

/* The scheduler receives a new thread to schedule
 *  +thread = thread to be inserted into the queue
 * returns: 1 if this is the first scheduled thread, 0 otherwise
 */
int queue_thread(struct thread_t *thread);

/* Checks if a thread with higher priority
 * can preempt the current running thread or
 * if the current running thread's time quantum
 * expired.
 * returns 1 if preempted, 0 otherwise
 */
void try_preempt();

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
void wait_for_threads();

#endif /* _UTILS_H */
