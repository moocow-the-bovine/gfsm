#ifndef PQUEUE_H
#define PQUEUE_H

typedef struct s_PriorityQueue PriorityQueue;

PriorityQueue*
pqueue_new (GCompareDataFunc cmp, gpointer cmp_data);

void
pqueue_clear (PriorityQueue *q);

void
pqueue_free (PriorityQueue *q);

gboolean
pqueue_isempty (PriorityQueue *q);

gboolean
pqueue_push (PriorityQueue *q, gpointer data);

gpointer
pqueue_peek (PriorityQueue *q);

gpointer
pqueue_find (PriorityQueue *q, gpointer data);

gpointer
pqueue_pop (PriorityQueue *q);

#endif /* PQUEUE_H */
