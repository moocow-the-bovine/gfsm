/*=============================================================================*\
 * File: gfsmDeterminize.c
 * Author: Bryan Jurish <moocow.bovine@gmail.com>
 * Description: finite state machine library
 *
 * Copyright (c) 2004-2011 Bryan Jurish.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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

#include <glib.h>
#include <gfsmAlgebra.h>
#include <gfsmAssert.h>
#include <gfsmArcIter.h>
#include <gfsmUtils.h>
#include <gfsmCompound.h>
#include <gfsmEnum.h>
//#include <gfsmAlphabet.h>

/*======================================================================
 * UTILS
 */

//--------------------------------------------------------------
static
guint gfsm_wqarray_hash(gfsmStateWeightPairArray *wqa)
{
  guint i;
  guint hv = 0;
  for (i=0; i < wqa->len; i++) {
    gfsmStateWeightPair *wq = &g_array_index(wqa, gfsmStateWeightPair, i);
    hv = (hv*1231) + (wq->id*1237) + (wq->w*1249);
  }
  return hv;
}

//--------------------------------------------------------------
static
gboolean gfsm_wqarray_equal(gfsmStateWeightPairArray *wqa1, gfsmStateWeightPairArray *wqa2)
{
  guint i;
  gfsmStateWeightPair *wq1, *wq2;
  if (wqa1->len != wqa2->len) return FALSE;
  for (i=0; i < wqa1->len; i++) {
    wq1 = &g_array_index(wqa1, gfsmStateWeightPair, i);
    wq2 = &g_array_index(wqa2, gfsmStateWeightPair, i);
    if ((wqa1->id != wqa2->id) || (wqa1->w != wqa2->w)) return FALSE;
  }
  return TRUE;
}

//--------------------------------------------------------------
static
void gfsm_wqarray_free(gfsmStateWeightPairArray *wqa)
{
  g_array_free(wqa,TRUE);
}

/*======================================================================
 * Methods: algebra: determinize
 */

/*--------------------------------------------------------------
 * determinize()
 */
gfsmAutomaton *gfsm_automaton_determinize(gfsmAutomaton *nfa)
{
  if (!nfa->flags.is_deterministic) {
    gfsmAutomaton *dfa = gfsm_automaton_determinize_full(nfa,NULL);
    gfsm_automaton_swap(nfa,dfa);
    gfsm_automaton_free(dfa);
  }
  return nfa;
}

/*--------------------------------------------------------------
 * determinize_full()
 */
gfsmAutomaton *gfsm_automaton_determinize_full(gfsmAutomaton *nfa, gfsmAutomaton *dfa)
{
  //gfsmAlphabet   *ec2id; //-- (global) maps literal(equiv-class@nfa) <=> state-id@dfa
  gfsnEnum       *ec2id; //-- (global) maps literal(equiv-class@nfa) <=> state-id@dfa
  gfsmStateId    dfa_id; //-- (temp) id @ dfa
  gfsmStateWeightPairArray *nfa_ec;  //-- (temp) equiv-class@nfa: array
  gfsmStateWeightPair *wq; //-- (temp): state+weight pair in nfa_ec
  GSList *queue = NULL;    //-- processing queue: GSList of gfsmStateId (id@dfa)
  gfsmArcCompData acdata = {gfsmASMLower, nfa->sr, NULL, NULL};
 
  //-- sanity check(s)
  if (!nfa) return NULL;
  else if (nfa->flags.is_deterministic) {
    if (dfa) gfsm_automaton_copy(dfa,nfa);
    else     dfa = gfsm_automaton_clone(nfa);
    return dfa;
  }

  //-- initialization: dfa
  if (!dfa) {
    dfa = gfsm_automaton_shadow(nfa);
  } else {
    gfsm_automaton_clear(dfa);
    gfsm_automaton_copy_shallow(dfa,nfa);
  }
  //-- avoid "smart" arc-insertion
  dfa->flags.sort_mode = gfsmASMNone;

  //-- initialization: ec2id
  ec2id = gfsm_enum_new_full(NULL /*(gfsmDupFunc)gfsm_wqarray_clone*/ ,
			     (GHashFunc)gfsm_wqarray_hash,
			     (GEqualFunc)gfsm_wqarray_equal,
			     (GDestroyNotify)gfsm_wqarray_free);

  //-- initialization: root
  nfa_ec = g_array_sized_new(32);
  g_array_set_size(nfa_ec,1);
  wq = &g_array_index(nfa_ec, gfsmStateWeightPair, 1);
  wq->id = nfa->root_id;
  wq->w  = fsm->sr->one;
  dfa_id = gfsm_automaton_ensure_state(dfa, gfsm_enum_insert(ec2id, nfa_ec));
  gfsm_automaton_set_root(dfa, dfa_id);

  //-- guts: queue processing
  queue = g_slist_prepend(queue, nfa_ec);
  while (queue != NULL) {
    GSList *head = queue;
    GSList *arcl = NULL; //-- (temp) arc-list: collected arcs from nfa
    GSLIst *ali0,*ali1;  //-- (temp) arc-list: iterators
    gfsmState *qptr;
    guint eci;

    //-- pop the queue
    queue  = queue->next;
    nfa_ec = (gfsmStateWeightPairArray*)head->data;
    dfa_id = gfsm_enum_lookup(ec2id, nfa_ec);
    g_slist_free_1(head);

    //-- generate & sort list of all outgoing arcs
    for (eci=0; eci < nfa_ec->len; eci++) {
      wq   = &g_array_index(nfa_ec, gfsmStateWeightPair, eci);
      qptr = gfsm_automaton_find_state(nfa,wq->id);
      for (gfsm_arciter_open_ptr(&ai,nfa,qptr); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
	gfsmArc *a = gfsm_arciter_arc(&ai);
	arcl = g_slist_prepend(arcl, a);
      }
    }
    arcl = g_slist_sort(arcl, (GCompareFunc)gfsm_arc_compare_bymask, &acdata);

    //-- extract & process (lo,hi)-runs --> collect by state into (lo,hi,qto)-runs; implement Morhi (2009) lines 7-16
    for (ali0=arcl; ali0 != NULL; ali0=ali1) {
      gfsmArc *a0 = (gfsmArc*)ali0->data;
      gfsmLabelId lo=a0->lower, hi=a0->upper;
      for (ali1=ali0->next; ali1 != NULL; ali1=ali1->next) {
	gfsmArc *a1 = (gfsmArc*)ali1->data;
	if (a1->lower != lo || a1->upper != hi) break;
      }

      //-- we have a (lo,hi) run in (ali0..(ali1-1))
      
    }
  }

  //-- cleanup
  gfsm_enum_free(ec2id);

  //-- return
  return dfa;
}

