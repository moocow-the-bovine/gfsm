
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

#include <gfsmState.h>
#include <gfsmAlphabet.h>

/*======================================================================
 * Types
 */
///\name Types
//@{

/// Known automaton implementation classes
/** Used to resolve \a impl.data for a ::gfsmAutomatonImpl \a impl */
typedef enum {
  gfsmACUnknown,            /**< unknown class; should never happen in practice */
  gfsmACOldBasic,           /**< backwards-compatible implementation, see ::gfsmOldBasicImpl */
  gfsmACBasic,              /**< basic implementation; see ::gfsmBasicImpl \b TODO */
  gfsmACArcTable,           /**< (read-only) arc table implementation; see ::gfsmArcTableImpl \b TODO */
  gfsmACFullTable,          /**< (read-only) full table implementation; see ::gfsmFullTableImpl \b TODO */
  gfsmACVirtual,            /**< user-defined full-virtual implementation; see ::gfsmVirtualImpl \b TODO */
  gfsmACNClasses            /**< not really an implementation class at all */
} gfsmAutomatonClass;

/// Automaton implementation data union type, for comfortable typecasting
typedef union {
  gpointer                     p;   /**< Generic (void*) */
  struct gfsmOldBasicImpl_   *ob;   /**< backwards-compatible implementation, see ::gfsmOldBasicImpl */
  struct gfsmBasicImpl_       *b;   /**< basic implementation; see ::gfsmBasicImpl */
  struct gfsmArcTableImpl_   *at;   /**< (read-only) arc table implementation; see ::gfsmArcTableImpl */
  struct gfsmFullTableImpl_  *ft;   /**< (read-only) full table implementation; see ::gfsmFullTableImpl */
  struct gfsmVirtualImpl_     *v;   /**< user-defined full-virtual implementation; see ::gfsmVirtualImpl */
} gfsmAutomatonImpl;

/// Automaton status flags
/** \todo multiply binary property flags out: add 'xKnown' flag for each property 'x' */
typedef struct {
  guint32 is_transducer     : 1;       /**< whether this automaton is a transducer */
  guint32 is_weighted       : 1;       /**< whether this automaton is weighted */
  guint32 sort_mode         : 4;       /**< sort-mode (cast to gfsmArcSortMode) */
  guint32 is_deterministic  : 1;       /**< whether fsm is known to be deterministic */
  guint32 unused            : 25;      /**< reserved */
} gfsmAutomatonFlags;


/// Generic automaton type
typedef struct
{
  gfsmAutomatonFlags   flags;    /**< automaton flags */
  gfsmAutomatonClass   itype;    /**< implementation class */
  gfsmAutomatonImpl    impl;     /**< underlying implementation, cast by \a switch{} over \a itype  */
} gfsmAutomaton;

//@}

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
extern const guint gfsmAutomatonDefaultNStates;

/** Default initial automaton size (number of arcs) */
extern const guint gfsmAutomatonDefaultNArcs;

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

/** Allocate and return a new ::gfsmAutomaton, using default flags, semiring and implementation type */
#define gfsm_automaton_new() \
  gfsm_automaton_classed_new_full(gfsmAutomatonDefaultClass,   \
                                  gfsmAutomatonDefaultFlags,   \
                                  gfsmAutomatonDefaultSRType,  \
                                  gfsmAutomatonDefaultNStates, \
                                  gfsmAutomatonDefaultNArcs)

/** Allocate and return a new ::gfsmAutomaton of class \a itype using default flags and semiring */
#define gfsm_automaton_classed_new(type) \
  gfsm_automaton_classed_new_full(itype,                       \
                                  gfsmAutomatonDefaultFlags,   \
                                  gfsmAutomatonDefaultSRType,  \
                                  gfsmAutomatonDefaultNStates, \
                                  gfsmAutomatonDefaultNArcs)


/** Allocate and return a new ::gfsmAutomaton
 *  \param itype    implementation class
 *  \param flags    automaton flags
 *  \param srtype   semiring type
 *  \param n_states number of states to pre-allocate (passed to \a impl, may be ignored)
 *  \param n_arcs   number of arcs to pre-allocate (passed to \a impl, may be ignored)
 *  \returns new automaton
 */
gfsmAutomaton *gfsm_automaton_classed_new_full(gfsmAutomatonClass itype,
					       gfsmAutomatonFlags flags,
					       gfsmSRType         srtype,
					       guint              n_states,
					       guint              n_arcs
					       );

/** Create a new gfsmAutomaton by cloning an existing one */
gfsmAutomaton *gfsm_automaton_clone(gfsmAutomaton *fsm);

/** Assign non-structural contents (flags, semiring, root) of \a src to \a dst,
 *  without altering \a dst's implementation class or structure.
 *  \param dst target automaton
 *  \param src source automaton
 *  \returns modified \a dst
 */
gfsmAutomaton *gfsm_automaton_copy_shallow(gfsmAutomaton *dst, gfsmAutomaton *src);

/** Assign the contents of \a src to \a dst,
 *  without altering \a dst's implementation class.
 *  \param dst target automaton
 *  \param src source automaton
 *  \returns modified \a dst
 */
gfsmAutomaton *gfsm_automaton_copy(gfsmAutomaton *dst, gfsmAutomaton *src);

/** Create a new ::gfsmAutomaton of the default class whose non-structural contents match those of \a fsm.
 *  \param fsm source automaton
 *  \returns automaton of default class whose non-structural fields match those of \a fsm
 */
#define gfsm_automaton_shadow(fsm) \
  gfsm_automaton_copy_shallow(gfsm_automaton_new(), (fsm))

/** Create a new ::gfsmAutomaton whose non-structural contents match those of \a fsm, specifying class
 *  \param fsm   source automaton
 *  \param itype implementation type for destination automaton
 *  \returns automaton of class \a itype whose non-structural fields match those of \a fsm
 */
#define gfsm_automaton_shadow_classed(fsm,itype) \
  gfsm_automaton_copy_shallow(gfsm_automaton_classed_new(itype), (fsm))

/** Swap the contents of ::gfsmAutomaton objects \a fsm1 and \a fsm2, including implementations. */
void gfsm_automaton_swap(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2);

/** Clear a ::gfsmAutomaton */
ovoid gfsm_automaton_clear(gfsmAutomaton *fsm);

/** Destroy a ::gfsmAutomaton and its implementation */
#define gfsm_automaton_free(fsm) \
   gfsm_automaton_free_full((fsm),TRUE)

/** Destroy an automaton, optionally retaining its implementation.
 *  \param fsm automaton to destroy
 *  \param free_impl if true, implementation will be freed
 *  \returns
 *    fsm->impl if \a free_impl is FALSE, otherwise NULL.
 */
gfsmAutomatonImpl *gfsm_automaton_free_full(gfsmAutomaton *fsm, gboolean free_impl);

//@}

/*======================================================================
 * API: Semiring
 */
/// \name API: Automaton Semiring
//@{

/** Get pointer to the semiring associated with this automaton */
#define gfsm_automaton_get_semiring(fsm) ((fsm)->sr)

/** Set the semiring associated with this automaton
 *  \param fsm automaton to modify
 *  \param sr  semiring to be copied into fsm->sr
 *  \returns new semiring for fsm
 *  \warning older versions of this method returned \a sr itself!
 */
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
void gfsm_automaton_reserve_states(gfsmAutomaton *fsm, gfsmStateId n_states);

/** Backwards-comatible alias for gfsm_automaton_reserve_states() */
#define gfsm_automaton_reserve(fsm,n_states) gfsm_automaton_reserve_states((fsm),(n_states))

/** Reserve space for at least \a n_arcs arcs (may do nothing)
 *  \param fsm automaton to modify
 *  \param n_arcs number of arcs to reserve, if supported by implementation
 */
void gfsm_automaton_reserve_arcs(gfsmAutomaton *fsm, gfsmArcId n_arcs);

/** Get number of states */
guint gfsm_automaton_n_states(gfsmAutomaton *fsm);

/** Get number of final states */
guint gfsm_automaton_n_final_states(gfsmAutomaton *fsm);

/** Get total number of arcs */
guint gfsm_automaton_n_arcs(gfsmAutomaton *fsm);

/** Get ID of root node, or ::gfsmNoState if undefined */
gfsmStateId gfsm_automaton_get_root(gfsmAutomaton *fsm);

/** Set ID of root node, creating state if necessary
 *  \warning may not be supported by all implementations
 */
void gfsm_automaton_set_root(gfsmAutomaton *fsm, gfsmStateId qid);

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
gfsmStateId gfsm_automaton_add_state_full(gfsmAutomaton *fsm, gfsmStateId qid);

/** Attempt to ensures that state \a qid exists.
 *  \warning not supported by all implementations.
 *
 *  \param fsm fsm to modify
 *  \param qid  identifier of the new state, or ::gfsmNoState
 *  \returns ID of the new state, or ::gfsmNoState on failure.
 */
#define gfsm_automaton_ensure_state(fsm,id) \
   gfsm_automaton_add_state_full((fsm),(id))

/** Add a new state to \a fsm.
 *  \warning not supported by all implementations.
 *
 *  \param fsm automaton to be extended
 *  \returns ID of new state, or ::gfsmNoState on failure.
 */
#define gfsm_automaton_add_state(fsm) \
   gfsm_automaton_add_state_full((fsm),gfsmNoState)


/** Get a pointer to the ::gfsmState object with the ID \a qid in \a fsm, if any.
 *  When you are done with the returned pointer, you must release it
 *  with gfsm_automaton_state_close().
 *  \param fsm fsm from which to extract a state
 *  \param qid ID of the state to extract
 *  \returns a ::gfsmState* for \a qid, or NULL if \a qid not a state of \a fsm
 */
gfsmState *gfsm_automaton_open_state(gfsmAutomaton *fsm, gfsmStateId qid);

/** Open a pointer to a (possibly new) state with ID \a qid.
 *  Really just a wrapper for gfsm_automaton_ensure_state() and gfsm_automaton_open_state().
 *  \returns as for gfsm_automaton_open_state()
 */
//gfsmState *gfsm_automaton_open_state_force(gfsmAutomaton *fsm, gfsmStateId qid);
#define gfsm_automaton_open_state_force(fsm,qid) \
  gfsm_automaton_open_state((fsm), gfsm_automaton_ensure_state((fsm),(id)))

/** Close a pointer to a ::gfsmState object which was previously opened by
 *  gfsm_automaton_open_state().
 *  \param fsm fsm for which the state was opened
 *  \param sp  state pointer returned by gfsm_automaton_open_state()
 *  \warning
 *     Bad things may happen if you change the implementation class of \a fsm
 *     while you have open state pointers for it hanging around.
 */
void gfsm_automaton_close_state(gfsmAutomaton *fsm, gfsmState *sp);

/** Check whether automaton \a fsm has a valid state with ID \a qid.
 *  \param fsm automaton to examine
 *  \param qid ID of state to examine
 *  \returns TRUE if \a fsm has a state with ID \a id, FALSE otherwise.
 */
gboolean gfsm_automaton_has_state(gfsmAutomaton *fsm, gfsmStateId qid);

/** Remove the state with ID \a qid, if any.
 *  \warning not supported by all implementations.
 *  \warning
 *     May have no effect on incoming arcs for state \a qid,
 *     although any outgoing arcs should be removed and freed.
 *  \param fsm automaton from which to remove a state
 *  \param qid ID of the state to be removed
 */
void gfsm_automaton_remove_state(gfsmAutomaton *fsm, gfsmStateId qid);


/** Check whether the state with ID \a qid is final in \a fsm.
 *  Really just a wrapper for gfsm_automaton_lookup_final().
 *  \param fsm automaton to examine
 *  \param qid ID of state to check for finality
 *  \returns TRUE if \a qid is final in \a fsm, FALSE otherwise.
 */
gboolean gfsm_automaton_is_final_state(gfsmAutomaton *fsm, gfsmStateId qid);

/** Get final weight for state with ID \a qid in automaton \a fsm.
 *  Really just a wrapper for gfsm_automaton_lookup_final().
 *  \param fsm automaton to examine
 *  \param qid ID of state to examine
 *  \returns final weight if \a qid is final, otherwise \a fsm->sr->zero.
 */
gfsmWeight gfsm_automaton_get_final_weight(gfsmAutomaton *fsm, gfsmStateId id);

/** Lookup final weight for state with ID \a qid in automaton \a fsm.
 *  \param fsm automaton to examine
 *  \param qid ID of state to examine
 *  \param  wp output parameter for final weight
 *  \returns
 *     TRUE if state \a id is final, FALSE otherwise,
 */
gboolean gfsm_automaton_lookup_final(gfsmAutomaton *fsm, gfsmStateId qid, gfsmWeight *wp);

/** Set final-weight and/or final-states membership flag for state with ID \a qid in \a fsm.
 * \param fsm automaton to modify
 * \param qid ID of state to modified
 * \param is_final whether state should be considered final
 * \param final_weight
 *    If \a is_final is true, final weight for state.  Otherwise
 *    final weight should implicitly be set to <tt>(fsm)->sr->zero</tt>
 */
void gfsm_automaton_set_final_state_full(gfsmAutomaton *fsm,
					 gfsmStateId qid,
					 gboolean       is_final,
					 gfsmWeight     final_weight);

/** Get number of outgoing arcs for state with ID \a qid in \a fsm
 *  \param fsm automaton to examine
 *  \param qid ID of state to examine
 *  \returns output degree of \a qid in \a fsm
 */
guint gfsm_automaton_out_degree(gfsmAutomaton *fsm, gfsmStateId qid);

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
void gfsm_automaton_renumber_states_full(gfsmAutomaton *fsm, GArray *old2new, gfsmStateId n_new_states);
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
gfsmAutomaton *gfsm_automaton_arcsort_full(gfsmAutomaton *fsm, GCompareDataFunc cmpfunc, gpointer data);

/** Alias for gfsm_automaton_arcsort_full() */
#define gfsm_automaton_arcsort_with_data(fsm,cmpfunc,data) \
  gfsm_automaton_arcsort_full((fsm),(cmpfunc),(data))

/*-- todo:

+ list-wise arc methods:
  open_arcs_list(fsm,qid)
  set_arcs_list(fsm,qid,arcs_l)
  close_arcs_list(fsm,arcs_l)
  (?) sort_arcs_list_full(arcs_l, cmpfunc, data)

+ array-wise arc methods:
  open_arcs_array(fsm,qid)
  set_arcs_array(fsm,qid,arcs_a)
  close_arcs_array(fsm,arcs_a)
  (?) sort_arcs_array_full(arcs_a, cmpfunc, data)

+ arc-iterator methods:
  arciter_open(fsm,qid)
  arciter_open_ptr(fsm,qp)
  (?) arciter_seek_lower(aip,lab)
  (?) arciter_seek_upper(aip,lab)
  (?) arciter_seek_both(aip,lo,hi)
  (??) arciter_open_lower(fsm,qid,lab)
  (??) arciter_open_upper(fsm,qid,lab)
*/

//@}

#endif /* _GFSM_AUTOMATON_H */
