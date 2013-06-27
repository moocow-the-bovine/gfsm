/*=============================================================================*\
 * File: gfsmUnion.c
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
 * Methods: algebra: union
 */

/*--------------------------------------------------------------
 * union_()
 */
gfsmAutomaton *gfsm_automaton_union(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  gfsmStateId offset;
  gfsmStateId id2;
  gfsmStateId oldroot1;
  gfsmArcCompData sortdata = {0,0,0,0};

  //-- sanity check
  if (!fsm2 || fsm2->root_id==gfsmNoState) return fsm1;

  offset = fsm1->states->len + 1;
  gfsm_automaton_reserve(fsm1, offset + fsm2->states->len);


  //-- add new root and eps-arc to old root for fsm1
  oldroot1 = fsm1->root_id;
  fsm1->root_id = gfsm_automaton_add_state(fsm1);
  if (oldroot1 != gfsmNoState) {
    gfsm_automaton_add_arc(fsm1, fsm1->root_id, oldroot1, gfsmEpsilon, gfsmEpsilon, fsm1->sr->one);
  }

  //-- avoid "smart" arc-insertion (temporary)
  sortdata.mask = fsm1->flags.sort_mode;
  sortdata.sr   = fsm1->sr;
  fsm1->flags.sort_mode = gfsmASMNone;

  //-- adopt states from fsm2 into fsm1
  for (id2 = 0; id2 < fsm2->states->len; id2++) {
    const gfsmState *s2 = gfsm_automaton_find_state_const(fsm2,id2);
    gfsmState       *s1 = gfsm_automaton_find_state(fsm1,id2+offset);
    gfsmArcIter      ai;
    gfsm_state_copy(s1,s2);
    for (gfsm_arciter_open_ptr(&ai, fsm1, s1); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      a->target += offset;
    }
    //-- index final states from @fsm2
    if (s2->is_final) {
      gfsm_automaton_set_final_state_full(fsm1, id2+offset, TRUE, gfsm_automaton_get_final_weight(fsm2, id2));
    }
    //-- maybe sort new arcs
    if (sortdata.mask != gfsmASMNone
	&& (fsm2->flags.sort_mode != sortdata.mask
	    || (sortdata.mask == gfsmASMWeight && fsm2->sr->type != fsm1->sr->type)))
      {
	s1->arcs = gfsm_arclist_sort(s1->arcs, &sortdata);
      }
  }

  //-- re-instate "smart" arc-insertion
  fsm1->flags.sort_mode = sortdata.mask;

  //-- add epsilon arc to translated root(fsm2) in fsm1
  gfsm_automaton_add_arc(fsm1,
			 fsm1->root_id,
			 offset + fsm2->root_id,
			 gfsmEpsilon,
			 gfsmEpsilon,
			 fsm1->sr->one);

  return fsm1;
}
