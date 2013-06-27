/*=============================================================================*\
 * File: gfsmReplace.c
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
 * Methods: algebra: replace
 */

/*--------------------------------------------------------------
 * replace()
 */
gfsmAutomaton *gfsm_automaton_replace(gfsmAutomaton *fsm1, gfsmLabelVal lo, gfsmLabelVal hi, gfsmAutomaton *fsm2)
{
  gfsmStateId id;
  gfsmArcIter ai;
  gfsmStateId nstates = fsm1->states->len;

  for (id=0; id < nstates; id++) {
    if (!gfsm_automaton_has_state(fsm1,id)) continue;
    for (gfsm_arciter_open(&ai,fsm1,id), gfsm_arciter_seek_both(&ai,lo,hi);
	 gfsm_arciter_ok(&ai);
	 gfsm_arciter_seek_both(&ai,lo,hi))
      {
	gfsmArc *a = gfsm_arciter_arc(&ai);
	gfsm_automaton_insert_automaton(fsm1, id, a->target, fsm2, a->weight);
	gfsm_arciter_remove(&ai); //-- implies gfsm_arciter_next()
      }
    //gfsm_arciter_close(&ai);
  }

  return fsm1;
}

/*--------------------------------------------------------------
 * insert_automaton()
 */
gfsmAutomaton *gfsm_automaton_insert_automaton(gfsmAutomaton *fsm1,
					       gfsmStateId    q1from,
					       gfsmStateId    q1to,
					       gfsmAutomaton *fsm2,
					       gfsmWeight     w)
{
  gfsmStateId offset;
  gfsmStateId size2;
  gfsmStateId id2;
  gfsmStateId      id1;
  const gfsmState *s2;
  gfsmState       *s1;
  gfsmArcIter      ai;
  gfsmWeight       s2fw;

  //-- reserve size
  offset = fsm1->states->len;
  size2  = fsm2->states->len;
  gfsm_automaton_reserve(fsm1, offset + size2);

  //-- avoid "smart" arc-insertion
  fsm1->flags.sort_mode = gfsmASMNone;

  //-- adopt states from fsm2 into fsm1
  for (id2 = 0; id2 < size2; id2++) {

    s2 = gfsm_automaton_find_state_const(fsm2,id2);
    id1 = id2+offset;
    s1 = gfsm_automaton_find_state(fsm1, id1);

    //-- sanity check(s)
    if (!s1 || !s2 || !s2->is_valid) continue;

    //-- copy state
    gfsm_state_copy(s1,s2);

    //-- translate targets for adopted arcs
    for (gfsm_arciter_open_ptr(&ai,fsm1,s1); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai))
      {
	gfsmArc *a = gfsm_arciter_arc(&ai);
	a->target += offset;
      }

    //-- check for fsm2-final states: get weight & add arc to our sink state
    if (gfsm_weightmap_lookup(fsm2->finals, GUINT_TO_POINTER(id2), &s2fw)) {
      s1->is_final = FALSE;
      gfsm_automaton_add_arc(fsm1,id1,q1to,gfsmEpsilon,gfsmEpsilon, s2fw);
    }
  }

  //-- add arc to new state
  gfsm_automaton_add_arc(fsm1, q1from, fsm2->root_id+offset, gfsmEpsilon, gfsmEpsilon, w);

  return fsm1;
}
