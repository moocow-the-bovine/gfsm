
/*=============================================================================*\
 * File: gfsmAlgebra.h
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

#ifndef _GFSM_ALGEBRA_H
#define _GFSM_ALGEBRA_H

#include <glib.h>
#include <gfsmAutomaton.h>
#include <gfsmAlphabet.h>
#include <gfsmStateSet.h>
#include <gfsmEnum.h>


/*======================================================================
 * Methods: algebra
 */
///\name Algebra
//@{

//------------------------------
/** Compute transitive (@is_plus!=FALSE) or reflexive+transitive (@is_plus==FALSE)
 *  closure of @fsm.  Destructively alters @fsm1.
 *  \returns @fsm
 */
gfsmAutomaton *gfsm_automaton_closure(gfsmAutomaton *fsm, gboolean is_plus);

/** Final-state pre-traversal utility for closure(fsm). */
gboolean _gfsm_automaton_closure_final_func(gfsmStateId id, gpointer dummy, gfsmAutomaton *fsm);

/** Compute @n ary closure of @fsm.  Destructively alters @fsm.
 *  \returns @fsm
 */
gfsmAutomaton *gfsm_automaton_n_closure(gfsmAutomaton *fsm, guint n);


//------------------------------
/**
 * Compute the complement of @fsm with respect to its own alphabet.
 * Destructively alters @fsm \returns @fsm
 */
gfsmAutomaton *gfsm_automaton_complement(gfsmAutomaton *fsm);

/**
 * Compute the complement of @fsm with respect to the alphabet @alph,
 * which should contain all of the lower-labels from @fsm1.  Destructively alters @fsm.
 * \returns @fsm
 */
gfsmAutomaton *gfsm_automaton_complement_full(gfsmAutomaton *fsm, gfsmAlphabet *alph);

/**
 * Complete the lower side of automaton @fsm with respect to the alphabet @alph
 * by directing "missing" arcs to the (new) state with id *@sink.  Destructively
 * alters @fsm.
 * \returns @fsm
 */
gfsmAutomaton *gfsm_automaton_complete(gfsmAutomaton    *fsm,
				       gfsmAlphabet     *alph,
				       gfsmStateId      *sinkp);

//------------------------------
/** Compute the composition of transducer @fsm1 with @fsm2. \returns altered @fsm1 */
/// TODO
gfsmAutomaton *gfsm_automaton_compose(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2);

//------------------------------
/** Append @fsm2 onto the end of @fsm1.  \returns @fsm1 */
gfsmAutomaton *gfsm_automaton_concat(gfsmAutomaton *fsm1, const gfsmAutomaton *fsm2);

/** Final-state pre-traversal utility for concat(fsm,fsm2).
 *  Assumes (fsm->root_id) has been temporarily set to the translated stateId
 *  of fsm2->root_id */
gboolean _gfsm_automaton_concat_final_func(gfsmStateId id, gpointer dummy, gfsmAutomaton *fsm);

//------------------------------
/** Utility for gfsm_automaton_determinize(). */
void _gfsm_determinize_visit_state(gfsmAutomaton *nfa,    gfsmAutomaton *dfa,
				   gfsmStateSet  *nfa_ec, gfsmStateId    dfa_id,
				   gfsmEnum      *ec2id,  gfsmStateSet  *ec_tmp);

/** Determinise @fsm1 pseudo-destructively.
 *  \note weights on epsilon-arcs are probably not handled correctly.
 *  \returns altered @fsm1
 */
gfsmAutomaton *gfsm_automaton_determinize(gfsmAutomaton *fsm1);

/** Alias for gfsm_automaton_determinize() */
#define gfsm_automaton_determinise gfsm_automaton_determinize

/** Determinise automaton @nfa to @dfa.
 *  \note weights on epsilon-arcs are probably not handled correctly.
 *  \returns @dfa
 */
gfsmAutomaton *gfsm_automaton_determinize_2(gfsmAutomaton *nfa, gfsmAutomaton *dfa);

//------------------------------
/** Remove language of @fsm2 from @fsm1. \returns @fsm1 */
/// TODO
gfsmAutomaton *gfsm_automaton_difference(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2);

//------------------------------
/** Compute the intersection of two acceptors @fsm1 and @fsm2. */
/// TODO
gfsmAutomaton *gfsm_automaton_intersection(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2);

//------------------------------
/** Invert upper and lower labels of an FSM */
gfsmAutomaton *gfsm_automaton_invert(gfsmAutomaton *fsm);

//------------------------------
/** Compute Cartesian product of @fsm1 and @fsm2.  \returns @fsm1  */
/// TODO
gfsmAutomaton *gfsm_automaton_product(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2);


//------------------------------
/** Project one "side" (lower or upper) of @fsm */
gfsmAutomaton *gfsm_automaton_project(gfsmAutomaton *fsm, gfsmLabelSide which);

//------------------------------
/** Prune unreachable states from @fsm.  \returns @fsm */
gfsmAutomaton *gfsm_automaton_prune(gfsmAutomaton *fsm);

//------------------------------
/** Reverse an @fsm. \returns @fsm */
gfsmAutomaton *gfsm_automaton_reverse(gfsmAutomaton *fsm);

//------------------------------
/** Remove epsilon arcs from @fsm.  \returns @fsm */
/// TODO
gfsmAutomaton *gfsm_automaton_rmepsilon(gfsmAutomaton *fsm);

//------------------------------
/** Assign the union of @fsm1 and @fsm2 to @fsm1. \returns @fsm1 */
gfsmAutomaton *gfsm_automaton_union(gfsmAutomaton *fsm1, const gfsmAutomaton *fsm2);
//@}

/** \todo
 *  + sigma
 *  + compose
 *  + bestpath
 *  + paths
 *  + ? encode
 *  + ? equiv
 *  + ? minimize
 */

#endif /* _GFSM_ALGEBRA_H */
