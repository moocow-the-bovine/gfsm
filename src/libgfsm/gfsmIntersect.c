/*=============================================================================*\
 * File: gfsmIntersect.c
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
 * Methods: algebra: intersection
 */

/*--------------------------------------------------------------
 * intersect()
 */
gfsmAutomaton *gfsm_automaton_intersect(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  gfsmAutomaton *fsm = gfsm_automaton_intersect_full(fsm1,fsm2,NULL,NULL);
  gfsm_automaton_swap(fsm1,fsm);
  gfsm_automaton_free(fsm);
  return fsm1;
}

/*--------------------------------------------------------------
 * intersect_full()
 */
gfsmAutomaton *gfsm_automaton_intersect_full(gfsmAutomaton *fsm1,
					     gfsmAutomaton *fsm2,
					     gfsmAutomaton *intersect,
					     gfsmStatePairEnum *spenum)
{
  gboolean      spenum_is_temp;
  gfsmStatePair rootpair;
  gfsmStateId   rootid;
  gfsmComposeFlags flags = 0;

  //-- setup: output fsm
  if (!intersect) {
    intersect=gfsm_automaton_shadow(fsm1);
  } else {
    gfsm_automaton_clear(intersect);
    gfsm_automaton_copy_shallow(intersect,fsm1);
  }
  //-- avoid "smart" arc-insertion
  intersect->flags.sort_mode     = gfsmASMNone;
  intersect->flags.is_transducer = 0;

  //-- setup: StatePairEnum
  if (spenum==NULL) {
    spenum_is_temp=TRUE;
    spenum = gfsm_statepair_enum_new();
  } else {
    spenum_is_temp=FALSE;
    gfsm_enum_clear(spenum);
  }

  //-- setup: flags
  if (gfsm_acmask_nth(fsm1->flags.sort_mode,0) != gfsmACLower) flags |= gfsmCFEfsm1NeedsArcSort;
  if (gfsm_acmask_nth(fsm2->flags.sort_mode,0) != gfsmACLower) flags |= gfsmCFEfsm2NeedsArcSort;

  //-- guts
  rootpair.id1 = fsm1->root_id;
  rootpair.id2 = fsm2->root_id;
  rootid = gfsm_automaton_intersect_visit_(rootpair, fsm1, fsm2, intersect, spenum,flags);

  //-- finalize: set root state
  if (rootid != gfsmNoState) {
    gfsm_automaton_set_root(intersect, rootid);
  } else {
    intersect->root_id = gfsmNoState;
  }

  //-- cleanup
  if (spenum_is_temp) gfsm_enum_free(spenum);

  return intersect;
}

/*--------------------------------------------------------------
 * intersect_visit()
 */
gfsmStateId gfsm_automaton_intersect_visit_(gfsmStatePair sp,
					    gfsmAutomaton *fsm1,
					    gfsmAutomaton *fsm2,
					    gfsmAutomaton *fsm,
					    gfsmStatePairEnum *spenum,
					    gfsmComposeFlags flags)
{
  gfsmState   *q1, *q2;
  gfsmStateId qid = gfsm_enum_lookup(spenum,&sp);
  gfsmStateId qid2;
  gfsmArcList *al1, *al2, *ai1, *ai2, *ai2eps;
  gfsmArc     *a1,*a2;

  //-- ignore already-visited states
  if (qid != gfsmEnumNone) return qid;

  //-- get state pointers for input automata
  q1 = gfsm_automaton_find_state(fsm1,sp.id1);
  q2 = gfsm_automaton_find_state(fsm2,sp.id2);

  //-- sanity check
  if ( !(q1 && q2 && q1->is_valid && q2->is_valid) ) return gfsmNoState;

  //-- insert new state into output automaton
  qid = gfsm_automaton_add_state(fsm);
  gfsm_enum_insert_full(spenum,&sp,qid);
  //q   = gfsm_automaton_get_state(fsm,qid);

  //-- check for final states
  if (q1->is_final && q2->is_final) {
    gfsm_automaton_set_final_state_full(fsm,qid,TRUE,
					gfsm_sr_times(fsm->sr,
						      gfsm_automaton_get_final_weight(fsm1,sp.id1),
						      gfsm_automaton_get_final_weight(fsm2,sp.id2)));
  }

  //-------------------------------------------
  // recurse on outgoing arcs

  //--------------------------------
  // recurse: arcs: sort

  //-- arcs: sort arclists: fsm1
  if (flags&gfsmCFEfsm1NeedsArcSort) {
    gfsmArcCompData sortdata = { (gfsmACLower|(gfsmACUpper<<gfsmACShift)),NULL,NULL,NULL };
    al1 = gfsm_arclist_sort(gfsm_arclist_clone(q1->arcs), &sortdata);
  }
  else { al1 = q1->arcs; }

  //-- arcs: sort arclists: fsm2
  if (flags&gfsmCFEfsm2NeedsArcSort) {
    gfsmArcCompData sortdata = { (gfsmACLower|(gfsmACUpper<<gfsmACShift)),NULL,NULL,NULL };
    al2 = gfsm_arclist_sort(gfsm_arclist_clone(q2->arcs), &sortdata);
  }
  else { al2 = q2->arcs; }

  //--------------------------------
  // recurse: arcs: iterate
  for (ai1=al1, ai2=al2; ai1 != NULL; ai1=ai1->next) {
    a1 = &(ai1->arc);
    if (a1->lower == gfsmEpsilon) {
      //-- handle epsilon arcs

      //-- eps: case fsm1:(q1 --eps-->  q1'), fsm2:(q2)
      qid2 = gfsm_automaton_intersect_visit_((gfsmStatePair){a1->target,sp.id2},
						fsm1, fsm2, fsm, spenum, flags);
      if (qid2 != gfsmNoState)
	gfsm_automaton_add_arc(fsm, qid, qid2, gfsmEpsilon, gfsmEpsilon, a1->weight);

      //-- eps: case fsm1:(q1 --eps-->  q1'), fsm2:(q2 --eps-->  q2')
      for (ai2eps=al2; ai2eps != NULL; ai2eps=ai2eps->next) {
	a2 = &(ai2eps->arc);
	if (a2->lower != gfsmEpsilon) break;

	qid2 = gfsm_automaton_intersect_visit_((gfsmStatePair){a1->target,a2->target},
						  fsm1, fsm2, fsm, spenum, flags);
	if (qid2 != gfsmNoState)
	  gfsm_automaton_add_arc(fsm, qid, qid2, gfsmEpsilon, gfsmEpsilon,
				 gfsm_sr_times(fsm1->sr, a1->weight, a2->weight));
      }
    }
    else {
      //-- handle non-epsilon arcs
      for ( ; ai2 != NULL; ai2=ai2->next) {
	a2 = &(ai2->arc);

	if      (a2->lower < a1->lower) continue;
	else if (a2->lower > a1->lower) break;

	qid2 = gfsm_automaton_intersect_visit_((gfsmStatePair){a1->target,a2->target},
						  fsm1, fsm2, fsm, spenum, flags);
	if (qid2 != gfsmNoState)
	  gfsm_automaton_add_arc(fsm, qid, qid2, a1->lower, a1->lower,
				 gfsm_sr_times(fsm1->sr, a1->weight, a2->weight));
      }
    }
  }

  //-- handle epsilon-arcs on fsm2
  for (ai2=al2 ; ai2 != NULL; ai2=ai2->next) {
    a2 = &(ai2->arc);
    if (a2->lower != gfsmEpsilon) break;

    //-- eps: case fsm1:(q1), fsm2:(q2 --eps-->  q2')
    qid2 = gfsm_automaton_intersect_visit_((gfsmStatePair){sp.id1,a2->target},
					   fsm1, fsm2, fsm, spenum, flags);
    if (qid2 != gfsmNoState)
      gfsm_automaton_add_arc(fsm, qid, qid2, gfsmEpsilon, gfsmEpsilon, a2->weight);
  }

  //-- cleanup
  if (flags&gfsmCFEfsm1NeedsArcSort) gfsm_arclist_free(al1);
  if (flags&gfsmCFEfsm2NeedsArcSort) gfsm_arclist_free(al2);

  return qid;
}
