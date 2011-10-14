#ifndef GFSM_PQUEUE_H
#define GFSM_PQUEUE_H

typedef struct s_gfsmPriorityQueue gfsmPriorityQueue;

gfsmPriorityQueue*
gfsm_pqueue_new (GCompareDataFunc cmp, gpointer cmp_data);

void
gfsm_pqueue_clear (gfsmPriorityQueue *q);

void
gfsm_pqueue_free (gfsmPriorityQueue *q);

gboolean
gfsm_pqueue_isempty (gfsmPriorityQueue *q);

gboolean
gfsm_pqueue_push (gfsmPriorityQueue *q, gpointer data);

gpointer
gfsm_pqueue_peek (gfsmPriorityQueue *q);

gpointer
gfsm_pqueue_find (gfsmPriorityQueue *q, gpointer data);

gpointer
gfsm_pqueue_pop (gfsmPriorityQueue *q);

#endif /* GFSM_PQUEUE_H */
