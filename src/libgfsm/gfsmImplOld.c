/*=============================================================================*\
 * File: gfsmImplOld.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: automata: old implementation
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
#include <stdlib.h>

/*======================================================================
 * API: Constructors etc.
 */
//-- inlined or generic

/*======================================================================
 * API: Semiring
 */
//-- inlined or generic

/*======================================================================
 * API: Automaton Structure
 */
//-- inlined or generic

/*======================================================================
 * API: Automaton Properties
 */
//-- inlined or generic

/*======================================================================
 * API: Automaton Arcs
 */
//-- inlined or generic

/*======================================================================
 * API: Arc Iterators
 */
//-- inlined or generic

/*======================================================================
 * API: Automaton I/O
 */

const gfsmVersion gfsm_version_bincompat_min_store_old = {0,0,8};
const gfsmVersion gfsm_version_bincompat_min_check_old = {0,0,2};

/*--------------------------------------------------------------
 * save_bin_handle()
 */
gboolean gfsm_automaton_save_bin_handle_old(gfsmAutomaton        *fsm,
					    gfsmAutomatonHeader  *hdr,
					    gfsmIOHandle         *ioh,
					    gfsmError           **errp)
{
  gfsmStateId         id;
  gfsmState           *st;
  gfsmStoredState     sst;
  gfsmStoredArc       sa;
  gfsmWeight           w;
  gfsmArcIter         ai;
  gboolean            rc = TRUE;

  //-- create & write header
  hdr->version    = gfsm_version;
  hdr.version_min = gfsm_version_bincompat_min_store_old;
  hdr->flags      = fsm->flags;
  hdr->root_id    = gfsm_automaton_get_root(fsm);
  hdr->n_states   = gfsm_automaton_n_states(fsm);
  hdr->n_arcs     = 0;
  hdr->srtype     = fsm->sr->type;
  hdr->itype      = fsm->itype;
  if (!gfsm_automaton_save_header(&hdr,ioh,errp)) return FALSE; //-- errp shoud already be set

  //-- zero stored state (allow zlib compression to work better for any 'unused' members)
  memset(&sst, 0, sizeof(gfsmStoredState));

  //-- write states
  for (id=0; rc && id < hdr->n_states; id++) {
    //-- store basic state information
    st           = gfsm_automaton_find_state_old(fsm,id);
    sst.is_valid = st ? st->is_valid : FALSE;
    sst.is_final = sst.is_valid ? st->is_final : FALSE;
    sst.n_arcs   = sst.is_valid ? gfsm_state_out_degree(st) : 0;
    if (!gfsmio_write(ioh, &sst, sizeof(sst))) {
      g_set_error(errp, g_quark_from_static_string("gfsm"),                         //-- domain
			g_quark_from_static_string("automaton_save_bin_old:state"), //-- code
			"could not store state %d", id);
      rc = FALSE;
    }

    //-- store final weight (maybe)
    if (rc && sst.is_final) {
      w = gfsm_automaton_get_final_weight(fsm,id);
      if (!gfsmio_write(ioh, &w, sizeof(gfsmWeight))) {
	g_set_error(errp, g_quark_from_static_string("gfsm"),                                //-- domain
		    g_quark_from_static_string("automaton_save_bin_old:state:final_weight"), //-- code
		    "could not store final weight for state %d", id);
	rc = FALSE;
      }
    }

    //-- store arcs
    if (sst.is_valid) {
      for (gfsm_arciter_open_ptr(&ai,fsm,st); rc && gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
	gfsmArc *a = gfsm_arciter_arc(&ai);
	sa.target = a->target;
	sa.lower  = a->lower;
	sa.upper  = a->upper;
	sa.weight = a->weight;
	if (!gfsmio_write(ioh, &sa, sizeof(sa))) {
	  g_set_error(errp, g_quark_from_static_string("gfsm"),                       //-- domain
		      g_quark_from_static_string("automaton_save_bin_old:state:arc"), //-- code
		      "could not store arcs for state %d", id);
	  rc = FALSE;
	} 
      }
      gfsm_arciter_close(&ai);
    }
  }

  return rc;
}

/*--------------------------------------------------------------
 * load_bin_handle(): v0.0.2 .. v0.0.7
 */
/// Type for a stored state (v0.0.2 .. v0.0.7)
typedef struct {
  gboolean is_valid : 1; /**< valid flag */
  gboolean is_final : 1; /**< final flag */
  guint    n_arcs;       /**< number of stored arcs */
  guint    min_arc;      /**< index of stored minimum arc (not really necessary) */
} gfsmStoredState_007;

gboolean gfsm_automaton_load_bin_handle_0_0_7(gfsmAutomaton *fsm,
					      gfsmAutomatonHeader *hdr,
					      gfsmIOHandle *ioh,
					      gfsmError **errp)
{
  gfsmImplOld    *fsmi = fsm->impl.old;
  gfsmStateId     id;
  guint           arci, n_arcs;
  gfsmStoredArc   s_arc;
  gfsmStoredState_007 s_state;
  gfsmState       *st;
  gboolean         rc = TRUE;

  //-- allocate states
  gfsm_automaton_reserve_states(fsm, hdr->n_states);

  //-- set automaton-global properties
  fsm->flags   = hdr->flags;
  gfsm_semiring_init(fsm->sr, hdr->srtype);
  fsmi->root_id = hdr->root_id;

  //------ load states (one-by-one)
  for (id=0; rc && id < hdr->n_states; id++) {
    if (!gfsmio_read(ioh, &s_state, sizeof(gfsmStoredState_007))) {
      g_set_error(errp,
		  g_quark_from_static_string("gfsm"),                         //-- domain
		  g_quark_from_static_string("automaton_load_bin_old:state"), //-- code
		  "could not read stored state %d", id);
      rc = FALSE;
      break;
    }

    if (!s_state.is_valid) continue;

    st           = gfsm_automaton_get_state_old(fsm,id);
    st->is_valid = TRUE;

    if (s_state.is_final) {
      st->is_final = TRUE;
      gfsm_weightmap_insert(fsmi->finals, GUINT_TO_POINTER(id), fsm->sr->one);
    } else {
      st->is_final = FALSE;
    }

    //-- HACK: remember number of arcs!
    st->arcs = (gfsmArcList*) GUINT_TO_POINTER(s_state.n_arcs);
  }

  //------ load arcs (state-by-state)
  for (id=0; rc && id < hdr->n_states; id++) {
    //-- get state
    st = gfsm_automaton_find_state(fsm,id);
    if (!st || !st->is_valid) continue;

    //-- read in arcs (one-by-one)
    n_arcs   = GPOINTER_TO_UINT(st->arcs);
    st->arcs = NULL;
    for (arci=0; arci < n_arcs; arci++) {
      if (!gfsmio_read(ioh, &s_arc, sizeof(gfsmStoredArc))) {
	g_set_error(errp, g_quark_from_static_string("gfsm"),                 //-- domain
		    g_quark_from_static_string("automaton_load_bin_old:arc"), //-- code
		    "could not read stored arcs for state %d", id);
	rc=FALSE;
	break;
      }

      st->arcs = g_slist_prepend(st->arcs, gfsm_arc_new_full(id,
							     s_arc.target,
							     s_arc.lower,
							     s_arc.upper,
							     s_arc.weight));
    }

    //-- reverse arc-list for sorted automata
    if (fsm->flags.sort_mode != gfsmASMNone) st->arcs = g_slist_reverse(st->arcs);
  }

  return rc;
}

/*--------------------------------------------------------------
 * load_bin_handle(): v0.0.8 .. CURRENT
 */

gboolean gfsm_automaton_load_bin_handle_0_0_8(gfsmAutomaton *fsm,
					      gfsmAutomatonHeader *hdr,
					      gfsmIOHandle *ioh,
					      gfsmError **errp)
{
  gfsmImplOld    *fsmi = fsm->impl.old;
  gfsmStateId     id;
  guint           arci;
  gfsmStoredArc   s_arc;
  gfsmStoredState s_state;
  gfsmState       *st;
  gboolean         rc = TRUE;
  gfsmWeight       w;

  //-- allocate states
  gfsm_automaton_reserve_states(fsm, hdr->n_states);

  //-- set automaton-global properties
  fsm->flags   = hdr->flags;
  gfsm_automaton_set_semiring_type(fsm, hdr->srtype);
  fsmi->root_id = hdr->root_id;

  //------ load states (one-by-one)
  for (id=0; rc && id < hdr->n_states; id++) {
    if (!gfsmio_read(ioh, &s_state, sizeof(gfsmStoredState))) {
      g_set_error(errp,
		  g_quark_from_static_string("gfsm"),                         //-- domain
		  g_quark_from_static_string("automaton_load_bin_old:state"), //-- code
		  "could not read stored state %d", id);
      rc = FALSE;
      break;
    }

    if (!s_state.is_valid) continue;

    st           = gfsm_impl_old_find_state(fsm,id);
    st->is_valid = TRUE;

    if (s_state.is_final) {
      //-- read final weight
      if (!gfsmio_read(ioh, &w, sizeof(gfsmWeight))) {
	g_set_error(errp,
		    g_quark_from_static_string("gfsm"),                                      //-- domain
		    g_quark_from_static_string("automaton_load_bin_old:state:final_weight"), //-- code
		    "could not read final weight for stored state %d", id);
	rc = FALSE;
	break;
      }

      //-- set final weight
      st->is_final = TRUE;
      gfsm_weightmap_insert(fsm->finals, GUINT_TO_POINTER(id), w);
    } else {
      st->is_final = FALSE;
    }

    //-- read arcs (one-by-one)
    st->arcs = NULL;
    for (arci=0; arci < s_state.n_arcs; arci++) {
      if (!gfsmio_read(ioh, &s_arc, sizeof(gfsmStoredArc))) {
	g_set_error(errp, g_quark_from_static_string("gfsm"),                       //-- domain
		    g_quark_from_static_string("automaton_load_bin_old:state:arc"), //-- code
		    "could not read stored arcs for state %d", id);
	rc=FALSE;
	break;
      }
      if (!rc) break;

      st->arcs = g_slist_prepend(st->arcs,
				 gfsm_arc_new_full(id,
						   s_arc.target,
						   s_arc.lower,
						   s_arc.upper,
						   s_arc.weight,
						   st->arcs));
    }

    //-- reverse arc-list for sorted automata
    if (fsm->flags.sort_mode != gfsmASMNone) st->arcs = g_slist_reverse(st->arcs);
  }

  return rc;
}

/*--------------------------------------------------------------
 * load_bin_handle(): version dispatch
 */
gboolean gfsm_automaton_load_bin_handle_old(gfsmAutomaton        *fsm,
					    gfsmAutomatonHeader  *hdr,
					    gfsmIOHandle         *ioh,
					    gfsmError           **errp);
{
  if (gfsm_version_ge(hdr->version,((gfsmVersionInfo){0,0,8}))) {
    //-- v0.0.8 .. CURRENT
    return gfsm_automaton_load_bin_handle_old_0_0_8(fsm,hdr,ioh,errp);
  }
  else {
    //-- v0.0.2 .. v0.0.7
    return gfsm_automaton_load_bin_handle_old_0_0_7(fsm,hdr,ioh,errp);
  }
}
