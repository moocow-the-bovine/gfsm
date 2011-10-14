#include <glib.h>

#include "pqueue.h"

static const gsize INITIAL_SIZE = 100;

struct s_PriorityQueue {
  gpointer *heap;
  GHashTable *map;
  gsize size;
  gsize heapsize;
  GCompareDataFunc cmp;
  gpointer cmp_data;
};

PriorityQueue*
pqueue_new (GCompareDataFunc cmp, gpointer cmp_data)
{
  PriorityQueue *q = g_slice_new(PriorityQueue);
  q->heap = g_new(gpointer, INITIAL_SIZE);
  q->map = g_hash_table_new(NULL, NULL);
  q->size = 0;
  q->heapsize = INITIAL_SIZE;
  q->cmp = cmp;
  q->cmp_data = cmp_data;
  return q;
}

void
pqueue_clear (PriorityQueue *q)
{
  q->size = 0;
  g_hash_table_remove_all(q->map);
}

void
pqueue_free (PriorityQueue *q)
{
  g_hash_table_destroy(q->map);
  g_free(q->heap);
  g_slice_free(PriorityQueue, q);
}

gboolean
pqueue_isempty (PriorityQueue *q)
{
  return q->size == 0;
}

static void
refresh_map (PriorityQueue *q)
{
  guint i;
  g_hash_table_remove_all(q->map);
  for (i = 0; i < q->size; i++) {
    g_hash_table_insert(q->map, q->heap[i], q->heap + i);
  }
}

static void
swap_entries (PriorityQueue *q, guint i, guint j)
{
  gpointer pi = q->heap[i];
  gpointer pj = q->heap[j];
  q->heap[i] = pj;
  q->heap[j] = pi;
  g_hash_table_insert(q->map, pi, q->heap + j);
  g_hash_table_insert(q->map, pj, q->heap + i);
}

static gboolean
entry_smaller (PriorityQueue *q, guint i, guint j)
{
  return q->cmp(q->heap[i], q->heap[j], q->cmp_data) < 0;
}

static guint
heapify_up (PriorityQueue *q, guint index)
{
  while ( (index > 0)
          && entry_smaller(q, index, (index - 1) / 2)
  ) {
    swap_entries(q, index, (index - 1) / 2);
    index = (index - 1) / 2;
  }
  return index;
}

static guint
heapify_down (PriorityQueue *q, guint index)
{
  guint child;
  while ( (child = 2 * index + 1) < q->size ) {
    if ( (child + 1 < q->size)
         && entry_smaller(q, child + 1, child)
    ) {
      child = child + 1;
    }
    if (entry_smaller(q, child, index)) {
      swap_entries(q, index, child);
      index = child;
    } else {
      break;
    }
  }
  return index;
}

gboolean
pqueue_push (PriorityQueue *q, gpointer data)
{
  if (q->size >= q->heapsize) {
    q->heapsize *= 2;
    gpointer *oldheap = q->heap;
    q->heap = g_renew(gpointer, q->heap, q->heapsize);
    if (q->heap != oldheap) {
      refresh_map(q);
    }
  }

  gpointer *oldentry = g_hash_table_lookup(q->map, data);
  if (oldentry != NULL) {
    gint oldindex = oldentry - q->heap;
    heapify_up(q, heapify_down(q, oldindex));
    return FALSE;
  }

  guint index = q->size;
  q->heap[index] = data;
  g_hash_table_insert(q->map, data, q->heap + index);
  q->size += 1;
  heapify_up(q, index);

  return TRUE;
}

gpointer
pqueue_peek (PriorityQueue *q)
{
  if (q->size > 0) {
    return q->heap[0];
  }
  return NULL;
}

gpointer
pqueue_find (PriorityQueue *q, gpointer data)
{
  gpointer *entry = g_hash_table_lookup(q->map, data);
  return (entry == NULL) ? NULL : *entry;
}

gpointer
pqueue_pop (PriorityQueue *q)
{
  if (q->size > 0) {
    gpointer data = q->heap[0];
    swap_entries(q, 0, q->size - 1);
    g_hash_table_remove(q->map, data);
    q->size -= 1;
    heapify_down(q, 0);
    if ( (q->heapsize > INITIAL_SIZE)
         && (q->size * 4 < q->heapsize)
    ) {
      q->heapsize /= 2;
      gpointer *oldheap = q->heap;
      q->heap = g_renew(gpointer, q->heap, q->heapsize);
      if (q->heap != oldheap) {
        refresh_map(q);
      }
    }
    return data;
  }
  return NULL;
}
