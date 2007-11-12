/*=============================================================================*\
 * File: gfsmAutomatonAPI.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: generic automaton API
 *
 * Copyright (c) 2007 Bryan Jurish.
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

#include <gfsmAutomatonAPI.h>
#include <gfsmOldBasicImpl.h>

/*======================================================================
 * Constants
 */

const gfsmAutomatonClass gfsmAutomatonDefaultClass = gfsmACBasic;

const gfsmAutomatonFlags gfsmAutomatonDefaultFlags =
  {
    TRUE,        //-- is_transducer:1
    TRUE,        //-- is_weighted:1
    gfsmASMNone, //-- sort_mode:4
    FALSE,       //-- is_deterministic:1
    0            //-- unused:25
  };

const gfsmSRType gfsmAutomatonDefaultSRType = gfsmSRTTropical;

const guint gfsmAutomatonDefaultNStates  = 128;

const guint gfsmAutomatonDefaultNArcs = 128;

/*======================================================================
 * Methods: Constructors etc.
 */

/*--------------------------------------------------------------
 * new_full()
 */
gfsmAutomaton *gfsm_automaton_new_full(gfsmAutomatonClass itype,
				       gfsmAutomatonFlags flags,
				       gfsmSRType         srtype,
				       guint              n_states,
				       guint              n_arcs)
{
  gfsmAutomaton *fsm = (gfsmAutomaton*)g_new0(gfsmAutomaton,1);
  fsm->flags         = flags;
  fsm->sr            = gfsm_semiring_new(srtype);
  fsm->itype         = itype;

  switch (itype) {
  case gfsmACOldBasic:
    fsm->impl.obi          = (gfsmOldBasicImpl*)g_new0(gfsmOldBasicImpl,1);
    fsm->impl.obi->states  = g_array_sized_new(FALSE, TRUE, sizeof(gfsmState), n_states);
    fsm->impl.obi->finals  = gfsm_set_new(gfsm_uint_compare);
    fsm->impl.obi->root_id = gfsmNoState;
    break;
  default:
    g_error("gfsm_automaton_new_full(): can't handle implementation type '%d'\n", itype);
  }

  return fsm;
}

/*--------------------------------------------------------------
 * copy_shallow()
 */
gfsmAutomaton *gfsm_automaton_copy_shallow(gfsmAutomaton *dst, gfsmAutomaton *src)
{
  dst->flags   = src->flags;
  gfsm_automaton_set_root(dst,gfsm_automaton_get_root(src)); //-- DANGEROUS!

  //-- copy semiring
  if (dst->sr && dst->sr->type != src->sr->type) {
    gfsm_semiring_free(dst->sr);
    dst->sr = gfsm_semiring_copy(src->sr);
  }

  return dst;
}


/*--------------------------------------------------------------
 * clone()
 */
gfsmAutomaton *gfsm_automaton_clone(gfsmAutomaton *fsm)
{
  return
    gfsm_automaton_copy(gfsm_automaton_classed_new_full(fsm->itype,
							gfsmAutomatonDefaultFlags,
							fsm->sr->type,
							0,
							0),
			fsm);
}

/*--------------------------------------------------------------
 * copy()
 */
gfsmAutomaton *gfsm_automaton_copy(gfsmAutomaton *dst, gfsmAutomaton *src)
{
  gfsmStateId qid;
  gfsmArcIter ai;

  gfsm_automaton_clear(dst);
  gfsm_automaton_copy_shallow(dst,src);

  switch (dst->itype) {
  case gfsmACOldBasic:
  default:
    //g_error("gfsm_automaton_copy(): can't handle destination implementation type '%d'\n", dst->itype);
    break; //-- use default
  }

  //-- copy: default
  gfsm_automaton_reserve_states(dst, gfsm_automaton_n_states(src));
  //gfsm_automaton_reserve_arcs(dst, gfsm_automaton_n_arcs(src));

  for (qid=0; qid < gfsm_automaton_n_states(src); qid++) {
    if (!gfsm_automaton_has_state(src,qid)) continue;

    gfsm_automaton_add_state_full(dst,qid);
    if (gfsm_automaton_is_final_state(src,qid)) {
      gfsm_automaton_set_final_state_full(dst,qid,TRUE,gfsm_automaton_get_final_weight(src,qid));
    }

    for (gfsm_arciter_open(&ai,src,qid); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      gfsm_automaton_add_arc(dst,qid,a->target,a->lower,a->upper,a->weight);
    }
    gfsm_arciter_close(&ai);
  }

  return dst;
}


/*--------------------------------------------------------------
 * swap()
 */
void gfsm_automaton_swap(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  if (fsm1 != fsm2) {
    gfsmAutomaton tmp;
    tmp   = *fsm2;
    *fsm2 = *fsm1;
    *fsm1 =  tmp;
  }
}

/*--------------------------------------------------------------
 * clear()
 */
void gfsm_automaton_clear(gfsmAutomaton *fsm)
{
  switch (fsm->itype) {
  case gfsmACOldBasic: return gfsm_old_basic_impl_clear(fsm);
  default:
    g_error("gfsm_automaton_clear(): can't handle implementation type '%d'\n", fsm->itype);
  }
}

/*--------------------------------------------------------------
 * free()
 */
void gfsm_automaton_free_full(gfsmAutomaton *fsm, gboolean free_impl)
{
  if (!fsm) return;
  if (fsm->sr)     gfsm_semiring_free(fsm->sr);
  if (free_impl && fsm->impl) {
    switch (fsm->itype) {
    case gfsmACOldBasic:
      gfsm_automaton_clear(fsm);
      if (fsm->impl.obi->states) g_array_free(fsm->impl.obi->states,TRUE);
      if (fsm->impl.obi->finals) gfsm_set_free(fsm->impl.obi->finals);
      break;
    default:
      g_error("gfsm_automaton_free_full(): can't handle implementation type '%d'\n", fsm->itype);
    }
  }
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
  return fsm->sr;
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

  for (i=0; i < gfsm_automaton_n_states(fsm); i++) {
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
    gfsm_arciter_close(&ai);
  }
  return total;
}

/*--------------------------------------------------------------
 * reserve_states()
 */
void gfsm_automaton_reserve_states(gfsmAutomaton *fsm, gfsmStateIdVal n_states)
{
  switch (fsm->itype) {
  case gfsmACOldBasic:
    if (n_states != gfsmNoState && n_states >= fsm->impl.obi->states->len)
      g_array_set_size(fsm->impl.obi->states, n_states);
   default:
    g_error("gfsm_automaton_reserve_states(): can't handle implementation type '%d'\n", fsm->itype);
  }
}

/*--------------------------------------------------------------
 * is_cyclic_state()
 */
gboolean gfsm_automaton_is_cyclic_state(gfsmAutomaton *fsm,
					gfsmStateIdVal qid,
					gfsmBitVector *visited,
					gfsmBitVector *completed)
{
  gfsmArcIter  ai;

  if (gfsm_bitvector_get(visited,qid)) {
    if (gfsm_bitvector_get(completed,qid)) return FALSE;
    return TRUE;
  }

  if (!gfsm_automaton_has_state(fsm,qid)) return FALSE; //-- invalid states don't count as cyclic

  //-- mark this state as visited (& not yet completed)
  gfsm_bitvector_set(visited,   qid, 1);
  gfsm_bitvector_set(completed, qid, 0);

  //-- visit outgoing arcs
  for (gfsm_arciter_open(&ai,fsm,qid); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
    if (gfsm_automaton_is_cyclic_state(fsm, gfsm_arciter_arc(&ai)->target, visited, completed)) {
      gfsm_arciter_close(&ai);
      return TRUE;
    }
  }
  gfsm_arciter_close(&ai);

  //-- mark this state as completed
  gfsm_bitvector_set(completed,qid,1);

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

  if (!fsm || gfsm_automaton_get_root(fsm)==gfsmNoState || gfsm_automaton_n_states(fsm)==0) return FALSE;

  visited   = gfsm_bitvector_sized_new(gfsm_automaton_n_states(fsm));
  completed = gfsm_bitvector_sized_new(gfsm_automaton_n_states(fsm));

  rc = gfsm_automaton_is_cyclic_state(fsm, gfsm_automaton_get_root(fsm), visited, completed);

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
  gfsmStateId qid;

  //-- ensure alphabet exists
  if (!alph) alph = gfsm_range_alphabet_new();

  for (qid=0; qid < fsm->states->len; qid++) {
    gfsmArcIter ai;
    for (gfsm_arciter_open(&ai,fsm,qid); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
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
 * add_state_full()
 */
gfsmStateIdVal gfsm_automaton_add_state_full(gfsmAutomaton *fsm, gfsmStateIdVal qid)
{
  switch (fsm->itype) {
  case gfsmACOldBasic: return gfsm_old_basic_impl_add_state(fsm,qid);
  default:
    g_error("gfsm_automaton_add_state_full(): can't handle implementation type '%d'\n", fsm->itype);
  }
}

/*--------------------------------------------------------------
 * remove_state()
 */
void gfsm_automaton_remove_state(gfsmAutomaton *fsm, gfsmStateId id)
{
  switch (fsm->itype) {
  case gfsmACOldBasic: return gfsm_old_basic_impl_remove_state(fsm,qid);
  default:
    g_error("gfsm_automaton_remove_state(): can't handle implementation type '%d'\n", fsm->itype);
  }
}

/*--------------------------------------------------------------
 * is_final_state
 */
gboolean gfsm_automaton_is_final_state(gfsmAutomaton *fsm, gfsmStateIdVal qid)
{
  switch (fsm->itype) {
  case gfsmACOldBasic: return gfsm_old_basic_impl_is_final_state(fsm,qid);
  default:
    g_error("gfsm_automaton_remove_state(): can't handle implementation type '%d'\n", fsm->itype);
  }
}

/*--------------------------------------------------------------
 * set_final_state_full
 */
void gfsm_automaton_set_final_state_full(gfsmAutomaton *fsm,
					 gfsmStateIdVal qid,
					 gboolean       is_final,
					 gfsmWeight     final_weight)
{
  switch (fsm->itype) {
  case gfsmACOldBasic: return gfsm_old_basic_impl_set_final_state(fsm,qid,is_final,final_weight);
  default:
    g_error("gfsm_automaton_set_final_state_full(): can't handle implementation type '%d'\n", fsm->itype);
  }
}

/*--------------------------------------------------------------
 * get_final_weight
 */
gfsmWeight gfsm_automaton_get_final_weight(gfsmAutomaton *fsm, gfsmStateIdVal qid)
{
  gfsmWeight w;
  if (gfsm_automaton_lookup_final(fsm,qid,&w)) return w;
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
