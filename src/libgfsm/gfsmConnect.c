/*=============================================================================*\
 * File: gfsmConnect.c
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
 * Methods: algebra: connect
 */

/*--------------------------------------------------------------
 * connect()
 */
gfsmAutomaton *gfsm_automaton_connect(gfsmAutomaton *fsm)
{
  gfsmBitVector *wanted;

  //-- sanity check
  if (!fsm || gfsm_automaton_n_states(fsm)==0) return fsm;

  wanted = gfsm_bitvector_sized_new(fsm->states->len);
  gfsm_automaton_connect_fw(fsm, wanted);

  gfsm_bitvector_zero(wanted);
  gfsm_automaton_connect_bw(fsm, NULL, wanted);

  gfsm_bitvector_free(wanted);
  return fsm;
}


/*--------------------------------------------------------------
 * connect_fw_visit_state()
 *  + marks all states on a path from (id) in (visited)
 */
void gfsm_connect_fw_visit_state(gfsmAutomaton *fsm,
				 gfsmStateId    id,
				 gfsmBitVector *visited)
{
  gfsmState *s;
  gfsmArcIter ai;

  //-- already visited
  if (gfsm_bitvector_get(visited,id)) return;

  s = gfsm_automaton_find_state(fsm,id);
  if (!s || !s->is_valid) return;                    //-- ignore invalid states

  //-- mark node as visited on this path
  gfsm_bitvector_set(visited,id,1);

  //-- visit targets of outgoing arcs
  for (gfsm_arciter_open_ptr(&ai,fsm,s); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
    gfsm_connect_fw_visit_state(fsm, gfsm_arciter_arc(&ai)->target, visited);
  }

  return;
}

/*--------------------------------------------------------------
 * connect_fw()
 */
gfsmAutomaton *gfsm_automaton_connect_fw(gfsmAutomaton *fsm, gfsmBitVector *visited)
{
  gboolean  visited_is_temp = FALSE;

  //-- sanity check
  if (!fsm || fsm->root_id == gfsmNoState)
    return gfsm_automaton_prune_states(fsm,NULL);

  //-- traversal record
  if (visited==NULL) {
    visited = gfsm_bitvector_sized_new(fsm->states->len);
    visited_is_temp = TRUE;
  }

  //-- traverse
  gfsm_connect_fw_visit_state(fsm, fsm->root_id, visited);
  gfsm_automaton_prune_states(fsm, visited);

  //-- cleanup
  if (visited_is_temp) gfsm_bitvector_free(visited);

  return fsm;
}

/*--------------------------------------------------------------
 * connect_bw(): final_foreach()
 */
struct gfsm_connect_bw_data_ {
  gfsmAutomaton *fsm;
  GPtrArray     *rarcs;
  gfsmBitVector *finalizable;
};

gboolean gfsm_connect_bw_visit_state(gfsmStateId id,
				     gpointer    pw,
				     struct      gfsm_connect_bw_data_ *data)
{
  GSList *rl;

  //-- already visited
  if (gfsm_bitvector_get(data->finalizable,id)     //-- already visited?
      || !gfsm_automaton_has_state(data->fsm, id)) //-- bad state?
    return FALSE;                                  //-----> continue traversal

  //-- mark state as finalizable
  gfsm_bitvector_set(data->finalizable,id,1);

  //-- visit sources of incoming arcs
  for (rl=g_ptr_array_index(data->rarcs,id); rl != NULL; rl=rl->next) {
    gfsmArc *arc = (gfsmArc*)rl->data;
    gfsm_connect_bw_visit_state(arc->source,pw,data);
  }

  return FALSE; //-- continue traversal
}

/*--------------------------------------------------------------
 * connect_bw()
 */
gfsmAutomaton *gfsm_automaton_connect_bw(gfsmAutomaton       *fsm,
					 gfsmReverseArcIndex *rarcs,
					 gfsmBitVector       *finalizable)
{
  gboolean rarcs_is_temp = FALSE;
  gboolean finalizable_is_temp = FALSE;
  struct gfsm_connect_bw_data_ data = {fsm,rarcs,finalizable};

  //-- sanity check(s)
  if (!fsm || gfsm_automaton_n_final_states(fsm)==0)
    return gfsm_automaton_prune_states(fsm,NULL);

  //-- reverse arc-index
  if (rarcs==NULL) {
    rarcs = data.rarcs = gfsm_automaton_reverse_arc_index(fsm,NULL);
    rarcs_is_temp = TRUE;
  }

  //-- traversal record
  if (finalizable==NULL) {
    finalizable = data.finalizable = gfsm_bitvector_sized_new(fsm->states->len);
    finalizable_is_temp = TRUE;
  }

  //-- traverse
  gfsm_automaton_finals_foreach(fsm, (GTraverseFunc)gfsm_connect_bw_visit_state,  &data);
  gfsm_automaton_prune_states(fsm, finalizable);

  //-- cleanup
  if (finalizable_is_temp) gfsm_bitvector_free(finalizable);
  if (rarcs_is_temp) gfsm_reverse_arc_index_free(rarcs,TRUE);

  return fsm;
}


/*--------------------------------------------------------------
 * prune_states()
 */
gfsmAutomaton *gfsm_automaton_prune_states(gfsmAutomaton *fsm, gfsmBitVector *wanted)
{
  gfsmStateId id, maxwanted=gfsmNoState;
  gfsmArcIter ai;

  for (id=0; id < fsm->states->len; id++) {
    if (!wanted || !gfsm_bitvector_get(wanted,id)) {
      //-- unwanted state: chuck it
      gfsm_automaton_remove_state(fsm,id);
    }
    else {
      maxwanted = id;
      //-- prune outgoing arcs to any unwanted states, too
      for (gfsm_arciter_open(&ai, fsm, id); gfsm_arciter_ok(&ai); ) {
	gfsmArc *arc = gfsm_arciter_arc(&ai);
	if (!wanted || !gfsm_bitvector_get(wanted,arc->target)) {
	  gfsm_arciter_remove(&ai);
	} else {
	  gfsm_arciter_next(&ai);
	}
      }
    }
  }

  //-- update number of states
  if (maxwanted != gfsmNoState) {
    g_array_set_size(fsm->states, maxwanted+1);
  } else {
    g_array_set_size(fsm->states, 0);
  }

  return fsm;
}
