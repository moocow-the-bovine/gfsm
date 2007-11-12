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
#include <gfsmDefaultImpl.h>
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

const char *gfsmAutomatonClassNames[gfsmACNClasses] =
  {
    "Unknown",
    "OldBasic", 
    "Basic",
    "ArcTable",
    "FullTable",
    "Virtual"
  };

const char *gfsm_class_name(gfsmAutomatonClass itype)
{
  if (itype >= gfsmACNClasses) return NULL;
  return gfsmAutomatonClassNames[itype];
}

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
    gfsm_automaton_init_basic_old(fsm,n_states,n_arcs); break;
  default:
    gfsm_automaton_init_default(fsm,n_states,n_arcs);
  }

  return fsm;
}

/*--------------------------------------------------------------
 * copy_shallow()
 */
gfsmAutomaton *gfsm_automaton_copy_shallow(gfsmAutomaton *dst, gfsmAutomaton *src)
{
  dst->flags = src->flags;
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
  switch (fsm->itype) {
  case gfsmACOldBasic:
    return gfsm_automaton_clone_old_basic(fsm);
  }
  return gfsm_automaton_clone_default(fsm);
}

/*--------------------------------------------------------------
 * copy()
 */
gfsmAutomaton *gfsm_automaton_copy(gfsmAutomaton *dst, gfsmAutomaton *src)
{
  switch (dst->itype) {
  case gfsmACOldBasic:
    return gfsm_automaton_copy_old_basic(dst,src);
  }
  return gfsm_automaton_copy_default(dst,src);
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
  case gfsmACOldBasic:
    gfsm_automaton_clear_old_basic(fsm); break;
  }
  gfsm_automaton_clear_default(fsm);
}

/*--------------------------------------------------------------
 * free()
 */
void gfsm_automaton_free_full(gfsmAutomaton *fsm, gboolean free_impl)
{
  if (!fsm) return;
  if (fsm->sr)  gfsm_semiring_free(fsm->sr);
  if (free_impl && fsm->impl) {
    switch (fsm->itype) {
    case gfsmACOldBasic: gfsm_automaton_free_old_basic(fsm); break;
    default: gfsm_automaton_free_default(fsm);
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
  return fsm->sr; //-- WARNING: in gfsm <= v0.0.09, returned literal 'sr' parameter!
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
 * reserve_states()
 */
void gfsm_automaton_reserve_states(gfsmAutomaton *fsm, gfsmStateIdVal n_states)
{
  switch (fsm->itype) {
  case gfsmACOldBasic: gfsm_automaton_reserve_states_old_basic(fsm,n_states);
  default:
    gsm_automaton_reserve_states_default(fsm,n_states);
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
  guint         n_states;    //-- automaton size
  gfsmStateId   root;        //-- root id

  g_assert(fsm!=NULL);
  n_states = gfsm_automaton_n_states(fsm);
  root     = gfsm_automaton_get_root(fsm);
  if (root==gfsmNoState || n_states==0) return FALSE;

  visited   = gfsm_bitvector_sized_new(n_states);
  completed = gfsm_bitvector_sized_new(n_states);

  rc = gfsm_automaton_is_cyclic_state(fsm, root, visited, completed);

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
  guint n_states = gfsm_automaton_n_states(fsm);
  gfsmArcIter ai;

  //-- ensure alphabet exists
  if (!alph) alph = gfsm_range_alphabet_new();

  for (qid=0; qid < n_states; qid++) {
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
  case gfsmACOldBasic: return gfsm_automaton_add_state_old_basic(fsm,qid);
  }
  return gfsm_automaton_add_state_full_default(fsm,qid);
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
  gfsmWeight w;
  return gfsm_automaton_lookup_final(fsm,qid,&w);
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
  switch (fsm->itype) {
  case gfsmACOldBasic: return gfsm_automaton_lookup_final_old_basic(fsm,id,wp);
  }
  return gfsm_automaton_lookup_final_default(fsm,id,wp);
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
  case gfsmACOldBasic:
    gfsm_automaton_set_final_state_full_old_basic(fsm,qid,is_final,final_weight); break;
  default:
    gfsm_automaton_set_final_state_full_default(fsm,qid,is_final,final_weight);
  }
}


/*--------------------------------------------------------------
 * out_degree()
 */
guint gfsm_automaton_out_degree(gfsmAutomaton *fsm, gfsmStateId qid)
{
  switch (fsm->itype) {
  case gfsmACOldBasic:
    return gfsm_automaton_out_degree_old_basic(fsm,qid);
  }
  return gfsm_automaton_out_degree_default(fsm,qid);
}


/*--------------------------------------------------------------
 * renumber_states()
 */
void gfsm_automaton_renumber_states(gfsmAutomaton *fsm)
{
  gfsmStateId    oldid, newid;
  GArray        *old2new = NULL;
  gfsmStateId    n_states;
  gfsmStateId    root    = gfsm_automaton_get_root(fsm);

  //-- always set root state to zero -- even add one
  if (root == gfsmNoState) {
    gfsm_automaton_set_root(fsm,gfsm_automaton_add_state(fsm));
  }

  //-- get old-to-new id map
  n_states = gfsm_automaton_n_states(fsm);
  old2new = g_array_sized_new(FALSE,FALSE,sizeof(gfsmStateId),n_states);
  g_array_index(old2new, gfsmStateId, root) = 0;
  for (oldid=0, newid=0; oldid < n_states; oldid++) {
    if (oldid==root) continue;
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
  switch (fsm->itype) {
  case gfsmACOldBasic:
    return gfsm_automaton_renumber_states_full_old_basic(fsm,qid);
  }
  return gfsm_automaton_renumber_states_full_default(fsm,qid,n_new_states);
}


/*======================================================================
 * API: Automaton Arcs
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
  switch (fsm->itype) {
  case gfsmACOldBasic: gfsm_automaton_add_arc_old_basic(fsm,q1,q2,lo,hi,w); return;
  }
  gfsm_automaton_add_arc_default(fsm,q1,q2,lo,hi,w);
}


/*--------------------------------------------------------------
 * arcsort()
 */
gfsmAutomaton *gfsm_automaton_arcsort(gfsmAutomaton *fsm, gfsmArcSortMode mode)
{
  if (mode != fsm->flags.sort_mode && mode != gfsmASMNone) {
    gfsmArcSortData sdata = { fsm, mode };
    gfsm_automaton_arcsort_full(fsm, (GCompareDataFunc)gfsm_arc_compare, &sdata);
  }
  fsm->flags.sort_mode = mode;
  return fsm;
}

/*--------------------------------------------------------------
 * arcsort_full()
 */
void gfsm_automaton_arcsort_full(gfsmAutomaton *fsm, GCompareDataFunc cmpfunc, gpointer data)
{
  switch (fsm->itype) {
  case gfsmACOldBasic: gfsm_automaton_arcsort_full_old_basic(fsm,cmpfunc,data); return;
  }
  gfsm_automaton_arcsort_full_default(fsm,cmpfunc,data);
}
