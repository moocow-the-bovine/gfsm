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

/*======================================================================
 * gfsmArcLabelKey Methods
 */

//--------------------------------------------------------------
gfsmAutomaton *gfsm_arclabel_key_to_fsm(gfsmArcLabelKey *key, gfsmAutomaton *fsm)
{
  gfsmPointerAlphabet *pkey = (gfsmPointerAlphabet*)key;
  gfsmLabelVal lab;

  if (!fsm) {
    fsm = gfsm_automaton_new_full(gfsmAutomatonDefaultFlags, gfsmAutomatonDefaultSRType, 1);
  } else {
    gfsm_automaton_clear(fsm);
  }

  //-- set root
  gfsm_automaton_set_root(fsm, 0);

  //-- copy labels (in order)
  for (lab=0; lab < pkey->labels2keys->len; lab++) {
    gfsmArcLabel *al = (gfsmArcLabel*)g_ptr_array_index(pkey->labels2keys, lab);
    gfsm_automaton_add_arc(fsm, 0,0, al->lo, al->hi, al->w);
  }

  //-- ... and return
  return fsm;
}

//--------------------------------------------------------------
gfsmArcLabelKey *gfsm_arclabel_fsm_to_key(gfsmAutomaton *fsm, gfsmArcLabelKey *key)
{
  gfsmPointerAlphabet *pkey = (gfsmPointerAlphabet*)key;
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

//--------------------------------------------------------------
gfsmArcLabelKey *gfsm_automaton_encode(gfsmAutomaton *fsm, gfsmArcLabelKey *key, gboolean encode_labels, gboolean encode_weights)
{
  gfsmWeight   w0 = gfsm_sr_one(fsm->sr);
  gfsmStateId qid, qf;
  gfsmArcIter ai;
  gfsmArcLabel al;
  gfsmLabelVal lab;

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
    if (!gfsm_automaton_has_state(fsm,qid)) continue;

    //-- encode arc-labels
    for (gfsm_arciter_open(&ai, fsm,qid); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);

      //-- build gfsmArcLabel key
      if (encode_labels) {
	//-- add 1 to non-epsilon labels to allow (0:1/W) keys to identify final weights
	al->lo = a->lower==0 ? 0 : (a->lower+1);
	al->hi = a->upper==0 ? 0 : (a->upper+1);
      }
      if (encode_weights) {
	al->w = a->weight;
      }

      //-- ensure enumeration for current arc-key
      lab = gfsm_alphabet_get_label(key, &al);

      //-- modify automaton
      if (encode_labels) al->lo = lab;
      a->hi = lab
      if (encode_weights) a->w = w0;
    }

    //-- encode final weights
    if (encode_weights && gfsm_automaton_state_is_final(fsm,qid)) {
      //-- special entry-type 0:1/W for final weights
      gfsm_arclabel_set(&al, 0,1, gfsm_automaton_get_final_weight(fsm,qid));
      lab = gfsm_alphabet_get_label(key, &al);
      gfsm_automaton_add_arc(fsm, qid,qf, 0,lab, w0);
      gfsm_automaton_set_final_state(fsm,qid,FALSE);
    }
  }

  //-- dump
  return key;
}

//--------------------------------------------------------------
gfsmAutomaton *gfsm_automaton_decode(gfsmAutomaton *fsm, gfsmArcLabelKey *key, gboolean decode_labels, gboolean decode_weights)
{
  gfsmStateId qid;
  gfsmArcIter ai;

  //-- decoding guts
  for (qid=0; qid < gfsm_automaton_n_states(fsm); qid++) {
    if (!gfsm_automaton_has_state(fsm,qid)) continue;

    for (gfsm_arciter_open(&ai, fsm,qid); gfsm_arciter_ok(&ai); ) {
      gfsmArc       *a = gfsm_arciter_arc(&ai);
      gfsmArcLabel *al = (gfsmArcLabel*)g_ptr_array_index(pkey->labels2keys, a->upper);

      if (al->hi==1 && decode_weights) {
	//-- decode: final weight
	gfsm_automaton_set_final_state_full(fsm,qid,TRUE,al->w);
	gfsm_arciter_remove(&ai);
	continue;
      }
      else if (al->hi!=1) {
	//-- decode: arc
	if (decode_labels) {
	  //-- subtract 1 from non-epsilon labels to allow (*:1/W) keys to identify final weights
	  a->lower = al->lo==0 ? 0 : (al->lo-1);
	}
	a->upper = al->hi==0 ? 0 : (al->hi-1);
	if (decode_weights) {
	  a->weight = al->w;
	}
      }

      //-- increment arc iterator
      gfsm_arciter_next(&ai);
    }
  }

  //-- todo: figure out whehter rsp. how to recognize and/or implicitly remove dummy final state(s)


  //-- dump
  return fsm;
}
