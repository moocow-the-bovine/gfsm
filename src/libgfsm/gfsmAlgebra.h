
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

/** \file gfsmAlgebra.h
 *  \brief Algebraic operations on automata
 */

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
/** Compute transitive or reflexive-\&-transitive
 *  closure of \a fsm. 
 * 
 *  \note Destructively alters \a fsm .
 *
 *  \param fsm Automaton
 *  \param is_plus Which type of closure to compute:
 *  \arg TRUE transitive closure
 *  \arg FALSE reflexive \& transitive closure
 *  \returns \c fsm
 */
gfsmAutomaton *gfsm_automaton_closure(gfsmAutomaton *fsm, gboolean is_plus);

/** Final-state pre-traversal utility for \c closure(fsm). */
gboolean _gfsm_automaton_closure_final_func(gfsmStateId id, gpointer dummy, gfsmAutomaton *fsm);

/** Compute \a n-ary closure of \a fsm.
 *  \note Destructively alters \a fsm.
 *
 *  \param fsm Automaton
 *  \param n Number of repetitions
 *  \returns \a fsm
 */
gfsmAutomaton *gfsm_automaton_n_closure(gfsmAutomaton *fsm, guint n);


//------------------------------
/**
 * Compute the lower-side complement of \a fsm with respect to its own lower alphabet.
 * \note Destructively alters \a fsm
 *
 * \param fsm Acceptor
 * \returns \a fsm.
 */
gfsmAutomaton *gfsm_automaton_complement(gfsmAutomaton *fsm);

/**
 * Compute the lower-side complement of \a fsm with respect to the alphabet \a alph,
 * which should contain all of the lower-labels from \a fsm.
 * \note Destructively alters \a fsm.
 *
 * \param fsm Acceptor
 * \param alph Alphabet with respect to which to compute complement.
 * \returns \a fsm
 */
gfsmAutomaton *gfsm_automaton_complement_full(gfsmAutomaton *fsm, gfsmAlphabet *alph);

/**
 * Complete the lower side of automaton \a fsm with respect to the alphabet \a alph
 * by directing "missing" arcs to the (new) state with id \a *sink.
 * \note Destructively alters \a fsm.
 *
 * \param fsm Acceptor
 * \param alpha Alphabet with respect to which \a fsm is to be completed
 * \param sinkp Pointer to a variable which on completion contains the Id of a (new) non-final sink state
 * \returns \a fsm
 */
gfsmAutomaton *gfsm_automaton_complete(gfsmAutomaton    *fsm,
				       gfsmAlphabet     *alph,
				       gfsmStateId      *sinkp);

//------------------------------
/** Compute the composition of \a fsm1 with \a fsm2
 *  (upper-side of \a fsm1 intersection with lower-side of \a fsm2).
 *
 *  \note Pseudo-destructive on \a fsm1.
 *
 *  \param fsm1 Lower-middle transducer
 *  \param fsm2 Middle-upper transducer
 *  \returns Altered \a fsm1
 *
 *  \sa gfsm_automaton_compose_full()
 */
gfsmAutomaton *gfsm_automaton_compose(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2);

/** Compute the composition of two transducers \a fsm1 and \a fsm2 
 *  into the transducer \a composition.
 *
 *  \param fsm1 Lower-middle transducer
 *  \param fsm2 Middle-upper transducer
 *  \param composition Lower-upper transducer.  May be passed as NULL to create a new automaton.
 *  \param spenum
 *    Mapping from (\a fsm1,\a fsm2) gfsmStatePairs to \a composition gfsmStateIds,
 *    if it is passed as \a NULL, a temporary enum will be created and freed.
 *
 *  \returns \a composition
 */
gfsmAutomaton *gfsm_automaton_compose_full(gfsmAutomaton *fsm1,
					   gfsmAutomaton *fsm2,
					   gfsmAutomaton *composition,
					   gfsmStatePairEnum *spenum);

/** Guts for gfsm_automaton_compose() \returns (new) StateId for \a sp. */
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
/** Append \a fsm2 onto the end of \a fsm1 \a n times.
 *  \note Destructively alters \a fsm1.
 *
 * \param fsm1 Automaton
 * \param fsm2 Automaton
 * \returns \a fsm1
 */
gfsmAutomaton *gfsm_automaton_n_concat(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2, guint n);

/** Append \a _fsm2 onto the end of \a fsm1.
 *  \note Destructively alters \a fsm1.
 *
 * \param fsm1 Automaton
 * \param fsm2 Automaton
 * \returns \a fsm1
 */
gfsmAutomaton *gfsm_automaton_concat(gfsmAutomaton *fsm1, gfsmAutomaton *_fsm2);

/** Final-state pre-traversal utility for \a concat(fsm,fsm2).
 *  Assumes \a fsm->root_id has been temporarily set to the translated gfsmStateId
 *  of \a fsm2->root_id.
 */
gboolean _gfsm_automaton_concat_final_func(gfsmStateId id, gpointer dummy, gfsmAutomaton *fsm);

//------------------------------
/**
 * Remove non-coaccessible states from \a fsm.
 * \note Destructively alters \a fsm
 *
 * \param fsm Automaton
 * \returns modified \a fsm
 */
gfsmAutomaton *gfsm_automaton_connect(gfsmAutomaton *fsm);


//------------------------------
/** Utility for \a gfsm_automaton_determinize(). */
void _gfsm_determinize_visit_state(gfsmAutomaton *nfa,    gfsmAutomaton *dfa,
				   gfsmStateSet  *nfa_ec, gfsmStateId    dfa_id,
				   gfsmEnum      *ec2id,  gfsmStateSet  *ec_tmp);

/** Determinize acceptor \a fsm1.
 *  \note Pseudo-destructive on \a fsm1
 *  \note Weights on epsilon-arcs are probably not handled correctly.
 *
 *  \param fsm1 Acceptor
 *  \returns altered \a fsm1
 *
 *  \sa gfsm_automaton_determinize_full()
 */
gfsmAutomaton *gfsm_automaton_determinize(gfsmAutomaton *fsm1);

/** Alias for gfsm_automaton_determinize() */
#define gfsm_automaton_determinise gfsm_automaton_determinize

/** Determinise automaton \a nfa to \a dfa.
 *  \note weights on epsilon-arcs are probably not handled correctly.
 *
 *  \param nfa non-deterministic acceptor 
 *  \param dfa deterministic acceptor to be constructed
 *             May be passed as \c NULL to create a new automaton.
 *  \returns \a dfa
 */
gfsmAutomaton *gfsm_automaton_determinize_full(gfsmAutomaton *nfa, gfsmAutomaton *dfa);

//------------------------------
/** Remove language of acceptor \a fsm2 from acceptor \a fsm1.
 *
 *  \note Pseudo-destructively alters \a fsm1.
 *  \note Really just an alias for intersect_full(fsm1,fsm2,NULL)
 *
 * \param fsm1 Acceptor
 * \param fsm2 Acceptor
 *  \returns \a fsm1
 */
gfsmAutomaton *gfsm_automaton_difference(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2);

/** Compute difference of acceptors (\a fsm1 - \a fsm2) into acceptor \a diff-
 *
 *  \note Really just an alias for intersect_full(fsm1,complement(clone(fsm2)),diff).
 *
 *  \param fsm1 Acceptor
 *  \param fsm2 Acceptor
 *  \param diff Output (difference) acceptor,
 *              may be passed as \c NULL to implicitly create a new automaton.
 *
 *  \returns (possibly new) difference automaton \a diff
 */
gfsmAutomaton *gfsm_automaton_difference_full(gfsmAutomaton *fsm1,
					      gfsmAutomaton *fsm2,
					      gfsmAutomaton *diff);

//------------------------------
/** Compute the intersection of two acceptors \a fsm1 and \a fsm2 (lower-side intersection).
 *  \note Pseudo-destructive on \a fsm1.
 *
 *  \param fsm1 Acceptor
 *  \param fsm2 Acceptor
 *  \returns \a fsm1.
 */
gfsmAutomaton *gfsm_automaton_intersect(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2);

/** Compute the intersection of two acceptors \a fsm1 and \a fsm2 
 *  into the acceptor \a intersect.
 *
 *  \param fsm1 Acceptor
 *  \param fsm2 Acceptor
 *  \param intersect Output acceptor, may be \c NULL to create a new automaton.
 *  \param spenum Mapping from (\a fsm1,\a fsm2) gfsmStatePairs to \a intersect gfsmStateIds,
 *                may be NULL to use a temporary mapping.
 *  \returns \a intersect.
 */
gfsmAutomaton *gfsm_automaton_intersect_full(gfsmAutomaton *fsm1,
					     gfsmAutomaton *fsm2,
					     gfsmAutomaton *intersect,
					     gfsmStatePairEnum *spenum);

/** Guts for gfsm_automaton_intersect()
 *  \returns (new) StateId for \a sp
 */
gfsmStateId _gfsm_automaton_intersect_visit(gfsmStatePair  sp,
					    gfsmAutomaton *fsm1,
					    gfsmAutomaton *fsm2,
					    gfsmAutomaton *fsm,
					    gfsmStatePairEnum *spenum);

//------------------------------
/** Invert upper and lower labels of an automaton.
 *  \note Destructively alters \a fsm.
 *
 *  \param fsm Transducer
 *  \returns \a fsm
 */
gfsmAutomaton *gfsm_automaton_invert(gfsmAutomaton *fsm);

//------------------------------
/** Compute Cartesian product of acceptors \a fsm1 and \a fsm2.
 *  \note Destructively alters \a fsm1.
 *
 *  \param fsm1 Acceptor (lower)
 *  \param fsm2 Acceptor (upper)
 *  \returns \a fsm1 (transducer)
 *
 *  \sa _gfsm_automaton_product()
 */
gfsmAutomaton *gfsm_automaton_product(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2);

/** Compute Cartesian product of acceptors \a fsm1 and \a fsm2.
 *  \note Destructively alters \a fsm1 \b and \a fsm2.
 *
 *  \param fsm1 Acceptor (lower)
 *  \param fsm2 Acceptor (upper)
 *  \returns \a fsm1 
 */
gfsmAutomaton *_gfsm_automaton_product(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2);


//------------------------------
/** Project one "side" (lower or upper) of \a fsm
 *  \note Destructively alters \a fsm
 *
 *  \param fsm Transducer
 *  \param which Which label side to project.
 *  \returns modified \a fsm
 */
gfsmAutomaton *gfsm_automaton_project(gfsmAutomaton *fsm, gfsmLabelSide which);


//------------------------------
/** Reverse an automaton \a fsm.
 *  \note Destructively alters \a fsm.
 *
 *  \param fsm Automaton
 *  \returns \a fsm
 */
gfsmAutomaton *gfsm_automaton_reverse(gfsmAutomaton *fsm);

//------------------------------
/**
 * Remove epsilon arcs from \a fsm.
 * \note Destructively alters \a fsm.
 * \note Multiple epsilon-paths between two states may not be weighted correctly in the output automaton.
 *
 * \param fsm Automaton
 * \returns \a fsm
 */
gfsmAutomaton *gfsm_automaton_rmepsilon(gfsmAutomaton *fsm);

/** Guts for rmepsilon() */
void gfsm_automaton_rmepsilon_visit_state(gfsmAutomaton *fsm,
					  gfsmStateId qid_noeps,
					  gfsmStateId qid_eps,
					  gfsmWeight weight_eps,
					  gfsmStatePairEnum *spenum);

//------------------------------
/** Add the language or relat6ion of \a fsm2 to \a fsm1.
 *  \note Destructively alters \a fsm1
 *
 *  \param fsm1 Automaton
 *  \param fsm2 Automaton
 *  \returns \a fsm1
 */
gfsmAutomaton *gfsm_automaton_union(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2);
//@}

/** \file gfsmAlgebra.h
 *  \todo sigma()
 *  \todo bestpath()
 *  \todo encode() ?
 *  \todo equiv() ?
 *  \todo minimize() ?
 *  \todo Regex compiler
 *  \todo deterministic union, tries
 */

#endif /* _GFSM_ALGEBRA_H */
