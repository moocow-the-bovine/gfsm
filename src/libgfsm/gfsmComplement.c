/*=============================================================================*\
 * File: gfsmComplement.c
 * Author: Bryan Jurish <moocow.bovine@gmail.com>
 * Description: finite state machine library
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

#include <glib.h>
#include <gfsmAlgebra.h>
#include <gfsmAssert.h>
#include <gfsmArcIter.h>
#include <gfsmEnum.h>
#include <gfsmUtils.h>
#include <gfsmCompound.h>

/*======================================================================
 * Methods: algebra: complement
 */

/*--------------------------------------------------------------
 * complete()
 */
gfsmAutomaton *gfsm_automaton_complete(gfsmAutomaton *fsm, gfsmAlphabet *alph, gfsmStateId *sinkp)
{
  gfsmStateId  id, sinkid;
  GPtrArray    *alabels;

  if (!fsm->flags.is_deterministic) fsm = gfsm_automaton_determinize(fsm);
  if (gfsm_acmask_nth(fsm->flags.sort_mode,0) != gfsmACLower) {
    gfsm_automaton_arcsort(fsm,gfsmACLower);
  }
  //-- avoid "smart" arc insertion
  fsm->flags.sort_mode = gfsmASMNone;

  //-- add sink-id
  sinkid = gfsm_automaton_add_state(fsm);
  if (sinkp) *sinkp = sinkid;

  //-- get alphabet label-vector
  alabels = g_ptr_array_sized_new(gfsm_alphabet_size(alph));
  gfsm_alphabet_labels_to_array(alph,alabels);

  for (id = 0; id < fsm->states->len; id++) {
    gfsmState    *s = gfsm_automaton_find_state(fsm,id);
    gfsmArcList *al;
    gfsmArc      *a;
    guint       labi;
    if (!s || !s->is_valid) continue;

    al = s->arcs;
    a  = gfsm_arclist_arc(al);
    for (labi=0; labi < alabels->len; ) {
      gfsmLabelVal lab = (gfsmLabelVal)GPOINTER_TO_UINT(g_ptr_array_index(alabels,labi));

      if (lab==gfsmEpsilon) {
	++labi;
      }
      else if (!a || a->lower > lab) {
	//-- no arc for this label: route it to sink
	gfsm_automaton_add_arc(fsm, id, sinkid, lab, lab, fsm->sr->one);
	++labi;
      }
      else if (a->lower == lab) {
	++labi;
      }
      else {
	while (al != NULL && a->lower < lab) {
	  al = al->next;
	  a  = gfsm_arclist_arc(al);
	}
      }
    }
  }

  //-- mark fsm as (still) deterministic
  fsm->flags.is_deterministic = TRUE;

  //-- cleanup
  //g_array_free(alabels,TRUE);
  g_ptr_array_free(alabels,TRUE);

  return fsm;
}

/*--------------------------------------------------------------
 * complement_full()
 */
gfsmAutomaton *gfsm_automaton_complement_full(gfsmAutomaton *fsm, gfsmAlphabet *alph)
{
  gfsmStateId id, sink_id;
  gfsm_automaton_complete(fsm, alph, &sink_id);

  //-- flip final states (no weights here)
  for (id = 0; id < fsm->states->len; id++) {
    gfsmState  *s = gfsm_automaton_find_state(fsm,id);
    if (!s || !s->is_valid) continue;
    gfsm_automaton_set_final_state(fsm, id, !s->is_final);
  }

  return fsm;
}

/*--------------------------------------------------------------
 * complement()
 */
gfsmAutomaton *gfsm_automaton_complement(gfsmAutomaton *fsm)
{
  gfsmAlphabet *alph = gfsm_identity_alphabet_new();
  gfsm_automaton_get_alphabet(fsm, gfsmLSLower, alph);
  gfsm_automaton_complement_full(fsm,alph);
  gfsm_alphabet_free(alph);
  return fsm;
}
