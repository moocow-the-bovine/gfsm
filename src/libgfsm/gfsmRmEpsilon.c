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
#include <gfsmArcIndex.h>
#include <gfsmEnum.h>
#include <gfsmUtils.h>
#include <gfsmCompound.h>

#include <gfsmPQueue.h>

/*======================================================================
 * Methods: algebra: rmepsilon
 */
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
  else if (
	   (   p1==q1 && p==q && q1>q) //-- hh2010 eq (3)
	   || (p1!=q1 && p!=q && q1>q) //-- hh2010 eq (4)
	   || (p1==q1 && p!=q)         //-- hh2010 eq (5)
	   )
    return 1;
#if 0
  return (p>p1 ? -1
	  : (p==p1
	     ? (q>q1 ? -1
		: (q==q1 ? 0
		   : 1))
	     : 1));
#else
  return 0;
#endif
}

//--------------------------------------------------------------
// + weight-independent arc comparison a la gfsmASMLower: (ReverseIsNull,lower,upper,target)
GFSM_INLINE
gint gfsm_rmeps_arc_compare_lut(const gfsmArc *a, const gfsmArc *b)
{
  if      (a==NULL && b==NULL)  return  0;
  else if (a==NULL && b!=NULL)  return  1;
  else if (a!=NULL && b==NULL)  return -1;
  else if (a->lower < b->lower) return -1;
  else if (a->lower > b->lower) return  1;
  else if (a->upper < b->upper) return -1;
  else if (a->upper > b->upper) return  1;
  else if (a->target < b->target) return -1;
  else if (a->target > b->target) return  1;
  return 0;
}


//#define RME_DEBUG 1

#ifdef RME_DEBUG
# include <gfsmAutomatonIO.h>
# define _debug(code) code
# define _nodebug(code)
#else
# define _debug(code)
# define _nodebug(code)
#endif

//-- debugging macros
#define _arcfmt      "(%u --%u:%u--> %u / %g)"
#define _arcargs(ap) (ap)->source, (ap)->lower, (ap)->upper, (ap)->target, (ap)->weight
#define _showpalp(prefix) \
   fprintf(stderr, prefix "palp=") \
   if (*palp) { fprintf(stderr, _arcfmt "\n", _arcargs(&(*palp)->arc)); } \
   else       { fprintf(stderr, "NULL\n"); })


//--------------------------------------------------------------
gfsmAutomaton *gfsm_automaton_rmepsilon(gfsmAutomaton *fsm)
{
  //-- variables
  gfsmPriorityQueue *pqueue;
  gfsmStateId pid;
  gfsmWeight westar, fw_p=0, fw_q=0;
  gfsmArc *pwq;
  gfsmState *pptr, *qptr;
  gfsmArcList *pal,*pal1, *qal, **palp;
  gint cmp;

  //-- pre-sort arcs
  if (gfsm_acmask_nth(fsm->flags.sort_mode,0) != gfsmACLower
      || gfsm_acmask_nth(fsm->flags.sort_mode,1) != gfsmACUpper
      || gfsm_acmask_nth(fsm->flags.sort_mode,2) != gfsmACTarget)
    {
      gfsm_automaton_arcsort(fsm, gfsm_acmask_from_chars("lut"));
    }

  //-- setup priority queue (hh2010:1-2)
  //   : moo: collect (q --eps:eps--> r) runs in the style of gfsm_automaton_arcuniq()
  pqueue = gfsm_pqueue_new((GCompareDataFunc)gfsm_rmeps_arc_compare, NULL);
  for (pid=0; pid < fsm->states->len; pid++) {
    pptr = gfsm_automaton_open_state(fsm,pid);
    if (pptr == NULL) continue;

    //-- seek epsilon arcs (should be initial since automaton is primarily sorted on (lower,upper))
    for (pal=pptr->arcs; pal != NULL; pal=pal->next) {
      if (pal->arc.lower != gfsmEpsilon || pal->arc.upper != gfsmEpsilon) {
	break;
      }

      //-- compress epsilon runs
      for (pal1=pal->next; pal1!=NULL && pal1->arc.lower==gfsmEpsilon && pal1->arc.upper==gfsmEpsilon && pal1->arc.target==pal->arc.target; pal1=pal->next) {
	pal->next = pal1->next;
	pal->arc.weight = gfsm_sr_plus(fsm->sr, pal->arc.weight, pal1->arc.weight);
	gfsm_arclist_free_1(pal1);
      }

      //-- initialize priority queue
      gfsm_pqueue_push(pqueue, &(pal->arc));
      _debug(fprintf(stderr, "rmeps[init]: push" _arcfmt "\n", _arcargs(&pal->arc)));
    }
  }

  //-- disable "smart" arc-insertion: NO!
  //fsm->flags.sort_mode = gfsmASMNone;

  //-- queue-processing loop (hh2010:3-24)
  while (!gfsm_pqueue_isempty(pqueue)) {
    //-- dequeue next arc to process, and remove it from the fsm (hh2010:4-5)
    pwq  = (gfsmArc*)gfsm_pqueue_pop(pqueue);
    pptr = gfsm_automaton_open_state(fsm,pwq->source);
    pptr->arcs = gfsm_arclist_remove_node(pptr->arcs, (gfsmArcList*)pwq);

    _debug(fprintf(stderr,"rmeps[main]: pop" _arcfmt "\n", _arcargs(pwq)));

    //-- prelim: get final weight for pptr
    if (pptr->is_final)
      gfsm_weightmap_lookup(fsm->finals, GUINT_TO_POINTER(pwq->source), &fw_p);

    //-- check for eps-loops (hh2010:6-10)
    if (pwq->source==pwq->target) {
      westar = gfsm_sr_star(fsm->sr, pwq->weight);
      for (pal=pptr->arcs; pal!=NULL; pal=pal->next) {
	pal->arc.weight = gfsm_sr_times(fsm->sr, westar, pal->arc.weight);
      }
      if (pptr->is_final) {
	gfsm_weightmap_insert(fsm->finals, GUINT_TO_POINTER(pwq->source), gfsm_sr_times(fsm->sr, westar, fw_p));
      }
    }
    else {
      //-- no eps-loops (hh2010:12-24)
      qptr = gfsm_automaton_open_state(fsm,pwq->target);
      palp = &(pptr->arcs);

      //-- adopt outgoing arcs from eps-reachable state q (hh2010:12-18)
      for (qal=qptr->arcs; qal != NULL; qal=qal->next) {
	_debug(fprintf(stderr,"rmeps[main]: adopt" _arcfmt " . " _arcfmt "\n", _arcargs(pwq), _arcargs(&qal->arc)));

	//-- seek matching arc from p (~hh2010:13)
	for (cmp=-1; *palp; palp=&((*palp)->next)) {
	  cmp = gfsm_rmeps_arc_compare_lut(&(*palp)->arc, &qal->arc);
	  if (cmp >= 0) break;
	}
	if (cmp==0) {
	  //-- found a structure-matching arc: modify it (hh2010:14)
	  (*palp)->arc.weight = gfsm_sr_plus(fsm->sr, (*palp)->arc.weight, gfsm_sr_times(fsm->sr, pwq->weight, qal->arc.weight));
	}
	else {
	  //-- no matching arc: insert a new one (hh2010:15-18)
	  pal1  = gfsm_arclist_new_full(pwq->source, qal->arc.target, qal->arc.lower, qal->arc.upper, gfsm_sr_times(fsm->sr, pwq->weight, qal->arc.weight), *palp);
	  *palp = pal1;
	  if (qal->arc.lower==gfsmEpsilon && qal->arc.upper==gfsmEpsilon) {
	    //-- ... and maybe enqueue it (hh2010:17-18)
	    gfsm_pqueue_push(pqueue, pal1);
	  }
	}
      } //-- end loop: arcs(q)

      //-- adopt final weight of eps-reachable state q (hh2010:19-24)
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

      gfsm_automaton_close_state(fsm,qptr);
    }

    //-- local cleanup
    gfsm_arclist_free_1((gfsmArcList*)pwq);
    gfsm_automaton_close_state(fsm,pptr);
  }

  //-- cleanup & return
  gfsm_pqueue_free(pqueue);
  return fsm;
}
