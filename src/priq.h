#ifndef PRIQ_H
#define PRIQ_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

typedef struct thread_t info_t;

struct node_t {
	struct node_t *next;
	info_t *data;
};

struct qhead_t {
	struct node_t *front;
};

/* Initializez the queue structure
 *  +head = pointer to the newly initialized queue
 */
void priq_init(struct qhead_t **head);

/* Checks if the queue is empty
 *  +head = queue to be checked
 * returns 1 if empty, 0 otherwise
 */
int priq_is_empty(struct qhead_t *head);

/* Inserts a new thread into the queue
 *  +head = queue in which to be inserted
 *  +info = thread info to be inserted as a new node
 */
void priq_insert(struct qhead_t *head, info_t *info);

/* Reschedules thread. Moves it as the last prioritized
 * thread of it's priority
 *  +head = queue to reschedule
 *  +info = thread to reschedule
 */
void priq_reschedule(struct qhead_t *head, info_t *info);

/* Retrieves the highest prioritized READY thread
 *  +head = queue to be updated
 * returns thread data if any, NULL otherwise
 */
info_t *priq_toll(struct qhead_t *head);

/* Frees all the memory hoarded by the queue
 *  +head = queue head to be freed
 */
void priq_destroy(struct qhead_t *head);

#endif /* PRIQ_H */
