
/*=============================================================================*\
 * File: gfsmAlgebra.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library
 *
 * Copyright (c) 2004 Bryan Jurish.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See file LICENSE for further informations on licensing terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
  gfsmAutomaton *fsm2 = _fsm2;
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
    gfsmAutomaton *dfa = gfsm_automaton_determinize_2(nfa,NULL);
    gfsm_automaton_swap(nfa,dfa);
    gfsm_automaton_free(dfa);
  }
  return nfa;
}

/*--------------------------------------------------------------
 * determinize_2()
 */
gfsmAutomaton *gfsm_automaton_determinize_2(gfsmAutomaton *nfa, gfsmAutomaton *dfa)
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
    gfsm_automaton_clear(nfa);
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
 * difference_visit()
 */
/// TODO
void _gfsm_difference_visit_states(gfsmAutomaton *fsm1,    gfsmAutomaton *fsm2,
				   gfsmStateId    id1,     gfsmStateId    id2,
				   GHashTable    *visited)
{
  gfsmStatePair sp_tmp = { id1, id2 };

  if (g_hash_table_lookup(visited,&sp_tmp) == NULL) return;

  //-- not yet visited... do something
}

/*--------------------------------------------------------------
 * difference()
 */
gfsmAutomaton *gfsm_automaton_difference(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  GHashTable  *visited;  //-- (global) maps pairs (q1,q2) -> TRUE iff (q1,q2) was visited

  //-- sanity check(s)
  if (!fsm1 || !fsm2 || fsm1->root_id==gfsmNoState || fsm2->root_id==gfsmNoState)
    return fsm1;

  //-- ERROR
  g_printerr("gfsm_automaton_difference(): not yet implemented!");
  g_assert_not_reached();

  //-- initialization: sort both automata
  gfsm_automaton_arcsort(fsm1, gfsmASMLower);
  gfsm_automaton_arcsort(fsm2, gfsmASMLower);
  fsm1->flags.sort_mode = gfsmASMNone; //-- avoid "smart" arc-insertion

  //-- initialization: visited
  visited = g_hash_table_new_full((GHashFunc)gfsm_statepair_hash,
				  (GEqualFunc)gfsm_statepair_equal,
				  (GDestroyNotify)g_free,
				  NULL);

  //-- guts: pair up root states & visit recursively
  _gfsm_difference_visit_states(fsm1, fsm2, fsm1->root_id, fsm2->root_id, visited);

  //-- cleanup
  g_hash_table_destroy(visited);

  return fsm1;
}

/*--------------------------------------------------------------
 * intersection()
 */
/// TODO
gfsmAutomaton *gfsm_automaton_intersection(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{


  g_printerr("gfsm_automaton_difference(): not yet implemented!");
  g_assert_not_reached();

  return fsm1;
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
 * product()
 */
/// TODO
gfsmAutomaton *gfsm_automaton_product(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  g_assert_not_reached();
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
 * prune(): visit_state()
 */
void gfsm_prune_visit_state(gfsmAutomaton *fsm,
			    gfsmStateId id,
			    gfsmBitVector *visited)
{
  gfsmState *s;
  gfsmArcIter ai;

  if (gfsm_bitvector_get(visited,id)) return;

  s = gfsm_automaton_find_state(fsm,id);
  if (!s || !s->is_valid) return;        //-- ignore invalid states

  //-- mark node as visited
  gfsm_bitvector_set(visited,id,1);

  //-- visit targets of outgoing arcs
  for (gfsm_arciter_open_ptr(&ai,fsm,s); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
    gfsm_prune_visit_state(fsm, gfsm_arciter_arc(&ai)->target, visited);
  }
}

/*--------------------------------------------------------------
 * prune()
 */
gfsmAutomaton *gfsm_automaton_prune(gfsmAutomaton *fsm)
{
  gfsmBitVector *visited;
  gfsmStateId    id;
  if (!fsm || fsm->root_id == gfsmNoState) return fsm;

  visited = gfsm_bitvector_sized_new(fsm->states->len);
  gfsm_prune_visit_state(fsm, fsm->root_id, visited);

  //-- prune
  for (id=0; id < fsm->states->len; id++) {
    if (!gfsm_bitvector_get(visited,id)) {
      gfsm_automaton_remove_state(fsm,id);
    }
  }

  //-- cleanup
  gfsm_bitvector_free(visited);

  return fsm;
}

/*--------------------------------------------------------------
 * rmepsilon()
 */
/// TODO
gfsmAutomaton *gfsm_automaton_rmepsilon(gfsmAutomaton *fsm)
{
  g_assert_not_reached();
  return fsm;
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
