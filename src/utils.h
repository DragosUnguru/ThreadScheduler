#ifndef _UTILS_H
#define _UTILS_H

#include <pthread.h>
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

#define MAX_NUM_EVENTS 256
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
    enum thread_state state;
    unsigned char waiting_io;
    unsigned char time_quantum;
    unsigned char priority;
    pthread_t thread;
};

struct scheduler_t {
    struct qhead_t *priq;
    struct thread_t *running_thread;
    unsigned int quantum;
    unsigned int io;
    pthread_mutex_t *locks;
    pthread_cond_t *conds;
};

extern struct scheduler_t *scheduler;

/* Checks if a thread with higher priority
 * can preempt the current running thread
 * returns 1 if preempted, 0 otherwise
 */
int update_running_thread();

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
