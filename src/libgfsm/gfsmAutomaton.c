/*=============================================================================*\
 * File: gfsmAutomaton.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: generic automaton: pure function definitions
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

#include <gfsmAutomaton.h>
#include <gfsmImplOld.h>

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

const gfsmStateId gfsmAutomatonDefaultNStates  = 128;

const gfsmArcId gfsmAutomatonDefaultNArcs = 128;

const char *gfsmAutomatonClassNames[gfsmACNClasses] =
  {
    "Unknown",
    "Old", 
    "Basic",
    "ArcTable",
    "FullTable",
    "Virtual"
  };

/*======================================================================
 * Wrappers: class name
 */
const char *gfsm_class_name(gfsmAutomatonClass itype)
{
  if (itype >= gfsmACNClasses) return NULL;
  return gfsmAutomatonClassNames[itype];
}

/*======================================================================
 * API: Constructors etc.
 */


/*======================================================================
 * API: Automaton Structure
 */

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
 * API: Automaton States
 */


/*--------------------------------------------------------------
 * map_states()
 */
gfsmAutomaton *gfsm_automaton_map_states(gfsmAutomaton *fsm,
					 GArray        *old2new,
					 gfsmStateId    n_new_states,
					 gfsmAutomaton *dst)
{
  gfsmStateId oldid, newid;
  gfsmWeight fw;

  //-- maybe get new number of states
  if (n_new_states==0) {
    for (oldid=0; oldid < fsm->states->len; oldid++) {
      if (!gfsm_automaton_has_state(fsm,oldid)) continue;
      newid = g_array_index(old2new,gfsmStateId,oldid);
      if (newid != gfsmNoState && newid >= n_new_states) { n_new_states=newid+1; }
    }
  }

  //-- reserve new states
  if (dst!=NULL) {
    gfsm_automaton_reserve_states(dst,n_new_states);
  } else {
    dst = gfsm_automaton_classed_new_full(gfsmDefaultClass,n_new_states,0);
  }
  gfsm_automaton_copy_shallow(dst,fsm); 

  //-- temp: set dst sort-mode to none
  dst->flags.sort_mode = gfsmASMNone;

  //-- renumber states
  for (oldid=0; oldid < fsm->states->len; oldid++) {
    gfsmArcIter ai; 
    newid = g_array_index(old2new,gfsmStateId,oldid);

    if (newid==gfsmNoState || !gfsm_automaton_has_state(fsm,oldid)) continue; //-- ignore bad states
    gfsm_automaton_ensure_state(dst,newid);

    //-- check for final state
    if (gfsm_automaton_lookup_final(fsm,oldid,&fw)) {
      gfsm_automaton_set_final_state_full(dst,newid,TRUE,fw);
    }

    //-- copy outgoing arcs, renumbering as we go
    for (gfsm_arciter_open(&ai,fsm,newid); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      gfsm_automaton_add_arc(dst,
			     newid,
			     g_array_index(old2new,gfsmStateId,a->target),
			     a->lower,
			     a->upper,
			     a->weight);
    }
    gfsm_arciter_close(&ai);
  }

  //-- set new root-id
  gfsm_automaton_set_root(fsm,g_array_index(old2new,gfsmStateId,gfsm_automaton_get_root(fsm)));

  return dst;
}

/*--------------------------------------------------------------
 * renumber_states()
 */
void gfsm_automaton_renumber_states(gfsmAutomaton *fsm)
{
  gfsmStateId    oldid, newid;
  GArray        *old2new = NULL;
  gfsmStateId    n_states;
  gfsmStateId    root = gfsm_automaton_get_root(fsm);
  gfsmAutomaton *dst;

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

  //-- perform actual state-id mapping
  dst = gfsm_automaton_map_states(fsm, old2new, newid+1, NULL);

  //-- swap & cleanup
  gfsm_automaton_swap(dst,src);
  gfsm_automaton_free(src);
}

/*======================================================================
 * API: Automaton Arcs
 */

/*======================================================================
 * API: Arc Iterators
 */

/*======================================================================
 * API: Automaton I/O
 */

/*--------------------------------------------------------------
 * save_header()
 */
gboolean gfsm_automaton_save_header(gfsmAutomatonHeader *hdr, gfsmIOHandle *ioh, gfsmError **errp)
{
  //-- write header
  if (!gfsmio_write(ioh, &hdr, sizeof(gfsmAutomatonHeader))) {
    g_set_error(errp, g_quark_from_static_string("gfsm"),                  //-- domain
		      g_quark_from_static_string("automaton_save_header"), //-- code
		      "could not store automaton header");
    return FALSE;
  }
  return TRUE;
}
