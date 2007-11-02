
/*=============================================================================*\
 * File: gfsmIndexed.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: arc indices
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

#include <gfsmIndexed.h>
#include <gfsmArcIter.h>

/*======================================================================
 * Constants
 */

const gfsmArcId gfsmNoArc = (gfsmArcId)-1;

/*======================================================================
 * Constructors etc.
 */

//----------------------------------------
gfsmIndexedAutomaton *gfsm_indexed_automaton_new_full(gfsmAutomatonFlags flags,
						      gfsmSRType         srtype,
						      gfsmStateId        n_states,
						      gfsmArcId          n_arcs)
{
  gfsmIndexedAutomaton *xfsm = g_new(gfsmIndexedAutomaton,1);
  gfsmStateId qid;
  gfsmWeight  srzero;

  xfsm->flags   = flags;
  xfsm->sr      = gfsm_semiring_new(srtype);
  xfsm->root_id = gfsmNoState;

  xfsm->state_final_weight = g_array_sized_new(FALSE, FALSE, sizeof(gfsmWeight), n_states);
  xfsm->arcs               = g_array_sized_new(FALSE, TRUE,  sizeof(gfsmArc),    n_arcs);
  xfsm->state_first_arc    = g_array_sized_new(FALSE, TRUE,  sizeof(gfsmArcId),  n_states+1);

  xfsm->arcix_lower        = g_array_sized_new(FALSE, TRUE,  sizeof(gfsmArcId),  n_arcs);
  xfsm->arcix_upper        = g_array_sized_new(FALSE, TRUE,  sizeof(gfsmArcId),  n_arcs);

  //-- initialize: states
  srzero = xfsm->sr->zero;
  for (qid=0; qid < n_states; qid++) {
    g_array_index(xfsm->state_final_weight,gfsmWeight,qid) = srzero;
  }

  return xfsm;
}

//----------------------------------------
void gfsm_indexed_automaton_clear(gfsmIndexedAutomaton *xfsm)
{
  xfsm->state_final_weight->len=0;
  xfsm->arcs->len=0;
  xfsm->state_first_arc->len=0;
  xfsm->arcix_lower->len=0;
  xfsm->arcix_upper->len=0;
  xfsm->root_id = gfsmNoState;
  return;
 }

//----------------------------------------
void gfsm_indexed_automaton_free(gfsmIndexedAutomaton *xfsm)
{
  if (!xfsm) return;
  //gfsm_indexed_automaton_clear(xfsm);
  if (xfsm->sr)                 gfsm_semiring_free(xfsm->sr);
  if (xfsm->state_final_weight) g_array_free(xfsm->state_final_weight, TRUE);
  if (xfsm->arcs)               g_array_free(xfsm->arcs,               TRUE);
  if (xfsm->state_first_arc)    g_array_free(xfsm->state_first_arc,    TRUE);
  if (xfsm->arcix_lower)        g_array_free(xfsm->arcix_lower,        TRUE);
  if (xfsm->arcix_upper)        g_array_free(xfsm->arcix_upper,        TRUE);
  g_free(xfsm);
}

/*======================================================================
 * Methods: Import & Export
 */

//----------------------------------------
gfsmIndexedAutomaton *gfsm_automaton_to_indexed(gfsmAutomaton *fsm, gfsmIndexedAutomaton *xfsm)
{
  gfsmStateId qid;
  gfsmArcIter ai;
  gfsmArcId   aid;
  gfsmWeight  fw;

  //-- maybe allocate new indexed automaton
  if (xfsm==NULL) {
    xfsm = gfsm_indexed_automaton_new_full(fsm->flags,
					   fsm->sr->type,
					   gfsm_automaton_n_states(fsm),
					   gfsm_automaton_n_arcs(fsm));
  } else {
    gfsm_indexed_automaton_clear(xfsm);
    xfsm->flags = fsm->flags;
    xfsm->sr    = gfsm_semiring_copy(fsm->sr);
    gfsm_indexed_automaton_reserve_states(xfsm,gfsm_automaton_n_states(fsm));
    gfsm_indexed_automaton_reserve_arcs(xfsm,gfsm_automaton_n_arcs(fsm));
  }

  //-- set root id
  xfsm->root_id = fsm->root_id;

  //-- update state-wise
  for (qid=0,aid=0; qid < fsm->states->len; qid++) {
    //-- state_final_weight
    if (!gfsm_automaton_has_state(fsm,qid) || !gfsm_automaton_is_final_state(fsm,qid)) {
      fw = fsm->sr->zero;
    } else {
      fw = gfsm_automaton_get_final_weight(fsm,qid);
    }
    g_array_index(xfsm->state_final_weight,gfsmWeight,qid) = fw;

    //-- state_first_arc, arcs
    g_array_index(xfsm->state_first_arc,gfsmArcId,qid) = aid;
    for (gfsm_arciter_open(&ai,fsm,qid); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai),aid++) {
      g_array_index(xfsm->arcs,gfsmArc,aid) = *(gfsm_arciter_arc(&ai));
    }
  }
  g_array_index(xfsm->state_first_arc, gfsmArcId, qid) = aid; //-- arc upper bound

  //-- rebuild upper & lower arc-indices
  gfsm_indexed_automaton_build_indices(xfsm);

  return xfsm;
}

//----------------------------------------
void gfsm_indexed_automaton_build_indices(gfsmIndexedAutomaton *xfsm)
{
  gfsmStateId qid;
  gfsmArcId   aid, lb, ub;
  gfsmIndexedAutomatonSortData sortdata = { ((gfsmArc*)xfsm->arcs->data), xfsm->sr };

  //-- initialize both arc indices to the full arc sequence
  for (aid=0; aid < xfsm->arcs->len; aid++) {
    g_array_index(xfsm->arcix_lower,gfsmArcId,aid) = aid;
    g_array_index(xfsm->arcix_upper,gfsmArcId,aid) = aid;
  }

  //-- sort subsequences by state
  for (qid=0; qid < gfsm_indexed_automaton_n_states(xfsm); qid++) {
    lb = g_array_index(xfsm->state_first_arc, gfsmArcId, qid);
    ub = g_array_index(xfsm->state_first_arc, gfsmArcId, qid+1);
    if (ub <= lb) continue;

    if (xfsm->flags.sort_mode != gfsmASMLower) {
      g_qsort_with_data(((gfsmArcId*)xfsm->arcix_lower->data)+lb,
			ub-lb,
			sizeof(gfsmArcId),
			(GCompareDataFunc)gfsm_indexed_automaton_build_cmp_lo,
			&sortdata
			);
    }
    if (xfsm->flags.sort_mode != gfsmASMUpper) {
      g_qsort_with_data(((gfsmArcId*)xfsm->arcix_upper->data)+lb,
			ub-lb,
			sizeof(gfsmArcId),
			(GCompareDataFunc)gfsm_indexed_automaton_build_cmp_hi,
			&sortdata
			);
    }
  }

  return;
}

//----------------------------------------
gint gfsm_indexed_automaton_build_cmp_lo(const gfsmArcId *aidp1,
					 const gfsmArcId *aidp2,
					 gfsmIndexedAutomatonSortData *sdata)
{
  gfsmArc *a1 = sdata->arcs + (*aidp1);
  gfsmArc *a2 = sdata->arcs + (*aidp2);
  if (a1->lower < a2->lower) return -1;
  if (a1->lower > a2->lower) return  1;
  return gfsm_sr_compare(sdata->sr, a1->weight, a2->weight);
};

//----------------------------------------
gint gfsm_indexed_automaton_build_cmp_hi(const gfsmArcId *aidp1,
					 const gfsmArcId *aidp2,
					 gfsmIndexedAutomatonSortData *sdata)
{
  gfsmArc *a1 = sdata->arcs + (*aidp1);
  gfsmArc *a2 = sdata->arcs + (*aidp2);
  if (a1->upper < a2->upper) return -1;
  if (a1->upper > a2->upper) return  1;
  return gfsm_sr_compare(sdata->sr, a1->weight, a2->weight);
};

//----------------------------------------
gfsmAutomaton *gfsm_indexed_to_automaton(gfsmAutomaton *fsm, gfsmIndexedAutomaton *xfsm)
{
  gfsmStateId qid;
  gfsmArcId   aid, aid_max;
  gfsmWeight  srzero;

  //-- maybe allocate new automaton
  if (fsm==NULL) {
    fsm = gfsm_automaton_new_full(xfsm->flags, xfsm->sr->type, gfsm_indexed_automaton_n_states(xfsm));
  } else {
    gfsm_automaton_clear(fsm);
    fsm->flags = xfsm->flags;
    gfsm_automaton_set_semiring(fsm, gfsm_semiring_copy(xfsm->sr));
    gfsm_automaton_reserve(fsm, gfsm_indexed_automaton_n_states(xfsm));
  }

  //-- set root id
  fsm->root_id = xfsm->root_id;

  //-- update state-wise
  srzero = xfsm->sr->zero;
  for (qid=0; qid < xfsm->state_final_weight->len; qid++) {

    //-- final weight
    gfsmWeight fw = g_array_index(xfsm->state_final_weight,gfsmWeight,qid);
    if (fw != srzero) gfsm_automaton_set_final_state_full(fsm,qid,TRUE,fw);

    //-- arcs
    aid_max = g_array_index(xfsm->state_first_arc, gfsmArcId, qid+1);
    for (aid=g_array_index(xfsm->state_first_arc,gfsmArcId,qid); aid<aid_max; aid++) {
      gfsmArc *a = &(g_array_index(xfsm->arcs,gfsmArc,aid));
      gfsm_automaton_add_arc(fsm,qid,a->target,a->lower,a->upper,a->weight);
    }
  }

  return fsm;
}

/*======================================================================
 * Methods: Accessors: gfsmIndexedAutomaton
 */

//----------------------------------------
gfsmStateId gfsm_indexed_automaton_reserve_states(gfsmIndexedAutomaton *xfsm, gfsmStateId n_states)
{
  gfsmStateId n_states_old = gfsm_indexed_automaton_n_states(xfsm);
  gfsmStateId qid;
  gfsmWeight  srzero;

  //-- resize state-indexed arrays
  g_array_set_size(xfsm->state_final_weight, n_states);
  g_array_set_size(xfsm->state_first_arc,    n_states+1);

  //-- ... adjust final weights & arcs
  srzero = xfsm->sr->zero;
  for (qid=n_states_old; qid < n_states; qid++) {
    g_array_index(xfsm->state_final_weight,gfsmWeight,qid) = srzero;
  }

  return n_states;
}

//----------------------------------------
gfsmArcId gfsm_indexed_automaton_reserve_arcs(gfsmIndexedAutomaton *xfsm, gfsmArcId n_arcs)
{
  g_array_set_size(xfsm->arcs,        n_arcs);
  g_array_set_size(xfsm->arcix_lower, n_arcs);
  g_array_set_size(xfsm->arcix_upper, n_arcs);
  return n_arcs;
}


/*======================================================================
 * Methods: Accessors: gfsmAutomaton API: Automaton
 */

//----------------------------------------
gfsmSemiring *gfsm_indexed_automaton_set_semiring(gfsmIndexedAutomaton *xfsm, gfsmSemiring *sr)
{
  if (xfsm->sr) gfsm_semiring_free(xfsm->sr);
  xfsm->sr = gfsm_semiring_copy(sr);
  return sr;
}

//----------------------------------------
void gfsm_indexed_automaton_set_semiring_type(gfsmIndexedAutomaton *xfsm, gfsmSRType srtype)
{
  if (!xfsm->sr) xfsm->sr = gfsm_semiring_new(srtype);
  else if (xfsm->sr->type != srtype) {
    gfsm_semiring_free(xfsm->sr);
    xfsm->sr = gfsm_semiring_new(srtype);  
  }
}

/*======================================================================
 * Methods: Accessors: gfsmAutomaton API: States
 */

//----------------------------------------
gfsmStateId gfsm_indexed_automaton_ensure_state(gfsmIndexedAutomaton *xfsm, gfsmStateId id)
{
  if (id < gfsm_indexed_automaton_n_states(xfsm)) return id;
  return gfsm_indexed_automaton_reserve_states(xfsm,id);
}

//----------------------------------------
void gfsm_indexed_automaton_remove_state(gfsmIndexedAutomaton *xfsm, gfsmStateId id)
{
  g_assert_not_reached();
}

//----------------------------------------
/** Set final weight. \returns (void) */
void gfsm_indexed_automaton_set_final_state_full(gfsmIndexedAutomaton *fsm,
						 gfsmStateId    qid,
						 gboolean       is_final,
						 gfsmWeight     final_weight)
{
  gfsm_indexed_automaton_ensure_state(fsm,qid);
  if (!is_final) final_weight = fsm->sr->zero;
  g_array_index(fsm->state_final_weight,gfsmWeight,qid) = final_weight;
}

//----------------------------------------
/** Lookup final weight. \returns TRUE iff state \a id is final, and sets \a *wp to its final weight. */
gboolean gfsm_indexed_automaton_lookup_final(gfsmIndexedAutomaton *fsm, gfsmStateId qid, gfsmWeight *wp)
{
  *wp = g_array_index(fsm->state_final_weight,gfsmWeight,qid);
  return ((*wp)!=fsm->sr->zero);
}

//----------------------------------------
/** Get number of outgoing arcs. \returns guint */
guint gfsm_indexed_automaton_out_degree(gfsmIndexedAutomaton *fsm, gfsmStateId qid)
{
  if (!gfsm_indexed_automaton_has_state(fsm,qid)) return 0;
  gfsmArcId aid1 = g_array_index(fsm->state_first_arc, gfsmArcId, qid);
  gfsmArcId aid2 = g_array_index(fsm->state_first_arc, gfsmArcId, qid+1);
  return aid2 > aid1 ? (aid2-aid1) : 0;
}
