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
const gfsmStateId gfsmAutomatonDefaultSize  = 128;

const gfsmAutomatonFlags gfsmAutomatonDefaultFlags =
  {
    TRUE,        //-- is_transducer:1
    TRUE,        //-- is_weighted:1
    gfsmASMNone, //-- sort_mode:4
    FALSE,       //-- is_deterministic:1
    FALSE,       //-- arcs_dirty:1
    FALSE,       //-- final_dirty:1
    0            //-- unused:23
  };

//const gfsmSRType gfsmAutomatonDefaultSRType = gfsmSRTReal;
const gfsmSRType gfsmAutomatonDefaultSRType = gfsmSRTTropical;

/*======================================================================
 * Methods: Constructors etc.
 */
/*--------------------------------------------------------------
 * new_full()
 */
gfsmAutomaton *gfsm_automaton_new_full(gfsmAutomatonFlags flags, gfsmSRType srtype, gfsmStateId size)
{
  gfsmAutomaton *fsm = (gfsmAutomaton*)g_new0(gfsmAutomaton,1);
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

  dst->flags.arcs_dirty = 1; //-- index maintainence

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

  //-- copy indices?
  if (!src->flags.arcs_dirty) {
    if (src->ix_lower) dst->ix_lower = gfsm_label_index_copy(NULL,src->ix_lower);
    if (src->ix_upper) dst->ix_upper = gfsm_label_index_copy(NULL,src->ix_upper);
  }
  if (!src->flags.final_dirty) {
    if (src->ix_final) dst->ix_final = gfsm_final_index_copy(NULL,src->ix_final);
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

  //-- clear indices?
  if (fsm->ix_lower) gfsm_lower_index_free(fsm->ix_lower);
  if (fsm->ix_upper) gfsm_lower_index_free(fsm->ix_upper);
  if (fsm->ix_final) gfsm_final_index_free(fsm->ix_final);
  fsm->ix_lower=NULL;
  fsm->ix_upper=NULL;
  fsm->ix_final=NULL;

  return;
}

/*--------------------------------------------------------------
 * free()
 */
void gfsm_automaton_free(gfsmAutomaton *fsm)
{
  if (!fsm) return;
  gfsm_automaton_clear(fsm); //-- implicitly frees indices
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
    fsm->flags.arcs_dirty=1;
    fsm->flags.final_dirty=1;
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
	gfsm_alphabet_insert(alph, GUINT_TO_POINTER((guint)(a->lower)), a->lower);

      if (which != gfsmLSLower)
	gfsm_alphabet_insert(alph, GUINT_TO_POINTER((guint)(a->upper)), a->upper);
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

  fsm->flags.arcs_dirty=1; //-- index tracking

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

  fsm->flags.final_dirty = 1; //-- index tracking
  fsm->flags.arcs_dirty  = 1; //-- index tracking
}

/*--------------------------------------------------------------
 * set_final_state_full
 */
void gfsm_automaton_set_final_state_full(gfsmAutomaton *fsm,
					 gfsmStateId    id,
					 gboolean       is_final,
					 gfsmWeight     final_weight)
{
  gfsm_state_set_final(gfsm_automaton_get_state(fsm,id),is_final);
  if (is_final) {
    gfsm_weightmap_insert(fsm->finals, GUINT_TO_POINTER(id), final_weight);
  }
  else {
    gfsm_weightmap_remove(fsm->finals, GUINT_TO_POINTER(id));
  }
  fsm->flags.final_dirty=1;
}

/*--------------------------------------------------------------
 * get_final_weight
 */
gfsmWeight gfsm_automaton_get_final_weight(gfsmAutomaton *fsm, gfsmStateId id)
{
  gfsmWeight w;
  if (gfsm_weightmap_lookup(fsm->finals, GUINT_TO_POINTER(id), &w)) return w;
  return fsm->sr->zero;
}

/*--------------------------------------------------------------
 * lookup_final
 */
gboolean gfsm_automaton_lookup_final(gfsmAutomaton *fsm, gfsmStateId id, gfsmWeight *wp)
{
  if (!gfsm_automaton_is_final_state(fsm,id)) {
    *wp = fsm->sr->zero;
    return FALSE;
  }
  else if (fsm->ix_final && !fsm->flags.final_dirty) {
    *wp = g_array_index(fsm->ix_final, gfsmWeight, id);
    return TRUE;
  }
  return gfsm_weightmap_lookup(fsm->finals, GUINT_TO_POINTER(id), wp);
}

/*--------------------------------------------------------------
 * out_degree()
 */
guint gfsm_automaton_out_degree(gfsmAutomaton *fsm, gfsmStateId qid)
{
  if (qid >= fsm->states->len) return 0;
  if (!fsm->arcs_dirty) {
    if (fsm->arcix_lower) return gfsm_label_index_out_degree(fsm->arcix_lower,qid);
    if (fsm->arcix_upper) return gfsm_label_index_out_degree(fsm->arcix_upper,qid);
  }
  return gfsm_state_out_degree(gfsm_automaton_get_state(fsm,id));
}


/*--------------------------------------------------------------
 * renumber_states()
 */
void gfsm_automaton_renumber_states(gfsmAutomaton *fsm)
{
  gfsmStateId    oldid, newid;
  GArray        *old2new = NULL;

  //-- always set root state to zero -- even add one
  if (fsm->root_id == gfsmNoState) fsm->root_id = gfsm_automaton_add_state(fsm);

  //-- get old-to-new id map
  old2new = g_array_sized_new(FALSE,FALSE,sizeof(gfsmStateId),fsm->states->len);
  g_array_index(old2new,gfsmStateId,fsm->root_id) = 0;
  for (oldid=0, newid=0; oldid < fsm->states->len; oldid++) {
    if (oldid==fsm->root_id) continue;
    if (gfsm_automaton_has_state(fsm,oldid)) {
      g_array_index(old2new,gfsmStateId,oldid) = ++newid;
    } else {
      g_array_index(old2new,gfsmStateId,oldid) = gfsmNoState;
    }
  }

  //-- perform actual renumbering
  gfsm_automaton_renumber_states_full(fsm, old2new, newid+1);

  //-- cleanup
  g_array_free(old2new,TRUE);
}

/*--------------------------------------------------------------
 * renumber_states_full()
 */
void gfsm_automaton_renumber_states_full(gfsmAutomaton *fsm, GArray *old2new, gfsmStateId n_new_states)
{
  gfsmStateId    oldid, newid;
  gfsmState     *s_old, *s_new;
  gfsmWeightMap *new_finals = gfsm_weightmap_new(gfsm_uint_compare);
  GArray        *new_states = NULL;

  //-- get new number of states
  if (n_new_states==0) {
    for (oldid=0; oldid < fsm->states->len; oldid++) {
      if (!gfsm_automaton_has_state(fsm,oldid)) continue;
      newid = g_array_index(old2new,gfsmStateId,oldid);
      if (newid != gfsmNoState && newid >= n_new_states) { n_new_states=newid+1; }
    }
  }

  //-- allocate new state-vector
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
      gfsmWeight fw;
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
  fsm->root_id = g_array_index(old2new,gfsmStateId,fsm->root_id);

  //-- set new final weights
  gfsm_weightmap_free(fsm->finals);
  fsm->finals = new_finals;

  //-- set new state vector
  g_array_free(fsm->states,TRUE);
  fsm->states = new_states;
  fsm->states->len = n_new_states;

  //-- mark dirty
  fsm->flags.arcs_dirty=1;
  fsm->flags.final_dirty=1;
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
			    gfsmWeight  w)
{
  gfsmState *q1s;
  gfsm_automaton_ensure_state(fsm,q2);
  q1s = gfsm_automaton_get_state(fsm,q1);
  gfsm_automaton_add_arc_link(fsm,
			      q1s,
			      gfsm_arclist_new_full(q1,q2,lo,hi,w,NULL));
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

  //-- always mark arcs "dirty"
  fsm->flags.arcs_dirty=1;

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