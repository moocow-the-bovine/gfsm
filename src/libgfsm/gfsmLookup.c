
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
  //-- cols: array of (GSList <gfsmViterbiConfig*> *)
  gfsmViterbiTable *cols = g_ptr_array_sized_new(input->len+1);
  gfsmViterbiNode  *node;
  guint             i;
  gfsmViterbiColumnMap *succmap = gfsm_viterbi_column_map_new();

  //-- ensure trellis automaton exists and is clear
  if (trellis==NULL) {
    trellis = gfsm_automaton_shadow(fst);
  } else {
    gfsm_automaton_clear(trellis);
  }
  trellis->flags.is_transducer = TRUE;

  //-- initial config
  node = g_new(gfsmViterbiNode,1);
  node->key = fst->root_id;
  node->val = g_new(gfsmViterbiNodeValue,1);
  node->val->qtrellis  = gfsm_automaton_add_state(trellis);
  node->val->pqtrellis = gfsmNoState;
  node->val->label     = gfsmNoLabel;
  node->val->w         = fst->sr->one;
  g_ptr_array_index(cols,0) = g_slist_prepend(NULL, node);
  _gfsm_viterbi_expand_column(fst, trellis, (gfsmViterbiColumn*)g_ptr_array_index(cols,0), succmap);
  gfsm_viterbi_column_map_free(succmap);

  //-- ye olde loope: for each input character (coli)
  for (i=0; i <= input->len; i++) {
    GSList *prevcol  = g_ptr_array_index(cols,i);
    GSList *prevcoli = NULL;
    GSList *col      = NULL;
    GSList *coli     = NULL;
    gfsmLabelVal a  = (i < input->len
		       ? (gfsmLabelVal)(g_ptr_array_index(input, i))
		       : gfsmNoLabel);

    succmap = gfsm_viterbi_column_map_new();

    //-- get possible successors & final-state markers
    for (prevcoli=prevcol; prevcoli != NULL; prevcoli=prevcoli->next) {
      const gfsmState *q_fst;
      gfsmState *q_trellis;

      //-- get the top element of the queue
      node = (gfsmViterbiNode*)(prevcoli->data);

      //-- get state pointers
      q_fst     = gfsm_automaton_find_state_const(fst,     node->key);
      q_trellis = gfsm_automaton_find_state      (trellis, node->val->qtrellis);

      //-- check for final states
      if (i >= input->len && gfsm_state_is_final(q_fst)) {
	gfsm_automaton_set_final_state_full(trellis, node->val->qtrellis, TRUE,
					    gfsm_sr_times(fst->sr,
							  gfsm_automaton_get_final_weight(fst, node->key),
							  node->val->w));
      }
      else if (i < input->len) {
	//-- handle outgoing arcs: add to possible-successors map
	gfsmArcIter ai;

	for (gfsm_arciter_open_ptr(&ai, fst, (gfsmState*)q_fst), gfsm_arciter_seek_lower(&ai,a);
	     gfsm_arciter_ok(&ai);
	     gfsm_arciter_next(&ai), gfsm_arciter_seek_lower(&ai,a))
	  {
	    gfsmArc *arc = gfsm_arciter_arc(&ai);
	    gfsmViterbiNodeValue *nxtval;

	    if ((nxtval=gfsm_viterbi_column_map_lookup(succmap, (gpointer)arc->target))) {
	      //-- known successor: possibly update its weight & precessor
	      gfsmWeight nxtw = gfsm_sr_times(fst->sr, node->val->w, arc->weight);

	      if (gfsm_sr_less(fst->sr, nxtw, nxtval->w)) {
		//-- this arc is better: update
		nxtval->pqtrellis = node->val->qtrellis;
		nxtval->label     = arc->upper;
		nxtval->w         = nxtw;
	      }
	    } else {
	      //-- new successor: create a node
	      gfsmViterbiNode *nxtnode = g_new(gfsmViterbiNode,1);
	      nxtnode->key = arc->target;
	      nxtnode->val = g_new(gfsmViterbiNodeValue,1);
	      nxtnode->val->qtrellis  = gfsm_automaton_add_state(trellis);
	      nxtnode->val->pqtrellis = node->val->qtrellis;
	      nxtnode->val->label     = arc->upper;
	      nxtnode->val->w         = gfsm_sr_times(fst->sr, node->val->w, arc->weight);

	      //-- ... and add it to the map & to the column
	      g_tree_insert(succmap, (gpointer)nxtnode->key, nxtnode->val);
	      col = g_slist_prepend(col,nxtnode);
	    }
	  }
      }
    }

    //-- expand epsilons
    _gfsm_viterbi_expand_column(fst, trellis, col, succmap);

    //-- annotate best-predecessor of each possible successor in trellis automaton
    for (coli=col; coli != NULL; coli=coli->next) {
      node = (gfsmViterbiNode*)(coli->data);
      gfsm_automaton_add_arc(trellis,
			     node->val->pqtrellis, node->val->qtrellis,
			     
			     node->val->w);
    }

    //-- update table
    g_ptr_array_index(cols,i+1) = col;

    //-- per-position cleanup
    gfsm_viterbi_column_map_free(succmap);
  }

  //-- cleanup
  for (i=0; i < cols->len; i++) {
    GSList *l;
    for (l=(GSList*)g_ptr_array_index(cols,i); l != NULL; l = l->next) {
      node = (gfsmViterbiNode*)(l->data);
      g_free(node->val);
      g_free(node);
    }
    g_slist_free((GSList*)g_ptr_array_index(cols,i));
  }
  g_ptr_array_free(cols,TRUE);

  return trellis;
}


//--------------------------------------------------------------
void _gfsm_viterbi_expand_column(gfsmAutomaton        *fst,
				 gfsmAutomaton        *trellis,
				 gfsmViterbiColumn    *col,
				 gfsmColumnMap        *cmap)
{
  gfsmArcIter ai;
  gfsmViterbiColumn *coli;
  gfsmViterbiNode   *node;

  //-- pass-1: add everything already in the column as a literal
  /*
  for (coli=col; coli != NULL; coli=coli->next) {
    node = (gfsmViterbiNode*)(coli->data);
    if (!g_tree_lookup(cmap,node->key)) {
      g_tree_insert(cmap,node->key,node->val);
    }
  }
  */

  //-- pass-2: add epsilon arcs from every literal in the column
  for (coli=col; coli != NULL; coli=coli->next) {
    //-- get node
    node = (gfsmViterbiNode*)(coli->data);

    //-- search for input-epsilon arcs & add them to this column
    for (gfsm_arciter_open(&ai,fst,qid_fst), gfsm_arciter_seek_lower(&ai,gfsmEpsilon);
	 gfsm_arciter_ok(&ai);
	 gfsm_arciter_next(&ai), gfsm_arciter_seek_lower(&ai,gfsmEpsilon))
      {
	gfsmArc *arc = gfsm_arciter_arc(&ai);
	gfsmViterbiNodeValue *nxtval;
	if ((nxtval=gfsm_viterbi_column_map_insert_sum_if_less(cmap,
							       arc->target,
							       gfsm_sr_times(fst->sr, node->val->w, arc->w),
							       fst->sr)))
	  {
	    //-- inserted a new value: queue it up
	    gfsmViterbiNode *nxtnode = g_new(gfsmViterbiNode,1);
	    nxtnode->key = qid_fst;
	    nxtnode->val = nxtval;
	    nxtval->qtrellis = gfsm_automaton_add_state(qtrellis);
            nxtval->pqtrellis = node->val->qtrellis;
	    gfsm_automaton_add_arc(qtrellis, node->val->qtrellis, nxtval->qtrellis,
				   arc->upper, arc->upper, nxtval->w);
	    coli->next = g_slist_prepend(coli->next, nxtval);
	  }
      }
  }
}


//--------------------------------------------------------------
gfsmViterbiNodeValue *gfsm_viterbi_column_map_insert_if_less(gfsmViterbiColumn    *cmap,
							     gfsmViterbiNodeKey    key,
							     gfsmWeight            w,
							     gfsmSemiring         *sr)
{
  gpointer s_val;
  if (s_val = gfsm_viterbi_column_lookup(col,key)) {
    //-- already present
    if (!gfsm_sr_less(sr,w,s_val->w)) return NULL; //-- (s_val->w) <= (w)
    s_val->w = w;
  } else {
    //-- not already present: copy & insert
    s_val  = g_new(gfsmViterbiNodeValue,1);
    s_val->qtrellis = gfsmNoState;
    a_val->pqtrellis = gfsmNoState;
    s_val->w        = w;
    g_tree_insert(col,key,s_val);
  }
  return s_val; //-- update occurred
}

//--------------------------------------------------------------
gfsmViterbiNodeValue *gfsm_viterbi_column_map_insert_sum_if_less(gfsmViterbiColumn    *cmap,
								 gfsmViterbiNodeKey    key,
								 gfsmWeight            w,
								 gfsmSemiring         *sr)
{
  gpointer s_val;
  if (s_val = gfsm_viterbi_column_lookup(col,key)) {
    //-- already present
    gfsmWeight w_sum = gfsm_sr_plus(sr, s_val->w, w);
    if (gfsm_sr_less(sr,w_sum,s_val->w)) { //-- (s_val->w) > (s_val->w + w)
      s_val->w = w_sum;
    }
    return NULL; //-- no new value inserted
  } else {
    //-- not already present: copy & insert
    s_val  = g_new(gfsmViterbiNodeValue,1);
    s_val->qtrellis = gfsmNoState;
    s_val->pqtrellis = gfsmNoState;
    s_val->w        = w;
    g_tree_insert(col,key,s_val);
  }
  return s_val; //-- new value inserted
}

#endif /* WIP */
