
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

  xfsm->flags   = flags;
  xfsm->sr      = gfsm_semiring_new(srtype);
  xfsm->root_id = gfsmNoState;

  xfsm->n_states_alloc     = n_states;
  xfsm->n_arcs_alloc       = n_arcs;

  xfsm->state_final_weight = g_array_sized_new(FALSE, FALSE, sizeof(gfsmWeight), n_states);
  xfsm->arcs               = g_array_sized_new(FALSE, TRUE,  sizeof(gfsmArc),    n_arcs);
  xfsm->state_first_arc    = g_array_sized_new(FALSE, FALSE, sizeof(gfsmArcId),  n_states+1);

  xfsm->arcix_lower        = g_array_sized_new(FALSE, FALSE, sizeof(gfsmArcId),  n_arcs);
  xfsm->arcix_upper        = g_array_sized_new(FALSE, FALSE, sizeof(gfsmArcId),  n_arcs);

  xfsm->state_final_weight->len = 0;
  xfsm->arcs->len = 0;
  xfsm->state_first_arc->len = 0;
  xfsm->arcix_lower->len = 0;
  xfsm->arcix_upper->len = 0;

  return xfsm;
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
  g_assert_not_reached();
  return NULL;
}

//----------------------------------------
gfsmAutomaton *gfsm_indexed_to_automaton(gfsmAutomaton *fsm, gfsmIndexedAutomaton *xfsm)
{
  g_assert_not_reached();
  return NULL;
}

/*======================================================================
 * Methods: Accessors: gfsmIndexedAutomaton
 */

//----------------------------------------
void gfsm_indexed_automaton_reserve_states(gfsmIndexedAutomaton *xfsm, gfsmStateId n_states)
{
  gfsmStateId n_states_old = gfsm_indexed_automaton_n_states(xfsm);

  //-- resize state-indexed arrays
  g_array_set_size(xfsm->state_final_weight, n_states);
  g_array_set_size(xfsm->state_first_arc,    n_states+1);

  //-- ... and maybe re-set their lengths
  if (n_states > n_states_old) {
    xfsm->state_final_weight->len =  n_states_old;
    xfsm->state_first_arc->len    =  n_states_old+1;
  }
}

//----------------------------------------
void gfsm_indexed_automaton_reserve_arcs(gfsmIndexedAutomaton *xfsm, gfsmArcId n_arcs)
{
  gfsmArcId n_arcs_old = xfsm->arcs->len;

  g_array_set_size(xfsm->arcs,        n_arcs);
  g_array_set_size(xfsm->arcix_lower, n_arcs);
  g_array_set_size(xfsm->arcix_upper, n_arcs);

  if (n_arcs > n_arcs_old) {
    xfsm->arcs->len = n_arcs_old;
    xfsm->arcix_lower = n_arcs_old;
    xfsm->arcix_upper = n_arcs_old;
  }
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
void gfsm_indexed_automaton_remove_state(gfsmIndexedAutomaton *xfsm, gfsmStateId id)
{
  if (!gfsm_indexed_automaton_has_state(xfsm,id)) return;
  g_array_index(xfsm->state_first_arc,gfsmArcId,id) = gfsmNoArc;
}

//----------------------------------------
/** Set final weight. \returns (void) */
void gfsm_indexed_automaton_set_final_state_full(gfsmIndexedAutomaton *fsm,
						 gfsmStateId    id,
						 gboolean       is_final,
						 gfsmWeight     final_weight)
{
  //-- CONTINUE HERE: ensure_state? ensure_arc?
  g_array_index(xfsm->state_first_arc,gfsmArcId,id) = gfsmNoArc;
}

//----------------------------------------
/** Lookup final weight. \returns TRUE iff state \a id is final, and sets \a *wp to its final weight. */
gboolean gfsm_indexed_automaton_lookup_final(gfsmIndexedAutomaton *fsm, gfsmStateId id, gfsmWeight *wp);

//----------------------------------------
/** Get number of outgoing arcs. \returns guint */
guint gfsm_indexed_automaton_out_degree(gfsmIndexedAutomaton *fsm, gfsmStateId id);

