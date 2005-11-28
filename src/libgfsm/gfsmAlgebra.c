
/*=============================================================================*\
 * File: gfsmAlgebra.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library
 *
 * Copyright (c) 2004 Bryan Jurish.
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
#include <gfsmArcIter.h>
#include <gfsmStateSet.h>
#include <gfsmEnum.h>
#include <gfsmUtils.h>
#include <gfsmCompound.h>

/*======================================================================
 * Methods: algebra
 */


/*--------------------------------------------------------------
 * closure_final_func()
 *  + called for each final @id of @fsm during closure(@fsm)
 */
gboolean _gfsm_automaton_closure_final_func(gfsmStateId id, gpointer dummy, gfsmAutomaton *fsm)
{
  if (id != fsm->root_id)
    gfsm_automaton_add_arc(fsm, id, fsm->root_id, gfsmEpsilon, gfsmEpsilon, fsm->sr->one);
  return FALSE;
}

/*--------------------------------------------------------------
 * closure()
 */
gfsmAutomaton *gfsm_automaton_closure(gfsmAutomaton *fsm, gboolean is_plus)
{
  //-- sanity check(s)
  if (!fsm || fsm->root_id == gfsmNoState) return fsm;

  //-- add epsilon arcs from old final states to translated new root
  g_tree_foreach(fsm->finals, (GTraverseFunc)_gfsm_automaton_closure_final_func, fsm);

  //-- reflexive+transitive or reflexive?
  if (!is_plus)
    gfsm_automaton_set_final_state(fsm,fsm->root_id,TRUE);

  return fsm;
}

/*--------------------------------------------------------------
 * n_closure()
 */
gfsmAutomaton *gfsm_automaton_n_closure(gfsmAutomaton *fsm, guint n)
{
  //-- sanity check(s)
  if (!fsm || fsm->root_id == gfsmNoState) return fsm;

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
  gfsmStateId id, sink_id;
  gfsm_automaton_complete(fsm, alph, &sink_id);

  //-- flip final states
  for (id = 0; id < fsm->states->len; id++) {
    gfsmState  *s = gfsm_automaton_find_state(fsm,id);
    if (!s || !s->is_valid) continue;
    gfsm_automaton_set_final_state(fsm, id, !s->is_final);
  }

  return fsm;
}

/*--------------------------------------------------------------
 * complete()
 */
gfsmAutomaton *gfsm_automaton_complete(gfsmAutomaton *fsm, gfsmAlphabet *alph, gfsmStateId *sinkp)
{
  gfsmStateId  id, sinkid;
  GPtrArray    *alabels;

  if (!fsm->flags.is_deterministic) fsm = gfsm_automaton_determinize(fsm);
  gfsm_automaton_arcsort(fsm,gfsmASMLower);
  fsm->flags.sort_mode = gfsmASMNone; //-- avoid "smart" sorting

  //-- add sink-id
  sinkid = gfsm_automaton_add_state(fsm);
  if (sinkp) *sinkp = sinkid;

  //-- get alphabet label-vector
  alabels = g_ptr_array_sized_new(gfsm_alphabet_size(alph));
  gfsm_alphabet_labels_to_array(alph,alabels);

  for (id = 0; id < fsm->states->len; id++) {
    gfsmState    *s = gfsm_automaton_find_state(fsm,id);
    gfsmArcList *al;
    gfsmArc      *a;
    guint       labi;
    if (!s || !s->is_valid) continue;

    al = s->arcs;
    a  = gfsm_arclist_arc(al);
    for (labi=0; labi < alabels->len; ) {
      //gfsmLabelVal lab = g_array_index(alabels,gfsmLabelVal,labi);
      gfsmLabelVal lab = (gfsmLabelVal)g_ptr_array_index(alabels,labi);

      if (lab==gfsmEpsilon) {
	++labi;
      }
      else if (!a || a->lower > lab) {
	//-- no arc for this label: route it to sink
	gfsm_automaton_add_arc(fsm, id, sinkid, lab, lab, fsm->sr->one);
	++labi;
      }
      else if (a->lower == lab) {
	++labi;
      }
      else {
	while (al != NULL && a->lower < lab) {
	  al = al->next;
	  a  = gfsm_arclist_arc(al);
	}
      }
    }
  }

  //-- mark fsm as (still) deterministic
  fsm->flags.is_deterministic = TRUE;

  //-- cleanup
  //g_array_free(alabels,TRUE);
  g_ptr_array_free(alabels,TRUE);

  return fsm;
}



/*--------------------------------------------------------------
 * compose()
 */
gfsmAutomaton *gfsm_automaton_compose(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  gfsmAutomaton *fsm = gfsm_automaton_compose_full(fsm1,fsm2,NULL,NULL);
  gfsm_automaton_swap(fsm1,fsm);
  gfsm_automaton_free(fsm);
  return fsm1;
}

/*--------------------------------------------------------------
 * compose_full()
 */
gfsmAutomaton *gfsm_automaton_compose_full(gfsmAutomaton *fsm1,
					   gfsmAutomaton *fsm2,
					   gfsmAutomaton *composition,
					   gfsmStatePairEnum *spenum)
{
  return _gfsm_automaton_compose_intersect_wrapper(fsm1,fsm2,composition,spenum,TRUE);
}

/*--------------------------------------------------------------
 * compose_intersect_wrapper()
 */
gfsmAutomaton *_gfsm_automaton_compose_intersect_wrapper(gfsmAutomaton *fsm1,
							 gfsmAutomaton *fsm2,
							 gfsmAutomaton *fsmout,
							 gfsmStatePairEnum *spenum,
							 gboolean is_composition)
{
  gboolean      spenum_is_temp;
  gfsmStatePair rootpair;
  gfsmStateId   rootid;

  //-- setup: output fsm
  if (!fsmout) {
    fsmout=gfsm_automaton_shadow(fsm1);
  } else {
    gfsm_automaton_clear(fsmout);
    gfsm_automaton_copy_shallow(fsmout,fsm1);
  }
  fsmout->flags.sort_mode = gfsmASMNone;

  //-- setup: StatePairEnum
  if (spenum==NULL) {
    spenum_is_temp=TRUE;
    spenum = gfsm_statepair_enum_new();
  } else {
    spenum_is_temp=FALSE;
    gfsm_enum_clear(spenum);
  }

  //-- guts
  rootpair.id1 = fsm1->root_id;
  rootpair.id2 = fsm2->root_id;
  if (is_composition) {
    fsmout->flags.is_transducer = 1;
    rootid = _gfsm_automaton_compose_visit(rootpair, fsm1, fsm2, fsmout, spenum);
  } else {
    fsmout->flags.is_transducer = 0;
    rootid = _gfsm_automaton_intersect_visit(rootpair, fsm1, fsm2, fsmout, spenum);
  }

  //-- finalize: set root state
  if (rootid != gfsmNoState) {
    gfsm_automaton_set_root(fsmout, rootid);
  } else {
    fsmout->root_id = gfsmNoState;
  }

  //-- cleanup
  if (spenum_is_temp) gfsm_enum_free(spenum);

  return fsmout;
}

/*--------------------------------------------------------------
 * compose_visit()
 */
gfsmStateId _gfsm_automaton_compose_visit(gfsmStatePair sp,
					  gfsmAutomaton *fsm1,
					  gfsmAutomaton *fsm2,
					  gfsmAutomaton *fsm,
					  gfsmStatePairEnum *spenum)
{
  gfsmState   *q1, *q2;
  gfsmStateId qid = gfsm_enum_lookup(spenum,&sp);
  gfsmStateId qid2;
  gfsmArcList *al1, *al2, *ai1, *ai2, *ai2eps;
  gboolean     al1_temp=FALSE, al2_temp=FALSE;
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

  //-- check for final states
  if (q1->is_final && q2->is_final) gfsm_automaton_set_final_state(fsm,qid,TRUE);

  //-------------------------------------------
  // recurse on outgoing arcs

  //--------------------------------
  // recurse: arcs: sort

  //-- arcs: sort arclists: fsm12
  if (gfsm_automaton_sortmode(fsm1)==gfsmASMUpper) al1 = q1->arcs;
  else {
    gfsmArcSortData sortdata = {gfsmASMUpper,NULL};
    al1 = gfsm_arclist_sort(g_slist_copy(q1->arcs), &sortdata);
    al1_temp = TRUE;
  }

  //-- arcs: sort arclists: fsm2
  if (gfsm_automaton_sortmode(fsm2)==gfsmASMLower) al2 = q2->arcs;
  else {
    gfsmArcSortData sortdata = {gfsmASMLower,NULL};
    al2 = gfsm_arclist_sort(g_slist_copy(q2->arcs), &sortdata);
    al2_temp = TRUE;
  }

  //--------------------------------
  // recurse: arcs: iterate
  for (ai1=al1, ai2=al2; ai1 != NULL; ai1=ai1->next) {
    a1 = (gfsmArc*)ai1->data;
    if (a1->upper == gfsmEpsilon) {
      //-- handle epsilon arcs on fsm1

      //-- eps: case fsm1:(q1 --*:eps-->  q1'), fsm2:(q2)
      qid2 = _gfsm_automaton_compose_visit((gfsmStatePair){a1->target,sp.id2},
					   fsm1, fsm2, fsm, spenum);
      if (qid2 != gfsmNoState)
	gfsm_automaton_add_arc(fsm, qid, qid2, a1->lower, gfsmEpsilon, a1->weight);

      //-- eps: case fsm1:(q1 --*:eps-->  q1'), fsm2:(q2 --eps:*-->  q2')
      for (ai2eps=al2; ai2eps != NULL; ai2eps=ai2eps->next) {
	a2 = (gfsmArc*)ai2eps->data;
	if (a2->lower != gfsmEpsilon) break;

	qid2 = _gfsm_automaton_compose_visit((gfsmStatePair){a1->target,a2->target},
					     fsm1, fsm2, fsm, spenum);
	if (qid2 != gfsmNoState)
	  gfsm_automaton_add_arc(fsm, qid, qid2, a1->lower, a2->upper,
				 gfsm_sr_times(fsm1->sr, a1->weight, a2->weight));
      }
    }
    else {
      //-- handle non-epsilon arcs on fsm1
      for ( ; ai2 != NULL; ai2=ai2->next) {
	a2 = (gfsmArc*)ai2->data;

	//-- eps: case fsm1:(q1), fsm2:(q2 --eps:*-->  q2')
	if (a2->lower == gfsmEpsilon) {
	  qid2 = _gfsm_automaton_compose_visit((gfsmStatePair){sp.id1,a2->target},
					       fsm1, fsm2, fsm, spenum);
	  if (qid2 != gfsmNoState)
	    gfsm_automaton_add_arc(fsm, qid, qid2, gfsmEpsilon, a2->upper, a2->weight);
	  continue;
	}
	else if (a2->lower < a1->upper) continue;
	else if (a2->lower > a1->upper) break;

	qid2 = _gfsm_automaton_compose_visit((gfsmStatePair){a1->target,a2->target},
					     fsm1, fsm2, fsm, spenum);
	if (qid2 != gfsmNoState)
	  gfsm_automaton_add_arc(fsm, qid, qid2, a1->lower, a2->upper,
				 gfsm_sr_times(fsm1->sr, a1->weight, a2->weight));
      }
    }
  }

  //-- cleanup
  if (al1_temp) g_slist_free(al1);
  if (al2_temp) g_slist_free(al2);

  return qid;
}




/*--------------------------------------------------------------
 * concat_final_func()
 *  + called for each final @id of @fsm during concat(@fsm,@fsm2)
 *  + during the call @fsm1->root_id should be set to the translated root of @fsm2
 */
gboolean _gfsm_automaton_concat_final_func(gfsmStateId id, gpointer dummy, gfsmAutomaton *fsm)
{
  gfsm_automaton_add_arc(fsm, id, fsm->root_id, gfsmEpsilon, gfsmEpsilon, fsm->sr->one);
  gfsm_automaton_find_state(fsm,id)->is_final = FALSE;
  return FALSE;
}

gboolean _gfsm_automaton_concat_final_func_1(gfsmStateId id, gpointer dummy, gfsmStateId *fp)
{
  *fp = id;
  return TRUE;
}


/*--------------------------------------------------------------
 * concat()
 */
gfsmAutomaton *gfsm_automaton_concat(gfsmAutomaton *fsm1, gfsmAutomaton *_fsm2)
{
  gfsmAutomaton *fsm2;
  gfsmStateId    offset;
  gfsmStateId    id2;
  gfsmStateId    size2;
  gfsmStateId    rootx;
  gfsmSet       *finals2 = NULL;

  //-- sanity check(s)
  if (!_fsm2 || _fsm2->root_id == gfsmNoState) return fsm1;
  if (_fsm2==fsm1) fsm2 = gfsm_automaton_clone(fsm1);
  else             fsm2 = _fsm2;

  if (fsm1->finals == fsm2->finals) {
    finals2 = gfsm_set_new(gfsm_uint_compare);
    gfsm_set_copy(finals2, fsm2->finals);
  }

  offset = fsm1->states->len;
  size2  = fsm2->states->len;
  gfsm_automaton_reserve(fsm1, offset + size2);

  //-- concatenative arcs
  if (fsm1->root_id != gfsmNoState) {
    if (gfsm_set_size(fsm1->finals) == 1) {
      //-- only one final state: concatenate without epsilon arcs
      g_tree_foreach(fsm1->finals, (GTraverseFunc)_gfsm_automaton_concat_final_func_1, &rootx);
      gfsm_automaton_set_final_state(fsm1, rootx, FALSE);
    } else {
      //-- multiple final states: add epsilon arcs from old finals to mapped root2
      gfsmStateId root_tmp = fsm1->root_id;
      rootx                = fsm2->root_id+offset;
      fsm1->root_id        = rootx;
      g_tree_foreach(fsm1->finals, (GTraverseFunc)_gfsm_automaton_concat_final_func, fsm1);
      fsm1->root_id        = root_tmp;
    }
  } else /*if (fsm2->root_id != gfsmNoState)*/ {
    fsm1->root_id = rootx = fsm2->root_id + offset;
  }
  gfsm_set_clear(fsm1->finals);

  for (id2 = 0; id2 < size2; id2++) {
    gfsmStateId      id1;
    const gfsmState *s2;
    gfsmState       *s1;
    gfsmArcIter      ai;
    gfsmArcList     *s1arcs;

    s2 = gfsm_automaton_find_state_const(fsm2,id2);
    if (id2 == fsm2->root_id) {
      id1 = rootx;
      s1 = gfsm_automaton_find_state(fsm1, id1);
      s1arcs = s1->arcs;
      s1->arcs = NULL;
    } else {
      id1 = id2+offset;
      s1 = gfsm_automaton_find_state(fsm1, id1);
      s1arcs = NULL;
    }
    if (!s1 || !s2) continue;

    gfsm_state_copy(s1,s2);
    for (gfsm_arciter_open_ptr(&ai,fsm1,s1); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai))
      {
	gfsmArc *a = gfsm_arciter_arc(&ai);
	if (a->target == fsm2->root_id) a->target = rootx;
	else a->target += offset;
      }
    //-- re-insert old arcs
    if (s1arcs) s1->arcs = g_slist_concat(s1->arcs, s1arcs);

    //-- check for new final states
    if ( (finals2 && gfsm_set_contains(finals2, ((gpointer)id2)))
	 ||
	 (!finals2 && s2->is_final) )
      {
	s1->is_final = TRUE;
	gfsm_set_insert(fsm1->finals, (gpointer)(id1));
      }
  }

  fsm1->flags.sort_mode = gfsmASMNone;

  //-- cleanup
  if (finals2) gfsm_set_free(finals2);
  if (fsm2 != _fsm2) gfsm_automaton_free(fsm2);

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
 * connect(): visit_state()
 *  + returns true if this state is on a path to a final state
 */
gboolean gfsm_connect_visit_state(gfsmAutomaton *fsm,
				gfsmStateId id,
				gfsmBitVector *visited,
				gfsmBitVector *coaccessible)
{
  gfsmState *s;
  gfsmArcIter ai;
  gboolean rc=FALSE;

  if (gfsm_bitvector_get(visited,id)) return FALSE;  //-- already visited

  s = gfsm_automaton_find_state(fsm,id);
  if (!s || !s->is_valid) return FALSE;              //-- ignore invalid states

  //-- mark node as visited on this path
  gfsm_bitvector_set(visited,id,1);

  //-- check final
  if (s->is_final) rc = TRUE;

  //-- visit targets of outgoing arcs
  for (gfsm_arciter_open_ptr(&ai,fsm,s); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
    rc = rc || gfsm_connect_visit_state(fsm, gfsm_arciter_arc(&ai)->target, visited, coaccessible);
  }

  //-- mark as co-accessible
  if (rc) {
    gfsm_bitvector_set(coaccessible,id,1);
  }

  return rc;
}

/*--------------------------------------------------------------
 * connect()
 */
gfsmAutomaton *gfsm_automaton_connect(gfsmAutomaton *fsm)
{
  gfsmBitVector *visited, *coaccessible;
  gfsmStateId    id;
  gfsmArcIter    ai;
  if (!fsm || fsm->root_id == gfsmNoState) return fsm;

  visited      = gfsm_bitvector_sized_new(fsm->states->len);
  coaccessible = gfsm_bitvector_sized_new(fsm->states->len);
  if (gfsm_connect_visit_state(fsm, fsm->root_id, visited, coaccessible)) {
    //-- root state id co-accessible; prune other states
    for (id=0; id < fsm->states->len; id++) {
      if (!gfsm_bitvector_get(coaccessible,id)) {
	gfsm_automaton_remove_state(fsm,id);
      }
      //-- prune outgoing arcs to non-coaccessible states, too
      for (gfsm_arciter_open(&ai, fsm, id); gfsm_arciter_ok(&ai); ) {
	gfsmArc *arc = gfsm_arciter_arc(&ai);
	if (!gfsm_bitvector_get(coaccessible,arc->target)) {
	  gfsm_arciter_remove(&ai);
	} else {
	  gfsm_arciter_next(&ai);
	}
      }
    }
  }
  else {
    //-- root state is not co-accessible: just clear the whole fsm
    gfsm_automaton_clear(fsm);
  }

  //-- cleanup
  gfsm_bitvector_free(visited);
  gfsm_bitvector_free(coaccessible);

  return fsm;
}


/*--------------------------------------------------------------
 * _determinize_lp2ec_foreach_func()
 */
typedef struct {
  gfsmAutomaton *nfa;
  gfsmAutomaton *dfa;
  gfsmStateId    dfa_src_id;
  gfsmEnum      *ec2id;
  gfsmStateSet  *ec_tmp;
} gfsmLp2EcForeachData;

gboolean _gfsm_determinize_lp2ec_foreach_func(gfsmLabelPair         lp,
					      gfsmWeightedStateSet *wss,
					      gfsmLp2EcForeachData *data)
{
  gfsmStateId    ec2id_val;
  gfsmStateSet  *ec2id_key;

  if ( gfsm_enum_lookup_extended(data->ec2id,
				 wss->set,
				 (gpointer)(&ec2id_key),
				 (gpointer)(&ec2id_val)) )
    {
      //-- target node-set is already present: just add an arc in @dfa
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
      _gfsm_determinize_visit_state(data->nfa,   data->dfa,
				    wss->set,    ec2id_val,
				    data->ec2id, data->ec_tmp);
    }
  return FALSE;
}


/*--------------------------------------------------------------
 * _determinize_visit_state()
 */
void _gfsm_determinize_visit_state(gfsmAutomaton *nfa,    gfsmAutomaton *dfa,
				   gfsmStateSet  *nfa_ec, gfsmStateId    dfa_id,
				   gfsmEnum      *ec2id,  gfsmStateSet  *ec_tmp)
{
  GTree            *lp2ecw;  //-- maps label-pairs@nfa.src.ec => (eq-class@nfa.sink, sum(weight))
  gfsmStateSetIter  eci;
  gfsmStateId       ecid;
  gfsmLp2EcForeachData lp2ec_foreach_data;

  //-- check for final state
  if (gfsm_stateset_has_final_state(nfa_ec,nfa)) {
    gfsm_automaton_set_final_state(dfa, dfa_id, TRUE);
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

	if (a->lower==gfsmEpsilon && a->upper==gfsmEpsilon) continue; //-- ignore eps arcs
	lp = gfsm_labelpair_new(a->lower, a->upper);

	//-- populate state-set with all nodes eps-reachable from this arc's target
	gfsm_stateset_clear(ec_tmp);
	gfsm_stateset_populate_eps(ec_tmp, nfa, a->target);

	//-- add equivalence class to local mapping
	if ( g_tree_lookup_extended(lp2ecw,
				    (gpointer)lp,
				    (gpointer)(&lp2ec_key),
				    (gpointer)(&lp2ec_val)) )
	  {
	    //-- already present: compute union and add new arc's weight
	    gfsm_stateset_union(lp2ec_val->set, ec_tmp);
	    lp2ec_val->weight = gfsm_sr_plus(nfa->sr, lp2ec_val->weight, a->weight);
	  }
	else
	  {
	    //-- not yet present: insert new value
	    lp2ec_val         = g_new(gfsmWeightedStateSet,1);
	    lp2ec_val->set    = gfsm_stateset_clone(ec_tmp);
	    lp2ec_val->weight = a->weight;
	    g_tree_insert(lp2ecw, (gpointer)lp, lp2ec_val);
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
  lp2ec_foreach_data.ec_tmp      = ec_tmp;
  g_tree_foreach(lp2ecw,
		 (GTraverseFunc)_gfsm_determinize_lp2ec_foreach_func,
		 (gpointer)(&lp2ec_foreach_data));

  //-- cleanup
  g_tree_destroy(lp2ecw);

  /*
  g_printerr("_gfsm_automaton_determinize_visit_state(): not yet implemented!");
  g_assert_not_reached();
  */
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
  gfsmStateSet  *ec_tmp; //-- (temp) equiv-class@nfa

  //-- sanity check(s)
  if (!nfa) return NULL;
  else if (nfa->flags.is_deterministic) {
    if (dfa) gfsm_automaton_copy(dfa,nfa);
    else     dfa = gfsm_automaton_clone(nfa);
    return dfa;
  }

  //-- initialization: nfa
  gfsm_automaton_arcsort(nfa,gfsmASMLower);

  //-- initialization: dfa
  if (!dfa) dfa = gfsm_automaton_shadow(nfa);
  else {
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
  ec_tmp = gfsm_stateset_sized_new(32);
  gfsm_stateset_populate_eps(nfa_ec, nfa, nfa->root_id);

  //-- set root in dfa
  dfa_id = gfsm_automaton_ensure_state(dfa, gfsm_enum_insert(ec2id, nfa_ec));
  gfsm_automaton_set_root(dfa, dfa_id);

  //-- guts: determinize recursively outwards from root node
  _gfsm_determinize_visit_state(nfa, dfa, nfa_ec, dfa_id, ec2id, ec_tmp);

  //-- set flag in dfa
  dfa->flags.is_deterministic = TRUE;

  //-- cleanup
  //gfsm_stateset_free(nfa_ec); //-- this ought to be freed by gfsm_enum_free(ec2id)
  gfsm_stateset_free(ec_tmp);   //-- ... but not this
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
  return _gfsm_automaton_compose_intersect_wrapper(fsm1,fsm2,intersect,spenum,FALSE);
}

/*--------------------------------------------------------------
 * intersect_visit()
 */
gfsmStateId _gfsm_automaton_intersect_visit(gfsmStatePair sp,
					       gfsmAutomaton *fsm1,
					       gfsmAutomaton *fsm2,
					       gfsmAutomaton *fsm,
					       gfsmStatePairEnum *spenum)
{
  gfsmState   *q1, *q2;
  gfsmStateId qid = gfsm_enum_lookup(spenum,&sp);
  gfsmStateId qid2;
  gfsmArcList *al1, *al2, *ai1, *ai2, *ai2eps;
  gboolean     al1_temp=FALSE, al2_temp=FALSE;
  gfsmArcSortData sortdata = {gfsmASMLower,NULL};
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
  if (q1->is_final && q2->is_final) gfsm_automaton_set_final_state(fsm,qid,TRUE);

  //-------------------------------------------
  // recurse on outgoing arcs

  //--------------------------------
  // recurse: arcs: sort

  //-- arcs: sort arclists: fsm12
  if (gfsm_automaton_sortmode(fsm1)==gfsmASMLower) al1 = q1->arcs;
  else {
    al1 = gfsm_arclist_sort(g_slist_copy(q1->arcs), &sortdata);
    al1_temp = TRUE;
  }

  //-- arcs: sort arclists: fsm2
  if (gfsm_automaton_sortmode(fsm2)==gfsmASMLower) al2 = q2->arcs;
  else {
    al2 = gfsm_arclist_sort(g_slist_copy(q2->arcs), &sortdata);
    al2_temp = TRUE;
  }

  //--------------------------------
  // recurse: arcs: iterate
  for (ai1=al1, ai2=al2; ai1 != NULL; ai1=ai1->next) {
    a1 = (gfsmArc*)ai1->data;
    if (a1->lower == gfsmEpsilon) {
      //-- handle epsilon arcs

      //-- eps: case fsm1:(q1 --eps-->  q1'), fsm2:(q2)
      qid2 = _gfsm_automaton_intersect_visit((gfsmStatePair){a1->target,sp.id2},
						fsm1, fsm2, fsm, spenum);
      if (qid2 != gfsmNoState)
	gfsm_automaton_add_arc(fsm, qid, qid2, gfsmEpsilon, gfsmEpsilon, a1->weight);

      //-- eps: case fsm1:(q1 --eps-->  q1'), fsm2:(q2 --eps-->  q2')
      for (ai2eps=al2; ai2eps != NULL; ai2eps=ai2eps->next) {
	a2 = (gfsmArc*)ai2eps->data;
	if (a2->lower != gfsmEpsilon) break;

	qid2 = _gfsm_automaton_intersect_visit((gfsmStatePair){a1->target,a2->target},
						  fsm1, fsm2, fsm, spenum);
	if (qid2 != gfsmNoState)
	  gfsm_automaton_add_arc(fsm, qid, qid2, gfsmEpsilon, gfsmEpsilon,
				 gfsm_sr_times(fsm1->sr, a1->weight, a2->weight));
      }
    }
    else {
      //-- handle non-epsilon arcs
      for ( ; ai2 != NULL; ai2=ai2->next) {
	a2 = (gfsmArc*)ai2->data;

	//-- eps: case fsm1:(q1), fsm2:(q2 --eps-->  q2')
	if (a2->lower == gfsmEpsilon) {
	  qid2 = _gfsm_automaton_intersect_visit((gfsmStatePair){sp.id1,a2->target},
						    fsm1, fsm2, fsm, spenum);
	  if (qid2 != gfsmNoState)
	    gfsm_automaton_add_arc(fsm, qid, qid2, gfsmEpsilon, gfsmEpsilon, a2->weight);
	  continue;
	}
	else if (a2->lower < a1->lower) continue;
	else if (a2->lower > a1->lower) break;

	qid2 = _gfsm_automaton_intersect_visit((gfsmStatePair){a1->target,a2->target},
						  fsm1, fsm2, fsm, spenum);
	if (qid2 != gfsmNoState)
	  gfsm_automaton_add_arc(fsm, qid, qid2, a1->lower, a1->lower,
				 gfsm_sr_times(fsm1->sr, a1->weight, a2->weight));
      }
    }
  }

  //-- cleanup
  if (al1_temp) g_slist_free(al1);
  if (al2_temp) g_slist_free(al2);

  return qid;
}


/*--------------------------------------------------------------
 * invert()
 */
gfsmAutomaton *gfsm_automaton_invert(gfsmAutomaton *fsm)
{
  gfsmStateId id;
  gfsmArcIter  ai;
  for (id=0; id < fsm->states->len; id++) {
    for (gfsm_arciter_open(&ai,fsm,id); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      gfsmLabelId tmp = a->lower;
      a->lower        = a->upper;
      a->upper        = tmp;
    }
  }
  return fsm;
}

/*--------------------------------------------------------------
 * product() (single-destructive)
 */
gfsmAutomaton *gfsm_automaton_product(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  gfsmAutomaton *fsm2_tmp = gfsm_automaton_clone(fsm2);
  _gfsm_automaton_product(fsm1,fsm2_tmp);
  gfsm_automaton_free(fsm2_tmp);
  return fsm1;
}

/*--------------------------------------------------------------
 * _product() (dual-destructive)
 */
gfsmAutomaton *_gfsm_automaton_product(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  gfsmStateId  qid;
  gfsmState   *qp;
  gfsmArcIter  ai;
  gfsmArc     *a;

  //-- chuck out all upper-labels from fsm1
  for (qid=0; qid < fsm1->states->len; qid++) {
    qp = gfsm_automaton_find_state(fsm1,qid);
    if (!qp || !qp->is_valid) continue;
    for (gfsm_arciter_open_ptr(&ai,fsm1,qp); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      a = gfsm_arciter_arc(&ai);
      a->upper = gfsmEpsilon;
    }
  }

  //-- chuck out all upper-labels from fsm2
  for (qid=0; qid < fsm2->states->len; qid++) {
    qp = gfsm_automaton_find_state(fsm2,qid);
    if (!qp || !qp->is_valid) continue;
    for (gfsm_arciter_open_ptr(&ai,fsm2,qp); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      a = gfsm_arciter_arc(&ai);
      a->lower = gfsmEpsilon;
    }
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
  gfsmStateId id;
  gfsmArcIter ai;
  if (which==gfsmLSBoth) return fsm;

  for (id=0; id < fsm->states->len; id++) {
    for (gfsm_arciter_open(&ai,fsm,id); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      if (which==gfsmLSLower) a->upper = a->lower;
      else                    a->lower = a->upper;
    }
  }
  fsm->flags.is_transducer = FALSE;
  return fsm;
}


/*--------------------------------------------------------------
 * rmepsilon()
 */
gfsmAutomaton *gfsm_automaton_rmepsilon(gfsmAutomaton *fsm)
{
  gfsmStatePairEnum *spenum = gfsm_statepair_enum_new();
  gfsmArcIter ai;
  gfsmStateId qid;
  gfsmArc *a;

  //-- traversal
  gfsm_automaton_rmepsilon_visit_state(fsm, fsm->root_id, fsm->root_id, fsm->sr->one,spenum);
  gfsm_enum_free(spenum);

  //-- actual removal of now-redundant epsilon arcs
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

  return fsm;
}

/*--------------------------------------------------------------
 * rmepsilon_visit_state()
 */
void gfsm_automaton_rmepsilon_visit_state(gfsmAutomaton *fsm,
					  gfsmStateId qid_noeps,
					  gfsmStateId qid_eps,
					  gfsmWeight weight_eps,
					  gfsmStatePairEnum *spenum)
{
  gfsmState *q_noeps, *q_eps;
  gfsmStatePair sp = {qid_noeps,qid_eps};
  gfsmArcIter ai;
  gfsmArc *a;

  //-- visited check
  if (gfsm_enum_lookup(spenum, &sp) != gfsmEnumNone) return;

  //-- sanity check
  q_noeps = gfsm_automaton_find_state(fsm,qid_noeps);
  q_eps   = gfsm_automaton_find_state(fsm,qid_eps);
  if (!q_noeps || !q_noeps->is_valid || !q_eps || !q_eps->is_valid) return;

  //-- mark as visited
  gfsm_enum_insert_full(spenum, &sp, qid_noeps);

  //-- check for final state
  if (q_eps->is_final) {
    gfsm_automaton_set_final_state(fsm,qid_noeps,1);
  }

  //-- adopt all arcs from eps-reachable state
  if (qid_eps != qid_noeps) {
    for (gfsm_arciter_open_ptr(&ai, fsm, q_eps);
	 gfsm_arciter_ok(&ai);
	 gfsm_arciter_next(&ai))
      {
	a = gfsm_arciter_arc(&ai);
	gfsm_automaton_add_arc(fsm, qid_noeps, a->target, a->lower, a->upper,
			       gfsm_sr_times(fsm->sr, weight_eps, a->weight));
      }
  }

  //-- visit outgoing arcs from non-eps state
  for (gfsm_arciter_open_ptr(&ai, fsm, q_eps); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
    a = gfsm_arciter_arc(&ai);

    if (a->lower==gfsmEpsilon && a->upper==gfsmEpsilon) {
      //-- visit epsilon-reachable states
      gfsm_automaton_rmepsilon_visit_state(fsm, qid_noeps, a->target,
					   gfsm_sr_times(fsm->sr, weight_eps, a->weight),
					   spenum);
    } else {
      //-- visit "normal" states
      gfsm_automaton_rmepsilon_visit_state(fsm, a->target, a->target, fsm->sr->one, spenum);
    }
  }  
}


/*--------------------------------------------------------------
 * reverse()
 */
gfsmAutomaton *gfsm_automaton_reverse(gfsmAutomaton *fsm)
{
  gfsmStateId new_root = gfsm_automaton_add_state(fsm);
  gfsmStateId id;
  gfsmState   *s, *ts;
  gfsmArcList *al, *al_next, *al_prev;

  //-- reverse arc directions, assigning reversed arcs 'target' values as 'old_src+new_root'
  for (id = 0; id < new_root; id++) {
    s = gfsm_automaton_find_state(fsm,id);
    if (!s || !s->is_valid) continue;

    //-- check for old final states
    if (s->is_final) {
      s->is_final = FALSE;
      gfsm_set_remove(fsm->finals,((gpointer)id));
      gfsm_automaton_add_arc(fsm, new_root, id, gfsmEpsilon, gfsmEpsilon, gfsm_sr_one(fsm->sr));
    }

    //-- reverse arcs
    for (al_prev=NULL, al=s->arcs; al != NULL; al=al_next) {
      gfsmArc *a = gfsm_arclist_arc(al);
      al_next    = al->next;
      if (a->target >= new_root) {
	//-- already moved
	al_prev = al;
	continue; 
      }

      //-- steal arc
      if (al_prev) al_prev->next = al->next;
      else               s->arcs = al->next;
      al->next = NULL;

      //-- move arc
      ts = gfsm_automaton_find_state(fsm,a->target);
      gfsm_automaton_add_arc_link(fsm, ts, al);
      
      //-- flag as reversed
      a->target = id + new_root;
    }
  }

  //-- sanitize
  for (id=0; id < new_root; id++) {
    gfsmArcIter ai;
    for (gfsm_arciter_open(&ai,fsm,id); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      a->target -= new_root;
    }
  }

  //-- root flop
  gfsm_automaton_set_final_state(fsm,fsm->root_id,TRUE);
  fsm->root_id = new_root;

  return fsm;
}



/*--------------------------------------------------------------
 * union()
 */
gfsmAutomaton *gfsm_automaton_union(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  gfsmStateId offset;
  gfsmStateId id2;

  //-- sanity check
  if (!fsm2 || fsm2->root_id == gfsmNoState) return fsm1;

  offset = fsm1->states->len;
  gfsm_automaton_reserve(fsm1, offset + fsm2->states->len);

  for (id2 = 0; id2 < fsm2->states->len; id2++) {
    const gfsmState *s2 = gfsm_automaton_find_state_const(fsm2,id2);
    gfsmState       *s1 = gfsm_automaton_find_state(fsm1,id2+offset);
    gfsmArcIter      ai;
    gfsm_state_copy(s1,s2);
    for (gfsm_arciter_open_ptr(&ai, fsm1, s1); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      a->target += offset;
    }
    //-- index final states from @fsm2
    if (s2->is_final) gfsm_set_insert(fsm1->finals, ((gpointer)(id2+offset)));
  }

  //-- add epsilon arc to translated root state
  if (fsm1->root_id == gfsmNoState) {
    fsm1->root_id = offset + fsm2->root_id;
  } else {
    gfsm_automaton_add_arc(fsm1,
			   fsm1->root_id,
			   offset + fsm2->root_id,
			   gfsmEpsilon,
			   gfsmEpsilon,
			   gfsmNoWeight);
  }
	 
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
