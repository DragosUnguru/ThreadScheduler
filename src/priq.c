#include "priq.h"

void priq_init(struct qhead_t **head)
{
    *head = (struct qhead_t *) malloc(sizeof(**head));
    DIE(*head == NULL, "malloc");

    (*head)->front = NULL;
}

int priq_is_empty(struct qhead_t *head)
{
    return (head->front == NULL);
}

void priq_insert(struct qhead_t *head, info_t *info)
{
    struct node_t *new_node;
    struct node_t *tmp;

    tmp = head->front;

    /* Create new node */
    new_node = (struct node_t *) malloc(sizeof(*new_node));
    DIE(new_node == NULL, "malloc");

    new_node->data = info;
    new_node->next = NULL;

    if (priq_is_empty(head)) {
        head->front = new_node;

        return;
    }

    /* Manage new queue head, if necessary */
    if (head->front->data->priority < new_node->data->priority) {
        new_node->next = head->front;
        head->front = new_node;

        return;
    }

    /* Stop before the first node with lesser priority */
    while (tmp->next != NULL && tmp->next->data->priority >= new_node->data->priority)
        tmp = tmp->next;

    /* Insert node */
    new_node->next = tmp->next;
    tmp->next = new_node;
}

info_t *priq_toll(struct qhead_t *head)
{
    struct node_t *node;
    
    node = head->front;
    while (node != NULL && node->data->state != READY)
        node = node->next;

    return (node == NULL) ? NULL : node->data;
}

void priq_destroy(struct qhead_t *head)
{
    struct node_t *front;
    struct node_t *prev;

    front = head->front;
    prev = front;
    
    while (front != NULL) {
        front = front->next;

        free(prev->data);
        free(prev);

        prev = front;
    }
    free(head);
}
