/*=============================================================================*\
 * File: gfsmAutomaton.c
 * Author: Bryan Jurish <moocow.bovine@gmail.com>
 * Description: finite state machine library: automata
 *
 * Copyright (c) 2004-2011 Bryan Jurish.
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

#include <gfsmAutomaton.h>
#include <gfsmArcIter.h>
#include <gfsmStateSort.h>
#include <gfsmUtils.h>
#include <gfsmBitVector.h>
#include <stdlib.h>

#ifndef GFSM_INLINE_ENABLED
# include <gfsmAutomaton.hi>
#endif

/*======================================================================
 * Constants
 */
const gfsmStateId gfsmAutomatonDefaultSize  = 128;

const gfsmAutomatonFlags gfsmAutomatonDefaultFlags =
  {
    TRUE,        //-- is_transducer:1
    TRUE,        //-- is_weighted:1
    //0,           //-- sort_mode_old__:4
    FALSE,       //-- is_deterministic:1
    0,           //-- unused:29 (was: 25)
    gfsmASMNone  //-- sort_mode
  };

//const gfsmSRType gfsmAutomatonDefaultSRType = gfsmSRTReal;
const gfsmSRType gfsmAutomatonDefaultSRType = gfsmSRTTropical;

//======================================================================
// API: Constructors etc.

/*--------------------------------------------------------------
 * copy()
 */
gfsmAutomaton *gfsm_automaton_copy(gfsmAutomaton *dst, gfsmAutomaton *src)
{
  gfsmStateId qid;
  gfsm_automaton_clear(dst);
  gfsm_automaton_copy_shallow(dst,src);
  dst->root_id = src->root_id;                    //-- since copy_shallow() no longer does this!
  gfsm_automaton_reserve(dst,src->states->len);
  gfsm_weightmap_copy(dst->finals, src->finals);
  //
  for (qid=0; qid < src->states->len; qid++) {
    const gfsmState *src_s = gfsm_automaton_find_state_const(src,qid);
          gfsmState *dst_s = gfsm_automaton_find_state(dst,qid);
    gfsm_state_copy(dst_s, src_s);
  }
  return dst;
}

/*--------------------------------------------------------------
 * clear()
 */
void gfsm_automaton_clear(gfsmAutomaton *fsm)
{
  gfsmStateId i;
  if (!fsm) return;
  for (i=0; fsm->states && i < fsm->states->len; i++) {
    gfsmState *st = gfsm_automaton_find_state(fsm,i);
    if (!st || !st->is_valid) continue;
    gfsm_state_clear(st);
  }
  if (fsm->states) g_array_set_size(fsm->states,0);
  if (fsm->finals) gfsm_set_clear(fsm->finals);
  fsm->root_id = gfsmNoState;
  return;
}


//======================================================================
// API: Automaton Semiring

//======================================================================
// API: Automaton Properties

/*--------------------------------------------------------------
 * n_arcs_full()
 */
guint gfsm_automaton_n_arcs_full(gfsmAutomaton *fsm,
				 guint *n_lo_epsilon,
				 guint *n_hi_epsilon,
				 guint *n_both_epsilon)
{
  guint i, total=0;
  guint n_lo_eps=0, n_hi_eps=0, n_both_eps=0;
  gfsmStateId n_states = gfsm_automaton_n_states(fsm);

  for (i=0; i < n_states; i++) {
    gfsmArcIter ai;
    for (gfsm_arciter_open(&ai, fsm, i); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      ++total;
      if (a->lower==gfsmEpsilon) {
	++n_lo_eps;
	if (a->upper==gfsmEpsilon) {
	  ++n_hi_eps;
	  ++n_both_eps;
	}
      }
      else if (a->upper==gfsmEpsilon) {
	++n_hi_eps;
      }
    }
    gfsm_arciter_close(&ai);
  }
  if (n_lo_epsilon)   *n_lo_epsilon = n_lo_eps;
  if (n_hi_epsilon)   *n_hi_epsilon = n_hi_eps;
  if (n_both_epsilon) *n_both_epsilon = n_both_eps;
  return total;
}


/*--------------------------------------------------------------
 * is_cyclic_state()
 */
gboolean gfsm_automaton_is_cyclic_state(gfsmAutomaton *fsm,
					gfsmStateId id,
					gfsmBitVector *visited,
					gfsmBitVector *completed)
{
  gfsmState   *s;
  gfsmArcIter  ai;
  //
  if (gfsm_bitvector_get(visited,id)) {
    if (gfsm_bitvector_get(completed,id)) return FALSE;
    return TRUE;
  }
  //
  s = gfsm_automaton_find_state(fsm,id);
  if (!s || !s->is_valid) return FALSE;  //-- invalid states don't count as cyclic
  //
  //-- mark node as visited (& not completed)
  gfsm_bitvector_set(visited,id,1);
  gfsm_bitvector_set(completed,id,0);
  //
  //-- visit outgoing arcs
  for (gfsm_arciter_open_ptr(&ai,fsm,s); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
    if (gfsm_automaton_is_cyclic_state(fsm, gfsm_arciter_arc(&ai)->target, visited, completed)) {
      gfsm_arciter_close(&ai);
      return TRUE;
    }
  }
  gfsm_arciter_close(&ai);
  //
  //-- mark node as completed
  gfsm_bitvector_set(completed,id,1);
  //
  //-- finished traversal; this state isn't cyclic
  return FALSE;
}

/*--------------------------------------------------------------
 * is_cyclic()
 */
gboolean gfsm_automaton_is_cyclic(gfsmAutomaton *fsm)
{
  gfsmBitVector *visited;    //-- records which states we've visited
  gfsmBitVector *completed;  //-- records which states we've completed
  gboolean       rc;         //-- return value

  if (fsm->root_id==gfsmNoState || fsm->states->len==0) return FALSE; //-- sanity check(s)

  visited   = gfsm_bitvector_sized_new(fsm->states->len);
  completed = gfsm_bitvector_sized_new(fsm->states->len);
  rc        = gfsm_automaton_is_cyclic_state(fsm, fsm->root_id, visited, completed);

  //-- cleanup
  gfsm_bitvector_free(visited);
  gfsm_bitvector_free(completed);

  return rc;
}


/*--------------------------------------------------------------
 * get_alphabet()
 */
gfsmAlphabet *gfsm_automaton_get_alphabet(gfsmAutomaton *fsm, gfsmLabelSide which, gfsmAlphabet *alph)
{
  gfsmStateId id;
  //-- ensure alphabet
  if (!alph) alph = gfsm_range_alphabet_new();

  for (id=0; id < fsm->states->len; id++) {
    gfsmArcIter ai;
    for (gfsm_arciter_open(&ai,fsm,id); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);

      if (which != gfsmLSUpper)
	gfsm_alphabet_insert(alph, GUINT_TO_POINTER((guint)(a->lower)), a->lower);

      if (which != gfsmLSLower)
	gfsm_alphabet_insert(alph, GUINT_TO_POINTER((guint)(a->upper)), a->upper);
    }
    gfsm_arciter_close(&ai);
  }
  return alph;
}

/*======================================================================
 * Methods: Accessors: Automaton States
 */

/*--------------------------------------------------------------
 * renumber_states()
 */
void gfsm_automaton_renumber_states(gfsmAutomaton *fsm)
{
  gfsm_statesort_aff(fsm,NULL);
}

/*--------------------------------------------------------------
 * renumber_states_full()
 */
void gfsm_automaton_renumber_states_full(gfsmAutomaton *fsm, GArray *old2new, gfsmStateId n_new_states)
{
  gfsm_statemap_apply(fsm, old2new, n_new_states);
}

//--------------------------------------------------------------
void gfsm_automaton_truncate_invalid_states(gfsmAutomaton *fsm)
{
  gfsmState *s;
  for (s = gfsm_automaton_find_state(fsm, fsm->states->len-1); s && s>(gfsmState*)fsm->states->data && !s->is_valid; s--) {
    --fsm->states->len;
  }
}

/*======================================================================
 * Methods: Accessors: Automaton Arcs
 */

/*--------------------------------------------------------------
 * arcsort_full()
 */
void gfsm_automaton_arcsort_full(gfsmAutomaton *fsm, GCompareDataFunc cmpfunc, gpointer data)
{
  gfsmStateId qid;
  for (qid=0; qid < fsm->states->len; qid++) {
    gfsmState *qp = gfsm_automaton_find_state(fsm,qid);
    if (!qp || !qp->is_valid) continue;
    qp->arcs = gfsm_arclist_sort_full(qp->arcs, cmpfunc, data);
  }
  fsm->flags.sort_mode = gfsmACUser;
}

/*--------------------------------------------------------------
 * arcuniq()
 */
gfsmAutomaton *gfsm_automaton_arcuniq(gfsmAutomaton *fsm)
{
  gfsmStateId qid;
  gfsmState *qptr;
  gfsmArcList *al0, *al1;

#if 0
  //-- maybe pre-sort arcs
  if (fsm->flags.sort_mode == gfsmASMNone) {
    gfsm_automaton_arcsort(fsm, gfsmASMLower);
  }
#endif

  //-- ye olde loope
  for (qid=0; qid < fsm->states->len; qid++) {
    qptr = gfsm_automaton_open_state(fsm,qid);
    for (al0=qptr->arcs; al0 != NULL; al0=al0->next) {
      for (al1=al0->next; al1!=NULL && al1->arc.lower==al0->arc.lower && al1->arc.upper==al0->arc.upper && al1->arc.target==al0->arc.target; al1=al0->next) {
	al0->arc.weight = gfsm_sr_plus(fsm->sr, al0->arc.weight, al1->arc.weight);
	al0->next       = al1->next;
	gfsm_arclist_free_1(al1);
      }
    }
    gfsm_automaton_close_state(fsm,qptr);
  }

  //-- return
  return fsm;
}
