/*=============================================================================*\
 * File: gfsmReverse.c
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
 * Methods: algebra: reverse
 */

/*--------------------------------------------------------------
 * reverse()
 */
gfsmAutomaton *gfsm_automaton_reverse(gfsmAutomaton *fsm)
{
  gfsmStateId new_root = gfsm_automaton_add_state(fsm);
  gfsmStateId id;
  gfsmState   *s, *ts;
  gfsmArcList *al, *al_next, *al_prev;
  gfsmWeight   w;
  //gfsmArcSortMode sm = gfsm_automaton_sortmode(fsm);

  //-- mark automaton as unsorted (avoid "smart" arc-insertion)
  fsm->flags.sort_mode = gfsmASMNone;

  //-- reverse arc directions, keeping old "source" and "target" values
  //   intact as sentinels
  for (id = 0; id < new_root; id++) {
    s = gfsm_automaton_find_state(fsm,id);
    if (!s || !s->is_valid) continue;

    //-- check for old final states
    if (gfsm_automaton_lookup_final(fsm,id,&w)) {
      s->is_final = FALSE;
      gfsm_weightmap_remove(fsm->finals, GUINT_TO_POINTER(id));
      gfsm_automaton_add_arc(fsm, new_root, id, gfsmEpsilon, gfsmEpsilon, w);
    }

    //-- reverse arcs
    for (al_prev=NULL, al=s->arcs; al != NULL; al=al_next) {
      gfsmArc *a = gfsm_arclist_arc(al);
      al_next    = al->next;
      if (a->target==id) {
	//-- already reversed (or a single-arc loop, which doesn't need reversal)
	al_prev = al;
	continue; 
      }

      //-- steal arc
      if (al_prev) al_prev->next = al->next;
      else               s->arcs = al->next;
      al->next = NULL;

      //-- move arc
      ts = gfsm_automaton_find_state(fsm,a->target);
      gfsm_automaton_add_arc_node(fsm, ts, al);
    }
  }

  //-- sanitize: swap 'source' and 'target' fields
  for (id=0; id < new_root; id++) {
    gfsmArcIter ai;
    for (gfsm_arciter_open(&ai,fsm,id); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc     *a  = gfsm_arciter_arc(&ai);
      gfsmStateId tmp = a->target;
      a->target = a->source;
      a->source = tmp;
    }
  }

  //-- root flop
  gfsm_automaton_set_final_state_full(fsm,fsm->root_id,TRUE,fsm->sr->one);
  fsm->root_id = new_root;

  return fsm;
}
