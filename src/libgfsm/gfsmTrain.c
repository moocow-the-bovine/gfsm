
/*=============================================================================*\
 * File: gfsmTrain.c
 * Author: Bryan Jurish <moocow.bovine@gmail.com>
 * Description: finite state machine library
 *
 * Copyright (c) 2014 Bryan Jurish.
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

#include <gfsmTrain.h>

#include <gfsmAlphabet.h>
#include <gfsmState.h>
#include <gfsmArc.h>
#include <gfsmArcIter.h>

#include <string.h>

/*======================================================================
 * Methods: train: low-level
 */

//--------------------------------------------------------------
GFSM_INLINE
gfsmTrainConfig* gfsm_train_config_new(guint32 i, guint32 j, gfsmStateId qf, gfsmArc *a, gfsmTrainConfig *prev)
{
  gfsmTrainConfig *cfg = (gfsmTrainConfig*)gfsm_slice_new(gfsmTrainConfig);
  cfg->i = i;
  cfg->j = j;
  cfg->qf = qf;
  cfg->a = a;
  cfg->prev = prev;
  return cfg;
}

//--------------------------------------------------------------
GFSM_INLINE
gfsmStateId gfsm_train_config_stateid(gfsmTrainConfig *cfg, gfsmAutomaton *fst)
{
  return (cfg->a ? cfg->a->target : (cfg->qf==gfsmNoState ? gfsm_automaton_get_root(fst) : cfg->qf));
}

//--------------------------------------------------------------
GFSM_INLINE
guint gfsm_train_config_pathlength(gfsmTrainConfig *cfg)
{
  guint l=0;
  for ( ; cfg!=NULL; cfg=cfg->prev, ++l) ;
  return l;
}

//--------------------------------------------------------------
GFSM_INLINE
gint gfsm_train_config_compare(const gfsmTrainConfig **c1, const gfsmTrainConfig **c2)
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
gint gfsm_train_path_compare(gfsmTrainPath *p1, gfsmTrainPath *p2)
{
  guint i1,i2;
  for (i1=0, i2=0; i1<p1->len && i2<p2->len; ++i1, ++i2) {
    gint cmp = gfsm_train_config_compare((const gfsmTrainConfig**)p1->pdata+i1, (const gfsmTrainConfig**)p2->pdata+i2);
    if (cmp != 0) return cmp;
  }
  if      (i1==p1->len && i2< p2->len) return -1;
  else if (i1< p1->len && i2==p2->len) return  1;
  return 0;
}

//--------------------------------------------------------------
GFSM_INLINE
gfsmTrainPath* gfsm_train_path_new(gfsmTrainConfig *cfg)
{
  gfsmTrainPath *p = g_ptr_array_sized_new(gfsm_train_config_pathlength(cfg)+1);   //-- allocate an extra pointer for path-weight
  for ( ; cfg!=NULL; cfg=cfg->prev) {
    p->pdata[p->len++] = cfg;
  }
  return p;
}

//--------------------------------------------------------------
// + computes and caches path weight in path->pdata[path->len]
static
gfsmWeight gfsm_trainer_get_path_weight(gfsmTrainer *trainer, gfsmTrainPath *path)
{
  gfsmSemiring *sr = trainer->fst->sr;
  gfsmWeight     w = gfsm_sr_one(sr);
  guint i, wi;
  for (i=0; i < path->len; ++i) {
    gfsmTrainConfig *cfg = (gfsmTrainConfig*)path->pdata[i];
    if (cfg->a) {
      wi = cfg->a->weight;
      w  = gfsm_sr_times(sr, w, trainer->weights[wi]);
    }
    else if (cfg->qf != gfsmNoState) {
      wi = gfsm_automaton_get_final_weight(trainer->fst, cfg->qf);
      w  = gfsm_sr_times(sr, w, trainer->weights[wi]);
    }
  }
  path->pdata[path->len] = gfsm_weight2ptr(w);
  return w;
}


//--------------------------------------------------------------
#ifdef GFSM_DEBUG_TRAIN
static
void gfsm_train_path_dump(FILE *f, gfsmTrainPath *path, gfsmAutomaton *fst)
{
  guint i;
  fprintf(f,"{ ");
  for (i=0; i<path->len; ++i) {
    gfsmTrainConfig *cfg = (gfsmTrainConfig*)path->pdata[i];
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
void gfsm_train_paths_dump(FILE *f, GSList *paths, gfsmAutomaton *fst)
{
  for ( ; paths != NULL; paths=paths->next) {
    gfsm_train_path_dump(f, ((gfsmTrainPath*)paths->data), fst);
  }
}
#endif /* GFSM_DEBUG_TRAIN */

/*======================================================================
 * Methods: train: high-level
 */

//--------------------------------------------------------------
gfsmTrainer* gfsm_trainer_new(gfsmAutomaton *fst)
{
  gfsmTrainer *trainer = (gfsmTrainer*)gfsm_slice_new0(gfsmTrainer);
  guint wi=0;
  gfsmStateId qid;
  gfsmArcIter ai;

  //-- set basic data
  trainer->fst = fst;
  trainer->nweights = gfsm_automaton_n_arcs(fst) + gfsm_automaton_n_final_states(fst);

  //-- allocate weights
  trainer->weights = gfsm_new (gfsmWeight, trainer->nweights);
  trainer->counts  = gfsm_new0(gfsmWeight, trainer->nweights);

  //-- swap out transducer weights -> trainer vector-indices, storing original weights as we go
  for (qid=0; qid < gfsm_automaton_n_states(fst); ++qid) {
    if (!gfsm_automaton_has_state(fst,qid)) continue;

    //-- weight-swap: final weights
    if (gfsm_automaton_state_is_final(fst,qid)) {
      trainer->weights[wi] = gfsm_automaton_get_final_weight(fst,qid);
      gfsm_automaton_set_final_state_full(fst,qid,TRUE,wi);
      ++wi;
    }

    //-- weight-swap: arc weights
    for (gfsm_arciter_open(&ai,fst,qid); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *arc = gfsm_arciter_arc(&ai);
      trainer->weights[wi] = arc->weight;
      arc->weight = wi;
      ++wi;
    }
  }

  return trainer;
}

//--------------------------------------------------------------
void gfsm_trainer_free(gfsmTrainer *trainer, gboolean free_fst)
{
  if (free_fst && trainer->fst) gfsm_automaton_free(trainer->fst);
  if (trainer->weights) gfsm_free(trainer->weights);
  if (trainer->counts)  gfsm_free(trainer->counts);
  gfsm_slice_free(gfsmTrainer,trainer);
}

//--------------------------------------------------------------
gfsmAutomaton* gfsm_trainer_automaton(gfsmTrainer *trainer, gboolean steal)
{
  gfsmAutomaton *fst;
  gfsmStateId qid;
  gfsmArcIter ai;
  guint wi;

  //-- target automaton
  if (steal) {
    fst = trainer->fst;
    trainer->fst = NULL;
  } else {
    fst = gfsm_automaton_clone(trainer->fst);
  }

  //-- swap in training counts for vector indices in transducer weights
  for (qid=0; qid < gfsm_automaton_n_states(fst); ++qid) {
    if (!gfsm_automaton_has_state(fst,qid)) continue;

    //-- weight-swap: final weights
    if (gfsm_automaton_state_is_final(fst,qid)) {
      wi = gfsm_automaton_get_final_weight(fst,qid);
      gfsm_automaton_set_final_state_full(fst, qid, TRUE, trainer->counts[wi]);
    }

    //-- weight-swap: arc weights
    for (gfsm_arciter_open(&ai,fst,qid); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *arc = gfsm_arciter_arc(&ai);
      wi = arc->weight;
      arc->weight = trainer->counts[wi];
    }
  }

  return fst;
}

//--------------------------------------------------------------
void gfsm_trainer_train(gfsmTrainer *trainer, gfsmLabelVector *input, gfsmLabelVector *output)
{
  gfsmAutomaton *fst = trainer->fst;
  gfsmTrainConfig *cfg=gfsm_train_config_new(0,0,gfsmNoState,NULL,NULL), *cfg_new=NULL;
  GSList *stack=NULL; //-- DFS search stack, GSList* of gfsmTrainConfig*
  GSList *used=NULL;  //-- used search configs, GSList* of gfsmTrainConfig*
  GSList *succ=NULL;  //-- final configs of successful paths, GSList* of path-final gfsmTrainConfig*
  GSList *paths=NULL; //-- full successful paths, GSList* of gfsmTrainPath* (==GPtrArray*)
  GSList *nod;	      //-- list-iterator temporary
  gfsmStateId	    q;
  const gfsmState  *qt;
  gfsmLabelVal      a,b;
  gfsmArcIter       ai;
  gfsmWeight        pathMass, arcMass;

  //-- sanity check(s)
  if (!trainer->fst) {
    g_printerr("gfsm_trainer_train(): cannot train NULL fst");
    exit(1);
  }

  //-- initialization
  stack = g_slist_prepend(stack, cfg);

  //-- ye olde loope
  while (stack != NULL) {
    //-- pop the top element off the stack, and push it to the 'used'-list
    cfg       = (gfsmTrainConfig*)(stack->data);
    nod       = stack;
    stack     = g_slist_remove_link(stack, nod);
    nod->next = used;
    used      = nod;

    //-- get state, labels
    q  = gfsm_train_config_stateid(cfg, fst);
    qt = gfsm_automaton_find_state_const(fst, q);
    a  = (cfg->i < input->len  ? (gfsmLabelVal)GPOINTER_TO_UINT(g_ptr_array_index(input,  cfg->i)) : gfsmNoLabel);
    b  = (cfg->j < output->len ? (gfsmLabelVal)GPOINTER_TO_UINT(g_ptr_array_index(output, cfg->j)) : gfsmNoLabel);

    //-- check for final states
    if (cfg->i >= input->len && cfg->j >= output->len && gfsm_state_is_final(qt)) {
      cfg_new = gfsm_train_config_new(cfg->i, cfg->j, q, NULL, cfg);
      succ    = g_slist_prepend(succ, cfg_new);
      used    = g_slist_prepend(used, cfg_new);
    }

    //-- handle outgoing arcs
    for (gfsm_arciter_open_ptr(&ai, fst, (gfsmState*)qt); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai))
      {
	gfsmArc *arc = gfsm_arciter_arc(&ai);

	//-- arc: (eps,eps)
	if (arc->lower==gfsmEpsilon && arc->upper==gfsmEpsilon) {
	  cfg_new = gfsm_train_config_new(cfg->i, cfg->j, gfsmNoState, arc, cfg);
	}
	//-- arc: (eps,b)
	else if (arc->lower==gfsmEpsilon && arc->upper==b && b!=gfsmNoLabel) {
	  cfg_new = gfsm_train_config_new(cfg->i, cfg->j+1, gfsmNoState, arc, cfg);
	}
	//-- arc: (a,eps)
	else if (arc->lower==a && arc->upper==gfsmEpsilon && a!=gfsmNoLabel) {
	  cfg_new = gfsm_train_config_new(cfg->i+1, cfg->j, gfsmNoState, arc, cfg);
	}
	//-- arc: (a,b)
	else if (arc->lower==a && arc->upper==b && a!=gfsmNoLabel && b!=gfsmNoLabel) {
	  cfg_new = gfsm_train_config_new(cfg->i+1, cfg->j+1, gfsmNoState, arc, cfg);
	}
	//-- arc: other
	else {
	  continue;
	}
	stack = g_slist_prepend(stack, cfg_new);
      }
    gfsm_arciter_close(&ai);
  }

  //-------- convert successful configs to paths
  for (nod=succ; nod!=NULL; nod=nod->next) {
    paths = g_slist_prepend(paths, gfsm_train_path_new((gfsmTrainConfig*)nod->data));;
  }

  //-------- prune (multiple successful paths only)
  if (paths && paths->next) {
    gfsmTrainPath *path;
    GSList **pnod;

    //---- prune: by cost
    if (trainer->bestPathsOnly) {
      gfsmSemiring   *sr = trainer->fst->sr;
      gfsmWeight w, wmin = gfsm_sr_zero(sr);

      //-- find minimum cost
      for (nod=paths; nod!=NULL; nod=nod->next) {
	w = gfsm_trainer_get_path_weight(trainer, (gfsmTrainPath*)nod->data); //-- cache path-weight
	if (gfsm_sr_less(sr, w, wmin)) wmin = w;
      }

      //-- prune all but minimal-cost paths
      for (pnod=&paths; *pnod != NULL; ) {
	path = (gfsmTrainPath*)(*pnod)->data;
	w = gfsm_ptr2weight(path->pdata[path->len]); //-- use cached path-weight
	if (gfsm_sr_less(sr, wmin, w)) {
	  g_ptr_array_free(path, TRUE);
	  *pnod = g_slist_delete_link(*pnod,*pnod);
	}
	else {
	  pnod = &(*pnod)->next;
	}
      }
    }

    //---- prune: permutations
    if (trainer->prunePathPermutations) {
      //-- sort arcs of each successful path (by arc-pointer value): get permutation equivalence-class
      for (nod=paths; nod!=NULL; nod=nod->next) {
	path = (gfsmTrainPath*)nod->data;
	g_ptr_array_sort(path, (GCompareFunc)gfsm_train_config_compare);
      }

      //-- sort list of successful paths lexicographically by permutation eq-class
      paths = g_slist_sort(paths, (GCompareFunc)gfsm_train_path_compare);

      //-- prune duplicate path-classes
      for (pnod=&paths, nod=paths->next; nod!=NULL; nod=(*pnod)->next) {
	if (gfsm_train_path_compare((gfsmTrainPath*)(*pnod)->data, (gfsmTrainPath*)nod->data)==0) {
	  g_ptr_array_free( (gfsmTrainPath*)nod->data, TRUE );
	  (*pnod)->next = g_slist_delete_link(nod,nod);
	} else {
	  pnod = &((*pnod)->next);
	}
      }
    }

  }


  //-- update: count-mass
  pathMass = trainer->distributeOverPaths ? (1.0/g_slist_length(paths)) : 1.0;
  while (paths != NULL) {
    guint i, wi;
    gfsmTrainPath* path = (gfsmTrainPath*)paths->data;
    paths    = g_slist_delete_link(paths,paths);
    arcMass  = trainer->distributeOverArcs ? (pathMass/(path->len-1.0)) : pathMass;

    //-- update: arc weights
    for (i=0; i < path->len; ++i) {
      cfg = (gfsmTrainConfig*)path->pdata[i];
      if (cfg->a) {
	wi = cfg->a->weight;
	trainer->counts[wi] += arcMass;
      }
      if (cfg->qf != gfsmNoState) {
	wi = gfsm_automaton_get_final_weight(fst,cfg->qf);
	trainer->counts[wi] += arcMass;
      }
    }

    //-- cleanup: path
    g_ptr_array_free(path, TRUE);
  }

  //-- cleanup: used configs
  while (used != NULL) {
    cfg  = (gfsmTrainConfig*)used->data;
    used = g_slist_delete_link(used,used);
    gfsm_slice_free(gfsmTrainConfig,cfg);
  }

}
