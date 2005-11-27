
/*=============================================================================*\
 * File: gfsmAlgebra.h
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

#ifndef _GFSM_ALGEBRA_H
#define _GFSM_ALGEBRA_H

#include <glib.h>
#include <gfsmAutomaton.h>
#include <gfsmAlphabet.h>
#include <gfsmStateSet.h>
#include <gfsmEnum.h>
#include <gfsmCompound.h>


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
 * Compute the lower-side complement of @fsm with respect to its own lower alphabet.
 * Destructively alters @fsm \returns @fsm
 */
gfsmAutomaton *gfsm_automaton_complement(gfsmAutomaton *fsm);

/**
 * Compute the lower-side complement of @fsm with respect to the alphabet @alph,
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
/** Compute the composition of @fsm1 with @fsm2
 *  (upper-side of @fsm1 intersection with lower-side of @fsm2).
 *  Pseudo-destructive on @fsm1.
 *  \returns altered @fsm1
 */
// WIP
gfsmAutomaton *gfsm_automaton_compose(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2);

/** Compute the composition of two transducers @fsm1 and @fsm2 
 *  into the transducer @composition, which may be passed as NULL to create a new FSM.
 *  @spenum stores a mapping from (@fsm1,@fsm2) StatePairs to @fsm StateIds,
 *  if it is passed as NULL, a temporary enum will be created.
 *  \returns @fsm3.
 */
// WIP
gfsmAutomaton *gfsm_automaton_compose_full(gfsmAutomaton *fsm1,
					   gfsmAutomaton *fsm2,
					   gfsmAutomaton *composition,
					   gfsmStatePairEnum *spenum);

/** Guts for gfsm_automaton_compose() \returns (new) StateId for @sp. */
gfsmStateId _gfsm_automaton_compose_visit(gfsmStatePair  sp,
					  gfsmAutomaton *fsm1,
					  gfsmAutomaton *fsm2,
					  gfsmAutomaton *fsm,
					  gfsmStatePairEnum *spenum);

/** Wrapper guts for compose() and intersect() */
gfsmAutomaton *_gfsm_automaton_compose_intersect_wrapper(gfsmAutomaton *fsm1,
							 gfsmAutomaton *fsm2,
							 gfsmAutomaton *fsmout,
							 gfsmStatePairEnum *spenum,
							 gboolean is_composition);



//------------------------------
/** Append @fsm2 onto the end of @fsm1 @n times.  \returns @fsm1 */
gfsmAutomaton *gfsm_automaton_n_concat(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2, guint n);

/** Append @_fsm2 onto the end of @fsm1.  \returns @fsm1 */
gfsmAutomaton *gfsm_automaton_concat(gfsmAutomaton *fsm1, gfsmAutomaton *_fsm2);

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
/** Remove language of acceptor @fsm2 from acceptor @fsm1.
 *  Pseudo-destructively alters @fsm1.
 *  Really just an alias for intersect_full(fsm1,fsm2,NULL)
 *  \returns @fsm1
 */
gfsmAutomaton *gfsm_automaton_difference(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2);

/** Compute difference of acceptors (@fsm1-@fsm2) into acceptor @diff,
 *  which may be passed as NULL to implicitly create a new automaton.
 *  Really just an alias for intersect_full(fsm1,complement(clone(fsm2)),diff).
 *  \returns (possibly new) difference automaton @diff
 */
gfsmAutomaton *gfsm_automaton_difference_full(gfsmAutomaton *fsm1,
					      gfsmAutomaton *fsm2,
					      gfsmAutomaton *diff);

//------------------------------
/** Compute the intersection of two acceptors @fsm1 and @fsm2 (lower-side intersection).
 *  Pseudo-destructive on @fsm1.
 *  \returns @fsm1.
 */
gfsmAutomaton *gfsm_automaton_intersect(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2);

/** Compute the intersection of two acceptors @fsm1 and @fsm2 
 *  into the acceptor @intersect, which may be passed as NULL to create a new FSM.
 *  @spenum stores a mapping from (@fsm1,@fsm2) StatePairs to @fsm StateIds,
 *  if it is passed as NULL, a temporary hash will be created.
 *  \returns @fsm3.
 */
gfsmAutomaton *gfsm_automaton_intersect_full(gfsmAutomaton *fsm1,
						gfsmAutomaton *fsm2,
						gfsmAutomaton *intersect,
						gfsmStatePairEnum *spenum);

/** Guts for gfsm_automaton_intersect() \returns (new) StateId for @sp */
gfsmStateId _gfsm_automaton_intersect_visit(gfsmStatePair  sp,
					       gfsmAutomaton *fsm1,
					       gfsmAutomaton *fsm2,
					       gfsmAutomaton *fsm,
					       gfsmStatePairEnum *spenum);

//------------------------------
/** Invert upper and lower labels of an FSM */
gfsmAutomaton *gfsm_automaton_invert(gfsmAutomaton *fsm);

//------------------------------
/** Compute Cartesian product of acceptors @fsm1 and @fsm2.
 *  Destructively alters @fsm1.
 *  \returns @fsm1 
 */
gfsmAutomaton *gfsm_automaton_product(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2);

/** Compute Cartesian product of acceptors @fsm1 and @fsm2.
 *  Destructively alters @fsm1 and @fsm2.
 *  \returns @fsm1 
 */
gfsmAutomaton *_gfsm_automaton_product(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2);


//------------------------------
/** Project one "side" (lower or upper) of @fsm
 *  \returns modified @fsm
 */
gfsmAutomaton *gfsm_automaton_project(gfsmAutomaton *fsm, gfsmLabelSide which);

//------------------------------
/**
 * Prune unreachable states from @fsm.
 * \returns pruned @fsm
 */
gfsmAutomaton *gfsm_automaton_prune(gfsmAutomaton *fsm);

//------------------------------
/** Reverse an @fsm.
 *  \returns @fsm
 */
gfsmAutomaton *gfsm_automaton_reverse(gfsmAutomaton *fsm);

//------------------------------
/**
 * Remove epsilon arcs from @fsm.
 * Destructively alters @fsm.
 * \returns @fsm
 */
gfsmAutomaton *gfsm_automaton_rmepsilon(gfsmAutomaton *fsm);

/** Guts for rmepsilon() */
void gfsm_automaton_rmepsilon_visit_state(gfsmAutomaton *fsm,
					  gfsmStateId qid_noeps,
					  gfsmStateId qid_eps,
					  gfsmWeight weight_eps,
					  gfsmStatePairEnum *spenum);

//------------------------------
/** Assign the union of @fsm1 and @fsm2 to @fsm1. \returns @fsm1 */
gfsmAutomaton *gfsm_automaton_union(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2);
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
