/*=============================================================================*\
 * File: gfsmEncode.c
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

#include <gfsmEncode.h>
#include <gfsmArcIter.h>

//-- no-inline definitions
#ifndef GFSM_INLINE_ENABLED
# include <gfsmEncode.hi>
#endif

/*======================================================================
 * gfsmArcLabel Methods
 */

//--------------------------------------------------------------
guint gfsm_arclabel_hash(gfsmArcLabel *al)
{
  //-- prime factors 4091,8179; see http://en.wikipedia.org/wiki/List_of_prime_numbers#Largest_primes_smaller_than_2n
  //   + glib hash table uses  4093,8191
  return 4091*al->lo + 8179*al->hi + GPOINTER_TO_UINT(gfsm_weight2ptr(al->w));
}

//--------------------------------------------------------------
guint gfsm_arclabel_equal(const gfsmArcLabel *al1, const gfsmArcLabel *al2)
{
  return al1->lo==al2->lo && al1->hi==al2->hi && al1->w==al2->w;
}

/*======================================================================
 * gfsmArcLabelKey Methods
 */

//--------------------------------------------------------------
gfsmAutomaton *gfsm_arclabel_key_to_fsm(gfsmArcLabelKey *key, gfsmAutomaton *fsm)
{
  gfsmPointerAlphabet *pkey = (gfsmPointerAlphabet*)key;
  gfsmState    *q0;
  gfsmLabelVal lab;

  if (!fsm) {
    fsm = gfsm_automaton_new_full(gfsmAutomatonDefaultFlags, gfsmAutomatonDefaultSRType, 1);
  } else {
    gfsm_automaton_clear(fsm);
  }

  //-- set root
  gfsm_automaton_set_root(fsm, 0);
  q0 = gfsm_automaton_find_state(fsm, 0);

  //-- copy labels (in order)
  for (lab=0; lab < pkey->labels2keys->len; lab++) {
    gfsmArcLabel *al = (gfsmArcLabel*)g_ptr_array_index(pkey->labels2keys, lab);
    q0->arcs = gfsm_arclist_new_full(0,0, al->lo,al->hi,al->w, q0->arcs);
  }

  //-- reverse arcs and set sort mode
  // + ensures that arcs will always be loaded in order, cf gfsm_automaton_load_bin_handle_0_0_8() in gfsmAutomatonIO.c
  q0->arcs = gfsm_arclist_reverse(q0->arcs);
  fsm->flags.sort_mode = gfsmACTarget;

  //-- ... and return
  return fsm;
}

//--------------------------------------------------------------
gfsmArcLabelKey *gfsm_arclabel_fsm_to_key(gfsmAutomaton *fsm, gfsmArcLabelKey *key)
{
  gfsmLabelVal lab;
  gfsmArcLabel al;
  gfsmArcIter ai;

  if (!key) {
    key = gfsm_arclabel_key_new();
  } else {
    gfsm_arclabel_key_clear(key);
  }

  //-- copy arcs to keys
  for (lab=0, gfsm_arciter_open(&ai, fsm,gfsm_automaton_get_root(fsm)); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai), lab++) {
    gfsmArc *a = gfsm_arciter_arc(&ai);
    gfsm_arclabel_set(&al, a->lower,a->upper,a->weight);
    gfsm_alphabet_insert(key, &al, lab);
  }

  //-- ... and return
  return key;

}

/*======================================================================
 * Top-Level Methods
 */
const gfsmLabelVal gfsmEncodeFinal = 1;

//--------------------------------------------------------------
gfsmArcLabelKey *gfsm_automaton_encode(gfsmAutomaton *fsm, gfsmArcLabelKey *key, gboolean encode_labels, gboolean encode_weights)
{
  gfsmWeight   w0 = gfsm_sr_one(fsm->sr), wf;
  gfsmStateId qid, qf=gfsmNoState;
  gfsmArcIter ai;
  gfsmArcLabel al;
  gfsmLabelVal lab, loFinal = (encode_labels ? gfsmEncodeFinal : gfsmEpsilon);

  if (!key) {
    key = gfsm_arclabel_key_new();
  }

  //-- ensure epsilon-entry in key
  gfsm_arclabel_set(&al, gfsmEpsilon,gfsmEpsilon,w0);
  gfsm_alphabet_insert(key, &al, gfsmEpsilon);

  //-- ensure new final state in encoded fsm for weight-encoding
  if (encode_weights) {
    qf = gfsm_automaton_add_state(fsm);
    gfsm_automaton_set_final_state_full(fsm, qf, TRUE, w0);
  }

  //-- encoding guts
  for (qid=0; qid < gfsm_automaton_n_states(fsm); qid++) {
    if (qid==qf || !gfsm_automaton_has_state(fsm,qid)) continue;

    //-- encode arc-labels
    for (gfsm_arciter_open(&ai, fsm,qid); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);

      //-- build gfsmArcLabel key
      if (encode_labels) {
	//-- add 1 to non-epsilon labels to allow (0:1/W) keys to identify final weights
	al.lo = a->lower==0 ? 0 : (a->lower+1);
      }
      al.hi = a->upper==0 ? 0 : (a->upper+1);
      if (encode_weights) {
	al.w = a->weight;
      }

      //-- ensure enumeration for current arc-key
      lab = gfsm_alphabet_get_label(key, &al);

      //-- modify automaton
      if (encode_labels) a->lower = lab;
      a->upper = lab;
      if (encode_weights) a->weight = w0;
    }

    //-- encode final weights
    if (encode_weights && gfsm_automaton_lookup_final(fsm, qid, &wf) && wf != w0) {
      //-- special entry-type *:1/W for final weights
      gfsm_arclabel_set(&al, loFinal,gfsmEncodeFinal, wf);
      lab = gfsm_alphabet_get_label(key, &al);
      if (encode_labels)
	gfsm_automaton_add_arc(fsm, qid,qf, lab,lab, w0);
      else
	gfsm_automaton_add_arc(fsm, qid,qf, loFinal,lab, w0);
      gfsm_automaton_set_final_state(fsm,qid,FALSE);
    }
  }

  //-- set flags
  if (encode_labels) fsm->flags.is_transducer = FALSE;
  if (encode_weights) fsm->flags.is_weighted = FALSE;
  fsm->flags.sort_mode = gfsmASMNone;

  //-- dump
  return key;
}

//--------------------------------------------------------------
gfsmAutomaton *gfsm_automaton_decode(gfsmAutomaton *fsm, gfsmArcLabelKey *key, gboolean decode_labels, gboolean decode_weights)
{
  gfsmWeight   w0 = gfsm_sr_one(fsm->sr);
  gfsmStateId qid, qf=gfsmNoState, nfdummy=0;
  gfsmArcIter ai;

  //-- decoding: attempt to find dummy final state
  if (decode_weights && gfsm_automaton_n_states(fsm) > 0) {
    qf = gfsm_automaton_n_states(fsm)-1;
    if (!gfsm_automaton_state_is_final(fsm,qf) || gfsm_automaton_out_degree(fsm,qf)!=0)
      qf=gfsmNoState;
  }

  //-- decoding guts
  for (qid=0; qid < gfsm_automaton_n_states(fsm); qid++) {
    if (!gfsm_automaton_has_state(fsm,qid)) continue;

    for (gfsm_arciter_open(&ai, fsm,qid); gfsm_arciter_ok(&ai); ) {
      gfsmArc       *a = gfsm_arciter_arc(&ai);
      gfsmArcLabel *al = (gfsmArcLabel*)gfsm_alphabet_find_key(key, a->upper);

      if (al && decode_weights && (al->hi==gfsmEncodeFinal || (a->target==qf && al->lo<=gfsmEncodeFinal && al->hi==gfsmEpsilon))) {
	//-- decode: encoded final-weight transition
	if (a->target==qf || (gfsm_automaton_state_is_final(fsm,a->target) && gfsm_automaton_out_degree(fsm,a->target)==0)) {
	  //-- decode into final weight and remove dummy arc
	  gfsm_automaton_set_final_state_full(fsm,qid,TRUE, gfsm_sr_plus(fsm->sr, gfsm_automaton_get_final_weight(fsm,qid), al->w));
	  gfsm_arciter_remove(&ai);
	  continue;
	}
	else {
	  //-- decode as epsilon-transition
	  a->lower  = a->upper = gfsmEpsilon;
	  a->weight = al->w;
	}
      }
      else {
	//-- decode: arc
	if (decode_labels) {
	  //-- subtract 1 from non-epsilon labels to allow (*:1/W) keys to identify final weights
	  a->lower = !al || al->lo==0 ? 0 : (al->lo-1);
	}
	a->upper = !al || al->hi==0 ? 0 : (al->hi-1);
	if (decode_weights) {
	  a->weight = al ? al->w : w0;
	}
      }

      //-- track adjusted in-degree of "dummy" final-state
      if (a->target == qf) ++nfdummy;

      //-- increment arc iterator
      gfsm_arciter_next(&ai);
    }
  }

  //-- implicitly remove dummy final state, if any
  if (qf != gfsmNoState && nfdummy==0) {
    gfsm_automaton_remove_state(fsm,qf);
  }

  //-- set flags
  if (decode_labels) fsm->flags.is_transducer = TRUE;
  if (decode_weights) fsm->flags.is_weighted = TRUE;
  fsm->flags.sort_mode = gfsmASMNone;

  //-- dump
  return fsm;
}
