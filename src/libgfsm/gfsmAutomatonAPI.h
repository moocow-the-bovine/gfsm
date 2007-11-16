
/*=============================================================================*\
 * File: gfsmAutomatonAPI.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: generic automata
 *
 * Copyright (c) 2007 Bryan Jurish.
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

/** \file gfsmAutomatonAPI.h
 *  \brief Generic automaton API
 */

#ifndef _GFSM_AUTOMATON_API_H
#define _GFSM_AUTOMATON_API_H

#include <gfsmAutomatonTypes.h>
#include <gfsmAlphabet.h>

/*======================================================================
 * Constants
 */
///\name Constants & Defaults
//@{

/** Default initial automaton type */
extern const gfsmAutomatonClass gfsmAutomatonDefaultClass;

/** Default initial automaton flags */
extern const gfsmAutomatonFlags gfsmAutomatonDefaultFlags;

/** Default semiring for automaton arc weights */
extern const gfsmSRType gfsmAutomatonDefaultSRType;

/** Default initial automaton size (number of states) */
extern const gfsmStateId gfsmAutomatonDefaultNStates;

/** Default initial automaton size (number of arcs) */
extern const gfsmArcId gfsmAutomatonDefaultNArcs;

/** Class-type to symbolic name lookup table */
extern const char *gfsmAutomatonClassNames[gfsmACNClasses];

/** Get symbolic class name for a class type, or NULL if \a itype is not a built-in type */
const char *gfsm_class_name(gfsmAutomatonClass itype);

//@}

/*======================================================================
 * API: constructors, etc.
 */
///\name API: Constructors, etc.
//@{

/** Allocate and return a new ::gfsmAutomaton, using default flags, semiring and implementation type
 *  \returns newly allocated and initialized ::gfsmAutomaton* of class ::gfsmAutomatonDefaultClass
 */
static inline
gfsmAutomaton *gfsm_automaton_new();
#define gfsm_automaton_new() \
  gfsm_automaton_classed_new_full(gfsmAutomatonDefaultClass,   \
                                  gfsmAutomatonDefaultFlags,   \
                                  gfsmAutomatonDefaultSRType,  \
                                  gfsmAutomatonDefaultNStates, \
                                  gfsmAutomatonDefaultNArcs)

/** Allocate and return a new ::gfsmAutomaton of class \a itype using default flags and semiring
 *  \param itype implementation class of automaton to create
 *  \returns newly allocated ::gfsmAutomaton* of class \a itype
 *   \li if \a itype is a builtin type, the returned automaton will already be initialized
 */
static inline
gfsmAutomaton *gfsm_automaton_classed_new(gfsmAutomatonClass itype);


/** Allocate and return a new ::gfsmAutomaton
 *  \param itype    implementation class
 *  \param flags    automaton flags
 *  \param srtype   semiring type
 *  \param n_states number of states to pre-allocate (passed to \a impl, may be ignored)
 *  \param n_arcs   number of arcs to pre-allocate (passed to \a impl, may be ignored)
 *  \returns newly allocated and intialized automaton
 */
static inline
gfsmAutomaton *gfsm_automaton_classed_new_full(gfsmAutomatonClass itype,
					       gfsmAutomatonFlags flags,
					       gfsmSRType         srtype,
					       guint              n_states,
					       guint              n_arcs
					       );

/** Create a new gfsmAutomaton by cloning an existing one
 *  \param src automaton to be cloned
 *  \returns new exact copy of \a src
 */
static inline
gfsmAutomaton *gfsm_automaton_clone(gfsmAutomaton *src);

/** Assign non-structural contents (flags, semiring) of \a src to \a dst,
 *  without altering \a dst's implementation class or structure.
 *  \param dst target automaton
 *  \param src source automaton
 *  \returns modified \a dst
 *  \note Earlier versions of this function also set the root state of \a dst
 *        to that of \a src, but this is no longer the case.
 */
static inline
gfsmAutomaton *gfsm_automaton_copy_shallow(gfsmAutomaton *dst, gfsmAutomaton *src);

/** Assign the contents of \a src to \a dst,
 *  without altering \a dst's implementation class.
 *  \param dst target automaton
 *  \param src source automaton
 *  \returns modified \a dst
 */
static inline
gfsmAutomaton *gfsm_automaton_copy(gfsmAutomaton *dst, gfsmAutomaton *src);

/** Create a new ::gfsmAutomaton of the default class whose non-structural contents match those of \a fsm.
 *  \param fsm source automaton
 *  \returns automaton of default class whose non-structural fields match those of \a fsm
 */
static inline
gfsmAutomaton *gfsm_automaton_shadow(gfsmAutomaton *fsm);

/** Create a new ::gfsmAutomaton whose non-structural contents match those of \a fsm, specifying class
 *  \param fsm   source automaton
 *  \param itype implementation type for destination automaton
 *  \returns automaton of class \a itype whose non-structural fields match those of \a fsm
 */
static inline
gfsmAutomaton *gfsm_automaton_shadow_classed(gfsmAutomaton *fsm, gfsmAutomatonClass itype);

/** Swap the contents of ::gfsmAutomaton objects \a fsm1 and \a fsm2, including implementations. */
static inline
void gfsm_automaton_swap(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2);

/** Clear a ::gfsmAutomaton */
static inline
void gfsm_automaton_clear(gfsmAutomaton *fsm);

/** Destroy a ::gfsmAutomaton and its implementation */
static inline
void gfsm_automaton_free(gfsmAutomaton *fsm);

/** Destroy an automaton, optionally retaining its implementation.
 *  \param fsm automaton to destroy
 *  \param free_impl if true, implementation will be freed
 *  \returns
 *    fsm->impl if \a free_impl is FALSE, otherwise undefined.
 */
static inline
gfsmAutomatonImpl gfsm_automaton_free_full(gfsmAutomaton *fsm, gboolean free_impl);

//@}

/*======================================================================
 * API: Automaton Semiring
 */
/// \name API: Automaton Semiring
//@{

/** Get pointer to the semiring associated with this automaton */
static inline
gfsmSemiring *gfsm_automaton_get_semiring(gfsmAutomaton *fsm);

/** Set the semiring associated with this automaton
 *  \param fsm automaton to modify
 *  \param sr  semiring to be copied into \a fsm->sr
 *  \returns new semiring for \a fsm
 *  \warning older versions of this method returned \a sr itself!
 */
static inline
gfsmSemiring *gfsm_automaton_set_semiring(gfsmAutomaton *fsm, gfsmSemiring *sr);

/** Set the semiring associated with this automaton by semiring-type */
void gfsm_automaton_set_semiring_type(gfsmAutomaton *fsm, gfsmSRType srtype);

//@}

/*======================================================================
 * API: Automaton Structure
 */
/** \name API: Automaton Structure
 */
//@{

/** Reserve space for at least \a n_states states (may do nothing)
 *  \param fsm automaton to modify
 *  \param n_states number of states to reserve, if supported by implementation
 */
static inline
void gfsm_automaton_reserve_states(gfsmAutomaton *fsm, gfsmStateId n_states);

/** Reserve space for at least \a n_arcs arcs (may do nothing)
 *  \param fsm automaton to modify
 *  \param n_arcs number of arcs to reserve, if supported by implementation
 */
static inline
void gfsm_automaton_reserve_arcs(gfsmAutomaton *fsm, gfsmArcId n_arcs);

/** Get number of states */
static inline
gfsmStateId gfsm_automaton_n_states(gfsmAutomaton *fsm);

/** Get number of final states */
static inline
gfsmStateId gfsm_automaton_n_final_states(gfsmAutomaton *fsm);

/** Get total number of arcs, may call gfsm_automaton_n_arcs_full() */
static inline
gfsmArcId gfsm_automaton_n_arcs(gfsmAutomaton *fsm);

/** Get ID of root node, or ::gfsmNoState if undefined */
static inline
gfsmStateId gfsm_automaton_get_root(gfsmAutomaton *fsm);

/** Set ID of root node, creating state if necessary
 *  \warning may not be supported by all implementations
 */
gfsmStateId gfsm_automaton_set_root(gfsmAutomaton *fsm, gfsmStateId new_root_id);

//@}


/*======================================================================
 * API: Automaton Properties
 */
/** \name API: Automaton Properties
 *  Currently quite sketchy; implementation-dependent dispatch on these methods ought to be allowed,
 *  as should better tracking and checking of automaton flags.
 */
//@{

/** True iff automaton is a transducer \todo improve tracking & checking */
#define gfsm_automaton_is_transducer(fsm) ((fsm)->flags.is_transducer)

/** True iff automaton is weighted \todo improve tracking & checking */
#define gfsm_automaton_is_weighted(fsm) ((fsm)->flags.is_weighted)

/** Get current automaton arc-sort mode \todo improve tracking & checking */
#define gfsm_automaton_sortmode(fsm) ((gfsmArcSortMode)((fsm)->flags.sort_mode))

/** Get verbose summary arc information
 *  \param fsm automaton to examine
 *  \param n_lo_epsilon output paramater for number of arcs with lower label ::gfsmEpsilon
 *  \param n_hi_epsilon output paramater for number of arcs with upper label ::gfsmEpsilon
 *  \param n_both_epsilon output parameter for number of arcs with both lower and upper labels ::gfsmEpsilon
 *  \returns total number of arcs
 *
 *  \todo allow implementation dispatch
 */
guint gfsm_automaton_n_arcs_full(gfsmAutomaton *fsm,
				 guint *n_lo_epsilon,
				 guint *n_hi_epsilon,
				 guint *n_both_epsilon);

/** Low-level utility function for gfsm_automaton_is_cyclic() */
gboolean gfsm_automaton_is_cyclic_state(gfsmAutomaton  *fsm,
					gfsmStateId  qid,
					gfsmBitVector  *visited,
					gfsmBitVector  *completed);

/** Test whether automaton is cyclic 
 *  \todo add flag & tracking, allow implementation dispatch
 */
gboolean gfsm_automaton_is_cyclic(gfsmAutomaton *fsm);

/** Test whether automaton is acyclic */
#define gfsm_automaton_is_acyclic(fsm) (!gfsm_automaton_is_cyclic(fsm))

/** Extract automaton-internal labels from \a fsm to \a alph.  If \a alph is NULL,
 *  a new default alphabet will be created and returned (you will need to
 *  free it yourself).
 *
 *  The alphabet should be able to match literal label values to themselved
 *  (i.e. don't pass a string alphabet)
 *
 *  \param fsm   automaton whose alphabet is to be extracted
 *  \param which indicates label side to be extracted
 *  \param alph  output alphabet, or NULL to return a new alphabet
 *  \returns \a alph or a newly created and populated alphabet
 */
gfsmAlphabet *gfsm_automaton_get_alphabet(gfsmAutomaton *fsm,
					  gfsmLabelSide  which,
					  gfsmAlphabet  *alph);

//@}


/*======================================================================
 * API: Automaton States
 */
/** \name API: Automaton States
 */
//@{

/** Add a new state, specifying qid.  If \a qid is ::gfsmNoState,
 *  first available state qid will be selected.
 *  \warning not supported by all implementations!
 *
 *  \param fsm fsm to modify
 *  \param qid  identifier of the new state, or ::gfsmNoState
 *  \returns ID of the new state, or ::gfsmNoState on failure
 */
static inline
gfsmStateId gfsm_automaton_add_state_full(gfsmAutomaton *fsm, gfsmStateId qid);

/** Attempt to ensures that state \a qid exists.
 *  Really Just an alias for gfsm_automaton_add_state_full(fsm,qid).
 */
#define gfsm_automaton_ensure_state(fsm,qid) \
   gfsm_automaton_add_state_full((fsm),(qid))

/** Add a new state to \a fsm.
 *  Really Just an alias for gfsm_automaton_add_state_full(fsm,::gfsmNoState).
 */
#define gfsm_automaton_add_state(fsm) \
   gfsm_automaton_add_state_full((fsm),gfsmNoState)

/** Check whether automaton \a fsm has a valid state with ID \a qid.
 *  \param fsm automaton to examine
 *  \param qid ID of state to examine
 *  \returns TRUE if \a fsm has a state with ID \a id, FALSE otherwise.
 */
static inline
gboolean gfsm_automaton_has_state(gfsmAutomaton *fsm, gfsmStateId qid);

/** Remove the state with ID \a qid, if any.
 *  \warning not supported by all implementations.
 *  \warning
 *     May have no effect on incoming arcs for state \a qid,
 *     although any outgoing arcs should be removed and freed.
 *  \param fsm automaton from which to remove a state
 *  \param qid ID of the state to be removed
 */
static inline
void gfsm_automaton_remove_state(gfsmAutomaton *fsm, gfsmStateId qid);

/** Lookup final weight for state with ID \a qid in automaton \a fsm.
 *  \param fsm automaton to examine
 *  \param qid ID of state to examine
 *  \param  wp output parameter for final weight
 *  \returns
 *     TRUE if state \a id is final, FALSE otherwise,
 */
static inline
gboolean gfsm_automaton_lookup_final(gfsmAutomaton *fsm, gfsmStateId qid, gfsmWeight *wp);

/** Check whether the state with ID \a qid is final in \a fsm.
 *  Really just a wrapper for gfsm_automaton_lookup_final().
 *  \param fsm automaton to examine
 *  \param qid ID of state to check for finality
 *  \returns TRUE if \a qid is final in \a fsm, FALSE otherwise.
 */
static inline
gboolean gfsm_automaton_is_final_state(gfsmAutomaton *fsm, gfsmStateId qid);

/** Alias for gfsm_automaton_is_final_state() */
#define gfsm_automaton_state_is_final(fsm,qid) \
  gfsm_automaton_is_final_state((fsm),(qid))

/** Get final weight for state with ID \a qid in automaton \a fsm.
 *  Really just a wrapper for gfsm_automaton_lookup_final().
 *  \param fsm automaton to examine
 *  \param qid ID of state to examine
 *  \returns final weight if \a qid is final, otherwise \a fsm->sr->zero.
 */
static inline
gfsmWeight gfsm_automaton_get_final_weight(gfsmAutomaton *fsm, gfsmStateId id);

/** Set final-weight and/or final-states membership flag for state with ID \a qid in \a fsm.
 * \param fsm automaton to modify
 * \param qid ID of state to modified
 * \param is_final whether state should be considered final
 * \param final_weight
 *    If \a is_final is true, final weight for state.  Otherwise
 *    final weight should implicitly be set to <tt>(fsm)->sr->zero</tt>
 */
static inline
void gfsm_automaton_set_final_state_full(gfsmAutomaton *fsm,
					 gfsmStateId qid,
					 gboolean       is_final,
					 gfsmWeight     final_weight);

/** Get number of outgoing arcs for state with ID \a qid in \a fsm
 *  \param fsm automaton to examine
 *  \param qid ID of state to examine
 *  \returns output degree of \a qid in \a fsm
 */
static inline
gfsmArcId gfsm_automaton_out_degree(gfsmAutomaton *fsm, gfsmStateId qid);

/** Renumber states of an automaton \a fsm.
 *  Removes "gaps" in state IDs and sets root ID to 0 (zero).
 *  \warning may not be supported by all implementation classes.
 *  \param fsm fsm to be modified.
 */
void gfsm_automaton_renumber_states(gfsmAutomaton *fsm);

/** Renumber states of an automaton using a user-specified renumbering scheme.
 *  Destructively alters \a fsm.
 *  \warning may not be supported by all implementation classes.
 *  \param fsm
 *    automaton whose states are to be renumbered
 *  \param old2new
 *    ::GArray of ::gfsmStateId s.t. \a qid_new=old2new[qid_old], newid may be ::gfsmNoState to ignore
 *  \param n_new_states
 *     number of new states, or 0 (zero) to auto-compute
 */
static inline
void gfsm_automaton_renumber_states_full(gfsmAutomaton *fsm, GArray *old2new, gfsmStateId n_new_states);

/** Open and return a pointer to a ::gfsmState struct for ::gfsmStateId \a qid in \a fsm.
 *  \warning
 *   The pointer returned should be closed with gfsm_automaton_close_state(), or
 *   else some implementations may leak memory.
 *  \param fsm automaton from which to draw state information
 *  \param qid ID of state to be opened
 *  \returns ::gfsmState* for state with ID \a qid in \a fsm, or NULL if no such state exists.
 *  \deprecated
 *    prefer gfsm_automaton_has_state(), gfsm_automaton_state_is_final(), gfsm_arciter_open() etc.
 *
 */
static inline
gfsmState *gfsm_automaton_open_state(gfsmAutomaton *fsm, gfsmStateId qid);

/** Close a pointer to a ::gfsmState opened with gfsm_automaton_open_state() for \a fsm.
 *  \param fsm automaton from which state was opened.
 *  \param qp  pointer as returned by gfsm_automaton_open_state()
 */
static inline
void gfsm_automaton_close_state(gfsmAutomaton *fsm, gfsmState *qp);

//@}



/*======================================================================
 * API: Automaton Arcs
 */
/// \name API: Automaton Arcs
//@{

/** Add an arc from state with ID \a qid1 to state with ID \a qid2
 *  on labels (\a lo,\a hi) with weight \a w.
 *  Missing states should be implicitly created.
 *  \warning may not be supported by all implementation classes
 *  \param fsm Automaton to modify
 *  \param qid1 ID of source state
 *  \param qid2 ID of target state
 *  \param lo   Lower label
 *  \param hi   Upper label
 *  \param w    Arc weight
 */
static inline
void gfsm_automaton_add_arc(gfsmAutomaton *fsm,
			    gfsmStateId qid1,
			    gfsmStateId qid2,
			    gfsmLabelVal   lo,
			    gfsmLabelVal   hi,
			    gfsmWeight     w);

/** Sort all arcs in the automaton by one of the built-in comparison functions
 *  \param fsm  Automaton to modify
 *  \param mode Specifies built-in arc comparison function
 *  \returns modified \a fsm
 */
static inline
gfsmAutomaton *gfsm_automaton_arcsort(gfsmAutomaton *fsm, gfsmArcSortMode mode);

/** Sort all arcs in the automaton by a user-specified comparison function.
 *  \param fsm
 *    Automaton to modify
 *  \param cmpfunc
 *    3-way comparison function, called as \a (*cmpfunc)(gfsmArc *a1p, gfsmArc *a2p, gpointer data)
 *    to compare arcs a1p and a2p.
 *  \param data
 *    User data for \a cmpfunc
 *  \returns
 *    modified \a fsm
 */
static inline
gfsmAutomaton *gfsm_automaton_arcsort_full(gfsmAutomaton *fsm, GCompareDataFunc cmpfunc, gpointer data);

/** Alias for gfsm_automaton_arcsort_full() */
#define gfsm_automaton_arcsort_with_data(fsm,cmpfunc,data) \
  gfsm_automaton_arcsort_full((fsm),(cmpfunc),(data))

//@}

/*======================================================================
 * API: Arc Iterators
 */
///\name API: Arc Iterators
//@{

/** Open a ::gfsmArcIter \a aip for the outgoing arcs from state with
 *  ID \a qid in the automaton \a fsm.
 *  \param aip Pointer to the ::gfsmArcIter to be opened; assumed to be already allocated
 *  \param fsm Automaton containing the state whose outgoing arcs are to be opened
 *  \param qid ID of the state whose outgoing arcs are to be opened
 *
 *  \note
 *   \li Arc iterators may be silently invalidated by destructive operations
 *   \li The arc iterator should be closed with gfsm_arciter_close() when
 *       it is no longer needed.
 *   \li Caller is responsible for allocation and freeing of \a *aip.
 *
 *  \warning
 *   No default implementation (aborts with an error message)
 */
static inline
void gfsm_arciter_open(gfsmArcIter *aip, gfsmAutomaton *fsm, gfsmStateId qid);

/** Close a ::gfsmArcIter \a aip if already opened.
 *  \param aip The ::gfsmArcIter to be closed.
 *  \note
 *   \li Really bad things may happen if you change an automaton's class
 *       between calling gfsm_arciter_open() and gfsm_arciter_close().
 *   \li If multiple copies of a ::gfsmArcIter exist, only one needs to
 *       be closed.
 *   \li Default implementation just does: \code
aip->fsm = NULL;
aip->qid = gfsmNoState; \endcode
 */
static inline
void gfsm_arciter_close(gfsmArcIter *aip);

/** Check validity of a ::gfsmArcIter* \a aip.
 *  \param aip The ::gfsmArcIter whose status is to be queried.
 *  \returns a true value if \a aip is considered valid, FALSE otherwise.
 *  \warning
 *   No default implementation (always returns FALSE).
 */
static inline
gboolean gfsm_arciter_ok(gfsmArcIter *aip);

/** Position the ::gfsmArcIter \a aip to the next available outgoing arc
 *  for which it was opened.
 *  \param aip The ::gfsmArcIter to be incremented.
 *  \warning
 *   No default implementation (aborts with an error message)
 */
static inline
void gfsm_arciter_next(gfsmArcIter *aip);

/** Reset an arc iterator to the first outgoing arc for which
 *  it was initially opened.
 *  \param aip the ::gfsmArcIter to be reset.
 *  \note
 *    Default implementation is just:
 *    \code gfsm_arciter_open(aip,aip->fsm,aip->qid); \endcode
 */
static inline
void gfsm_arciter_reset(gfsmArcIter *aip);

/** Copy positional data from \a src to \a dst.
 * \param src The ::gfsmArcIter from which to copy positional data
 * \param dst The ::gfsmArcIter to which positional data is to be written
 * \note
 *  \li Only the position pointed to should be copied by this method,
 *      and not the underlying data.
 *  \li If you use this method to copy ::gfsmArcIter positions,
 *      you should subsequently call gfsm_arciter_close() on only
 *      \e one of them!
 *  \li Default implementation is just:
 *      \code (*dst)=(*src); \endcode
 */
static inline
void gfsm_arciter_copy(gfsmArcIter *dst, gfsmArcIter *src);

/* Create and return a new copy of a ::gfsmArcIter.
 * \param src The ::gfsmArcIter whose positional data is to be duplicated.
 * \note
 *  \li Only the position pointed to should be copied by this method,
 *      and not the underlying data.
 *  \li If you use this method to copy ::gfsmArcIter positions,
 *      you should subsequently call gfsm_arciter_close() on only
 *      \e one of them!
 *  \li Default implementation is just
 *    \code gfsm_mem_dup_n(src,sizeof(gfsmArcIter)) \endcode
 */
static inline
gfsmArcIter *gfsm_arciter_clone(const gfsmArcIter *src);

/** Get current arc associated with a :gfsmArcIter, or NULL if none is available.
 *  \param aip The ::gfsmArcIter to be 'dereferenced'.
 *  \returns A pointer to the current ::gfsmArc 'pointed to' by \a aip, or NULL if
 *           no more arcs are available.
 *  \note
 *   \li A ::gfsmAutomaton implementation is free to return
 *       a dynamically generated arc here: there is no general
 *       guarantee that modifications to the ::gfsmArc returned by this
 *       function will be propagated to the underlying ::gfsmAutomaton.
 *   \li The ::gfsmACOld, ::gfsmACBasic, ::gfsmACArcTable, and ::gfsmACFullTable
 *       classes do all support direct in-place modification of the arcs returned
 *       by gfsm_arciter_arc()
 *
 *  \warning
 *    No default implementation (always returns NULL)
 */
static inline
gfsmArc *gfsm_arciter_arc(gfsmArcIter *aip);

/** Remove the arc referred to by a ::gfsmArcIter \a aip from the associated ::gfsmAutomaton,
 *  and position \aip to the next available arc, if any.
 *  \param aip The ::gfsmArcIter whose 'current' arc is to be removed.
 *  \warning
 *    No default implementation (aborts with an error message)
 */
static inline
void gfsm_arciter_remove(gfsmArcIter *aip);

/** Position an arc-iterator to the current or next arc with lower label \a lo and upper label \a hi.
 *  If either \a lo or \a hi is ::gfsmNoLabel, the corresponding arc label(s) will be ignored.
 *  \param aip The ::gfsmArcIter to reposition
 *  \param lo  Lower arc label to seek, or ::gfsmNoLabel to ignore lower labels
 *  \param hi  Upper arc label to seek, or ::gfsmNoLabel to ignore upper labels
 *  \note
 *    Default implementation wraps gfsm_arciter_ok(), gfsm_arciter_next() and gfsm_arciter_arc()
 *    in a linear search from the current position.
 */
static inline
void gfsm_arciter_seek_both(gfsmArcIter *aip, gfsmLabelVal lo, gfsmLabelVal hi);

/** Position an arc-iterator to the current or next arc with lower label \a lo.
 *  \param aip The ::gfsmArcIter to reposition
 *  \param lo  Lower arc label to seek
 *  \note
 *    Default implementation wraps gfsm_arciter_ok(), gfsm_arciter_next() and gfsm_arciter_arc()
 *    in a linear search from the current position.
 */
static inline
void gfsm_arciter_seek_lower(gfsmArcIter *aip, gfsmLabelVal lo);

/** Position an arc-iterator to the current or next arc with upper label \a hi.
 *  \param aip The ::gfsmArcIter to reposition
 *  \param lo  Upper arc label to seek
 *  \note
 *    Default implementation wraps gfsm_arciter_ok(), gfsm_arciter_next() and gfsm_arciter_arc()
 *    in a linear search from the current position.
 */
static inline
void gfsm_arciter_seek_upper(gfsmArcIter *aip, gfsmLabelVal hi);

//@}


/*======================================================================
 * API: Automaton I/O
 */
///\name API: Automaton I/O
//@{

/** Populate header information struct for binary automaton storage.
 *  \param fsm[in]     Automaton to be stored
 *  \param hdr[in,out] Pointer to a ::gfsmAutomatonHeader struct to be populated
 *   \li Only version and automaton structural information
 *       needs to be set in hdr; is taken care of elsewhere.
 *  \note
 *    Default implementation should be more or less sensible.
 */
static inline
void gfsm_automaton_get_bin_header(gfsmAutomaton *fsm, gfsmAutomatonHeader *hdr);

/** Store a ::gfsmAutomaton \a fsm in binary form to a ::gfsmIOHandle* \a ioh.
 *  \param fsm  Automaton to store.
 *  \param ioh  ::gfsmIOHandle to which binary automaton data should be written.
 *  \param errp[out] if an error occurs, \a *errp will hold a diagnostic message.
 *  \returns a true value iff save was successful.
 *  \note
 *    No default implementation (always returns FALSE)
 */
static inline
gboolean gfsm_automaton_save_bin_handle(gfsmAutomaton *fsm, gfsmIOHandle *ioh, gfsmError **errp);

/** Load an automaton from a named binary file (implicitly clear()s \a fsm)
 *  \param fsm  Automaton to load, should already be instantiated.
 *  \param hdr  Pointer to pre-loaded header information
 *  \param ioh  ::gfsmIOHandle from which to read binary automaton data
 *  \param errp[out] if an error occurs, \a *errp will hold a diagnostic message.
 *  \returns a true value iff load was successful.
 *  \note
 *   No default implementation (always returns FALSE)
 */
static inline
gboolean gfsm_automaton_load_bin_handle_old(gfsmAutomaton       *fsm,
					    gfsmAutomatonHeader *hdr,
					    gfsmIOHandle        *ioh,
					    gfsmError          **errp);
//@}

#endif /* _GFSM_AUTOMATON_API_H */
