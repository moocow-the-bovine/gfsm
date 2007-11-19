
/*=============================================================================*\
 * File: gfsmAlgebra.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library
 *
 * Copyright (c) 2004-2007 Bryan Jurish.
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
#include <gfsmStateSet.h>
#include <gfsmEnum.h>
#include <gfsmUtils.h>
#include <gfsmCompound.h>

/*======================================================================
 * Methods: algebra: closure
 */

struct gfsmAutomatonClosureData_ {
  gfsmAutomaton *fsm;
  gfsmStateId    root;
};

/*--------------------------------------------------------------
 * closure_final_func_()
 *  + called for each final @id of @fsm during closure(@fsm)
 */
gboolean gfsm_automaton_closure_final_func_(gpointer pid, gpointer pw, struct gfsmAutomatonClosureData *cdata)
{
  gfsmStateId id = GPOINTER_TO_UINT(pid);
  gfsmWeight   w = gfsm_ptr2weight(pw);
  if (id != cdata->root)
    gfsm_automaton_add_arc(cdata->fsm, id, cdata->root, gfsmEpsilon, gfsmEpsilon, w);
  return FALSE;
}


/*--------------------------------------------------------------
 * closure()
 */
gfsmAutomaton *gfsm_automaton_closure(gfsmAutomaton *fsm, gboolean is_plus)
{
  gfsmStateId root = fsm ? gfsm_automaton_get_root(fsm) : gfsmNoState;
  gfsmAutomatonClosureData cdata = { fsm, root };

  //-- sanity check(s)
  if (root==gfsmNoState) return fsm;
  gfsm_automaton_ensure_mutable(fsm);

  //-- add epsilon arcs from old final states to translated new root
  gfsm_automaton_foreach_final(fsm, (GTraverseFunc)gfsm_automaton_closure_final_func_, &cdata);

  //-- reflexive+transitive or reflexive?
  if (!is_plus) gfsm_automaton_optional(fsm);

  return fsm;
}


/*--------------------------------------------------------------
 * n_closure()
 */
gfsmAutomaton *gfsm_automaton_n_closure(gfsmAutomaton *fsm, guint n)
{
  gfsmStateId root = fsm ? gfsm_automaton_get_root(fsm) : gfsmNoState;

  //-- sanity check(s)
  if (!fsm || gfsm_automaton_get_root(fsm)==gfsmNoState) return fsm;

  //-- check for simple closures
  if (n == 0)      return gfsm_automaton_closure(fsm, FALSE);
  else if (n == 1) return gfsm_automaton_closure(fsm, TRUE);
  else {
    gfsm_automaton_n_concat(fsm, fsm, n-1);
  }

  return gfsm_automaton_closure(fsm, TRUE);
}


/*--------------------------------------------------------------
 * complement()
 */
gfsmAutomaton *gfsm_automaton_complement(gfsmAutomaton *fsm)
{
  gfsmAlphabet *alph = gfsm_identity_alphabet_new();
  gfsm_automaton_get_alphabet(fsm, gfsmLSLower, alph);
  gfsm_automaton_complement_full(fsm,alph);
  gfsm_alphabet_free(alph);
  return fsm;
}

/*--------------------------------------------------------------
 * complement_full()
 */
gfsmAutomaton *gfsm_automaton_complement_full(gfsmAutomaton *fsm, gfsmAlphabet *alph)
{
  gfsmStateId id, sink_id, n_states;
  gfsm_automaton_complete(fsm, alph, &sink_id);

  //-- flip final states (no weights here)
  n_states = gfsm_automaton_n_states(fsm);
  for (id = 0; id < n_states; id++) {
    if (!gfsm_automaton_has_state(fsm,id)) continue;
    gfsm_automaton_set_final_state_full(fsm, id, (!s->is_final), fsm->sr->one);
  }

  return fsm;
}


/*--------------------------------------------------------------
 * complete()
 */
gfsmAutomaton *gfsm_automaton_complete(gfsmAutomaton *fsm, gfsmAlphabet *alph, gfsmStateId *sinkp)
{
  gfsmStateId  id, sinkid, n_states;
  GPtrArray    *alabels;

  if (!fsm->flags.is_deterministic) { gfsm_automaton_determinize(fsm); }
  gfsm_automaton_arcsort(fsm,gfsmASMLower);
  fsm->flags.sort_mode = gfsmASMNone; //-- avoid "smart" arc insertion

  //-- add sink-id
  sinkid = gfsm_automaton_add_state(fsm);
  if (sinkp) *sinkp = sinkid;

  //-- get alphabet label-vector
  alabels = g_ptr_array_sized_new(gfsm_alphabet_size(alph));
  gfsm_alphabet_labels_to_array(alph,alabels);

  n_states = gfsm_automaton_n_states(fsm);
  for (id = 0; id < n_states; id++) {
    gfsmArcIter  ai;
    gfsmArc     *a;
    guint        labi;
    GSList      *addlabs=NULL; //-- temporary GSList whose data is gfsmLabelVal for "missing" arcs
    if (!gfsm_automaton_has_state(fsm)) continue;

    gfsm_arciter_open(&ai,fsm,id);
    a = gfsm_arciter_arc(&ai);
    for (labi=0; labi < alabels->len; ) {
      gfsmLabelVal lab = (gfsmLabelVal)GPOINTER_TO_UINT(g_ptr_array_index(alabels,labi));

      if (lab==gfsmEpsilon) {
	++labi;
      }
      else if (!a || a->lower > lab) {
	//-- no arc for this label: route it to sink
	addlabs = g_slist_prepend(addarcs, GUINT_TO_POINTER(lab));
	++labi;
      }
      else if (a->lower == lab) {
	++labi;
      }
      else {
	while (gfsm_arciter_ok(&ai) && a->lower < lab) {
	  gfsm_arciter_next(&ai);
	  a = gfsm_arciter_arc(&ai);
	}
      }
    }
    gfsm_arciter_close(&ai);

    //-- add collected arcs
    while (addlabs != NULL) {
      lab = GPOINTER_TO_UINT(addlabs->data);
      gfsm_automaton_add_arc(fsm, id, sinkid, lab, lab, fsm->sr->one);
      addlabs = g_slist_delete_link(addlabs,addlabs);
    }
  }

  //-- mark fsm as (still) deterministic
  fsm->flags.is_deterministic = TRUE;

  //-- cleanup
  g_ptr_array_free(alabels,TRUE);

  return fsm;
}


/*--------------------------------------------------------------
 * compose()
 */
gfsmAutomaton *gfsm_automaton_compose(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  gfsmAutomaton *tmp = gfsm_automaton_compose_full(fsm1,fsm2, NULL,NULL);
  gfsm_automaton_swap(fsm1,tmp);
  gfsm_automaton_free(tmp);
  return fsm1;
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
  gfsmStateId       rootid;
#ifdef GFSM_DEBUG_COMPOSE
  gfsmError *err =NULL;
#endif

  //-- setup: output fsm
  if (!composition) {
    composition=gfsm_automaton_shadow_classed(fsm1,gfsmAutomatonDefaultClass);
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

  //-- guts: recursively visit states depth-first from root
  rootpair.id1 = gfsm_automaton_get_root(fsm1);
  rootpair.id2 = gfsm_automaton_get_root(fsm2);
  rootpair.idf = 0;
  rootid = gfsm_automaton_compose_visit_(rootpair, fsm1, fsm2, composition, spenum);

  //-- finalize: set new root state
  gfsm_automaton_set_root(composition, rootid);

  //-- cleanup
  if (spenum_is_temp) gfsm_enum_free(spenum);

  return composition;
}

/*--------------------------------------------------------------
 * compose_visit()
 */
//#define GFSM_DEBUG_COMPOSE_VISIT 1
#ifdef GFSM_DEBUG_COMPOSE_VISIT
# include <stdio.h>
#endif
gfsmStateId gfsm_automaton_compose_visit_(gfsmComposeState   sp,
					  gfsmAutomaton     *fsm1,
					  gfsmAutomaton     *fsm2,
					  gfsmAutomaton     *fsm,
					  gfsmComposeStateEnum *spenum)
{
  gfsmStateId qid = gfsm_enum_lookup(spenum,&sp);
  gfsmStateId qid2;
  gfsmArcIter ai1,ai2, ai1_cont,ai2_cont;
  gfsmArc     *a1,*a2;
  gfsmWeight  fw1,fw2;

#ifdef GFSM_DEBUG_COMPOSE_VISIT
  fprintf(stderr, "compose(): visit : (q%u,f%u,q%u) => q%d\n", sp.id1, sp.idf, sp.id2,
	  (int)(qid==gfsmEnumNone ? -1 : qid));
#endif

  //-- ignore already-visited states
  if (qid != gfsmEnumNone) return qid;

  //-- sanity check
  if ( !(gfsm_automaton_has_state(fsm1,sp.id1) && gfsm_automaton_has_state(fsm2,sp.id2)) )
    return gfsmNoState;

  //-- insert new state into output automaton
  qid = gfsm_automaton_add_state(fsm);
  gfsm_enum_insert_full(spenum,&sp,qid);

#ifdef GFSM_DEBUG_COMPOSE_VISIT
  fprintf(stderr, "compose(): CREATE: (q%u,f%u,q%u) => q%u ***\n", sp.id1, sp.idf, sp.id2, qid);
#endif

  //-- check for final states
  if (gfsm_automaton_lookup_final(fsm1,sp.id1,&fw1) && gfsm_automaton_lookup_final(fsm2,sp.id2,&fw2)) {
    gfsm_automaton_set_final_state_full(fsm,qid,TRUE, gfsm_sr_times(fsm->sr,fw1,fw2));
  }

  //-------------------------------------------
  // recurse on outgoing arcs

  //--------------------------------
  // recurse: arcs: sort

  //-- arcs: sort arclists: fsm1
  gfsm_arciter_open(&ai1,fsm1,sp.id1);
  if (gfsm_automaton_sortmode(fsm1) != gfsmASMUpper) {
    gfsmArcSortData sortdata = {gfsmASMUpper,fsm->sr};
    gfsm_arciter_sort(&ai1_cont, gfsm_arc_compare, &sortdata);
  }

  //-- arcs: sort arclists: fsm2
  gfsm_arciter_open(&ai2,fsm2,sp.id2);
  if (gfsm_automaton_sortmode(fsm2) != gfsmASMLower) {
    gfsmArcSortData sortdata = {gfsmASMLower,fsm->sr};
    gfsm_arciter_sort(&ai2, gfsm_arc_compare, &sortdata);
  }

  //--------------------------------
  // recusrse: arcs: seek non-epsilons
  for (gfsm_arciter_copy(&ai1_cont,&ai1);
       gfsm_arciter_ok(&ai1_cont) && gfsm_arciter_arc(&ai1_cont)->upper==gfsmEpsilon;
       gfsm_arciter_next(&ai1_cont))
    {;}
  for (gfsm_arciter_copy(&ai2_cont,&ai2);
       gfsm_arciter_ok(&ai2_cont) && gfsm_arciter_arc(&ai2_cont)->lower==gfsmEpsilon;
       gfsm_arciter_next(&ai2_cont))
    {;}

  //-- (eps,NULL): case fsm1(q1 --a:eps(~eps2)--> q1b), filter:({0,2} --eps2:eps2--> 2), fsm2(q2 --(NULL~eps2:eps)--> q2)
  if (sp.idf != 1) {
    for (; gfsm_arciter_ok(&ai1); gfsm_arciter_next(&ai1)) {
      a1   = gfsm_arciter_arc(&ai1);
      if (a1->upper != gfsmEpsilon) break;
      qid2 = gfsm_automaton_compose_visit_((gfsmComposeState){a1->target, sp.id2, 2}, fsm1, fsm2, fsm, spenum);
      if (qid2 != gfsmNoState)
	gfsm_automaton_add_arc(fsm, qid, qid2, a1->lower, gfsmEpsilon, a1->weight);
    }
  }
  //-- (NULL,eps): case fsm1(q1 --(NULL~eps:eps1)--> q1), filter:({0,1} --eps1:eps1--> 1), fsm2(q2 --eps(~eps1):b--> q2b)
  if (sp.idf != 2) {
    for (; gfsm_arciter_ok(&ai2); gfsm_arciter_next(&ai2)) {
      a2   = gfsm_arciter_arc(&ai2);
      if (a2->lower != gfsmEpsilon) break;
      qid2 = gfsm_automaton_compose_visit_((gfsmComposeState){sp.id1, a2->target, 1}, fsm1, fsm2, fsm, spenum);
      if (qid2 != gfsmNoState)
	gfsm_automaton_add_arc(fsm, qid, qid2, gfsmEpsilon, a2->upper, a2->weight);
    }
  }
  //-- (eps,eps): case fsm1(q1 --a:eps(~eps2)--> q1b), filter:({0} --eps2:eps1--> 0), fsm2(q2 --eps:b--> q2b)
  if (sp.idf == 0) {
    for (gfsm_arciter_reset(&ai1); gfsm_arciter_ok(&ai1); gfsm_arciter_next(&ai1)) {
      a1 = gfsm_arciter_arc(&ai1);
      if (a1->upper != gfsmEpsilon) break;
      for (gfsm_arciter_reset(&ai2); gfsm_arciter_ok(&ai2); gfsm_arciter_next(&ai2)) {
	a2   = gfsm_arciter_arc(&ai2);
	if (a2->lower != gfsmEpsilon) break;
	qid2 = gfsm_automaton_compose_visit_((gfsmComposeState){a1->target, a2->target, 0}, fsm1, fsm2, fsm, spenum);
	if (qid2 != gfsmNoState)
	  gfsm_automaton_add_arc(fsm, qid, qid2, a1->lower, a2->upper,
				 gfsm_sr_times(fsm->sr, a1->weight, a2->weight));
      }
    }
  }

  //--------------------------------
  // recurse: arcs: non-eps: iterate
  for (gfsm_arciter_copy(&ai1,&ai1_cont); gfsm_arciter_ok(&ai1); gfsm_arciter_next(&ai1)) {
    a1 = gfsm_arciter_arc(&ai1);

    for (gfsm_arciter_copy(&ai2,&ai2_cont); gfsm_arciter_ok(&ai2); gfsm_arciter_next(&ai2)) {
      a2 = gfsm_arciter_arc(&ai2);

      if (a2->lower < a1->upper) {
	gfsm_arciter_copy(&ai2_cont,&ai2);
	gfsm_arciter_next(&ai2_cont);
	continue;
      } else if (a2->lower > a1->upper) {
	break;
      }

      //-- non-eps: case fsm1:(q1 --a:b--> q1'), fsm2:(q2 --b:c-->  q2')
      qid2 = gfsm_automaton_compose_visit_((gfsmComposeState){a1->target,a2->target,0},
					   fsm1, fsm2, fsm, spenum);
      if (qid2 != gfsmNoState)
	gfsm_automaton_add_arc(fsm, qid, qid2, a1->lower, a2->upper,
			       gfsm_sr_times(fsm1->sr, a1->weight, a2->weight));
    }
  }

  gfsm_arciter_close(&ai1);
  gfsm_arciter_close(&ai2);

  return qid;
}


/*--------------------------------------------------------------
 * concat_full()
 */
gfsmAutomaton *gfsm_automaton_concat_full(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2, gfsmAutomaton *concat)
{
  gfsmStateId   qid, size1,size2, root1,root2, rootx,qidx;
  gfsmWeight    fw;
  gfsmArcIter   ai;

  //-- get sizes
  size1 = gfsm_automaton_n_states(fsm1);
  size2 = gfsm_automaton_n_states(fsm2);

  root1 = gfsm_automaton_get_root(fsm1);
  root2 = gfsm_automaton_get_root(fsm2);
  rootx = root2 + size1;

  //-- create concat
  if (concat!=NULL) {
    gfsm_automaton_clear(concat);
    gfsm_automaton_copy_shallow(concat,fsm1);
    gfsm_automaton_ensure_mutable(concat);
  } else {
    concat = gfsm_automaton_shadow_classed(fsm1,gfsmACDefault);
  }

  //-- sanity checks
  if (!fsm2 || root2==gfsmNoState) {
    gfsm_autmaton_copy(concat,fsm1);
    return concat;
  }
  if (!fsm1 || root1==gfsmNoState) {
    return concat;
  }

  //-- prepare output automaton
  gfsm_automaton_reserve_states(concat,size1+size2);
  concat->flags.sort_mode = gfsmASMNone; //-- avoid "smart" arc insertion
  gfsm_automaton_set_root(concat,root1);

  //-- copy in fsm1
  for (qid=0; qid<size1; qid++) {
    if (!gfsm_automaton_has_state(fsm1,qid)) continue;
    gfsm_automaton_ensure_state(concat,qid);

    //-- check for fsm1 final states
    if (gfsm_automaton_lookup_final(fsm1,qid,&fw))
      gfsm_automaton_add_arc(concat, qid,rootx, gfsmEpsilon,gfsmEpsilon, fw);

    //-- add "plain" arcs from fsm1
    for (gfsm_arciter_open(&ai,fsm1,qid); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      gfsm_automaton_add_arc(concat, qid,a->target, a->lower,a->upper, a->weight);
    }
    gfsm_arciter_close(&ai);
  }

  //-- copy in fsm2
  for (qid=0; qid<size2; qid++) {
    if (!gfsm_automaton_has_state(fsm2,qid)) continue;
    qidx = qid + size1;
    gfsm_automaton_ensure_state(concat,qidx);

    //-- check for fsm2 final states
    if (gfsm_automaton_lookup_final(fsm2, qid, &fw))
      gfsm_automaton_set_final_state_full(concat,qidx,TRUE,fw);

    //-- add "plain" arcs from fsm2
    for (gfsm_arciter_open(&ai,fsm2,qid); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      gfsm_automaton_add_arc(concat, qidx,a->target+size1, a->lower,a->upper, a->weight);
    }
    gfsm_arciter_close(&ai);
  }

  return concat;
}

/*--------------------------------------------------------------
 * concat()
 */
gfsmAutomaton *gfsm_automaton_concat(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  gfsmAutomaton *tmp = gfsm_automaton_concat_full(fsm1,fsm2,NULL);
  gfsm_automaton_swap(fsm1,tmp);
  gfsm_automaton_free(tmp);
  return fsm1;
}

/*--------------------------------------------------------------
 * n_concat()
 */
gfsmAutomaton *gfsm_automaton_n_concat(gfsmAutomaton *fsm1, gfsmAutomaton *_fsm2, guint n)
{
  gfsmAutomaton *fsm2 = _fsm2;

  //-- sanity check(s)
  if (!_fsm2 || _fsm2->root_id == gfsmNoState) return fsm1;
  if (_fsm2==fsm1) fsm2 = gfsm_automaton_clone(fsm1);

  for ( ; n > 0; n--) { gfsm_automaton_concat(fsm1, fsm2); }

  if (fsm2 != _fsm2) gfsm_automaton_free(fsm2);

  return fsm1;
}


/*--------------------------------------------------------------
 * connect()
 */
gfsmAutomaton *gfsm_automaton_connect(gfsmAutomaton *fsm)
{
  gfsmBitVector *wanted;

  //-- sanity check
  if (!fsm) return fsm;

  wanted = gfsm_bitvector_sized_new(fsm->states->len);
  gfsm_automaton_connect_fw(fsm, wanted);

  gfsm_bitvector_zero(wanted);
  gfsm_automaton_connect_bw(fsm, NULL, wanted);

  gfsm_bitvector_free(wanted);
  return fsm;
}


/*--------------------------------------------------------------
 * connect_fw_visit_state()
 *  + marks all states on a path from (id) in (visited)
 */
void gfsm_connect_fw_visit_state(gfsmAutomaton *fsm,
				 gfsmStateId    id,
				 gfsmBitVector *visited)
{
  gfsmArcIter ai;

  //-- already visited
  if (gfsm_bitvector_get(visited,id)) return;        //-- ignore states we've already visited
  if (!gfsm_automaton_has_state(fsm,id)) return;     //-- ignore invalid states

  //-- mark node as visited on this path
  gfsm_bitvector_set(visited,id,1);

  //-- visit targets of outgoing arcs
  for (gfsm_arciter_open(&ai,fsm,id); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
    gfsm_connect_fw_visit_state(fsm, gfsm_arciter_arc(&ai)->target, visited);
  }
  gfsm_arciter_close(&ai);

  return;
}

/*--------------------------------------------------------------
 * connect_fw()
 */
gfsmAutomaton *gfsm_automaton_connect_fw(gfsmAutomaton *fsm, gfsmBitVector *visited)
{
  gboolean  visited_is_temp = FALSE;
  gfsmStateId root = gfsm_automaton_get_root(fsm);

  //-- sanity check
  if (!fsm || root==gfsmNoState)
    return gfsm_automaton_prune_states(fsm,NULL);

  //-- traversal record
  if (visited==NULL) {
    visited = gfsm_bitvector_sized_new(gfsm_automaton_n_states(fsm));
    visited_is_temp = TRUE;
  }

  //-- traverse
  gfsm_connect_fw_visit_state(fsm, root, visited);
  gfsm_automaton_prune_states(fsm, visited);

  //-- cleanup
  if (visited_is_temp) gfsm_bitvector_free(visited);

  return fsm;
}

/*--------------------------------------------------------------
 * connect_bw(): final_foreach()
 */
struct gfsm_connect_bw_data_ {
  gfsmAutomaton *fsm;
  GPtrArray     *rarcs;
  gfsmBitVector *finalizable;
};

gboolean gfsm_connect_bw_visit_state(gfsmStateId id,
				     gpointer    pw,
				     struct      gfsm_connect_bw_data_ *data)
{
  gfsmArcList *al;

  //-- already visited
  if (gfsm_bitvector_get(data->finalizable,id)     //-- already visited?
      || !gfsm_automaton_has_state(data->fsm, id)) //-- bad state?
    return FALSE;                                  //-----> continue traversal

  //-- mark state as finalizable
  gfsm_bitvector_set(data->finalizable,id,1);

  //-- visit sources of incoming arcs
  for (al=g_ptr_array_index(data->rarcs,id); al != NULL; al=al->next) {
    gfsmArc *arc = (gfsmArc*)al->data;
    gfsm_connect_bw_visit_state(arc->source,pw,data);
  }

  return FALSE; //-- continue traversal
}

/*--------------------------------------------------------------
 * connect_bw()
 */
gfsmAutomaton *gfsm_automaton_connect_bw(gfsmAutomaton       *fsm,
					 gfsmReverseArcIndex *rarcs,
					 gfsmBitVector       *finalizable)
{
  gboolean rarcs_is_temp = FALSE;
  gboolean finalizable_is_temp = FALSE;
  struct gfsm_connect_bw_data_ data = {fsm,rarcs,finalizable};

  //-- sanity check(s)
  if (!fsm || gfsm_automaton_n_final_states(fsm)==0)
    return gfsm_automaton_prune_states(fsm,NULL);

  //-- reverse arc-index
  if (rarcs==NULL) {
    rarcs = data.rarcs = gfsm_automaton_reverse_arc_index(fsm,NULL);
    rarcs_is_temp = TRUE;
  }

  //-- traversal record
  if (finalizable==NULL) {
    finalizable = data.finalizable = gfsm_bitvector_sized_new(gfsm_automaton_n_states(fsm));
    finalizable_is_temp = TRUE;
  }

  //-- traverse
  g_tree_foreach(fsm->finals, (GTraverseFunc)gfsm_connect_bw_visit_state,  &data);
  gfsm_automaton_prune_states(fsm, finalizable);

  //-- cleanup
  if (finalizable_is_temp) gfsm_bitvector_free(finalizable);
  if (rarcs_is_temp) gfsm_reverse_arc_index_free(rarcs,TRUE);

  return fsm;
}


/*--------------------------------------------------------------
 * prune_states()
 */
gfsmAutomaton *gfsm_automaton_prune_states(gfsmAutomaton *fsm, gfsmBitVector *wanted)
{
  gfsmStateId id, maxwanted=gfsmNoState, n_states=gfsm_automaton_n_states(fsm);
  gfsmArcIter ai;

  for (id=0; id < n_states; id++) {
    if (!wanted || !gfsm_bitvector_get(wanted,id)) {
      //-- unwanted state: chuck it
      gfsm_automaton_remove_state(fsm,id);
    }
    else {
      maxwanted = id;
      //-- prune outgoing arcs to any unwanted states, too
      for (gfsm_arciter_open(&ai, fsm, id); gfsm_arciter_ok(&ai); ) {
	gfsmArc *arc = gfsm_arciter_arc(&ai);
	if (!wanted || !gfsm_bitvector_get(wanted,arc->target)) {
	  gfsm_arciter_remove(&ai);
	} else {
	  gfsm_arciter_next(&ai);
	}
      }
      gfsm_arciter_close(&ai);
    }
  }

  //-- update number of states
  if (maxwanted != gfsmNoState) {
    g_array_set_size(fsm->states, maxwanted+1);
  } else {
    g_array_set_size(fsm->states, 0);
  }

  return fsm;
}

/*--------------------------------------------------------------
 * determinize_lp2ec_foreach_func_()
 */
typedef struct {
  gfsmAutomaton *nfa;
  gfsmAutomaton *dfa;
  gfsmStateId    dfa_src_id;
  gfsmEnum      *ec2id;
} gfsmLp2EcForeachData;

gboolean gfsm_determinize_lp2ec_foreach_func_(gfsmLabelPair         lp,
					      gfsmWeightedStateSet *wss,
					      gfsmLp2EcForeachData *data)
{
  gfsmStateId    ec2id_val;
  gpointer       ec2id_val_as_ptr;
  gfsmStateSet  *ec2id_key;

  if ( gfsm_enum_lookup_extended(data->ec2id,
				 wss->set,
				 (gpointer)(&ec2id_key),
				 (gpointer)(&ec2id_val_as_ptr)) )
    {
      //-- target node-set is already present: just add an arc in @dfa
      ec2id_val = GPOINTER_TO_UINT(ec2id_val_as_ptr);
      gfsm_automaton_add_arc(data->dfa,
			     data->dfa_src_id,
			     ec2id_val,
			     gfsm_labelpair_lower(lp),
			     gfsm_labelpair_upper(lp),
			     wss->weight);

      //-- ... and maybe free the embedded state set
      if (wss->set != ec2id_key) gfsm_stateset_free(wss->set);
      wss->set = NULL;
    }
  else
    {
      //-- image of equiv-class (wss->set) was not yet present: make a new one
      ec2id_val = gfsm_automaton_ensure_state(data->dfa,
					      gfsm_enum_insert(data->ec2id, wss->set));

      //-- ... add @dfa arc
      gfsm_automaton_add_arc(data->dfa,
			     data->dfa_src_id,
			     ec2id_val,
			     gfsm_labelpair_lower(lp),
			     gfsm_labelpair_upper(lp),
			     wss->weight);

      //-- ... and recurse
      gfsm_determinize_visit_state_(data->nfa,   data->dfa,
				    wss->set,    ec2id_val,
				    data->ec2id);
    }
  return FALSE;
}

/*--------------------------------------------------------------
 * determinize_visit_state_()
 */
void gfsm_determinize_visit_state_(gfsmAutomaton *nfa,    gfsmAutomaton *dfa,
				   gfsmStateSet  *nfa_ec, gfsmStateId    dfa_id,
				   gfsmEnum      *ec2id)
{
  GTree            *lp2ecw;  //-- maps label-pairs@nfa.src.ec => (eq-class@nfa.sink, sum(weight))
  gfsmStateSetIter  eci;
  gfsmStateId       ecid;
  gfsmLp2EcForeachData lp2ec_foreach_data;
  gfsmWeight           fw;

  //-- check for final state
  if (gfsm_stateset_lookup_final_weight(nfa_ec,nfa,&fw)) {
    gfsm_automaton_set_final_state_full(dfa, dfa_id, TRUE, fw);
  }

  //-- build label-pair => (sink-eqc, sum(weight)) mapping 'lp2ecw' for node-set nfa_ec
  lp2ecw = g_tree_new_full(((GCompareDataFunc)
			    gfsm_labelpair_compare_with_data), //-- key_comp_func
			   NULL, //-- key_comp_data
			   NULL, //-- key_free_func
			   (GDestroyNotify)g_free);            //-- val_free_func

  for (eci=gfsm_stateset_iter_begin(nfa_ec);
       (ecid=gfsm_stateset_iter_id(eci)) != gfsmNoState;
       eci=gfsm_stateset_iter_next(nfa_ec,eci))
    {
      gfsmArcIter  ai;
      for (gfsm_arciter_open(&ai, nfa, ecid); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
	gfsmArc *a = gfsm_arciter_arc(&ai);
	gfsmLabelPair lp;
	gfsmLabelPair *lp2ec_key;
	gfsmWeightedStateSet *lp2ec_val;

	//if (a->lower==gfsmEpsilon && a->upper==gfsmEpsilon) continue; //-- ignore eps arcs
	lp = gfsm_labelpair_new(a->lower, a->upper);

	//-- add equivalence class to local mapping
	if ( g_tree_lookup_extended(lp2ecw,
				    GUINT_TO_POINTER(lp),
				    (gpointer)(&lp2ec_key),
				    (gpointer)(&lp2ec_val)) )
	  {
	    //-- already present: compute union and add new arc's weight
	    gfsm_stateset_insert(lp2ec_val->set, a->target);
	    lp2ec_val->weight = gfsm_sr_plus(nfa->sr, lp2ec_val->weight, a->weight);
	  }
	else
	  {
	    //-- not yet present: insert new value
	    lp2ec_val         = g_new(gfsmWeightedStateSet,1);
	    lp2ec_val->set    = gfsm_stateset_new_singleton(a->target);
	    lp2ec_val->weight = a->weight;
	    g_tree_insert(lp2ecw, GUINT_TO_POINTER(lp), lp2ec_val);
	  }
      }

      //-- tmp-cleanup
      gfsm_arciter_close(&ai);
    }

  //-- stateset-iter (eci) cleanup
  //(none)

  //-- insert computed arcs into @dfa
  lp2ec_foreach_data.nfa         = nfa;
  lp2ec_foreach_data.dfa         = dfa;
  lp2ec_foreach_data.dfa_src_id  = dfa_id;
  lp2ec_foreach_data.ec2id       = ec2id;
  g_tree_foreach(lp2ecw,
		 (GTraverseFunc)gfsm_determinize_lp2ec_foreach_func_,
		 (gpointer)(&lp2ec_foreach_data));

  //-- cleanup
  g_tree_destroy(lp2ecw);
}

/*--------------------------------------------------------------
 * determinize()
 */
gfsmAutomaton *gfsm_automaton_determinize(gfsmAutomaton *nfa)
{
  if (!nfa->flags.is_deterministic) {
    gfsmAutomaton *dfa = gfsm_automaton_determinize_full(nfa,NULL);
    gfsm_automaton_swap(nfa,dfa);
    gfsm_automaton_free(dfa);
  }
  return nfa;
}

/*--------------------------------------------------------------
 * determinize_full()
 */
gfsmAutomaton *gfsm_automaton_determinize_full(gfsmAutomaton *nfa, gfsmAutomaton *dfa)
{
  gfsmEnum      *ec2id;  //-- (global) maps literal(equiv-class@nfa) => node-id@dfa
  gfsmStateSet  *nfa_ec; //-- (temp) equiv-class@nfa
  gfsmStateId    dfa_id; //-- (temp) id @ dfa

  //-- sanity check(s)
  if (!nfa) return NULL;
  else if (nfa->flags.is_deterministic) {
    if (dfa) gfsm_automaton_copy(dfa,nfa);
    else     dfa = gfsm_automaton_clone(nfa);
    return dfa;
  }

  //-- initialization: dfa
  if (!dfa) {
    dfa = gfsm_automaton_shadow(nfa);
  } else {
    gfsm_automaton_clear(dfa);
    gfsm_automaton_copy_shallow(dfa,nfa);
  }
  dfa->flags.sort_mode = gfsmASMNone; //-- avoid "smart" arc-insertion

  //-- initialization: ec2id
  ec2id = gfsm_enum_new_full(NULL /*(gfsmDupFunc)gfsm_stateset_clone*/ ,
			     (GHashFunc)gfsm_stateset_hash,
			     (GEqualFunc)gfsm_stateset_equal,
			     (GDestroyNotify)gfsm_stateset_free);

  //-- initialization: nfa_ec
  nfa_ec = gfsm_stateset_sized_new(32);
  gfsm_stateset_insert(nfa_ec, nfa->root_id);

  //-- set root in dfa
  dfa_id = gfsm_automaton_ensure_state(dfa, gfsm_enum_insert(ec2id, nfa_ec));
  gfsm_automaton_set_root(dfa, dfa_id);

  //-- guts: determinize recursively outwards from root node
  gfsm_determinize_visit_state_(nfa, dfa, nfa_ec, dfa_id, ec2id);

  //-- set flag in dfa
  dfa->flags.is_deterministic = TRUE;

  //-- cleanup
  //gfsm_stateset_free(nfa_ec); //-- this ought to be freed by gfsm_enum_free(ec2id)
  gfsm_enum_free(ec2id);

  return dfa;
}



/*--------------------------------------------------------------
 * difference()
 */
gfsmAutomaton *gfsm_automaton_difference(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  gfsmAutomaton *fsm = gfsm_automaton_difference_full(fsm1,fsm2,NULL);
  gfsm_automaton_swap(fsm1,fsm);
  gfsm_automaton_free(fsm);
  return fsm1;
}

/*--------------------------------------------------------------
 * difference_full()
 */
gfsmAutomaton *gfsm_automaton_difference_full(gfsmAutomaton *fsm1,
					      gfsmAutomaton *fsm2,
					      gfsmAutomaton *diff)
{
  gfsmAutomaton *not_fsm2;
  gfsmAlphabet *alph1 = gfsm_identity_alphabet_new();

  gfsm_automaton_get_alphabet(fsm1, gfsmLSLower, alph1);
  not_fsm2 = gfsm_automaton_clone(fsm2);
  gfsm_automaton_complement_full(not_fsm2, alph1);
  diff = gfsm_automaton_intersect_full(fsm1, not_fsm2, diff, NULL);

  gfsm_automaton_free(not_fsm2);
  gfsm_alphabet_free(alph1);

  return diff;
}



/*--------------------------------------------------------------
 * intersect()
 */
gfsmAutomaton *gfsm_automaton_intersect(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  gfsmAutomaton *tmp = gfsm_automaton_intersect_full(fsm1,fsm2,NULL,NULL);
  gfsm_automaton_swap(fsm1,tmp);
  gfsm_automaton_free(tmp);
  return fsm1;
}


/*--------------------------------------------------------------
 * intersect_visit()
 */
gfsmStateId gfsm_automaton_intersect_visit_(gfsmStatePair sp,
					    gfsmAutomaton *fsm1,
					    gfsmAutomaton *fsm2,
					    gfsmAutomaton *fsm,
					    gfsmStatePairEnum *spenum)
{
  gfsmStateId qid = gfsm_enum_lookup(spenum,&sp);
  gfsmStateId qid2;
  gfsmArcIter ai1,ai2;
  gfsmArc     *a1,*a2;
  gfsmWeight  fw1,fw2;

  //-- ignore already-visited states
  if (qid != gfsmEnumNone) return qid;

  //-- sanity check
  if ( !(gfsm_automaton_has_state(fsm1,sp.id1) && gfsm_automaton_has_state(fsm2,sp.id2)) )
    return gfsmNoState;

  //-- insert new state into output automaton
  qid = gfsm_automaton_add_state(fsm);
  gfsm_enum_insert_full(spenum,&sp,qid);

  //-- check for final states
  if (gfsm_automaton_lookup_final(fsm1,sp.id1,&fw1) && gfsm_automaton_lookup_final(fsm2,sp.id2,&fw2)) {
    gfsm_automaton_set_final_state_full(fsm,qid,TRUE,gfsm_sr_times(fsm->sr,fw1,fw2));
  }

  //-------------------------------------------
  // recurse on outgoing arcs

  //--------------------------------
  // recurse: arcs: sort

  //-- arcs: sort arclists: fsm1
  gfsm_arciter_open(&ai1,fsm1,sp.id1);
  if (gfsm_automaton_sortmode(fsm1) != gfsmASMLower) {
    gfsmArcSortData sortdata = {gfsmASMLower,fsm->sr};
    gfsm_arciter_sort(&ai1_cont, gfsm_arc_compare, &sortdata);
  }
  //-- arcs: sort arclists: fsm2
  gfsm_arciter_open(&ai2,fsm2,sp.id2);
  if (gfsm_automaton_sortmode(fsm2) != gfsmASMLower) {
    gfsmArcSortData sortdata = {gfsmASMLower,fsm->sr};
    gfsm_arciter_sort(&ai2, gfsm_arc_compare, &sortdata);
  }

  //--------------------------------
  // recurse: arcs: iterate
  for ( ; gfsm_arciter_ok(&ai1); gfsm_arciter_next(&ai1)) {
    a1 = gfsm_arciter_arc(&ai1);

    if (a1->lower == gfsmEpsilon) {
      //-- handle epsilon arcs

      //-- eps: case fsm1:(q1 --eps-->  q1'), fsm2:(q2)
      qid2 = gfsm_automaton_intersect_visit_((gfsmStatePair){a1->target,sp.id2},
					     fsm1, fsm2, fsm, spenum);
      if (qid2 != gfsmNoState)
	gfsm_automaton_add_arc(fsm, qid, qid2, gfsmEpsilon, gfsmEpsilon, a1->weight);

      //-- eps: case fsm1:(q1 --eps-->  q1'), fsm2:(q2 --eps-->  q2')
      for (gfsm_arciter_reset(&ai2); gfsm_arciter_ok(&ai2); gfsm_arciter_next(&ai2)) {
	a2 = gfsm_arciter_arc(&ai2);
	if (a2->lower != gfsmEpsilon) break;

	qid2 = gfsm_automaton_intersect_visit_((gfsmStatePair){a1->target,a2->target},
					       fsm1, fsm2, fsm, spenum);
	if (qid2 != gfsmNoState)
	  gfsm_automaton_add_arc(fsm, qid, qid2, gfsmEpsilon, gfsmEpsilon,
				 gfsm_sr_times(fsm1->sr, a1->weight, a2->weight));
      }
    }
    else {
      //-- handle non-epsilon arcs
      for ( ; gfsm_arciter_ok(&ai2); gfsm_arciter_next(&ai2)) {
	a2 = gfsm_arciter_arc(&ai2);

	if      (a2->lower < a1->lower) continue;
	else if (a2->lower > a1->lower) break;

	qid2 = gfsm_automaton_intersect_visit_((gfsmStatePair){a1->target,a2->target},
					       fsm1, fsm2, fsm, spenum);
	if (qid2 != gfsmNoState)
	  gfsm_automaton_add_arc(fsm, qid, qid2, a1->lower, a1->lower,
				 gfsm_sr_times(fsm1->sr, a1->weight, a2->weight));
      }
    }
  }

  //-- handle epsilon-arcs on fsm2
  for (gfsm_arciter_reset(&ai2) ; gfsm_arciter_ok(&ai2); gfsm_arciter_next(&ai2)) {
    a2 = gfsm_arciter_arc(&ai2);
    if (a2->lower != gfsmEpsilon) break;

    //-- eps: case fsm1:(q1), fsm2:(q2 --eps-->  q2')
    qid2 = gfsm_automaton_intersect_visit_((gfsmStatePair){sp.id1,a2->target},
					   fsm1, fsm2, fsm, spenum);
    if (qid2 != gfsmNoState)
      gfsm_automaton_add_arc(fsm, qid, qid2, gfsmEpsilon, gfsmEpsilon, a2->weight);
  }

  //-- cleanup
  gfsm_arciter_close(&ai1);
  gfsm_arciter_close(&ai2);

  return qid;
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

  //-- setup: output fsm
  if (!intersect) {
    intersect=gfsm_automaton_shadow_classed(fsm1,gfsmACDefault);
  } else {
    gfsm_automaton_clear(intersect);
    gfsm_automaton_copy_shallow(intersect,fsm1);
  }
  gfsm_automaton_ensure_mutable(intersect);
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

  //-- guts
  rootpair.id1 = gfsm_automaton_get_root(fsm1);
  rootpair.id2 = gfsm_automaton_get_root(fsm2);
  rootid = gfsm_automaton_intersect_visit_(rootpair, fsm1, fsm2, intersect, spenum);

  //-- finalize: set root state
  gfsm_automaton_set_root(intersect, rootid);

  //-- cleanup
  if (spenum_is_temp) gfsm_enum_free(spenum);

  return intersect;
}


/*--------------------------------------------------------------
 * invert()
 */
gfsmAutomaton *gfsm_automaton_invert(gfsmAutomaton *fsm)
{
  gfsmStateId qid, n_states=gfsm_automaton_n_states(fsm);
  gfsmArcIter  ai;
  for (qid=0; qid < n_states; qid++) {
    for (gfsm_arciter_open(&ai,fsm,qid); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      gfsmLabelId tmp = a->lower;
      a->lower        = a->upper;
      a->upper        = tmp;
    }
    gfsm_arciter_close(&ai);
  }
  return fsm;
}

/*--------------------------------------------------------------
 * optional()
 */
gfsmAutomaton *gfsm_automaton_optional(gfsmAutomaton *fsm)
{
  gfsmStateId root = gfsm_automaton_get_root(fsm);
  if (!gfsm_automaton_is_final_state(fsm,root))
    gfsm_automaton_set_final_state_full(fsm,root,TRUE,fsm->sr->one);
  return fsm;
}

/*--------------------------------------------------------------
 * product() (single-destructive)
 */
gfsmAutomaton *gfsm_automaton_product(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  gfsmAutomaton *fsm2_tmp = gfsm_automaton_clone(fsm2);
  gfsm_automaton_product_(fsm1,fsm2_tmp);
  gfsm_automaton_free(fsm2_tmp);
  return fsm1;
}

/*--------------------------------------------------------------
 * product_() (dual-destructive)
 */
gfsmAutomaton *gfsm_automaton_product_(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  gfsmStateId  qid, n_states;
  gfsmArcIter  ai;
  gfsmArc     *a;

  //-- ensure mutability
  gfsm_automaton_ensure_mutable(fsm1);
  gfsm_automaton_ensure_mutable(fsm2);

  //-- chuck out all upper-labels from fsm1
  n_states = gfsm_automaton_n_states(fsm1);
  for (qid=0; qid < n_states; qid++) {
    if (!gfsm_automaton_has_state(fsm1,qid)) continue;
    for (gfsm_arciter_open(&ai,fsm1,qid); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      a = gfsm_arciter_arc(&ai);
      a->upper = gfsmEpsilon;
    }
    gfsm_arciter_close(&ai);
  }

  //-- chuck out all upper-labels from fsm2
  n_states = gfsm_automaton_n_states(fsm2);
  for (qid=0; qid < n_states; qid++) {
    if (!gfsm_automaton_has_state(fsm2,qid)) continue;
    for (gfsm_arciter_open(&ai,fsm2,qid); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      a = gfsm_arciter_arc(&ai);
      a->lower = gfsmEpsilon;
    }
    gfsm_arciter_close(&ai);
  }

  //-- concatenate
  gfsm_automaton_concat(fsm1,fsm2);
  
  //-- mark output fsm as transducer
  fsm1->flags.is_transducer = 1;

  return fsm1;
}

/*--------------------------------------------------------------
 * project()
 */
gfsmAutomaton *gfsm_automaton_project(gfsmAutomaton *fsm, gfsmLabelSide which)
{
  gfsmStateId id, n_states;
  gfsmArcIter ai;
  if (which==gfsmLSBoth) return fsm;

  n_states = gfsm_automaton_n_states(fsm);
  for (id=0; id < n_states; id++) {
    for (gfsm_arciter_open(&ai,fsm,id); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      if (which==gfsmLSLower) a->upper = a->lower;
      else                    a->lower = a->upper;
    }
    gfsm_arciter_close(&ai);
  }
  fsm->flags.is_transducer = FALSE;
  return fsm;
}

/*--------------------------------------------------------------
 * replace()
 */
gfsmAutomaton *gfsm_automaton_replace(gfsmAutomaton *fsm1, gfsmLabelVal lo, gfsmLabelVal hi, gfsmAutomaton *fsm2)
{
  gfsmStateId id;
  gfsmArcIter ai;
  gfsmStateId nstates = gfsm_automaton_n_states(fsm1);

  gfsm_automaton_ensure_mutable(fsm1);

  for (id=0; id < nstates; id++) {
    if (!gfsm_automaton_has_state(fsm1,id)) continue;
    for (gfsm_arciter_open(&ai,fsm1,id), gfsm_arciter_seek_both(&ai,lo,hi);
	 gfsm_arciter_ok(&ai);
	 gfsm_arciter_seek_both(&ai,lo,hi))
      {
	gfsmArc *a = gfsm_arciter_arc(&ai);
	gfsm_automaton_insert_automaton(fsm1, id, a->target, fsm2, a->weight);
	gfsm_arciter_remove(&ai); //-- implies gfsm_arciter_next()
      }
    gfsm_arciter_close(&ai);
  }

  return fsm1;
}

/*--------------------------------------------------------------
 * insert_automaton()
 */
gfsmAutomaton *gfsm_automaton_insert_automaton(gfsmAutomaton *fsm1,
					       gfsmStateId    q1from,
					       gfsmStateId    q1to,
					       gfsmAutomaton *fsm2,
					       gfsmWeight     w)
{
  gfsmStateId size1, size2;
  gfsmStateId id1, id2;
  gfsmArcIter ai;
  gfsmWeight  fw;

  //-- ensure mutability
  gfsm_automaton_ensure_mutable(fsm1);

  //-- reserve size
  size1 = gfsm_automaton_n_states(fsm1);
  size2 = gfsm_automaton_n_states(fsm2);
  gfsm_automaton_reserve(fsm1, size1+size2);

  fsm1->flags.sort_mode = gfsmASMNone; //-- avoid "smart" arc-insertion

  //-- adopt states from fsm2 into fsm1
  for (id2 = 0; id2 < size2; id2++) {
    if (!gfsm_automaton_has_state(fsm2,id2)) continue;
    id1 = id2+size1;

    //-- copy state: arcs
    gfsm_automaton_ensure_state(fsm1,id1);
    for (gfsm_arciter_open(&ai,fsm2,id2); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      gfsm_automaton_add_arc(fsm1,
			     id1, a->target+size1,
			     a->lower, a->upper, a->weight);
    }
    gfsm_arciter_close(&ai);

    //-- check for fsm2-final states: get weight & add arc to our sink state
    if (gfsm_automaton_lookup_final(fsm2, id2, &fw)) {
      gfsm_automaton_add_arc(fsm1,id1,q1to,gfsmEpsilon,gfsmEpsilon, fw);
    }
  }

  //-- add epsilon arc to new state
  gfsm_automaton_add_arc(fsm1,
			 q1from, gfsm_automaton_get_root(fsm2)+size1,
			 gfsmEpsilon, gfsmEpsilon, w);

  return fsm1;
}


/*--------------------------------------------------------------
 * rmepsilon()
 */
gfsmAutomaton *gfsm_automaton_rmepsilon(gfsmAutomaton *fsm)
{
  gfsmStatePair2WeightHash *sp2wh = gfsm_statepair2weighthash_new();
  gfsmArcIter ai;
  gfsmStateId qid, nstates;
  gfsmArc *a;

  //-- ensure mutable
  gfsm_automaton_ensure_mutable(fsm);

  //-- pass-1: populate sp2wh with epsilon-reachable states
  for (qid=0, nstates=gfsm_automaton_n_states(fsm); qid < nstates; qid++) {
    gfsm_automaton_rmeps_visit_state_(fsm, qid, qid, fsm->sr->one, sp2wh);
  }

  //-- pass-2: adopt non-epsilon arcs & final weights from eps-reachable states
  gfsm_weighthash_foreach(sp2wh, (GHFunc)gfsm_automaton_rmeps_pass2_foreach_func_, fsm);

  //-- pass-3: actual removal of now-redundant epsilon arcs
  for (qid=0; qid < nstates; qid++) {
    for (gfsm_arciter_open(&ai,fsm,qid); gfsm_arciter_ok(&ai); ) {
      a = gfsm_arciter_arc(&ai);
      if (a->lower==gfsmEpsilon && a->upper==gfsmEpsilon) {
	gfsm_arciter_remove(&ai);
      } else {
	gfsm_arciter_next(&ai);
      }
    }
    gfsm_arciter_close(&ai);
  }

  //-- cleanup
  gfsm_weighthash_free(sp2wh);

  return fsm;
}

/*--------------------------------------------------------------
 * rmepsilon_visit_state()
 */
void gfsm_automaton_rmeps_visit_state_(gfsmAutomaton *fsm,
				       gfsmStateId qid_noeps, //-- state reachable by non-eps arcs
				       gfsmStateId qid_eps,   //-- eps-reachable state from qid_noeps
				       gfsmWeight weight_eps, //-- total weight of followed eps-arcs
				       gfsmStatePair2WeightHash *sp2wh //-- maps (qid_noeps,qid_noeps)=>sum_weight_eps
				       )
{
  gfsmStatePair sp = {qid_noeps,qid_eps};
  gfsmArcIter ai;

  //-- visited check, mark
  if (!gfsm_weighthash_insert_sum_if_less(sp2wh, &sp, weight_eps, fsm->sr))
    return; //-- no update required

  //-- sanity check
  if ( !(gfsm_automaton_has_state(fsm,qid_noeps) && gfsm_automaton_has_state(fsm,qid_eps)) ) return;

  //-- visit epsilon-reachable states from q_eps
  for (gfsm_arciter_open(&ai, fsm, qid_eps), gfsm_arciter_seek_both(&ai,gfsmEpsilon,gfsmEpsilon);
       gfsm_arciter_ok(&ai);
       gfsm_arciter_next(&ai), gfsm_arciter_seek_both(&ai,gfsmEpsilon,gfsmEpsilon))
    {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      gfsm_automaton_rmeps_visit_state_(fsm, qid_noeps, a->target,
					gfsm_sr_times(fsm->sr, weight_eps, a->weight),
					sp2wh);
    }
  gfsm_arciter_close(&ai);
}

/*--------------------------------------------------------------
 * rmepsilon_pass2_foreach_func()
 */
void gfsm_automaton_rmeps_pass2_foreach_func_(gfsmStatePair *sp, gpointer pw, gfsmAutomaton *fsm)
{
  gfsmWeight  w = gfsm_ptr2weight(pw);
  gfsmWeight  fw2;
  gfsmArcIter ai;
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
    gfsmArc *a = gfsm_arciter_arc(&ai);
    if (a->lower != gfsmEpsilon || a->upper != gfsmEpsilon) {
      gfsm_automaton_add_arc(fsm, sp->id1, a->target, a->lower, a->upper,
			     gfsm_sr_times(fsm->sr, a->weight, w));
    }
  }
  gfsm_arciter_close(&ai);
}


/*--------------------------------------------------------------
 * reverse_full()
 */
gfsmAutomaton *gfsm_automaton_reverse_full(gfsmAutomaton *fsm, gfsmAutomaton *fsm2)
{
  gfsmStateId qid, new_root=gfsm_automaton_n_states(fsm);
  gfsmWeight  fw;
  gfsmArcIter ai;

  //-- check & maybe allocate fsm2
  if (fsm2==NULL) {
    fsm2 = gfsm_automaton_shadow_classed(fsm,gfsmACDefault);
  } else {
    gfsm_automaton_clear(fsm2);
    gfsm_automaton_copy_shallow(fsm2,fsm);
  }
  gfsm_automaton_reserve_states(fsm2,new_root+1);
  gfsm_automaton_ensure_mutable(fsm2);

  //-- mark automaton as unsorted (avoid "smart" arc-insertion)
  fsm2->flags.sort_mode = gfsmASMNone;

  //-- copy merrily merrily merrily
  for (qid=0; qid < new_root; qid++) {
    if (!gfsm_automaton_has_state(fsm,id)) continue;
    gfsm_automaton_ensure_state(fsm2,qid);

    //-- check for old final states
    if (gfsm_automaton_lookup_final(fsm,qid,&fw)) {
      gfsm_automaton_add_arc(fsm, new_root, id, gfsmEpsilon, gfsmEpsilon, fw);
    }

    //-- reverse arcs
    for (gfsm_arciter_open(&ai,fsm,id); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      gfsm_automaton_add_arc(fsm2, a->target, id, a->lower, a->upper, a->weight);
    }
    gfsm_arciter_close(&ai);
  }

  //-- root flop
  gfsm_automaton_set_final_state_full(fsm2, gfsm_automaton_get_root(fsm1), TRUE, fsm->sr->one);
  gfsm_automaton_set_root(fsm2,new_root);

  return fsm2;
}

/*--------------------------------------------------------------
 * reverse()
 */
gfsmAutomaton *gfsm_automaton_reverse(gfsmAutomaton *fsm)
{
  gfsmAutomaton *tmp = gfsm_automaton_reverse_full(fsm,NULL);
  gfsm_automaton_swap(fsm,tmp);
  gfsm_automaton_free(tmp);
  return fsm;
}


/*--------------------------------------------------------------
 * sigma()
 */
gboolean gfsm_automaton_sigma_foreach_func_(gfsmAlphabet *abet, gpointer key, gfsmLabelVal lab, gfsmAutomaton *fsm)
{
  gfsm_automaton_add_arc(fsm,0,1,lab,lab,fsm->sr->one);
  return FALSE;
}

gfsmAutomaton *gfsm_automaton_sigma(gfsmAutomaton *fsm, gfsmAlphabet *abet)
{
  gfsm_automaton_clear(fsm);
  gfsm_automaton_ensure_mutable(fsm);
  fsm->flags.sort_mode = gfsmASMNone; //-- avoid "smart" arc-insertion
  gfsm_automaton_set_root(fsm,gfsm_automaton_add_state_full(fsm,0));
  gfsm_automaton_add_state_full(fsm,1);
  gfsm_automaton_set_final_state_full(fsm,1,TRUE,fsm->sr->one);
  gfsm_alphabet_foreach(abet, (gfsmAlphabetForeachFunc)gfsm_automaton_sigma_foreach_func_, fsm);
  return fsm;
}

/*--------------------------------------------------------------
 * union()
 */
gfsmAutomaton *gfsm_automaton_union(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  gfsmStateId size1,size2, offset;
  gfsmStateId root1=gfsm_automaton_get_root(fsm1), root2=gfsm_automaton_get_root(fsm2);
  gfsmArcSortData sortdata;
  gfsmStateId id2;
  gfsmWeight  fw;

  //-- sanity check
  if (root2==gfsmNoState) return fsm1;
  gfsm_automaton_ensure_mutable(fsm1);

  size1   = gfsm_automaton_n_states(fsm1);
  size2   = gfsm_automaton_n_states(fsm2);
  offset  = size1+1;
  gfsm_automaton_reserve_states(fsm1, offset+size2);

  //-- add new root and eps-arc to old root for fsm1
  gfsm_automaton_set_root(fsm1, gfsm_automaton_add_state_full(fsm1,size1));
  if (root1 != gfsmNoState) {
    gfsm_automaton_add_arc(fsm1, size1, root1, gfsmEpsilon, gfsmEpsilon, fsm1->sr->one);
  }

  //-- avoid "smart" arc-insertion (temporary)
  fsm1->flags.sort_mode = gfsmASMNone;

  //-- adopt states from fsm2 into fsm1
  for (id2 = 0; id2 < size2; id2++) {
    gfsmStateId idnew = id2 + offset;
    gfsmArcIter ai;

    if (!gfsm_automaton_has_state(fsm2,id2)) continue;
    gfsm_automaton_ensure_state(fsm1,idnew);

    for (gfsm_arciter_open(&ai,fsm2,id2); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      gfsm_automaton_add_arc(fsm1, idnew, a->target+offset, a->lower, a->upper, a->weight);
    }
    gfsm_arciter_close(&ai);

    //-- index final states from fsm2
    if (gfsm_automaton_lookup_final(fsm2,id2,&fw)) {
      gfsm_automaton_set_final_state_full(fsm1, idnew, TRUE, fw);
    }
  }

  //-- add epsilon arc to translated root(fsm2) in fsm1
  gfsm_automaton_add_arc(fsm1, size1, offset+root2, gfsmEpsilon, gfsmEpsilon, fsm1->sr->one);

  return fsm1;
}


/*--------------------------------------------------------------
 * dummy()
 */
gfsmAutomaton *gfsm_automaton_dummy(gfsmAutomaton *fsm)
{
  g_assert_not_reached();
  return fsm;
}
