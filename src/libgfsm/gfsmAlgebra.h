
/*=============================================================================*\
 * File: gfsmAlgebra.h
 * Author: Bryan Jurish <moocow.bovine@gmail.com>
 * Description: finite state machine library
 *
 * Copyright (c) 2004-2007 Bryan Jurish.
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

/** \file gfsmAlgebra.h
 *  \brief Algebraic operations on automata
 */

#ifndef _GFSM_ALGEBRA_H
#define _GFSM_ALGEBRA_H

#include <gfsmCompound.h>
#include <gfsmArcIndex.h>

/*======================================================================
 * Methods: algebra
 */

//------------------------------
///\name gfsmClosure.c: closure (self-concatenation)
//@{
/** Compute transitive or reflexive-\&-transitive
 *  closure of \a fsm. 
 *  \note Destructively alters \a fsm .
 *
 *  \param fsm Automaton
 *  \param is_plus Which type of closure to compute:
 *   \arg TRUE transitive closure
 *   \arg FALSE reflexive \& transitive closure
 *  \returns modified \a fsm
 */
gfsmAutomaton *gfsm_automaton_closure(gfsmAutomaton *fsm, gboolean is_plus);

/* Final-state pre-traversal utility for \c closure(fsm). */
//gboolean gfsm_automaton_closure_final_func_(gfsmStateId id, gpointer pw, gfsmAutomaton *fsm);

/** Compute \a n-ary closure of \a fsm.
 *  \note Destructively alters \a fsm.
 *
 *  \param fsm Automaton
 *  \param n Number of repetitions
 *  \returns modified \a fsm
 */
gfsmAutomaton *gfsm_automaton_n_closure(gfsmAutomaton *fsm, guint n);

/** Make \a fsm optional.
 *  \note Destructively alters \a fsm
 *
 *  \param fsm Automaton
 *  \returns \a modified fsm
 */
gfsmAutomaton *gfsm_automaton_optional(gfsmAutomaton *fsm);
//@}

//------------------------------
///\name gfsmComplement.c: Complementation and Completion
//@{
/** Compute the lower-side complement of \a fsm with respect to its own lower alphabet.
 * \note Destructively alters \a fsm
 *
 * \param fsm Acceptor
 * \returns \a fsm
 */
gfsmAutomaton *gfsm_automaton_complement(gfsmAutomaton *fsm);

/** Compute the lower-side complement of \a fsm with respect to the alphabet \a alph,
 * which should contain all of the lower-labels from \a fsm.
 * \note Destructively alters \a fsm.
 *
 * \param fsm Acceptor
 * \param alph Alphabet with respect to which to compute complement.
 * \returns \a fsm
 */
gfsmAutomaton *gfsm_automaton_complement_full(gfsmAutomaton *fsm, gfsmAlphabet *alph);

/** Complete the lower side of automaton \a fsm with respect to the alphabet \a alph
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
//@}

//------------------------------
///\name gfsmCompose.c: Composition
//@{

/** Compute the composition of \a fsm1 with \a fsm2
 *  (upper-side of \a fsm1 intersection with lower-side of \a fsm2).
 *
 *  \param fsm1 Lower-middle transducer
 *  \param fsm2 Middle-upper transducer
 *  \returns Altered \a fsm1
 *
 *  \note
 *    \li Pseudo-destructive on \a fsm1.
 *    \li Runtime efficiency can be greatly improved if
 *        \a fsm1 is sorted on upper labels (::gfsmASMUpper)
 *        and \a fsm2 is sorted on lower labels (::gfsmASMLower).
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
 *    Mapping from (\a fsm1,\a fsm2,\a filter) ::gfsmComposeState s to \a composition (::gfsmStateId)s,
 *    if it is passed as \a NULL, a temporary enum will be created and freed.
 *
 *  \sa Mohri, Pereira, and Riley (1996), "Weighted Automata in Text and Speech Processing",
 *      In: <em>Proc. ECAI '96</em>, John Wiley & Sons, Ltd.
 *
 *  \returns \a composition
 */
gfsmAutomaton *gfsm_automaton_compose_full(gfsmAutomaton *fsm1,
					   gfsmAutomaton *fsm2,
					   gfsmAutomaton *composition,
					   gfsmComposeStateEnum *spenum);

typedef guint32 gfsmComposeFlags; /**< flags for gfsm_automaton_compose_visit_state_() */

/** \brief Enum type for low-level flags to gfsm_automaton_compose_visit_state_() */
typedef enum {
  gfsmCFEfsm1NeedsArcSort = 0x1,
  gfsmCFEfsm2NeedsArcSort = 0x2
} gfsmComposeFlagsE;

/** Guts for gfsm_automaton_compose() \returns (new) StateId for \a sp */
void gfsm_automaton_compose_visit_(gfsmStateId       qid,
				   gfsmAutomaton    *fsm1,
				   gfsmAutomaton    *fsm2,
				   gfsmAutomaton    *fsm,
				   gfsmComposeStateEnum *spenum,
				   GArray               *spenumr, //-- GArray of gfsmComposeState
				   GQueue 	        *queue,   //-- queue of gfsmStateId
				   gfsmComposeFlags  flags);
//@}

//------------------------------
///\name gfsmConcat.c: Concatenation
//@{

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

/* Final-state pre-traversal utility for \a concat(fsm,fsm2).
 *
 *  \note Assumes \a fsm->root_id has been temporarily set to the translated gfsmStateId
 *        of \a fsm2->root_id.
 *
 *  \param pw  final ::gfsmWeight encoded as a gpointer (e.g. with gfsm_weight2ptr())
 *  \param fsm concatenation first argument / return value
 *  \returns FALSE
 */
//gboolean gfsm_automaton_concat_final_func_(gfsmStateId id, gpointer pw, gfsmAutomaton *fsm);

//@}

//------------------------------
///\name gfsmConnect.c: Co-Accessibility & Pruning
//@{

/** Remove non-coaccessible states from \a fsm.
 * Calls gfsm_automaton_connect_fw() and gfsm_automaton_connect_bw()
 * \note Destructively alters \a fsm
 *
 * \param fsm Automaton
 * \returns modified \a fsm
 */
gfsmAutomaton *gfsm_automaton_connect(gfsmAutomaton *fsm);

/** Remove non root-accessible states from \a fsm.
 * Called by gfsm_automaton_connect()
 *
 * \note Destructively alters \a fsm
 *
 * \param fsm Automaton
 * \param visited
 *   Bit-vector for traversal.  Should have all bits set to zero.
 *   If passed as NULL, a new bit-vector will be created and freed.
 * \returns modified \a fsm
 */
gfsmAutomaton *gfsm_automaton_connect_fw(gfsmAutomaton *fsm, gfsmBitVector *visited);

/** Remove non-finalizable states from \a fsm.
 * Called by gfsm_automaton_connect().
 * \note Destructively alters \a fsm
 *
 * \param fsm Automaton
 * \param rarcs
 *   Reverse arc-index as returned by gfsm_automaton_reverse_arc_index().
 *   If passed as NULL, gfsm_automaton_reverse_arc_index() will be called
 *   on a temporary arc index.
 * \param finalizable
 *   Bit-vector for traversal.  Should have all bits set to zero.
 *   If passed as NULL, a new bit-vector will be created and freed.
 * \returns modified \a fsm
 */
gfsmAutomaton *gfsm_automaton_connect_bw(gfsmAutomaton       *fsm,
					 gfsmReverseArcIndex *rarcs,
					 gfsmBitVector       *finalizable);

/** Utility for gfsm_automaton_connect_fw() and gfsm_automaton_connect_bw().
 * Prunes states from \a fsm whose id bit is not set in \a want
 *
 * \param fsm    Automaton
 * \param wanted
 *   Bit-vector indexed by state id: bit \a id should be unset
 *   iff state \a id is to be removed.
 * \returns modified \a fsm
 */
gfsmAutomaton *gfsm_automaton_prune_states(gfsmAutomaton *fsm, gfsmBitVector *wanted);

//@}

//------------------------------
///\name gfsmDeterminize.c: Determinization
//@{

/** Determinize acceptor \a fsm
 *
 *  - Pseudo-destructive on \a fsm
 *  - Epsilon is treated like any other symbol.
 *  - Arc labels are treated as (input,output) pairs for purposes
 *    of state-equivalence-class construction: this may not be what you want.
 *  .
 *
 *  \param fsm Automaton
 *  \returns altered \a fsm
 *
 *  \sa gfsm_automaton_determinize_full()
 */
gfsmAutomaton *gfsm_automaton_determinize(gfsmAutomaton *fsm);

/** Determinize automaton \a nfa to \a dfa
 *  - Epsilon is treated like any other symbol.
 *  - Arc labels are treated as (input,output) pairs.
 *  .
 *
 *  \param nfa non-deterministic acceptor 
 *  \param dfa deterministic acceptor to be constructed
 *             May be passed as \c NULL to create a new automaton.
 *  \returns \a dfa
 */
gfsmAutomaton *gfsm_automaton_determinize_full(gfsmAutomaton *nfa, gfsmAutomaton *dfa);

/** Alias for gfsm_automaton_determinize() */
#define gfsm_automaton_determinise(fsm) gfsm_automaton_determinize(fsm)

/** Alias for gfsm_automaton_determinize_full() */
#define gfsm_automaton_determinise_full(nfa,dfa) gfsm_automaton_determinize_full((nfa),(dfa))

//@}

//------------------------------
///\name gfsmDifference.c: Difference
//@{

/** Remove language of acceptor \a fsm2 from acceptor \a fsm1.
 *
 *  - Pseudo-destructively alters \a fsm1.
 *  - Really just an alias for intersect_full(fsm1,fsm2,NULL)
 *  .
 *
 *  \param fsm1 Acceptor
 *  \param fsm2 Acceptor
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

//@}

//------------------------------
///\name gfsmIntersect.c: Intersection
//@{

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
 *  \param spenum Mapping from (\a fsm1,\a fsm2) (::gfsmStatePair)s to \a intersect (::gfsmStateId)s,
 *                may be NULL to use a temporary mapping.
 *  \returns \a intersect.
 */
gfsmAutomaton *gfsm_automaton_intersect_full(gfsmAutomaton *fsm1,
					     gfsmAutomaton *fsm2,
					     gfsmAutomaton *intersect,
					     gfsmStatePairEnum *spenum);

/** Guts for gfsm_automaton_intersect()
 *  \returns (new) ::gfsmStateId for \a sp
 */
gfsmStateId gfsm_automaton_intersect_visit_(gfsmStatePair  sp,
					    gfsmAutomaton *fsm1,
					    gfsmAutomaton *fsm2,
					    gfsmAutomaton *fsm,
					    gfsmStatePairEnum *spenum,
					    gfsmComposeFlags   flags);
//@}

//------------------------------
///\name gfsmMinimize.c: Minimization
//@{

/** Minimize an automaton, treating transducers as pair-acceptors.
 *  Just a wrapper for \c gfsm_automaton_minimimize_full(fsm,TRUE).
 *
 *  \note pseudo-destructive on \a fsm
 *
 *  \param fsm Automaton to minimize.
 *  \returns minimized \a fsm
 */
gfsmAutomaton *gfsm_automaton_minimize(gfsmAutomaton *fsm);

/** Quasi-minimization using Brzozowski (1963) algorithm, with optional epsilon-removal. 
 * \note pseudo-destructive on \a fsm
 *
 * \sa J. A. Brzozowski, "Canonical regular expressions and minimal state graphs for definite
 *  events", In: <em>Proc. Sympos. Math. Theory of Automata (New York, 1962)</em>,
 *  Polytechnic Press of Polytechnic Inst. of Brooklyn, Brooklyn, NY, pp. 529–561,
 *  URL: http://www.ams.org/mathscinet-getitem?mr=0175719
 *
 * \param fsm Automaton to minimize.
 * \param rmeps Whether to include epsilon-removal (true minimization) or not (quasi-minimization)
 * \returns (quasi-)minimized \a fsm
 */
gfsmAutomaton *gfsm_automaton_minimize_full(gfsmAutomaton *fsm, gboolean rmeps);

/** Heuristically compact an automaton.
 *  Really just a wrapper for \c gfsm_automaton_compact_full(fsm,TRUE).
 *
 *  \note pseudo-destructive on \a fsm
 *
 *  \param fsm Automaton to compact.
 *  \returns compacted \a fsm
 */
gfsmAutomaton *gfsm_automaton_compact(gfsmAutomaton *fsm);

/** Heuristic compaction like <code>gfsmencode fsm KEY | gfsmminimize | gfsmdecode - KEY</code>
 * \note destructively alters \a fsm
 *
 * \param fsm automaton to compact
 * \param rmeps Whether to include epsilon-removal in encoded-acceptor minimization step
 * \returns compacted \a fsm
 */
gfsmAutomaton *gfsm_automaton_compact_full(gfsmAutomaton *fsm, gboolean rmeps);

//@}


//------------------------------
///\name gfsmProject.c: Projection & Inversion
//@{

/** Invert upper and lower labels of an automaton.
 *  \note Destructively alters \a fsm.
 *
 *  \param fsm Transducer
 *  \returns \a fsm
 */
gfsmAutomaton *gfsm_automaton_invert(gfsmAutomaton *fsm);

/** Project one "side" (lower or upper) of \a fsm
 *  \note Destructively alters \a fsm
 *
 *  \param fsm Transducer
 *  \param which Which label side to project.
 *    \arg gfsmLSLower project lower side
 *    \arg gfsmLSUpper project upper side (default)
 *  \returns modified \a fsm
 */
gfsmAutomaton *gfsm_automaton_project(gfsmAutomaton *fsm, gfsmLabelSide which);

//@}

//------------------------------
///\name gfsmProduct.c: Cartesian Product
//@{
/** Compute Cartesian product of acceptors \a fsm1 and \a fsm2.
 *  \note Destructively alters \a fsm1.
 *
 *  \param fsm1 Acceptor (lower)
 *  \param fsm2 Acceptor (upper)
 *  \returns \a fsm1 (transducer)
 *
 *  \sa gfsm_automaton_product2()
 */
gfsmAutomaton *gfsm_automaton_product(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2);

/** Compute Cartesian product of acceptors \a fsm1 and \a fsm2.
 *  \note Destructively alters \a fsm1 \b and \a fsm2.
 *
 *  \param fsm1 Acceptor (lower)
 *  \param fsm2 Acceptor (upper)
 *  \returns \a fsm1 
 */
gfsmAutomaton *gfsm_automaton_product2(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2);

/* Backwards-compatible alias for gfsm_automaton_product2() [DISABLED] */
//#define _gfsm_automaton_product gfsm_automaton_product2

//@}


//------------------------------
///\name gfsmReplace.c: Replacement
//@{
/** Replace label-pair \a (lo,hi) with \a fsm2 in \a fsm1 .
 *  \note Destructively alters \a fsm.
 *
 *  \param fsm1 Automaton
 *  \param lo   lower label or gfsmNoLabel to ignore lower labels
 *  \param hi   upper label or gfsmNoLabel to ignore upper labels
 *  \returns modified \a fsm1
 */
gfsmAutomaton *gfsm_automaton_replace(gfsmAutomaton *fsm1, gfsmLabelVal lo, gfsmLabelVal hi, gfsmAutomaton *fsm2);

/** Insert automaton \a fsm2 into \a fsm1 between states \a q1from and \a q1to with weight \a w.
 *  \note Destructively alters \a fsm1.
 *
 *  \param fsm1 Automaton into which \a fsm2 is inserted
 *  \param fsm2 Automaton to be inserted
 *  \param q1from Source state for inserted automaton in \a fsm1
 *  \param q1to   Final state for inserted automaton in \a fsm1
 *  \param w    Weight to add to final arcs for translated \a fsm2 in \a fsm1
 *  \returns modified \a fsm1
 */
gfsmAutomaton *gfsm_automaton_insert_automaton(gfsmAutomaton *fsm1,
					       gfsmStateId    q1from,
					       gfsmStateId    q1to,
					       gfsmAutomaton *fsm2,
					       gfsmWeight     w);

//@}


//------------------------------
///\name gfsmReverse.c: Reversal
//@{
/** Reverse an automaton \a fsm.
 *  \note Destructively alters \a fsm.
 *
 *  \param fsm Automaton
 *  \returns \a fsm
 */
gfsmAutomaton *gfsm_automaton_reverse(gfsmAutomaton *fsm);

//@}

//------------------------------
///\name gfsmRmEpsilon.c: Epsilon Removal
//@{

/** Remove epsilon arcs from \a fsm.
 * - Destructively alters \a fsm.
 * - Implementation of incremental algorithm from Hanneforth & de la Higueara (2010).
 *
 * \warning negative-cost epsilon cycles in \a fsm will cause an infinite loop!
 *
 * \sa T. Hanneforth & C. de la Higuera (2010),
 *  "ε-Removal by Loop Reduction for Finite-state Automata over Complete Semirings."
 *  In T. Hanneforth and G. Fanselow, editors, <em>Language and Logos: Studies in
 *  Theoretical and Computational Linguistics</em>, volume 72 of Studia grammatica.
 *  Akademie Verlag, Berlin.  ISBN 978-3-05-004931-1.
 *
 * \param fsm Automaton
 * \returns \a fsm
 */
gfsmAutomaton *gfsm_automaton_rmepsilon(gfsmAutomaton *fsm);

//@}

//------------------------------
///\name gfsmSigma.c: Alphabet Recognizer
//@{
/**
 * Make \a fsm an identity-transducer for alphabet \a abet
 * \note Destructively alters \a fsm.
 *
 * \param fsm Automaton
 * \returns \a fsm
 */
gfsmAutomaton *gfsm_automaton_sigma(gfsmAutomaton *fsm, gfsmAlphabet *abet);
//@}

//------------------------------
///\name gfsmUnion.c: Union
//@{
/** Add the language or relation of \a fsm2 to \a fsm1.
 *  \note Destructively alters \a fsm1
 *
 *  \param fsm1 Automaton
 *  \param fsm2 Automaton
 *  \returns \a fsm1
 */
gfsmAutomaton *gfsm_automaton_union(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2);
//@}

/** \file gfsmAlgebra.h
 *  \todo bestpath() ?
 *  \todo encode() ?
 *  \todo equiv() ?
 *  \todo deterministic union ?
 *  \todo xerox-style property-savvy lookup()?
 */

#endif /* _GFSM_ALGEBRA_H */
