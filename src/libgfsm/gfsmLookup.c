
/*=============================================================================*\
 * File: gfsmLookup.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library
 *
 * Copyright (c) 2005 Bryan Jurish.
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

#include <gfsmLookup.h>

#include <gfsmAlphabet.h>
#include <gfsmState.h>
#include <gfsmArc.h>
#include <gfsmArcIter.h>

#include <string.h>

/*======================================================================
 * Constants
 */
const guint gfsmLookupStateMapGet = 16;

/*======================================================================
 * Methods: lookup
 */

//--------------------------------------------------------------
gfsmAutomaton *gfsm_automaton_lookup_full(gfsmAutomaton     *fst,
					  gfsmLabelVector   *input,
					  gfsmAutomaton     *result,
					  gfsmStateIdVector *statemap)
{
  GSList           *stack = NULL;
  gfsmLookupConfig *cfg   = (gfsmLookupConfig*)g_new(gfsmLookupConfig,1);
  gfsmLookupConfig *cfg_new;
  const gfsmState  *qt;
  gfsmState        *qr;
  gfsmLabelVal      a;
  gfsmArcIter       ai;

  //-- ensure result automaton exists and is clear
  if (result==NULL) {
    result = gfsm_automaton_shadow(fst);
    result->flags.is_transducer = FALSE;
  } else {
    gfsm_automaton_clear(result);
  }

  //-- initialization
  result->root_id = gfsm_automaton_add_state(result);
  cfg->qt = fst->root_id;
  cfg->qr = result->root_id;
  cfg->i  = 0;
  stack = g_slist_prepend(stack, cfg);

  //-- ye olde loope
  while (stack != NULL) {
    //-- pop the top element off the stack
    cfg   = (gfsmLookupConfig*)(stack->data);
    stack = g_slist_delete_link(stack, stack);

    //-- add config to the state-map, if non-NULL
    if (statemap) {
      if (cfg->qr >= statemap->len) {
	g_ptr_array_set_size(statemap, cfg->qr + gfsmLookupStateMapGet);
      }
      g_ptr_array_index(statemap, cfg->qr) = (gpointer)cfg->qt;
    }

    //-- get states
    qt = gfsm_automaton_find_state_const(fst,    cfg->qt);
    qr = gfsm_automaton_find_state      (result, cfg->qr);
    a  = (cfg->i < input->len
	  ? (gfsmLabelVal)(g_ptr_array_index(input, cfg->i))
	  : gfsmNoLabel);

    //-- check for final states
    if (cfg->i >= input->len && gfsm_state_is_final(qt)) {
      gfsm_automaton_set_final_state_full(result, cfg->qr, TRUE,
					  gfsm_automaton_get_final_weight(fst, cfg->qt));
    }

    //-- handle outgoing arcs
    for (gfsm_arciter_open_ptr(&ai, fst, (gfsmState*)qt); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai))
      {
	gfsmArc *arc = gfsm_arciter_arc(&ai);

	//-- epsilon arcs
	if (arc->lower == gfsmEpsilon) {
	  cfg_new = (gfsmLookupConfig*)g_new(gfsmLookupConfig,1);
	  cfg_new->qt = arc->target;
	  cfg_new->qr = gfsm_automaton_add_state(result);
	  cfg_new->i  = cfg->i;
	  gfsm_automaton_add_arc(result, cfg->qr, cfg_new->qr, arc->upper, arc->upper, arc->weight);
	  stack = g_slist_prepend(stack, cfg_new);
	}
	//-- input-matching arcs
	else if (a != gfsmNoLabel && arc->lower == a) {
	  cfg_new = (gfsmLookupConfig*)g_new(gfsmLookupConfig,1);
	  cfg_new->qt = arc->target;
	  cfg_new->qr = gfsm_automaton_add_state(result);
	  cfg_new->i  = cfg->i+1;
	  gfsm_automaton_add_arc(result, cfg->qr, cfg_new->qr, arc->upper, arc->upper, arc->weight);
	  stack = g_slist_prepend(stack, cfg_new);
	}
      }

    //-- we're done with this config
    g_free(cfg);
  }

  //-- set final size of the state-map
  if (statemap) { statemap->len = result->states->len; }
  
  return result;
}

//--------------------------------------------------------------
#if 0
gfsmAutomaton *gfsm_automaton_lookup_viterbi(gfsmAutomaton     *fst,
					     gfsmLabelVector   *input,
					     gfsmAutomaton     *trellis)
{
  g_assert_not_reached();

  GPtrArray cols   = g_ptr_array_sized_new(input->len+1);
  gfsmViterbiConfig *cfg, *cfg_new;
  const gfsmState  *qt;
  gfsmLabelVal      a;
  gfsmArcIter       ai;
  gfsmArc          *arc;
  guint             rowi;

  //-- ensure trellis automaton exists and is clear
  if (trellis==NULL) {
    trellis = gfsm_automaton_shadow(fst);
  } else {
    gfsm_automaton_clear(trellis);
  }
  trellis->flags.is_transducer = TRUE;

  //-- initialization
  cfg = (gfsmViterbiConfig*)g_new(gfsmViterbiConfig,1);
  cfg->qt = fst->root_id;
  cfg->qr = gfsm_automaton_add_state(trellis);
  cfg->i  = 0;
  cfg->w  = fst->sr->one;
  g_ptr_array_index(cols,0) = g_slist_prepend(NULL, cfg);

  //-- ye olde loope: for each input character (coli)
  for (coli=1; coli <= input->len; coli++) {
    GSList *prevcol = g_ptr_array_index(cols,coli);
    GSList *col     = NULL;
    GSList *prevcoli;
    gfsmLabelVal a  = (coli <= input->len
		       ? (gfsmLabelVal)(g_ptr_array_index(input, coli-1))
		       : gfsmNoLabel);
    gfsmStateId  bestprev_id = gfsmNoState;
    gfsmWeight   bestprev_w  = fsm->sr->zero;

    //-- foreach config in previous column (prevcoli)
    for (prevcoli=prevcol; prevcoli != NULL; prevcoli=prevcoli->next) {
      GSlist *stack = g_slist_prepend(NULL,prevcoli->data);

      while (stack != NULL) {
	//-- pop the top element off the stack
	cfg = (gfsmViterbiConfig*)(stack->data);
	stack = g_slist_delete_link(stack,stack);

	//-- search for input-epsilon arcs & add them to the previous column
	for (gfsm_arciter_open(&ai,fst,cfg->qt), gfsm_arciter_seek_lower(&ai,gfsmEpsilon);
	     gfsm_arciter_ok(&ai);
	     gfsm_arciter_next(&ai), gfsm_arciter_seek_lower(&ai,gfsmEpsilon))
	  {
	    arc     = gfsm_arciter_arc(&ai);
	    cfg_new = (gfsmViterbiConfig*)g_new(gfsmViterbiConfig,1);
	    cfg_new->qt = arc->target;
	    //cfg_new->  ///-----------------------> ARGH !
	    g_ptr_array_index(cols,coli) = g_slist_prepend((GSList*)g_ptr_array_index(cols,coli), cfg_new);
	  }
      }
    }
  }
  while (deque_head != NULL) {
    //-- pop the top element off the queue
    cfg        = (gfsmLookupConfig*)(deque_head->data);
    deque_head = g_slist_delete_link(deque_head, deque_head);

    //-- get states
    qt = gfsm_automaton_find_state_const(fst,     cfg->qt);
    qr = gfsm_automaton_find_state      (trellis, cfg->qr);
    a  = (cfg->i < input->len
	  ? (gfsmLabelVal)(g_ptr_array_index(input, cfg->i))
	  : gfsmNoLabel);

    //-- check for final states
    if (cfg->i >= input->len && gfsm_state_is_final(qt)) {
      gfsm_automaton_set_final_state_full(result, cfg->qr, TRUE,
					  gfsm_sr_times(fst->sr,
							gfsm_automaton_get_final_weight(fst, cfg->qt),
							cfg->w));
    }

    //-- handle outgoing arcs
    for (gfsm_arciter_open_ptr(&ai, fst, (gfsmState*)qt); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai))
      {
	gfsmArc *arc = gfsm_arciter_arc(&ai);

	//-- epsilon arcs
	if (arc->lower == gfsmEpsilon) {
	  cfg_new = (gfsmLookupConfig*)g_new(gfsmLookupConfig,1);
	  cfg_new->qt = arc->target;
	  cfg_new->qr = gfsm_automaton_add_state(result);
	  cfg_new->i  = cfg->i;
	  gfsm_automaton_add_arc(result, cfg->qr, cfg_new->qr, arc->upper, arc->upper, arc->weight);
	  stack = g_slist_prepend(stack, cfg_new);
	}
	//-- input-matching arcs
	else if (a != gfsmNoLabel && arc->lower == a) {
	  cfg_new = (gfsmLookupConfig*)g_new(gfsmLookupConfig,1);
	  cfg_new->qt = arc->target;
	  cfg_new->qr = gfsm_automaton_add_state(result);
	  cfg_new->i  = cfg->i+1;
	  gfsm_automaton_add_arc(result, cfg->qr, cfg_new->qr, arc->upper, arc->upper, arc->weight);
	  stack = g_slist_prepend(stack, cfg_new);
	}
      }

    //-- we're done with this config
    g_free(cfg);
  }

  return trellis;
}
#endif
