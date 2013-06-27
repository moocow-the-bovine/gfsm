/*=============================================================================*\
 * File: gfsmDeterminize.c
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
#include <gfsmArray.h>
#include <gfsmAlgebra.h>
#include <gfsmAssert.h>
#include <gfsmArcIter.h>
#include <gfsmUtils.h>
#include <gfsmCompound.h>
#include <gfsmEnum.h>
//#include <gfsmAlphabet.h>

/*======================================================================
 * UTILS
 */

//--------------------------------------------------------------
static
guint gfsm_wqarray_hash(gfsmStateWeightPairArray *wqa)
{
  guint i;
  guint hv = 0;
  for (i=0; i < wqa->len; i++) {
    gfsmStateWeightPair *wq = &g_array_index(wqa, gfsmStateWeightPair, i);
    hv = (hv*1231) + (wq->id*1237) + (wq->w*1249);
  }
  return hv;
}

//--------------------------------------------------------------
static
gboolean gfsm_wqarray_equal(gfsmStateWeightPairArray *wqa1, gfsmStateWeightPairArray *wqa2)
{
  guint i;
  gfsmStateWeightPair *wq1, *wq2;
  if (wqa1->len != wqa2->len) return FALSE;
  for (i=0; i < wqa1->len; i++) {
    wq1 = &g_array_index(wqa1, gfsmStateWeightPair, i);
    wq2 = &g_array_index(wqa2, gfsmStateWeightPair, i);
    if ((wq1->id != wq2->id) || (wq1->w != wq2->w)) return FALSE;
  }
  return TRUE;
}

//--------------------------------------------------------------
static
void gfsm_wqarray_free(gfsmStateWeightPairArray *wqa)
{
  g_array_free(wqa,TRUE);
}

#if 0
//--------------------------------------------------------------
// wqptr = gfsm_wqarray_find(wqa,qid) : get element for state qid in wqa
//  + uses binary search, wqa must be sorted by state-ids
//  + returns NULL if no matching item is found
//  + UNUSED
static
gfsmStateWeightPair* gfsm_wqarray_find(gfsmStateWeightPairArray *wqa, gfsmStateId qid)
{
  const gfsmStateWeightPair key = {qid,0};
  return (gfsmStateWeightPair*)gfsm_array_lower_bound(wqa, &key, (GCompareDataFunc)gfsm_state_weight_pair_compare_q, NULL);
}

//--------------------------------------------------------------
// weight = gfsm_wqarray_weight(wqa,qid,sr) : get weight for state qid in wqa, or default
//  + UNUSED
static
gfsmStateWeightPair* gfsm_wqarray_weight(gfsmStateWeightPairArray *wqa, gfsmStateId qid, gfsmWeight notFound)
{
  gfsmStateWeightPair *wq = gfsm_wqarray_find(wqa, qid);
  if (wq==NULL) return notFound;
  return wq->w;
}
#endif

//--------------------------------------------------------------
typedef struct _gfsmResidualArc {
  gfsmStateWeightPair *wq; //-- residual-weighted source state
  gfsmArc             *a;  //-- arc from source state
} gfsmResidualArc;

//--------------------------------------------------------------#
static
gint gfsm_residual_arc_compare(gfsmResidualArc* ra1, gfsmResidualArc* ra2, gfsmArcCompData *acdata)
{
  return gfsm_arc_compare_bymask_inline(ra1->a, ra2->a, acdata);
}

/*======================================================================
 * Methods: algebra: determinize
 */

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
  //gfsmAlphabet   *ec2id; //-- (global) maps literal(equiv-class@nfa) <=> state-id@dfa
  gfsmEnum       *ec2id; //-- (global) maps literal(equiv-class@nfa) <=> state-id@dfa
  gfsmStateId    dfa_id; //-- (temp) id @ dfa
  gfsmStateWeightPairArray *nfa_ec;  //-- (temp) equiv-class@nfa: array
  gfsmStateWeightPair *wq; //-- (temp): state+weight pair in nfa_ec
  GSList *queue = NULL;    //-- processing queue: GSList of gfsmStateId (id@dfa)
  gfsmArcCompData acdata = {gfsmASMLower, nfa->sr, NULL, NULL};
  gfsmSemiring *sr;
  GArray *nfa_arcs = NULL;
 
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
  //-- avoid "smart" arc-insertion
  dfa->flags.sort_mode = gfsmASMNone;
  sr = dfa->sr;

  //-- initialization: dynamically allocated locals: ec2id, nfa_arcs
  ec2id = gfsm_enum_new_full(NULL /*(gfsmDupFunc)gfsm_wqarray_clone*/ ,
			     (GHashFunc)gfsm_wqarray_hash,
			     (GEqualFunc)gfsm_wqarray_equal,
			     (GDestroyNotify)gfsm_wqarray_free);
  nfa_arcs = g_array_sized_new(FALSE, FALSE, sizeof(gfsmResidualArc), 32);

  //-- initialization: root
  nfa_ec = g_array_sized_new(FALSE,FALSE,sizeof(gfsmStateWeightPair),1);
  g_array_set_size(nfa_ec,1);
  wq = &g_array_index(nfa_ec, gfsmStateWeightPair, 0);
  wq->id = nfa->root_id;
  wq->w  = sr->one;
  dfa_id = gfsm_automaton_ensure_state(dfa, gfsm_enum_insert(ec2id, nfa_ec));
  gfsm_automaton_set_root(dfa, dfa_id);

  //-- initialization: root: final?
  if (gfsm_automaton_state_is_final(nfa, nfa->root_id)) {
    gfsm_automaton_set_final_state_full(dfa, dfa->root_id, TRUE, gfsm_automaton_get_final_weight(nfa, nfa->root_id));
  }

  //-- guts: queue processing
  queue = g_slist_prepend(queue, nfa_ec);
  while (queue != NULL) {
    GSList *head = queue;
    gfsmState *qptr;
    guint eci;
    gfsmArcIter ai;
    guint xi0,xi1, xi;

    //-- pop the queue
    queue  = queue->next;
    nfa_ec = (gfsmStateWeightPairArray*)head->data;
    dfa_id = gfsm_enum_lookup(ec2id, nfa_ec);
    g_slist_free_1(head);

    //-- nfa_arcs: generate & sort list of all outgoing arcs, with residual-weighted source-states
    g_array_set_size(nfa_arcs,0);
    for (eci=0; eci < nfa_ec->len; eci++) {
      wq   = &g_array_index(nfa_ec, gfsmStateWeightPair, eci);
      qptr = gfsm_automaton_find_state(nfa,wq->id);
      for (gfsm_arciter_open_ptr(&ai,nfa,qptr); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
	gfsmResidualArc ra;
	ra.wq = wq;
	ra.a  = gfsm_arciter_arc(&ai);
	g_array_append_val(nfa_arcs,ra);
      }
    }
    g_array_sort_with_data(nfa_arcs, (GCompareDataFunc)gfsm_residual_arc_compare, &acdata);

    //-- extract & process (lo,hi)-runs in nfa_arcs[] --> collect by state into (lo,hi,qto)-runs; implement Morhi (2009) lines 7-16
    //   : compute wx = sum of (residual(qsrc)*weight(arc)) costs for these arcs
    for (xi0=0; xi0 < nfa_arcs->len; xi0=xi1) {
      gfsmResidualArc *ra0 = &g_array_index(nfa_arcs, gfsmResidualArc, xi0);
      gfsmLabelId lo=ra0->a->lower, hi=ra0->a->upper;
      gfsmWeight wx = gfsm_sr_times(sr, ra0->wq->w, ra0->a->weight), wx_inv;
      gfsmStateWeightPair wq_new;
      gfsmStateWeightPairArray *qx=NULL;
      gfsmStateId qid_new;

      for (xi1=xi0+1; xi1 < nfa_arcs->len; xi1++) {
	gfsmResidualArc *ra1 = &g_array_index(nfa_arcs, gfsmResidualArc, xi1);
	if (ra1->a->lower != lo || ra1->a->upper != hi) break;
	wx = gfsm_sr_plus(sr, wx, gfsm_sr_times(sr, ra1->wq->w, ra1->a->weight));
      }
      wx_inv = gfsm_sr_inv_l(sr,wx);

      //-- we have an $x=(lo,hi) run in (xli0..(xli1-1)), identical target states should be adjacent
      //   : compute weighted target state-set qx
      qx = g_array_sized_new(FALSE,FALSE,sizeof(gfsmStateWeightPair),xi1-xi0);
      for (xi=xi0; xi < xi1; xi++) {
	gfsmResidualArc *ra = &g_array_index(nfa_arcs, gfsmResidualArc, xi);
	wq = qx->len==0 ? NULL : &g_array_index(qx, gfsmStateWeightPair, (qx->len-1));
	if (wq==NULL || wq->id != ra->a->target) {
	  //-- new sink state: append
	  wq_new.id = ra->a->target;
	  wq_new.w  = gfsm_sr_times(sr, wx_inv, gfsm_sr_times(sr, ra->wq->w, ra->a->weight));
	  g_array_append_val(qx, wq_new);
	} else {
	  //-- sink-state run: just add in weights
	  wq->w = gfsm_sr_plus(sr, wq->w, gfsm_sr_times(sr, wx_inv, gfsm_sr_times(sr, ra->wq->w, ra->a->weight)));
	}
      }

      //-- check whether the sink state-set is known
      qid_new = gfsm_enum_lookup(ec2id, qx);
      if (qid_new != gfsmEnumNone) {
	//-- known sink-state: just add an arc & free the temporary state-set
	gfsm_automaton_add_arc(dfa, dfa_id, qid_new, lo, hi, wx);
	g_array_free(qx,TRUE);
      }
      else {
	//-- unknown sink-state: add new state
	guint qxi;
	gfsmWeight fw_qx=sr->zero, fw_i=0;

	qid_new = gfsm_automaton_add_state(dfa);
	gfsm_enum_insert_full(ec2id, qx, qid_new);
	gfsm_automaton_add_arc(dfa, dfa_id, qid_new, lo, hi, wx);

	//-- is any qx element-state final in nfa?
	for (qxi=0; qxi < qx->len; qxi++) {
	  wq = &g_array_index(qx, gfsmStateWeightPair, qxi);
	  if (gfsm_automaton_lookup_final(nfa, wq->id, &fw_i)) {
	    fw_qx = gfsm_sr_plus(sr, fw_qx, gfsm_sr_times(sr, wq->w, fw_i));
	  }
	}
	if (fw_qx != sr->zero) {
	  gfsm_automaton_set_final_state_full(dfa, qid_new, TRUE, fw_qx);
	}

	//-- enqueue new dfa state
	queue = g_slist_prepend(queue, qx);
      }
    }
  }

  //-- cleanup
  g_array_free(nfa_arcs,TRUE);
  gfsm_enum_free(ec2id);

  //-- return
  return dfa;
}

