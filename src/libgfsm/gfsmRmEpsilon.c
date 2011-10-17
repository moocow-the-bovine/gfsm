/*=============================================================================*\
 * File: gfsmRmEpsilon.c
 * Author: Bryan Jurish <moocow.bovine@gmail.com>
 * Description: finite state machine library
 *
 * Copyright (c) 2004-2011 Bryan Jurish.
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

#include <glib.h>
#include <gfsmAlgebra.h>
#include <gfsmAssert.h>
#include <gfsmArcIter.h>
#include <gfsmStateSet.h>
#include <gfsmEnum.h>
#include <gfsmUtils.h>
#include <gfsmCompound.h>

#include <gfsmPQueue.h>

/*======================================================================
 * Methods: algebra: rmepsilon
 */

#if 1
//-- implementation of algorithm from hh2010: (Hanneforth & de la Higuera, 2010)

//--------------------------------------------------------------
// + arc comparison from hh2010
static
int gfsm_rmeps_arc_compare(const gfsmArc *a, const gfsmArc *b, gpointer user_data)
{
  const gfsmStateId p=a->source, q=a->target, p1=b->source, q1=b->target;
  if (
      (   p==q && p1==q1 && q>q1) //-- hh2010 eq (3)
      || (p!=q && p1!=q1 && q>q1) //-- hh2010 eq (4)
      || (p==q && p1!=q1)         //-- hh2010 eq (5)
      )
    return -1;
  return 0;
}


//--------------------------------------------------------------
gfsmAutomaton *gfsm_automaton_rmepsilon(gfsmAutomaton *fsm)
{
  //-- variables
  gfsmPriorityQueue *pqueue;
  gfsmStateId qid;
  gfsmArcIter ai, aai;
  gfsmWeight westar, fw_p=0, fw_q=0;

  //-- setup priority queue (hh2010:1-2)
  pqueue = gfsm_pqueue_new((GCompareDataFunc)gfsm_rmeps_arc_compare, NULL);
  for (qid=0; qid < fsm->states->len; qid++) {
    for (gfsm_arciter_open(&ai, fsm, qid), gfsm_arciter_seek_both(&ai,gfsmEpsilon,gfsmEpsilon);
	 gfsm_arciter_ok(&ai);
	 gfsm_arciter_next(&ai), gfsm_arciter_seek_both(&ai,gfsmEpsilon,gfsmEpsilon))
      {
	gfsmArc *a = gfsm_arciter_arc(&ai);
	gfsm_pqueue_push(pqueue, a);
	//fprintf(stderr, "rmeps[init]: push(%d --%d:%d--> %d / %g)\n", a->source, a->lower,a->upper,a->target,a->weight); //--DEBUG
      }
  }

  //-- queue-processing loop (hh2010:3-24)
  while (!gfsm_pqueue_isempty(pqueue)) {
    //-- dequeue next arc to process, and remove it from the fsm (hh2010:4-5)
    gfsmArc    *pwq = (gfsmArc*)gfsm_pqueue_pop(pqueue);
    gfsmState *pptr = gfsm_automaton_open_state(fsm,pwq->source);
    gfsmState *qptr = gfsm_automaton_open_state(fsm,pwq->target);
    gfsm_automaton_remove_arc_ptr(fsm,pwq);

    //-- prelim: get final weight for pptr
    if (pptr->is_final)
      gfsm_weightmap_lookup(fsm->finals, GUINT_TO_POINTER(pwq->source), &fw_p);

    //-- check for eps-loops (hh2010:6-10)
    if (pwq->source==pwq->target) {
      westar = gfsm_sr_star(fsm->sr, pwq->weight);
      for (gfsm_arciter_open_ptr(&ai, fsm, pptr); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
	gfsmArc *a = gfsm_arciter_arc(&ai);
	a->weight  = gfsm_sr_times(fsm->sr, westar, a->weight);
      }
      if (pptr->is_final) {
	gfsm_weightmap_insert(fsm->finals, GUINT_TO_POINTER(pwq->source), gfsm_sr_times(fsm->sr, westar, fw_p));
      }
    }
    else {
      //-- no eps-loops (hh2010:12-24)
      gfsm_arciter_open_ptr(&aai, fsm, pptr); //-- source arc iterator (to find matches)

      //-- adopt outgoing arcs from eps-reachable state (hh2010:12-18)
      for (gfsm_arciter_open_ptr(&ai, fsm, qptr); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
	gfsmArc *aa, *a = gfsm_arciter_arc(&ai);

	//-- try to find a matching arc from pwq->source we can modify
	gfsm_arciter_reset(&aai);
	gfsm_arciter_seek_all(&aai, a->lower, a->upper, a->target);
	aa = gfsm_arciter_arc(&aai);
	if (aa != NULL) {
	  aa->weight = gfsm_sr_plus(fsm->sr, aa->weight, gfsm_sr_times(fsm->sr, pwq->weight, a->weight));
	}
	else {
	  //-- no matching arc to modify: copy the source (hh2010:15-18)
	  gfsmArc *paa = (gfsmArc*)gfsm_arclist_new_full(pwq->source, a->target, a->lower, a->upper, gfsm_sr_times(fsm->sr, pwq->weight, a->weight), NULL);
	  gfsm_automaton_add_arc_node(fsm, pptr, (gfsmArcList*)paa);
	  if (paa->lower==gfsmEpsilon && paa->upper==gfsmEpsilon) {
	    //-- ... and maybe enqueue it (hh2010:17-18)
	    gfsm_pqueue_push(pqueue, paa);
	    //fprintf(stderr, "rmeps[main]: push(%d --%d:%d--> %d / %g)\n", paa->source,paa->lower,paa->upper,paa->target,paa->weight); //--DEBUG
	  }
	}
      }

      //-- adopt final weight of eps-reachable state (hh2010:19-24)
      if (qptr->is_final) {
	gfsm_weightmap_lookup(fsm->finals, GUINT_TO_POINTER(pwq->target), &fw_q);
	if (pptr->is_final) {
	  gfsm_automaton_set_final_state_full(fsm, pwq->source, TRUE,
					      gfsm_sr_plus(fsm->sr, fw_p, gfsm_sr_times(fsm->sr, pwq->weight, fw_q)));
	} else {
	  gfsm_automaton_set_final_state_full(fsm, pwq->source, TRUE,
					      gfsm_sr_times(fsm->sr, pwq->weight, fw_q));
	}
      }
    }

    //-- local cleanup
    gfsm_arc_free(pwq);
  }

  //-- implicit connect (hh2010:25)
  //gfsm_automaton_connect(fsm);

  //-- cleanup & return
  gfsm_pqueue_free(pqueue);
  return fsm;
}

#else
/*--------------------------------------------------------------
 * rmepsilon_foreach_func() : v1
 */
static
void gfsm_automaton_rmeps_pass2_foreach_func_(gfsmStatePair *sp, gpointer pw, gfsmAutomaton *fsm)
{
  gfsmWeight  w = gfsm_ptr2weight(pw);
  gfsmWeight  fw2;
  gfsmArcIter ai;
  gfsmArc     *a;
  if (sp->id1==sp->id2) return; //-- sanity check

  //-- adopt final weights (plus)
  if (gfsm_automaton_lookup_final(fsm, sp->id2, &fw2)) {
    gfsm_automaton_set_final_state_full(fsm, sp->id1, TRUE,
					gfsm_sr_plus(fsm->sr,
						     gfsm_automaton_get_final_weight(fsm, sp->id1),
						     gfsm_sr_times(fsm->sr, w, fw2)));
  }

  //-- adopt non-epsilon arcs
  for (gfsm_arciter_open(&ai,fsm,sp->id2); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
    a = gfsm_arciter_arc(&ai);
    if (a->lower != gfsmEpsilon || a->upper != gfsmEpsilon) {
      gfsm_automaton_add_arc(fsm, sp->id1, a->target, a->lower, a->upper,
			     gfsm_sr_times(fsm->sr, a->weight, w));
    }
  }
}

/*--------------------------------------------------------------
 * rmepsilon_visit_state() : v1
 */
void gfsm_automaton_rmeps_visit_state_(gfsmAutomaton *fsm,
				       gfsmStateId qid_noeps, //-- state reachable by non-eps arcs
				       gfsmStateId qid_eps,   //-- eps-reachable state from qid_noeps
				       gfsmWeight weight_eps, //-- total weight of followed eps-arcs
				       gfsmStatePair2WeightHash *sp2wh //-- maps (qid_noeps,qid_noeps)=>sum_weight_eps
				       )
{
  gfsmState *q_noeps, *q_eps;
  gfsmStatePair sp = {qid_noeps,qid_eps};
  gfsmArcIter ai;
  gfsmArc *a;

  //-- visited check, mark
  if (!gfsm_weighthash_insert_sum_if_less(sp2wh, &sp, weight_eps, fsm->sr))
    return; //-- no update required

  //-- sanity check
  q_noeps = gfsm_automaton_find_state(fsm,qid_noeps);
  q_eps   = gfsm_automaton_find_state(fsm,qid_eps);
  if (!q_noeps || !q_noeps->is_valid || !q_eps || !q_eps->is_valid) return;

  //-- visit epsilon-reachable states from q_eps
  for (gfsm_arciter_open_ptr(&ai, fsm, q_eps), gfsm_arciter_seek_both(&ai,gfsmEpsilon,gfsmEpsilon);
       gfsm_arciter_ok(&ai);
       gfsm_arciter_next(&ai), gfsm_arciter_seek_both(&ai,gfsmEpsilon,gfsmEpsilon))
    {
      a = gfsm_arciter_arc(&ai);
      gfsm_automaton_rmeps_visit_state_(fsm, qid_noeps, a->target,
					gfsm_sr_times(fsm->sr, weight_eps, a->weight),
					sp2wh);
    }
}

/*--------------------------------------------------------------
 * rmepsilon() : v1
 */
gfsmAutomaton *gfsm_automaton_rmepsilon(gfsmAutomaton *fsm)
{
  gfsmStatePair2WeightHash *sp2wh = gfsm_statepair2weighthash_new();
  gfsmArcIter ai;
  gfsmStateId qid;
  gfsmArc *a;

  //-- pass-1: populate sp2wh with epsilon-reachable states
  for (qid=0; qid < fsm->states->len; qid++) {
    gfsm_automaton_rmeps_visit_state_(fsm, qid, qid, fsm->sr->one, sp2wh);
  }

  //-- pass-2: adopt non-epsilon arcs & final weights from eps-reachable states
  gfsm_weighthash_foreach(sp2wh, (GHFunc)gfsm_automaton_rmeps_pass2_foreach_func_, fsm);

  //-- pass-3: actual removal of now-redundant epsilon arcs
  for (qid=0; qid < fsm->states->len; qid++) {
    for (gfsm_arciter_open(&ai,fsm,qid); gfsm_arciter_ok(&ai); ) {
      a = gfsm_arciter_arc(&ai);
      if (a->lower==gfsmEpsilon && a->upper==gfsmEpsilon) {
	gfsm_arciter_remove(&ai);
      } else {
	gfsm_arciter_next(&ai);
      }
    }
  }

  //-- cleanup
  gfsm_weighthash_free(sp2wh);

  return fsm;
}

#endif
