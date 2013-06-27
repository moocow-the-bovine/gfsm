/*=============================================================================*\
 * File: gfsmStateSort.c
 * Author: Bryan Jurish <moocow.bovine@gmail.com>
 * Description: finite state machine library
 *
 * Copyright (c) 2012 Bryan Jurish.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *=============================================================================*/

#include <gfsmStateSort.h>
#include <gfsmArcIter.h>
#include <gfsmUtils.h>
#include <gfsmBitVector.h>
#include <stdlib.h>

//======================================================================
// Low-level fsmStateIdMap utilities

gfsmStateIdMap *gfsm_statemap_init(gfsmStateIdMap *smap, gfsmStateId sz)
{
  gfsmStateId i;
  if (smap==NULL) {
    smap = g_array_sized_new(FALSE,FALSE,sizeof(gfsmStateId),sz);
  }
  g_array_set_size(smap,sz);
  for (i=0; i < sz; ++i) {
    g_array_index(smap,gfsmStateId,i) = gfsmNoState;
  }
  return smap;
}

//======================================================================
// Methods: gfsmStateIdMap acquisition

//--------------------------------------------------------------
gfsmStateIdMap* gfsm_statemap_aff(gfsmAutomaton *fsm, gfsmStateIdMap *old2new)
{
  gfsmStateId  oldid, newid;
  old2new = gfsm_statemap_init(old2new,gfsm_automaton_n_states(fsm));

  g_array_index(old2new,gfsmStateId,fsm->root_id) = 0;
  for (oldid=0, newid=0; oldid < fsm->states->len; oldid++) {
    if (oldid==fsm->root_id) continue;
    if (gfsm_automaton_has_state(fsm,oldid)) {
      g_array_index(old2new,gfsmStateId,oldid) = ++newid;
    } else {
      g_array_index(old2new,gfsmStateId,oldid) = gfsmNoState;
    }
  }

  return old2new;
}

//--------------------------------------------------------------
gfsmStateIdMap* gfsm_statemap_dfs(gfsmAutomaton *fsm, gfsmStateIdMap *old2new)
{
  GSList *stack=NULL; //-- GSList of gfsmStateId
  gfsmStateId newid = 0;
  old2new = gfsm_statemap_init(old2new,gfsm_automaton_n_states(fsm));

  //-- ye olde dfs loope
  stack = g_slist_prepend(NULL, GUINT_TO_POINTER((guint)fsm->root_id));
  while (stack != NULL) {
    gfsmStateId oldid = (gfsmStateId)GPOINTER_TO_UINT(stack->data);
    GSList    *next = stack->next;
    gfsmArcIter ai;

    //-- pop stack
    g_slist_free_1(stack);
    stack = next;

    //-- sanity checks
    if (!gfsm_automaton_has_state(fsm,oldid)) continue; //-- invalid state
    if (g_array_index(old2new, gfsmStateId, oldid) != gfsmNoState) continue; //-- already enumerated

    //-- enumerate state
    g_array_index(old2new, gfsmStateId, oldid) = newid++;

    //-- follow outgoing arcs
    for (gfsm_arciter_open(&ai,fsm,oldid); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc * a = gfsm_arciter_arc(&ai);
      stack = g_slist_prepend(stack,GUINT_TO_POINTER((guint)a->target));
    }
  }

  return old2new;
}

//--------------------------------------------------------------
gfsmStateIdMap* gfsm_statemap_bfs(gfsmAutomaton *fsm, gfsmStateIdMap *old2new)
{
  GQueue queue; //-- queue of gfsmStateId
  gfsmStateId newid = 0;
  old2new = gfsm_statemap_init(old2new,gfsm_automaton_n_states(fsm));

  //-- ye olde bfs loope
  g_queue_init(&queue);
  g_queue_push_tail(&queue, GUINT_TO_POINTER((guint)fsm->root_id));
  while (queue.length > 0) {
    gfsmStateId oldid = (gfsmStateId)g_queue_pop_head(&queue);
    gfsmArcIter ai;

    //-- sanity checks
    if (!gfsm_automaton_has_state(fsm,oldid)) continue; //-- invalid state
    if (g_array_index(old2new, gfsmStateId, oldid) != gfsmNoState) continue; //-- already enumerated

    //-- enumerate state
    g_array_index(old2new, gfsmStateId, oldid) = newid++;

    //-- follow outgoing arcs
    for (gfsm_arciter_open(&ai,fsm,oldid); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc * a = gfsm_arciter_arc(&ai);
      g_queue_push_tail(&queue, GUINT_TO_POINTER((guint)a->target));
    }
  }

  return old2new;
}

//--------------------------------------------------------------
gfsmStateIdMap* gfsm_statemap_depths(gfsmAutomaton *fsm, gfsmStateIdMap *depths)
{
  gfsmStateId qid;
  gfsmStateId  *d0, d1;
  gfsmArcIter  ai;
  gfsmArc      *a;
  GQueue queue = G_QUEUE_INIT; //-- queue of gfsmStateId


  //-- initialize depth-map & bfs-queue
  depths = gfsm_statemap_init(depths,gfsm_automaton_n_states(fsm));
  g_queue_push_tail(&queue, GUINT_TO_POINTER((guint)fsm->root_id));
  g_array_index(depths, gfsmStateId, fsm->root_id) = 0;

  //-- ye olde loope
  while (queue.length > 0) {
    qid = GPOINTER_TO_UINT(g_queue_pop_head(&queue));
    d1  = g_array_index(depths, gfsmStateId, qid) + 1;

    //-- sanity check(s)
    if (!gfsm_automaton_has_state(fsm,qid)) continue; //-- invalid state: should never happen

    //-- follow outgoing arcs
    for (gfsm_arciter_open(&ai,fsm,qid); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      a = gfsm_arciter_arc(&ai);

      //-- enumerate new depth
      d0 = &g_array_index(depths, gfsmStateId, a->target);
      if (*d0 > d1) {
	*d0 = d1;
	g_queue_push_tail(&queue, GUINT_TO_POINTER((guint)a->target));
      }
    }
  }

  return depths;
}


//======================================================================
// Methods: gfsmStateIdMap application 

//--------------------------------------------------------------
void gfsm_statemap_apply(gfsmAutomaton *fsm, gfsmStateIdMap *old2new, gfsmStateId n_new_states)
{
  gfsmStateId    oldid, newid;
  gfsmState     *s_old, *s_new;
  gfsmWeightMap *new_finals = gfsm_weightmap_new(gfsm_uint_compare);
  GArray        *new_states = NULL;

  //-- get new number of states
  if (n_new_states==0 || n_new_states==gfsmNoState) {
    n_new_states = 0;
    for (oldid=0; oldid < fsm->states->len; oldid++) {
      if (!gfsm_automaton_has_state(fsm,oldid)) continue;
      newid = g_array_index(old2new,gfsmStateId,oldid);
      if (newid != gfsmNoState && newid >= n_new_states) { n_new_states=newid+1; }
    }
  }

  //-- allocate new state-vector (initialize to NULL)
  new_states = g_array_sized_new(FALSE,TRUE,sizeof(gfsmState),n_new_states);

  //-- renumber states
  for (oldid=0; oldid < fsm->states->len; oldid++) {
    gfsmArcIter ai; 
    newid = g_array_index(old2new,gfsmStateId,oldid);

    if (newid==gfsmNoState || !gfsm_automaton_has_state(fsm,oldid)) continue; //-- ignore bad states

    //-- copy state data
    s_old  = gfsm_automaton_find_state(fsm, oldid);
    s_new  = &(g_array_index(new_states,gfsmState,newid));
    *s_new = *s_old;

    //-- check for final state
    if (s_new->is_final) {
      gfsmWeight fw =0; //-- hack to convince gcc not to complain about unitialized fw
      gfsm_weightmap_lookup(fsm->finals, GUINT_TO_POINTER(oldid), &fw);
      gfsm_weightmap_insert(new_finals,  GUINT_TO_POINTER(newid),  fw);
    }

    //-- renumber sources & targets of outgoing arcs
    for (gfsm_arciter_open_ptr(&ai, fsm, s_new); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      a->source  = newid;
      a->target  = g_array_index(old2new,gfsmStateId,a->target);
    }
    gfsm_arciter_close(&ai);
  }

  //-- set new root-id
  fsm->root_id = g_array_index(old2new,gfsmStateId,fsm->root_id);

  //-- set new final weights
  gfsm_weightmap_free(fsm->finals);
  fsm->finals = new_finals;

  //-- set new state vector
  g_array_free(fsm->states,TRUE);
  fsm->states = new_states;
  fsm->states->len = n_new_states;
}


//======================================================================
// Methods: High-level state sorting

//--------------------------------------------------------------
void gfsm_statesort_aff(gfsmAutomaton *fsm, gfsmStateIdMap *old2new)
{
  gfsmStateIdMap *o2n = gfsm_statemap_aff(fsm,old2new);
  gfsm_statemap_apply(fsm,o2n,0);
  if (!old2new) g_array_free(o2n,TRUE);
}

//--------------------------------------------------------------
void gfsm_statesort_dfs(gfsmAutomaton *fsm, gfsmStateIdMap *old2new)
{
  gfsmStateIdMap *o2n = gfsm_statemap_dfs(fsm,old2new);
  gfsm_statemap_apply(fsm,o2n,0);
  if (!old2new) g_array_free(o2n,TRUE);
}

//--------------------------------------------------------------
void gfsm_statesort_bfs(gfsmAutomaton *fsm, gfsmStateIdMap *old2new)
{
  gfsmStateIdMap *o2n = gfsm_statemap_bfs(fsm,old2new);
  gfsm_statemap_apply(fsm,o2n,0);
  if (!old2new) g_array_free(o2n,TRUE);
}
