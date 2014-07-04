
/*=============================================================================*\
 * File: gfsmLookup.c
 * Author: Bryan Jurish <moocow.bovine@gmail.com>
 * Description: finite state machine library
 *
 * Copyright (c) 2005-2011 Bryan Jurish.
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

#include <gfsmLookup.h>

#include <gfsmAlphabet.h>
#include <gfsmState.h>
#include <gfsmArc.h>
#include <gfsmArcIter.h>

#include <string.h>

/*======================================================================
 * Constants
 */
const gfsmStateId gfsmLookupStateMapGet = 16;

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
  gfsmLookupConfig *cfg   = (gfsmLookupConfig*)gfsm_slice_new(gfsmLookupConfig);
  gfsmLookupConfig *cfg_new;
  const gfsmState  *qt;
  gfsmLabelVal      a;
  gfsmArcIter       ai;

  //-- ensure result automaton exists and is clear
  if (result==NULL) {
    result = gfsm_automaton_shadow(fst);
  } else {
    gfsm_automaton_clear(result);
  }
  result->flags.is_transducer = TRUE;

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
      g_ptr_array_index(statemap, cfg->qr) = GUINT_TO_POINTER(cfg->qt);
    }

    //-- get states
    qt = gfsm_automaton_find_state_const(fst,    cfg->qt);
    /*qr =*/gfsm_automaton_find_state           (result, cfg->qr);
    a  = (cfg->i < input->len
	  ? (gfsmLabelVal)GPOINTER_TO_UINT(g_ptr_array_index(input, cfg->i))
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
	  cfg_new = (gfsmLookupConfig*)gfsm_slice_new(gfsmLookupConfig);
	  cfg_new->qt = arc->target;
	  cfg_new->qr = gfsm_automaton_add_state(result);
	  cfg_new->i  = cfg->i;
	  gfsm_automaton_add_arc(result, cfg->qr, cfg_new->qr, arc->lower, arc->upper, arc->weight);
	  stack = g_slist_prepend(stack, cfg_new);
	}
	//-- input-matching arcs
	else if (a != gfsmNoLabel && arc->lower == a) {
	  cfg_new = (gfsmLookupConfig*)gfsm_slice_new(gfsmLookupConfig);
	  cfg_new->qt = arc->target;
	  cfg_new->qr = gfsm_automaton_add_state(result);
	  cfg_new->i  = cfg->i+1;
	  gfsm_automaton_add_arc(result, cfg->qr, cfg_new->qr, arc->lower, arc->upper, arc->weight);
	  stack = g_slist_prepend(stack, cfg_new);
	}
      }

    //-- we're done with this config
    gfsm_slice_free(gfsmLookupConfig,cfg);
  }

  //-- set final size of the state-map
  if (statemap) { statemap->len = result->states->len; }
  
  return result;
}

/*======================================================================
 * Methods: lookup_train
 */

/** \brief Type for gfsm_automaton_lookup_train() computation state */
typedef struct gfsmLookupTrainConfig_ {
  guint32     i;   /**< current position in input vector */
  guint32     j;   /**< current position in output vector */
  gfsmStateId qf;  /**< final state id, or gfsmNoState for non-final state */
  gfsmArc*    a;   /**< last followed arc, or NULL for initial and/or final config */
  struct gfsmLookupTrainConfig_ *prev; /**< previous config in this path */
} gfsmLookupTrainConfig;

/** \brief type for a successful training-path: GPtrArray* of gfsmLookupTrainConfig* */
typedef GPtrArray gfsmLookupTrainPath;

//--------------------------------------------------------------
GFSM_INLINE
gfsmLookupTrainConfig* gfsm_lookup_train_config_new(guint32 i, guint32 j, gfsmStateId qf, gfsmArc *a, gfsmLookupTrainConfig *prev)
{
  gfsmLookupTrainConfig *cfg = (gfsmLookupTrainConfig*)gfsm_slice_new(gfsmLookupTrainConfig);
  cfg->i = i;
  cfg->j = j;
  cfg->qf = qf;
  cfg->a = a;
  cfg->prev = prev;
  return cfg;
}

//--------------------------------------------------------------
GFSM_INLINE
gfsmStateId gfsm_lookup_train_config_stateid(gfsmLookupTrainConfig *cfg, gfsmAutomaton *fst)
{
  return (cfg->a ? cfg->a->target : (cfg->qf==gfsmNoState ? gfsm_automaton_get_root(fst) : cfg->qf));
}

//--------------------------------------------------------------
GFSM_INLINE
guint gfsm_lookup_train_config_pathlength(gfsmLookupTrainConfig *cfg)
{
  guint l=0;
  for ( ; cfg!=NULL; cfg=cfg->prev, ++l) ;
  return l;
}

//--------------------------------------------------------------
GFSM_INLINE
gint gfsm_lookup_train_config_compare(const gfsmLookupTrainConfig **c1, const gfsmLookupTrainConfig **c2)
{
  /*
  if      ((*c1)->i < (*c2)->i) return -1;
  else if ((*c1)->i > (*c2)->i) return  1;
  else if ((*c1)->j < (*c2)->j) return -1;
  else if ((*c1)->j > (*c2)->j) return  1;
  else*/
       if ((*c1)->qf < (*c2)->qf) return -1;
  else if ((*c1)->qf > (*c2)->qf) return  1;
  else if ((*c1)->a < (*c2)->a) return -1;
  else if ((*c1)->a > (*c2)->a) return  1;
  return 0;
}

//--------------------------------------------------------------
GFSM_INLINE
gint gfsm_lookup_train_path_compare(gfsmLookupTrainPath *p1, gfsmLookupTrainPath *p2)
{
  guint i1,i2;
  for (i1=0, i2=0; i1<p1->len && i2<p2->len; ++i1, ++i2) {
    gint cmp = gfsm_lookup_train_config_compare((const gfsmLookupTrainConfig**)p1->pdata+i1, (const gfsmLookupTrainConfig**)p2->pdata+i2);
    if (cmp != 0) return cmp;
  }
  if      (i1==p1->len && i2< p2->len) return -1;
  else if (i1< p1->len && i2==p2->len) return  1;
  return 0;
}

//--------------------------------------------------------------
GFSM_INLINE
gfsmLookupTrainPath* gfsm_lookup_train_path_new(gfsmLookupTrainConfig *cfg)
{
  gfsmLookupTrainPath *p = g_ptr_array_sized_new(gfsm_lookup_train_config_pathlength(cfg));
  for ( ; cfg!=NULL; cfg=cfg->prev) {
    p->pdata[p->len++] = cfg;
  }
  return p;
}


//--------------------------------------------------------------
#ifdef GFSM_DEBUG_TRAIN
static
void gfsm_lookup_train_path_dump(FILE *f, gfsmLookupTrainPath *path, gfsmAutomaton *fst)
{
  guint i;
  fprintf(f,"{ ");
  for (i=0; i<path->len; ++i) {
    gfsmLookupTrainConfig *cfg = (gfsmLookupTrainConfig*)path->pdata[i];
    if (cfg->a)
      fprintf(f, "(%d --[%d:%d]--> %d) ", cfg->a->source, cfg->a->lower, cfg->a->upper, cfg->a->target); //--DEBUG
    else if (cfg->qf != gfsmNoState) 
      fprintf(f, "(->%d) ", cfg->qf);
    else
      fprintf(f, "(%d->) ", gfsm_automaton_get_root(fst));
  }
  fprintf(f,"}\n");
}

//--------------------------------------------------------------
static
void gfsm_lookup_train_paths_dump(FILE *f, GSList *paths, gfsmAutomaton *fst)
{
  for ( ; paths != NULL; paths=paths->next) {
    gfsm_lookup_train_path_dump(f, ((gfsmLookupTrainPath*)paths->data), fst);
  }
}
#endif /* GFSM_DEBUG_TRAIN */

//--------------------------------------------------------------
gfsmAutomaton* gfsm_automaton_lookup_train(gfsmAutomaton     *fst,
					   gfsmLabelVector   *input,
					   gfsmLabelVector   *output,
					   gboolean prunePathPermutations,
					   gboolean distributeOverPaths,
					   gboolean distributeOverArcs
					   )
{
  gfsmLookupTrainConfig *cfg=gfsm_lookup_train_config_new(0,0,gfsmNoState,NULL,NULL), *cfg_new=NULL;
  GSList *stack=NULL; //-- DFS search stack, GSList* of gfsmLookupTrainConfig*
  GSList *used=NULL;  //-- used search configs, GSList* of gfsmLookupTrainConfig*
  GSList *succ=NULL;  //-- final configs of successful paths, GSList* of path-final gfsmLookupTrainConfig*
  GSList *paths=NULL; //-- full successful paths, GSList* of gfsmLookupTrainPath* (==GPtrArray*)
  GSList *nod;	      //-- list-iterator temporary
  gfsmStateId	    q;
  const gfsmState  *qt;
  gfsmLabelVal      a,b;
  gfsmArcIter       ai;
  gfsmWeight        pathMass, arcMass;

  //-- initialization
  stack = g_slist_prepend(stack, cfg);

  //-- ye olde loope
  while (stack != NULL) {
    //-- pop the top element off the stack, and push it to the 'used'-list
    cfg       = (gfsmLookupTrainConfig*)(stack->data);
    nod       = stack;
    stack     = g_slist_remove_link(stack, nod);
    nod->next = used;
    used      = nod;

    //-- get state, labels
    q  = gfsm_lookup_train_config_stateid(cfg, fst);
    qt = gfsm_automaton_find_state_const(fst, q);
    a  = (cfg->i < input->len
	  ? (gfsmLabelVal)GPOINTER_TO_UINT(g_ptr_array_index(input, cfg->i))
	  : gfsmNoLabel);
    b  = (cfg->j < output->len
	  ? (gfsmLabelVal)GPOINTER_TO_UINT(g_ptr_array_index(output, cfg->j))
	  : gfsmNoLabel);

    //-- check for final states
    if (cfg->i >= input->len && cfg->j >= output->len && gfsm_state_is_final(qt)) {
      cfg_new = gfsm_lookup_train_config_new(cfg->i, cfg->j, q, NULL, cfg);
      succ    = g_slist_prepend(succ, cfg_new);
      used    = g_slist_prepend(used, cfg_new);
    }

    //-- handle outgoing arcs
    for (gfsm_arciter_open_ptr(&ai, fst, (gfsmState*)qt); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai))
      {
	gfsmArc *arc = gfsm_arciter_arc(&ai);

	//-- arc: (eps,eps)
	if (arc->lower==gfsmEpsilon && arc->upper==gfsmEpsilon) {
	  cfg_new = gfsm_lookup_train_config_new(cfg->i, cfg->j, gfsmNoState, arc, cfg);
	}
	//-- arc: (eps,b)
	else if (arc->lower==gfsmEpsilon && arc->upper==b && b!=gfsmNoLabel) {
	  cfg_new = gfsm_lookup_train_config_new(cfg->i, cfg->j+1, gfsmNoState, arc, cfg);
	}
	//-- arc: (a,eps)
	else if (arc->lower==a && arc->upper==gfsmEpsilon && a!=gfsmNoLabel) {
	  cfg_new = gfsm_lookup_train_config_new(cfg->i+1, cfg->j, gfsmNoState, arc, cfg);
	}
	//-- arc: (a,b)
	else if (arc->lower==a && arc->upper==b && a!=gfsmNoLabel && b!=gfsmNoLabel) {
	  cfg_new = gfsm_lookup_train_config_new(cfg->i+1, cfg->j+1, gfsmNoState, arc, cfg);
	}
	//-- arc: other
	else {
	  continue;
	}
	stack = g_slist_prepend(stack, cfg_new);
      }
    gfsm_arciter_close(&ai);
  }

  //-- convert successful configs to paths
  for (nod=succ; nod!=NULL; nod=nod->next) {
    paths = g_slist_prepend(paths, gfsm_lookup_train_path_new((gfsmLookupTrainConfig*)nod->data));;
  }
  //fputs("\nDEBUG:RAW\n", stderr); gfsm_lookup_train_paths_dump(stderr,paths,fst); //-- DEBUG

  //-- prune path permutations
  if (prunePathPermutations && paths && paths->next) {
    gfsmLookupTrainPath *path;
    GSList **pnod;

    //-- sort arcs of each successful path (by arc-pointer value): get permutation equivalence-class
    for (nod=paths; nod!=NULL; nod=nod->next) {
      path = (gfsmLookupTrainPath*)nod->data;
      //fputs("- DEBUG:ARC-SORT:PRE : ", stderr); gfsm_lookup_train_path_dump(stderr, path, fst); //-- DEBUG
      g_ptr_array_sort(path, (GCompareFunc)gfsm_lookup_train_config_compare);
      //fputs("- DEBUG:ARC-SORT:POST: ", stderr); gfsm_lookup_train_path_dump(stderr, path, fst); //-- DEBUG
    }
    //fputs("\nDEBUG:ARC-SORTED\n", stderr); gfsm_lookup_train_paths_dump(stderr, paths, fst); //-- DEBUG

    //-- sort list of successful paths lexicographically by permutation eq-class
    paths = g_slist_sort(paths, (GCompareFunc)gfsm_lookup_train_path_compare);
    //fputs("\nDEBUG:PATH-SORTED\n", stderr); gfsm_lookup_train_paths_dump(stderr, paths, fst); //-- DEBUG

    //-- prune duplicate path-classes
    for (pnod=&paths, nod=paths->next; nod!=NULL; nod=(*pnod)->next) {
      if (gfsm_lookup_train_path_compare((gfsmLookupTrainPath*)(*pnod)->data, (gfsmLookupTrainPath*)nod->data)==0) {
	g_ptr_array_free( (gfsmLookupTrainPath*)nod->data, TRUE );
	(*pnod)->next = g_slist_delete_link(nod,nod);
      } else {
	pnod = &((*pnod)->next);
      }
    }
    //fputs("\nDEBUG:PRUNED\n", stderr); gfsm_lookup_train_paths_dump(stderr, paths, fst); //-- DEBUG
  }

  //-- update: count-mass
  pathMass = distributeOverPaths ? (1.0/g_slist_length(paths)) : 1.0;
  while (paths != NULL) {
    guint i;
    gfsmLookupTrainPath* path = (gfsmLookupTrainPath*)paths->data;
    paths    = g_slist_delete_link(paths,paths);
    arcMass  = distributeOverArcs ? (pathMass/(path->len-1.0)) : pathMass;

    //-- update: arc weights
    for (i=0; i < path->len; ++i) {
      cfg = (gfsmLookupTrainConfig*)path->pdata[i];
      if (cfg->a)
	cfg->a->weight += arcMass;
      if (cfg->qf != gfsmNoState) {
	gfsm_automaton_set_final_state_full(fst, cfg->qf, TRUE, gfsm_automaton_get_final_weight(fst,cfg->qf) + arcMass);
      }
    }

    //-- cleanup: path
    g_ptr_array_free(path, TRUE);
  }

  //-- cleanup: used configs
  while (used != NULL) {
    cfg  = (gfsmLookupTrainConfig*)used->data;
    used = g_slist_delete_link(used,used);
    gfsm_slice_free(gfsmLookupTrainConfig,cfg);
  }

  return fst;
}


/*======================================================================
 * Methods: Viterbi
 */


//--------------------------------------------------------------
gfsmAutomaton *gfsm_automaton_lookup_viterbi_full(gfsmAutomaton     *fst,
						  gfsmLabelVector   *input,
						  gfsmAutomaton     *trellis,
						  gfsmStateIdVector *trellis2fst)
{
  //-- cols: array of (GSList <gfsmViterbiConfig*> *)
  gfsmViterbiTable *cols = g_ptr_array_sized_new(input->len+1);
  GSList           *col, *prevcoli;
  gfsmViterbiMap   *fst2trellis = gfsm_viterbi_map_new();
  guint             i;
  gboolean          trellis2fst_is_tmp = FALSE;
  gfsmStateId       qid_trellis, qid_trellis_nxt, qid_fst;
  gpointer          ptr_qid_trellis_nxt;
  gfsmState        *q_trellis, *q_trellis_nxt, *q_fst;
  gfsmArcIter       ai;
  gfsmWeight        w_trellis;

  //-- ensure trellis automaton exists and is clear
  if (trellis==NULL) {
    trellis = gfsm_automaton_shadow(fst);
  } else {
    gfsm_automaton_clear(trellis);
  }
  trellis->flags.is_transducer = TRUE;

  //-- ensure trellis->fst stateid-map exists and is clear
  if (!trellis2fst) {
    trellis2fst = g_ptr_array_sized_new(input->len+1);
    trellis2fst_is_tmp = TRUE;
  } else if (trellis2fst->len < 2) {
    g_ptr_array_set_size(trellis2fst, input->len+1);
  }

  //-- initial config: trellis structure
  qid_trellis = trellis->root_id = gfsm_automaton_add_state(trellis);
  q_trellis = gfsm_automaton_find_state(trellis, qid_trellis);
  gfsm_automaton_set_final_state_full(trellis, qid_trellis, TRUE, fst->sr->one);
  gfsm_automaton_add_arc(trellis, qid_trellis, qid_trellis, gfsmNoLabel, gfsmNoLabel, fst->sr->one);

  //-- initial config: stateid-mappings
  g_ptr_array_index(trellis2fst, qid_trellis) = GUINT_TO_POINTER(fst->root_id);
  g_tree_insert(fst2trellis, GUINT_TO_POINTER(fst->root_id), GUINT_TO_POINTER(qid_trellis));

  //-- initial config: epsilon-expansion on column
  g_ptr_array_index(cols,0) = col = g_slist_prepend(NULL, GUINT_TO_POINTER(qid_trellis));
  _gfsm_viterbi_expand_column(fst, trellis, col, trellis2fst, fst2trellis);

  //-- initial config: cleanup
  gfsm_viterbi_map_free(fst2trellis);


  //-- ye olde loope: for each input character (i)
  for (i=0; i < input->len; i++) {
    gfsmLabelVal a  = (gfsmLabelVal)GPOINTER_TO_UINT(g_ptr_array_index(input, i));

    fst2trellis = gfsm_viterbi_map_new();
    col         = NULL;

    //-- get possible successors
    for (prevcoli=(GSList*)g_ptr_array_index(cols,i); prevcoli != NULL; prevcoli=prevcoli->next) {

      //-- get the top element of the queue
      qid_trellis = (gfsmStateId)GPOINTER_TO_UINT(prevcoli->data);
      qid_fst     = (gfsmStateId)GPOINTER_TO_UINT(g_ptr_array_index(trellis2fst, qid_trellis));

      //-- get state pointers
      q_trellis = gfsm_automaton_find_state(trellis, qid_trellis);
      q_fst     = gfsm_automaton_find_state(fst,     qid_fst);

      //-- get Viterbi properties
      w_trellis = gfsm_viterbi_node_best_weight(q_trellis);


      //-- search for input-matching arcs & add them to the successor map for next column
      for (gfsm_arciter_open_ptr(&ai, fst, q_fst), gfsm_arciter_seek_lower(&ai,a);
	   gfsm_arciter_ok(&ai);
	   gfsm_arciter_next(&ai), gfsm_arciter_seek_lower(&ai,a))
	{
	  gfsmArc     *arc_fst         = gfsm_arciter_arc(&ai);
	  gfsmWeight   w_trellis_nxt;
	  gpointer     orig_key;

	  //-- found a matching arc: is its target state already marked as a successor?
	  if (g_tree_lookup_extended(fst2trellis,
				     GUINT_TO_POINTER(arc_fst->target),
				     &orig_key,
				     &ptr_qid_trellis_nxt))
	    {
	      //-- yep: known successor: get old ("*_nxt") & new ("*_nxt_new") weights
	      gfsmWeight w_trellis_nxt_new = gfsm_sr_times(fst->sr, w_trellis, arc_fst->weight);
	      qid_trellis_nxt = GPOINTER_TO_UINT(ptr_qid_trellis_nxt);
	      q_trellis_nxt   = gfsm_automaton_find_state(trellis, qid_trellis_nxt);
	      w_trellis_nxt   = gfsm_viterbi_node_best_weight(q_trellis_nxt);

	      //-- is the new path better than the stored path?
	      if (gfsm_sr_less(fst->sr, w_trellis_nxt_new, w_trellis_nxt)) {
		//-- yep: update mappings: trellis automaton
		gfsmArc *arc_trellis_nxt = gfsm_viterbi_node_arc(q_trellis_nxt);
		arc_trellis_nxt->target  = qid_trellis;
		arc_trellis_nxt->lower   = a;
		arc_trellis_nxt->upper   = arc_fst->upper;
		arc_trellis_nxt->weight  = w_trellis_nxt_new;

		//-- update mappings: trellis->fst stateid-map
		g_ptr_array_index(trellis2fst, qid_trellis_nxt) = GUINT_TO_POINTER(arc_fst->target);

		//-- update mappings: fst->trellis stateid-map
		g_tree_insert(fst2trellis, GUINT_TO_POINTER(arc_fst->target), GUINT_TO_POINTER(qid_trellis_nxt));
	      }
	    }
	else
	  {
	    //-- target state not already marked as a successor: mark it
	    qid_trellis_nxt = gfsm_automaton_add_state(trellis);
	    q_trellis_nxt   = gfsm_automaton_find_state(trellis,qid_trellis_nxt);
	    gfsm_automaton_add_arc(trellis,
				   qid_trellis_nxt, qid_trellis,
				   a,               arc_fst->upper,
				   gfsm_sr_times(fst->sr, w_trellis, arc_fst->weight));

	    //-- save trellis->fst stateid-map
	    if (qid_trellis_nxt >= trellis2fst->len) {
	      g_ptr_array_set_size(trellis2fst, qid_trellis_nxt + gfsmLookupStateMapGet);
	    }
	    g_ptr_array_index(trellis2fst,qid_trellis_nxt) = GUINT_TO_POINTER(arc_fst->target);

	    //-- save fst->trellis stateid-map
	    g_tree_insert(fst2trellis, GUINT_TO_POINTER(arc_fst->target), GUINT_TO_POINTER(qid_trellis_nxt));

	    //-- add new trellis state to the column
	    col = g_slist_prepend(col, GUINT_TO_POINTER(qid_trellis_nxt));
	  }

	} //-- END: seek input-matching arcs
    } //-- END: previous column iteration (prevcoli)

    //-- expand epsilons in current column
    _gfsm_viterbi_expand_column(fst, trellis, col, trellis2fst, fst2trellis);

    //-- update column table
    g_ptr_array_index(cols,i+1) = col;

    //-- per-input-index cleanup
    gfsm_viterbi_map_free(fst2trellis);
  }

  //-- final iteration (EOS): get possible "final" states
  qid_trellis_nxt = gfsm_automaton_add_state(trellis); //-- qid_trellis_nxt: new root
  for (prevcoli=(GSList*)g_ptr_array_index(cols,input->len); prevcoli != NULL; prevcoli=prevcoli->next) {

    //-- get the top element of the queue
    qid_trellis = (gfsmStateId)GPOINTER_TO_UINT(prevcoli->data);
    qid_fst     = (gfsmStateId)GPOINTER_TO_UINT(g_ptr_array_index(trellis2fst, qid_trellis));
      
    //-- get state pointers
    q_trellis = gfsm_automaton_find_state(trellis, qid_trellis);
    q_fst     = gfsm_automaton_find_state(fst,     qid_fst);
    
    //-- get Viterbi properties
    w_trellis = gfsm_viterbi_node_best_weight(q_trellis);

    //-- check for finality
    if (q_fst->is_final) {
      gfsm_automaton_add_arc(trellis, qid_trellis_nxt, qid_trellis,
			     gfsmEpsilon, gfsmEpsilon,
			     gfsm_sr_times(fst->sr,
					   w_trellis,
					   gfsm_automaton_get_final_weight(fst,qid_fst)));
    }
  }

  //-- mark single best path from new root
  qid_trellis = qid_trellis_nxt;
  q_trellis = gfsm_automaton_find_state(trellis,qid_trellis);
  q_trellis->arcs = gfsm_arclist_sort(q_trellis->arcs,
				      &((gfsmArcCompData){gfsmASMWeight,fst->sr,NULL,NULL}));

  //-- break dummy arc on trellis final state (old root)
  q_trellis = gfsm_automaton_find_state(trellis,trellis->root_id);
  gfsm_arclist_free(q_trellis->arcs);
  q_trellis->arcs = NULL;

  //-- mark new root
  trellis->root_id = qid_trellis;


  //-- cleanup: columns
  for (i=0; i < cols->len; i++) {
    g_slist_free((GSList*)g_ptr_array_index(cols,i));
  }

  //-- cleanup: column array
  g_ptr_array_free(cols,TRUE);
  if (trellis2fst_is_tmp) g_ptr_array_free(trellis2fst,TRUE);
  else {
    //-- just set length
    trellis2fst->len = trellis->states->len;
  }

  return trellis;
}


/*======================================================================
 * Methods: Viterbi: expand_column
 */

//--------------------------------------------------------------
void _gfsm_viterbi_expand_column(gfsmAutomaton        *fst,
				 gfsmAutomaton        *trellis,
				 gfsmViterbiColumn    *col,
				 gfsmStateIdVector    *trellis2fst,
				 gfsmViterbiMap       *fst2trellis)
{
  gfsmArcIter ai;
  gfsmViterbiColumn *coli;
  gfsmStateId        qid_trellis, qid_fst;
  gfsmState         *q_trellis;
  //gfsmArc           *arc_trellis;
  gfsmWeight         w_trellis;

  //-- pass-1: add everything already in the column as a literal
  /*
  for (coli=col; coli != NULL; coli=coli->next) {
    node = (gfsmViterbiNode*)(coli->data);
    if (!g_tree_lookup(fst2trellis,node->key)) {
      g_tree_insert(cmap,node->key,node->val);
    }
  }
  */

  //-- pass-2: add epsilon arcs from every literal in the column
  for (coli=col; coli != NULL; coli=coli->next) {
    //-- get node
    qid_trellis = (gfsmStateId)GPOINTER_TO_UINT(coli->data);
    q_trellis   = gfsm_automaton_find_state(trellis,qid_trellis);
    //arc_trellis = gfsm_viterbi_node_arc(q_trellis);
    w_trellis   = gfsm_viterbi_node_best_weight(q_trellis);
    qid_fst     = (gfsmStateId)GPOINTER_TO_UINT(g_ptr_array_index(trellis2fst,qid_trellis));

    //-- search for input-epsilon arcs & add them to this column
    for (gfsm_arciter_open(&ai,fst,qid_fst), gfsm_arciter_seek_lower(&ai,gfsmEpsilon);
	 gfsm_arciter_ok(&ai);
	 gfsm_arciter_next(&ai), gfsm_arciter_seek_lower(&ai,gfsmEpsilon))
      {
	gfsmArc     *arc_fst = gfsm_arciter_arc(&ai);
	gfsmStateId  qid_trellis_nxt = gfsmNoState;
	gpointer     ptr_qid_trellis_nxt;
	gfsmState   *q_trellis_nxt;
	gfsmWeight   w_trellis_nxt;
	gpointer     orig_key;

	//-- found an eps-arc: is its target state already marked as a successor?
	if (g_tree_lookup_extended(fst2trellis,
				   GUINT_TO_POINTER(arc_fst->target),
				   &orig_key,
				   &ptr_qid_trellis_nxt))
	  {
	    //-- yep: get the old ("*_eps") & new ("*_nxt") weights
	    gfsmWeight w_trellis_eps = gfsm_sr_times(fst->sr, w_trellis, arc_fst->weight);
	    qid_trellis_nxt = GPOINTER_TO_UINT(ptr_qid_trellis_nxt);
	    q_trellis_nxt   = gfsm_automaton_find_state(trellis,qid_trellis_nxt);
	    w_trellis_nxt   = gfsm_viterbi_node_best_weight(q_trellis_nxt);

	    //-- is the new eps-path better than the stored path?
	    if (gfsm_sr_less(fst->sr,w_trellis_eps,w_trellis_nxt)) {
	      //-- yep: update mappings: trellis automaton
	      gfsmArc *arc_trellis_nxt = gfsm_viterbi_node_arc(q_trellis_nxt);
	      arc_trellis_nxt->target  = qid_trellis;
	      arc_trellis_nxt->lower   = gfsmEpsilon;
	      arc_trellis_nxt->upper   = arc_fst->upper;
	      arc_trellis_nxt->weight  = w_trellis_eps;

	      //-- update mappings: trellis->fst stateid-map
	      g_ptr_array_index(trellis2fst, qid_trellis_nxt) = GUINT_TO_POINTER(arc_fst->target);

	      //-- update mappings: fst->trellis stateid-map
	      g_tree_insert(fst2trellis, GUINT_TO_POINTER(arc_fst->target), GUINT_TO_POINTER(qid_trellis_nxt));
	    }
	    else {
	      //-- eps-path is worse than the existing path: forget about it
	      ;
	    }
	  }
	else
	  {
	    //-- eps-target state not already marked as a successor: mark it
	    qid_trellis_nxt = gfsm_automaton_add_state(trellis);
	    q_trellis_nxt   = gfsm_automaton_find_state(trellis,qid_trellis_nxt);
	    gfsm_automaton_add_arc(trellis,
				   qid_trellis_nxt, qid_trellis,
				   gfsmEpsilon,     arc_fst->upper,
				   gfsm_sr_times(fst->sr, w_trellis, arc_fst->weight));

	    //-- save trellis->fst stateid-map
	    if (qid_trellis_nxt >= trellis2fst->len) {
	      g_ptr_array_set_size(trellis2fst, qid_trellis_nxt + gfsmLookupStateMapGet);
	    }
	    g_ptr_array_index(trellis2fst,qid_trellis_nxt) = GUINT_TO_POINTER(arc_fst->target);

	    //-- save fst->trellis stateid-map
	    g_tree_insert(fst2trellis, GUINT_TO_POINTER(arc_fst->target), GUINT_TO_POINTER(qid_trellis_nxt));

	    //-- queue-up new trellis state for eps-seek
	    coli->next = g_slist_prepend(coli->next, GUINT_TO_POINTER(qid_trellis_nxt));
	  }

      } //-- END: seek epsilon arcs

  } //-- END column iteration

}



/*======================================================================
 * Methods: Viterbi: Map
 */

//--------------------------------------------------------------
#if 0
gfsmViterbiNodeValue *gfsm_viterbi_column_map_insert_if_less(gfsmViterbiMap      *vmap,
							     gfsmViterbiNodeKey    key,
							     gfsmWeight            w,
							     gfsmSemiring         *sr)
{
  gpointer s_val;
  if (s_val = gfsm_viterbi_map_lookup(vmap,key)) {
    //-- already present
    if (!gfsm_sr_less(sr,w,s_val->w)) return NULL; //-- (s_val->w) <= (w)
    s_val->w = w;
  } else {
    //-- not already present: copy & insert
    s_val  = gfsm_slice_new(gfsmViterbiNodeValue);
    s_val->qtrellis = gfsmNoState;
    a_val->pqtrellis = gfsmNoState;
    s_val->w        = w;
    g_tree_insert(col,key,s_val);
  }
  return s_val; //-- update occurred
}
#endif

