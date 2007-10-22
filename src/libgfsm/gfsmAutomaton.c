/*=============================================================================*\
 * File: gfsmAutomaton.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: automata
 *
 * Copyright (c) 2004-2007 Bryan Jurish.
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

#include <gfsmAutomaton.h>
#include <gfsmArcIter.h>
#include <gfsmUtils.h>
#include <gfsmBitVector.h>
#include <stdlib.h>

/*======================================================================
 * Constants
 */
const guint gfsmAutomatonDefaultSize  = 128;

const gfsmAutomatonFlags gfsmAutomatonDefaultFlags =
  {
    TRUE,        //-- is_transducer:1
    TRUE,        //-- is_weighted:1
    gfsmASMNone, //-- sort_mode:4
    FALSE,       //-- is_deterministic:1
    0            //-- unused:25
  };

//const gfsmSRType gfsmAutomatonDefaultSRType = gfsmSRTReal;
const gfsmSRType gfsmAutomatonDefaultSRType = gfsmSRTTropical;

/*======================================================================
 * Methods: Constructors etc.
 */
/*--------------------------------------------------------------
 * new_full()
 */
gfsmAutomaton *gfsm_automaton_new_full(gfsmAutomatonFlags flags, gfsmSRType srtype, guint size)
{
  gfsmAutomaton *fsm = (gfsmAutomaton*)g_new(gfsmAutomaton,1);
  fsm->flags         = flags;
  fsm->sr            = gfsm_semiring_new(srtype);
  fsm->states        = g_array_sized_new(FALSE, TRUE, sizeof(gfsmState), size);
  fsm->finals        = gfsm_set_new(gfsm_uint_compare);
  fsm->root_id       = gfsmNoState;
  return fsm;
}

/*--------------------------------------------------------------
 * copy_shallow()
 */
gfsmAutomaton *gfsm_automaton_copy_shallow(gfsmAutomaton *dst, gfsmAutomaton *src)
{
  dst->flags   = src->flags;
  dst->root_id = src->root_id; //-- DANGEROUS!

  //-- copy semiring
  if (dst->sr && dst->sr->type != src->sr->type) {
    gfsm_semiring_free(dst->sr);
    dst->sr = gfsm_semiring_copy(src->sr);
  }

  return dst;
}


/*--------------------------------------------------------------
 * copy()
 */
gfsmAutomaton *gfsm_automaton_copy(gfsmAutomaton *dst, gfsmAutomaton *src)
{
  gfsmStateId id;

  gfsm_automaton_clear(dst);
  gfsm_automaton_copy_shallow(dst,src);
  gfsm_automaton_reserve(dst,src->states->len);
  gfsm_weightmap_copy(dst->finals, src->finals);

  for (id = 0; id < src->states->len; id++) {
    const gfsmState *src_s = gfsm_automaton_find_state_const(src,id);
          gfsmState *dst_s = gfsm_automaton_find_state(dst,id);
    gfsm_state_copy(dst_s, src_s);
  }

  return dst;
}

/*--------------------------------------------------------------
 * swap()
 */
void gfsm_automaton_swap(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  if (fsm1 != fsm2) {
    gfsmAutomaton *tmp = g_new0(gfsmAutomaton,1);
    *tmp = *fsm2;
    *fsm2 = *fsm1;
    *fsm1 = *tmp;
    g_free(tmp);
  }
}

/*--------------------------------------------------------------
 * clear()
 */
void gfsm_automaton_clear(gfsmAutomaton *fsm)
{
  guint i;
  if (!fsm) return;
  for (i=0; fsm->states && i < fsm->states->len; i++) {
    gfsmState *st = gfsm_automaton_find_state(fsm,i);
    if (!st || !st->is_valid) continue;
    gfsm_state_clear(st);
  }
  if (fsm->states) g_array_set_size(fsm->states,0);
  if (fsm->finals) gfsm_set_clear(fsm->finals);
  fsm->root_id = gfsmNoState;
}

/*--------------------------------------------------------------
 * free()
 */
void gfsm_automaton_free(gfsmAutomaton *fsm)
{
  if (!fsm) return;
  gfsm_automaton_clear(fsm);
  if (fsm->sr)     gfsm_semiring_free(fsm->sr);
  if (fsm->states) g_array_free(fsm->states,TRUE);
  if (fsm->finals) gfsm_set_free(fsm->finals);
  g_free(fsm);
}

/*======================================================================
 * Methods: Accessors: Automaton
 */

/*--------------------------------------------------------------
 * set_semiring()
 */
gfsmSemiring *gfsm_automaton_set_semiring(gfsmAutomaton *fsm, gfsmSemiring *sr)
{
  if (fsm->sr) gfsm_semiring_free(fsm->sr);
  fsm->sr = gfsm_semiring_copy(sr);
  return sr;
}

/*--------------------------------------------------------------
 * set_semiring_type()
 */
void gfsm_automaton_set_semiring_type(gfsmAutomaton *fsm, gfsmSRType srtype)
{
  if (!fsm->sr) fsm->sr = gfsm_semiring_new(srtype);
  else if (fsm->sr->type != srtype) {
    gfsm_semiring_free(fsm->sr);
    fsm->sr = gfsm_semiring_new(srtype);  
  }
}

/*--------------------------------------------------------------
 * n_arcs_full()
 */
guint gfsm_automaton_n_arcs_full(gfsmAutomaton *fsm,
				 guint *n_lo_epsilon,
				 guint *n_hi_epsilon,
				 guint *n_both_epsilon)
{
  guint i, total=0;

  if (n_lo_epsilon) *n_lo_epsilon = 0;
  if (n_hi_epsilon) *n_hi_epsilon = 0;
  if (n_both_epsilon) *n_both_epsilon = 0;

  for (i=0; i < fsm->states->len; i++) {
    gfsmArcIter ai;
    for (gfsm_arciter_open(&ai, fsm, i); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      ++total;
      if (n_lo_epsilon && a->lower==gfsmEpsilon) {
	++(*n_lo_epsilon);
	if (n_hi_epsilon && a->upper==gfsmEpsilon) {
	  ++(*n_hi_epsilon);
	  if (n_both_epsilon) ++(*n_both_epsilon);
	}
      }
      else if (n_hi_epsilon && a->upper==gfsmEpsilon) {
	++(*n_hi_epsilon);
      }
    }
  }
  return total;
}

/*--------------------------------------------------------------
 * reserve()
 */
void gfsm_automaton_reserve(gfsmAutomaton *fsm, gfsmStateId nstates)
{
  if (nstates != gfsmNoState && nstates >= fsm->states->len) {
    g_array_set_size(fsm->states,nstates);
  }
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

  if (gfsm_bitvector_get(visited,id)) {
    if (gfsm_bitvector_get(completed,id)) return FALSE;
    return TRUE;
  }

  s = gfsm_automaton_find_state(fsm,id);
  if (!s || !s->is_valid) return FALSE;  //-- invalid states don't count as cyclic

  //-- mark node as visited (& not completed)
  gfsm_bitvector_set(visited,id,1);
  gfsm_bitvector_set(completed,id,0);

  //-- visit outgoing arcs
  for (gfsm_arciter_open_ptr(&ai,fsm,s); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
    if (gfsm_automaton_is_cyclic_state(fsm, gfsm_arciter_arc(&ai)->target, visited, completed))
      return TRUE;
  }

  //-- mark node as completed
  gfsm_bitvector_set(completed,id,1);

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

  if (!fsm || fsm->root_id == gfsmNoState || fsm->states->len <= 0) return FALSE;

  visited   = gfsm_bitvector_sized_new(fsm->states->len);
  completed = gfsm_bitvector_sized_new(fsm->states->len);

  rc = gfsm_automaton_is_cyclic_state(fsm, fsm->root_id, visited, completed);

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
	gfsm_alphabet_insert(alph, GUINT_TO_POINTER((gfsmLabelVal)(a->lower)), a->lower);

      if (which != gfsmLSLower)
	gfsm_alphabet_insert(alph, GUINT_TO_POINTER((gfsmLabelVal)(a->upper)), a->upper);
    }
  }
  return alph;
}

/*======================================================================
 * Methods: Accessors: Automaton States
 */

/*--------------------------------------------------------------
 * add_state_full()
 */
gfsmStateId gfsm_automaton_add_state_full(gfsmAutomaton *fsm, gfsmStateId id)
{
  gfsmState *st;

  if (id == gfsmNoState)
    id = fsm->states->len;

  if (id >= fsm->states->len)
    gfsm_automaton_reserve(fsm,id+1);
  
  st           = gfsm_automaton_find_state(fsm,id);
  st->is_valid = TRUE;

  return id;
}

/*--------------------------------------------------------------
 * remove_state()
 */
void gfsm_automaton_remove_state(gfsmAutomaton *fsm, gfsmStateId id)
{
  gfsmState *s = gfsm_automaton_find_state(fsm,id);
  if (!s || !s->is_valid) return;

  if (s->is_final) gfsm_set_remove(fsm->finals,GUINT_TO_POINTER(id));
  if (id == fsm->root_id) fsm->root_id = gfsmNoState;

  gfsm_arclist_free(s->arcs);
  s->arcs = NULL;
  s->is_valid = FALSE;
}

/*--------------------------------------------------------------
 * set_final_state_full
 */
void gfsm_automaton_set_final_state_full(gfsmAutomaton *fsm,
					 gfsmStateId    id,
					 gboolean       is_final,
					 gfsmWeightU    final_weight)
{
  gfsm_state_set_final(gfsm_automaton_get_state(fsm,id),is_final);
  if (is_final) gfsm_weightmap_insert(fsm->finals, GUINT_TO_POINTER(id), final_weight);
  else gfsm_weightmap_remove(fsm->finals, GUINT_TO_POINTER(id));
}

/*--------------------------------------------------------------
 * get_final_weight
 */
gfsmWeightU gfsm_automaton_get_final_weight(gfsmAutomaton *fsm, gfsmStateId id)
{
  gfsmWeightU w;
  if (gfsm_weightmap_lookup(fsm->finals, GUINT_TO_POINTER(id), &w)) return w;
  return fsm->sr->zero;
}

/*--------------------------------------------------------------
 * lookup_final
 */
gboolean gfsm_automaton_lookup_final(gfsmAutomaton *fsm, gfsmStateId id, gfsmWeightU *wp)
{
  return gfsm_weightmap_lookup(fsm->finals, GUINT_TO_POINTER(id), wp);
}


/*--------------------------------------------------------------
 * renumber_states()
 */
void gfsm_automaton_renumber_states(gfsmAutomaton *fsm)
{
  gfsmStateId    oldid, newid, n_states;
  gfsmState     *s_old, *s_new;
  gfsmWeightMap *new_finals = gfsm_weightmap_new(gfsm_uint_compare);
  GArray        *new_states = NULL;
  GArray        *old2new    = NULL;

  //-- always set root state to zero -- even add one
  if (fsm->root_id == gfsmNoState) fsm->root_id = gfsm_automaton_add_state(fsm);

  //-- get old-to-new id map
  old2new = g_array_sized_new(FALSE,FALSE,sizeof(gfsmState),fsm->states->len);
  g_array_index(old2new,gfsmStateId,fsm->root_id) = 0;
  for (oldid=0, newid=0; oldid < fsm->states->len; oldid++) {
    if (oldid==fsm->root_id) continue;
    if (gfsm_automaton_has_state(fsm,oldid)) {
      g_array_index(old2new,gfsmStateId,oldid) = ++newid;
    } else {
      g_array_index(old2new,gfsmStateId,oldid) = gfsmNoState;
    }
  }
  n_states = newid+1;

  //-- allocate new states
  new_states = g_array_sized_new(FALSE,TRUE,sizeof(gfsmState),n_states);

  //-- renumber states
  for (oldid=0; oldid < fsm->states->len; oldid++) {
    gfsmArcIter ai;  
    newid = g_array_index(old2new,gfsmStateId,oldid);

    if (newid==gfsmNoState) continue; //-- ignore bad states

    //-- copy state data
    s_old  = gfsm_automaton_find_state(fsm, oldid);
    s_new  = &(g_array_index(new_states,gfsmState,newid));
    *s_new = *s_old;

    //-- check for final state
    if (s_new->is_final) {
      gfsmWeightU fw;
      gfsm_weightmap_lookup(fsm->finals, GUINT_TO_POINTER(oldid), &fw);
      gfsm_weightmap_insert(new_finals,  GUINT_TO_POINTER(newid),  fw);
    }

    //-- renumber targets of outgoing arcs
    for (gfsm_arciter_open_ptr(&ai, fsm, s_new); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai))
      {
	gfsmArc *a = gfsm_arciter_arc(&ai);
	a->target = g_array_index(old2new,gfsmStateId,a->target);
      }
  }

  //-- set new root-id
  fsm->root_id = 0;

  //-- set new final weights
  gfsm_weightmap_free(fsm->finals);
  fsm->finals = new_finals;

  //-- set new state vector
  g_array_free(fsm->states,TRUE);
  fsm->states = new_states;
  fsm->states->len = n_states;

  //-- cleanup
  g_array_free(old2new,TRUE);
}

void gfsm_automaton_renumber_states_old(gfsmAutomaton *fsm)
{
  gfsmStateId  i, oldid, newid;
  gfsmStateId  gap, rgap, gap_max;
  gfsmState   *s_old, *s_new;
  GArray      *i2gap;
  gfsmWeightMap *new_finals = gfsm_weightmap_new(gfsm_uint_compare);
  
  //-- allocate iteration-index-to-gap map
  i2gap = g_array_sized_new(FALSE, FALSE, sizeof(gfsmStateId), fsm->states->len);

  //-- get root-gap
  rgap = fsm->root_id==gfsmNoState ? 0 : fsm->root_id;

  //-- get (negative) state-offsets
  for (i=0, gap=0; i < fsm->states->len; i++) {
    oldid = (i+rgap)%fsm->states->len;
    s_old = gfsm_automaton_find_state(fsm,oldid);
    if (!s_old || !s_old->is_valid) {
      g_array_index(i2gap, gfsmStateId, i) = gfsmNoState;
      ++gap;
      continue;
    }
    g_array_index(i2gap, gfsmStateId, i) = gap;
  }
  gap_max = gap;

  //-- renumber states
  for (i=0; i < fsm->states->len; i++) {
    gfsmArcIter ai;
    oldid  = (i+rgap)%fsm->states->len;
    gap    = g_array_index(i2gap,gfsmStateId,i);
    newid  = i-gap;

    if (gap == gfsmNoState) continue; //-- bad state

    s_old  = gfsm_automaton_find_state(fsm,oldid);
    s_new  = gfsm_automaton_find_state(fsm, newid);

    //-- swap state data
    if (s_old != s_new) {
      *s_new = *s_old;
      s_old->is_valid = FALSE;
      s_old->arcs = NULL;

      //-- check for final state
      if (s_new->is_final) {
	gfsmWeightU fw;
	gfsm_weightmap_lookup(fsm->finals, GUINT_TO_POINTER(oldid), &fw);
	gfsm_weightmap_insert(new_finals,  GUINT_TO_POINTER(newid),  fw);
      }
    }

    //-- renumber targets of outgoing arcs
    for (gfsm_arciter_open_ptr(&ai, fsm, s_new); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai))
      {
	gfsmArc       *a = gfsm_arciter_arc(&ai);
	gfsmStateId   ti = (a->target >= rgap
			    ? (a->target - rgap)
			    : (a->target + (fsm->states->len - rgap)));
	gfsmStateId tgap = g_array_index(i2gap, gfsmStateId, ti);
	a->target        = ti-tgap;
      }
  }
  //-- set new root-id
  if (fsm->root_id != gfsmNoState) fsm->root_id = 0;

  //-- resize states vector
  g_array_set_size(fsm->states, fsm->states->len - gap_max);

  //-- set new final weights
  gfsm_weightmap_free(fsm->finals);
  fsm->finals = new_finals;

  //-- cleanup
  g_array_free(i2gap,TRUE);
}

/*======================================================================
 * Methods: Accessors: Automaton Arcs
 */

/*--------------------------------------------------------------
 * add_arc()
 */
void gfsm_automaton_add_arc(gfsmAutomaton *fsm,
			    gfsmStateId q1,
			    gfsmStateId q2,
			    gfsmLabelId lo,
			    gfsmLabelId hi,
			    gfsmWeightU w)
{
  gfsmState *q1s;
  gfsm_automaton_ensure_state(fsm,q2);
  q1s = gfsm_automaton_get_state(fsm,q1);
  gfsm_automaton_add_arc_link(fsm,
			      q1s,
			      gfsm_arclist_new_full(q2,lo,hi,w,NULL));
}

/*--------------------------------------------------------------
 * add_arc_ptr()
 */
void gfsm_automaton_add_arc_link(gfsmAutomaton *fsm,
				 gfsmState      *sp,
				 gfsmArcList    *link)
{
  //-- possibly sorted
  gfsmArcSortData sdata = { fsm->flags.sort_mode, fsm->sr };
  sp->arcs = gfsm_arclist_insert_link(sp->arcs, link, &sdata);

  //-- always unmark 'deterministic' flag -- better: check
  fsm->flags.is_deterministic = FALSE;

  /*-- unsorted
  alp->next = sp->arcs;
  sp->arcs = alp;
  fsm->flags.sort_mode = gfsmASMNone;
  */
}


/*--------------------------------------------------------------
 * arcsort()
 */
gfsmAutomaton *gfsm_automaton_arcsort(gfsmAutomaton *fsm, gfsmArcSortMode mode)
{
  gfsmStateId id;
  gfsmArcSortData sdata = { mode, fsm->sr };

  if (mode != fsm->flags.sort_mode && mode != gfsmASMNone) {
    for (id = 0; id < fsm->states->len; id++) {
      gfsmState *s = gfsm_automaton_find_state(fsm,id);
      if (!s || !s->is_valid) continue;
      s->arcs = gfsm_arclist_sort(s->arcs, &sdata);
    }
  }

  fsm->flags.sort_mode = mode;
  return fsm;
}
