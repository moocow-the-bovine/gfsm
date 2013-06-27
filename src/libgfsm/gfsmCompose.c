/*=============================================================================*\
 * File: gfsmCompose.c
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
 * Methods: algebra: compose
 */

/*--------------------------------------------------------------
 * compose()
 */
gfsmAutomaton *gfsm_automaton_compose(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  gfsmAutomaton *fsm = gfsm_automaton_compose_full(fsm1,fsm2, NULL,NULL);
  gfsm_automaton_swap(fsm1,fsm);
  gfsm_automaton_free(fsm);
  return fsm1;
}

/*--------------------------------------------------------------*/
/** inner guts for gfsm_automaton_compose_visit_() */
gfsmStateId gfsm_compose_qid_(gfsmAutomaton *fsm, gfsmComposeState sp, GQueue *queue, gfsmComposeStateEnum *spenum, GArray *spenumr)
{
  gfsmStateId qid = gfsm_enum_lookup(spenum,&sp);
  if (qid==gfsmEnumNone) {
    qid = gfsm_automaton_add_state(fsm);
    gfsm_enum_insert_full(spenum, &sp, qid);
    if (qid >= spenumr->len) g_array_set_size(spenumr,qid+1);
    g_array_index(spenumr, gfsmComposeState, qid) = sp;
    g_queue_push_tail(queue, GUINT_TO_POINTER(qid));
  }
  return qid;
}

/*--------------------------------------------------------------
 * compose_visit_()
 */
//#define GFSM_DEBUG_COMPOSE_VISIT 1
#ifdef GFSM_DEBUG_COMPOSE_VISIT
# include <stdio.h>
#endif
void gfsm_automaton_compose_visit_(gfsmStateId	      qid,
				   gfsmAutomaton     *fsm1,
				   gfsmAutomaton     *fsm2,
				   gfsmAutomaton     *fsm,
				   gfsmComposeStateEnum *spenum,
				   GArray               *spenumr,
				   GQueue               *queue,
				   gfsmComposeFlags      flags)
{
  gfsmState   *q1, *q2;
  gfsmComposeState sp = g_array_index(spenumr,gfsmComposeState,qid);
  gfsmStateId qid2;
  gfsmArcList *al1, *al2, *ai1, *ai2;
  gfsmArcList *ai1_noneps, *ai2_noneps, *ai2_continue;
  gfsmArc     *a1,*a2;

#ifdef GFSM_DEBUG_COMPOSE_VISIT
  fprintf(stderr, "compose(): visit : (q%u,f%u,q%u) => q%d\n", sp.id1, sp.idf, sp.id2,
	  (int)(qid==gfsmEnumNone ? -1 : qid));
#endif

  //-- get state pointers for input automata
  q1 = gfsm_automaton_find_state(fsm1,sp.id1);
  q2 = gfsm_automaton_find_state(fsm2,sp.id2);

  //-- sanity check
  if ( !(q1 && q2 && q1->is_valid && q2->is_valid) ) {
#ifdef GFSM_DEBUG_COMPOSE_VISIT
    fprintf(stderr, "compose(): BAD   : (q%u,f%u,q%u)     XXXXX\n", sp.id1, sp.idf, sp.id2);
#endif
    return;
  }

#ifdef GFSM_DEBUG_COMPOSE_VISIT
      fprintf(stderr, "compose(): CREATE: (q%u,f%u,q%u) => q%u ***\n", sp.id1, sp.idf, sp.id2, qid);
#endif

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
    gfsmArcCompData sortdata = { gfsmACUpper,NULL,NULL,NULL };
    al1 = gfsm_arclist_sort(gfsm_arclist_clone(q1->arcs), &sortdata);
  }
  else { al1 = q1->arcs; }

  //-- arcs: sort arclists: fsm2
  if (flags&gfsmCFEfsm2NeedsArcSort) {
    gfsmArcCompData sortdata = { gfsmACLower,NULL,NULL,NULL };
    al2 = gfsm_arclist_sort(gfsm_arclist_clone(q2->arcs), &sortdata);
  }
  else { al2 = q2->arcs; }

  //--------------------------------
  // recusrse: arcs: handle epsilons
  for (ai1_noneps=al1; ai1_noneps!=NULL && ai1_noneps->arc.upper==gfsmEpsilon; ai1_noneps=ai1_noneps->next) {;}
  for (ai2_noneps=al2; ai2_noneps!=NULL && ai2_noneps->arc.lower==gfsmEpsilon; ai2_noneps=ai2_noneps->next) {;}

  //-- (eps,NULL): case fsm1(q1 --a:eps(~eps2)--> q1b), filter:({0,2} --eps2:eps2--> 2), fsm2(q2 --(NULL~eps2:eps)--> q2)
  if (sp.idf != 1) {
    for (ai1=al1; ai1!=ai1_noneps; ai1=ai1->next) {
      a1   = &(ai1->arc);
#ifdef GFSM_DEBUG_COMPOSE_VISIT
      fprintf(stderr,
	      "compose(): MATCH[e,NULL]: (q%u --%d:eps(e2)--> q%u) ~ ({0,2}--(e2:e2)-->2) ~ (q%u --(NULL~e2:eps)--> q%u) ***\n",
	      sp.id1, a1->lower, a1->target,
	      sp.id2, sp.id2);
#endif
      qid2 = gfsm_compose_qid_(fsm, (gfsmComposeState){a1->target, sp.id2, 2}, queue,spenum,spenumr);
      if (qid2 != gfsmNoState)
	gfsm_automaton_add_arc(fsm, qid, qid2, a1->lower, gfsmEpsilon, a1->weight);
    }
  }
  //-- (NULL,eps): case fsm1(q1 --(NULL~eps:eps1)--> q1), filter:({0,1} --eps1:eps1--> 1), fsm2(q2 --eps(~eps1):b--> q2b)
  if (sp.idf != 2) {
    for (ai2=al2; ai2!=ai2_noneps; ai2=ai2->next) {
      a2   = &(ai2->arc);
#ifdef GFSM_DEBUG_COMPOSE_VISIT
      fprintf(stderr,
	      "compose(): MATHC[NULL,e]: (q%u --(NULL~eps:e1)--> q%u) ~ ({0,1}--(e1:e1)-->1) ~ (q%u --eps(e1):%d--> q%u) ***\n",
	      sp.id1, sp.id1,
	      sp.id2, a2->upper, a2->target);
#endif
      qid2 = gfsm_compose_qid_(fsm, (gfsmComposeState){sp.id1, a2->target, 1}, queue,spenum,spenumr);
      if (qid2 != gfsmNoState)
	gfsm_automaton_add_arc(fsm, qid, qid2, gfsmEpsilon, a2->upper, a2->weight);
    }
  }
  //-- (eps,eps): case fsm1(q1 --a:eps(~eps2)--> q1b), filter:({0} --eps2:eps1--> 0), fsm2(q2 --eps:b--> q2b)
  if (sp.idf == 0) {
    for (ai1=al1; ai1!=ai1_noneps; ai1=ai1->next) {
      a1 = &(ai1->arc);
      for (ai2=al2; ai2!=ai2_noneps; ai2=ai2->next) {
	a2   = &(ai2->arc);
#ifdef GFSM_DEBUG_COMPOSE_VISIT
	fprintf(stderr,
		"compose(): MATCH[e,e]: (q%u --%d:eps(e2)--> q%u) ~ ({0}--(e2:e1)-->0) ~ (q%u --eps(e1):%d--> q%u) ***\n",
		sp.id1, a1->lower, a1->target,
		sp.id2, a2->upper, a2->target);
#endif
	qid2 = gfsm_compose_qid_(fsm, (gfsmComposeState){a1->target, a2->target, 0}, queue,spenum,spenumr);
	if (qid2 != gfsmNoState)
	  gfsm_automaton_add_arc(fsm, qid, qid2, a1->lower, a2->upper,
				 gfsm_sr_times(fsm->sr, a1->weight, a2->weight));
      }
    }
  }

  //--------------------------------
  // recurse: arcs: non-eps: iterate
  for (ai1=ai1_noneps, ai2_continue=ai2_noneps; ai1!=NULL; ai1=ai1->next) {
    a1 = &(ai1->arc);

    for (ai2=ai2_continue; ai2!=NULL; ai2=ai2->next) {
      a2 = &(ai2->arc);

#ifdef GFSM_DEBUG_COMPOSE_VISIT
      fprintf(stderr,
	      "compose(): check[x,x]: (q%u --%d:%d--> q%u) ~ ({0,1,2}--(x:x)-->0) ~ (q%u --%d:%d--> q%u)\n",
	      sp.id1, a1->lower, a1->upper, a1->target,
	      sp.id2, a2->lower, a2->upper, a2->target);
#endif

      if      (a2->lower < a1->upper) { ai2_continue=ai2->next; continue; }
      else if (a2->lower > a1->upper) { break; }

#ifdef GFSM_DEBUG_COMPOSE_VISIT
      fprintf(stderr,
	      "compose(): MATCH[x,x]: (q%u --%d:%d--> q%u) ~ ({0,1,2}--(x:x)-->0) ~ (q%u --%d:%d--> q%u) ***\n",
	      sp.id1, a1->lower, a1->upper, a1->target,
	      sp.id2, a2->lower, a2->upper, a2->target);
#endif

      //-- non-eps: case fsm1:(q1 --a:b--> q1'), fsm2:(q2 --b:c-->  q2')
      qid2 = gfsm_compose_qid_(fsm, (gfsmComposeState){a1->target, a2->target, 0}, queue,spenum,spenumr);
      if (qid2 != gfsmNoState)
	gfsm_automaton_add_arc(fsm, qid, qid2, a1->lower, a2->upper,
			       gfsm_sr_times(fsm1->sr, a1->weight, a2->weight));
    }
  }

  //-- maybe cleanup temporary arc-lists
  if (flags&gfsmCFEfsm1NeedsArcSort) gfsm_arclist_free(al1);
  if (flags&gfsmCFEfsm2NeedsArcSort) gfsm_arclist_free(al2);

  return;
}

/*--------------------------------------------------------------
 * compose_full()
 */
//#define GFSM_DEBUG_COMPOSE
#ifdef GFSM_DEBUG_COMPOSE
# include <gfsmAutomatonIO.h>
#endif
gfsmAutomaton *gfsm_automaton_compose_full(gfsmAutomaton *fsm1,
					   gfsmAutomaton *fsm2,
					   gfsmAutomaton *composition,
					   gfsmComposeStateEnum *spenum
					   )
{
  gboolean          spenum_is_temp;
  gfsmComposeState  rootpair;
  gfsmStateId       rootid = 0;
  gfsmComposeFlags  flags = 0;
  GQueue	   *queue = NULL;
  GArray           *spenumr = NULL;
#ifdef GFSM_DEBUG_COMPOSE
  gfsmError *err =NULL;
#endif

  //-- setup: output fsm
  if (!composition) {
    composition=gfsm_automaton_shadow(fsm1);
  } else {
    gfsm_automaton_clear(composition);
    gfsm_automaton_copy_shallow(composition,fsm1);
  }
  composition->flags.sort_mode     = gfsmASMNone;
  composition->flags.is_transducer = 1;

  //-- setup: ComposeStateEnum
  if (spenum==NULL) {
    spenum_is_temp=TRUE;
    spenum = gfsm_compose_state_enum_new();
  } else {
    spenum_is_temp=FALSE;
    gfsm_enum_clear(spenum);
  }
  spenumr = g_array_sized_new(FALSE,FALSE,sizeof(gfsmComposeState),gfsmAutomatonDefaultSize);
  spenumr->len = 1;

  //-- setup: flags
  if (gfsm_acmask_nth(fsm1->flags.sort_mode,0) != gfsmACUpper) flags |= gfsmCFEfsm1NeedsArcSort;
  if (gfsm_acmask_nth(fsm2->flags.sort_mode,0) != gfsmACLower) flags |= gfsmCFEfsm2NeedsArcSort;

  //-- setup: queue
  queue = g_queue_new();

  //-- guts: recursively visit states depth-first from root
  rootpair.id1 = fsm1->root_id;
  rootpair.id2 = fsm2->root_id;
  rootpair.idf = 0;
  gfsm_automaton_ensure_state(composition,rootid);
  gfsm_enum_insert_full(spenum, &rootpair, rootid);
  g_array_index(spenumr, gfsmComposeState, rootid) = rootpair;
  g_queue_push_tail(queue, GUINT_TO_POINTER(rootid));

  while (!g_queue_is_empty(queue)) {
    gfsmStateId qid = GPOINTER_TO_UINT(g_queue_pop_head(queue));
    gfsm_automaton_compose_visit_(qid, fsm1,fsm2,composition, spenum,spenumr,queue, flags);
  }

  //-- finalize: set new root state
  if (rootid != gfsmNoState) {
    gfsm_automaton_set_root(composition, rootid);
  } else {
    composition->root_id = gfsmNoState;
  }
  //-- cleanup
  if (spenum_is_temp) gfsm_enum_free(spenum);
  g_array_free(spenumr,TRUE);
  g_queue_free(queue);

  return composition;
}
