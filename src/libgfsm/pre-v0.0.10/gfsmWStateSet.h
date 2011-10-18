
/*=============================================================================*\
 * File: gfsmWStateSet.h
 * Author: Bryan Jurish <moocow.bovine@gmail.com>
 * Description: finite state machine library
 *
 * Copyright (c) 2004-2011 Bryan Jurish.
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

/** \file gfsmWStateSet.h
 *  \brief Sets of weighted (gfsmStateId)s
 */

#ifndef _GFSM_WSTATESET_H
#define _GFSM_WSTATESET_H

#include <gfsmAutomaton.h>

/*======================================================================
 * Types
 */

/** \brief typedef for a set of weighted states
 * \detail current implementation uses gfsmWeightMap (GTree) */
typedef gfsmWeightMap gfsmWStateSet;

/** \brief typedef for a weighted state
*/
typedef struct {
  gfsmStateId q; /**< state id */
  gfsmWeight  w; /**< (residual) weight */
} gfsmWState;

/** \brief typedef for a list of weighted states
 *  \detail current implementation uses a (sorted) list of gfsmWState*
 */
typedef GSList gfsmWStateList;

/*======================================================================
 * Methods: gfsmWStateSet
 */
/// \name gfsmWStateSet methods
//@{

/** Create and return a new state-set */
GFSM_INLINE
gfsmWStateSet *gfsm_wqset_new(void);

/** Create and return a new singleton weighted state-set */
GFSM_INLINE
gfsmWStateSet *gfsm_wqset_new_singleton(gfsmStateId id, gfsmWeight w);

/** clear a weighted state-set */
GFSM_INLINE
void gfsm_wqset_clear(gfsmWStateSet *wqset);

/** destroy a weighted state-set */
GFSM_INLINE
void gfsm_wqset_free(gfsmWStateSet *wqset);

/** Insert an item into a ::gfsmWStateSet, using \a sr to compute sum.
 * \returns new weight of \a q in \a wqset.
 */
GFSM_INLINE
gfsmWeight gfsm_wqset_insert(gfsmWStateSet *wqset, gfsmSemiring *sr, gfsmStateId q, gfsmWeight w);

/** Convert a ::gfsmWStateSet to a ::gfsmWStateList */
gfsmWStateList *gfsm_wqset_to_list(gfsmWStateSet *wqset);

//@}

/*======================================================================
 * Methods: Accessors
 */
///\name Accessors
//@{

/** Get minimum element of a state-set */
GFSM_INLINE
gfsmStateId gfsm_stateset_min(gfsmWStateSet *sset);

/** Get number of elements in a state-set */
GFSM_INLINE
guint gfsm_stateset_size(gfsmWStateSet *sset);

/** Check whether a state-id is contained in a state-set */
GFSM_INLINE
gboolean gfsm_stateset_contains(gfsmWStateSet *sset, gfsmStateId id);

/** Insert a single state-id into a state-set.
 *  \returns true iff \a sset already contained \a id
 */
gboolean gfsm_stateset_insert(gfsmWStateSet *sset, gfsmStateId id);

/** Assign \a sset1 to be the union of itself with \a sset2 */
gfsmWStateSet *gfsm_stateset_union(gfsmWStateSet *sset1, gfsmWStateSet *sset2);

/** Remove a state-id from a state-set
 * \returns true iff \a sset contained \a id
 */
gboolean gfsm_stateset_remove(gfsmWStateSet *sset, gfsmStateId id);

/** Equality test */
gboolean gfsm_stateset_equal(gfsmWStateSet *sset1, gfsmWStateSet *sset2);
//@}

/*======================================================================
 * Methods: Iteration
 */
///\name Iteration
//@{

/// state-set iterator type
typedef gfsmStateId* gfsmWStateSetIter;

/** Iterator access: get current state-id, or gfsmNoState if none defined */
#define gfsm_stateset_iter_id(sseti) \
  ((sseti) ? *(sseti) : gfsmNoState)

/** Check validity of a state-set iterator */
#define gfsm_stateset_iter_ok(sseti) \
  ((sseti)!=NULL)

/** Get first state in the state-set.
 *  \returns iterator (value) pointing to the first state in the
 *  state-set.
 */
#define gfsm_stateset_iter_begin(sset) \
  ((gfsmStateId*)((sset)->data))

/** Increment a state-set iterator one position. */
#define gfsm_stateset_iter_next(sset,sseti) \
  ((++(sseti)) < (((gfsmStateId*)((sset)->data))+((sset)->len)) \
   ? (sseti) \
   : NULL)

/** Find an iterator pointing to the element for \a id in \a sset,
 *  or a bad iterator if no such element exists */
gfsmWStateSetIter gfsm_stateset_find(gfsmWStateSet *sset, gfsmStateId id);

//@}

/*======================================================================
 * Methods: Utiltiies
 */
///\name Utilities
//@{

/// typedef for iteration functions: return TRUE to stop iteration
typedef gboolean (*gfsmWStateSetForeachFunc) (gfsmStateId id, gpointer data);

/** General iteration utilitiy for state-sets */
void gfsm_stateset_foreach(gfsmWStateSet *sset, gfsmWStateSetForeachFunc func, gpointer data);

/** Hashing function for state-sets */
guint gfsm_stateset_hash(gfsmWStateSet *sset);

//@}

/*======================================================================
 * Methods: Automaton
 */
///\name Methods: Automaton access
//@{

/** Populate a state-set representing targets of arcs with
 *  lower label \a lo and upper label \a hi leaving state with id \a id
 *  in automaton \a fsm.
 *
 *  If either \a lo or \a hi is gfsmNoLabel, the corresponding labels
 *  will be ignored.
 *
 *  Note that this method does not clear \a sset.
 */
void gfsm_stateset_populate(gfsmWStateSet *sset,
			    gfsmAutomaton *fsm,
			    gfsmStateId     id,
			    gfsmLabelVal    lo,
			    gfsmLabelVal    hi);

/** Convenience macro to populate a state-set from I/O-epsilon arcs */
#define gfsm_stateset_populate_eps(sset,fsm,id) \
  gfsm_stateset_populate((sset),(fsm),(id),gfsmEpsilon,gfsmEpsilon)

/** Returns true iff some \a id in \a sset is a final state in \a fsm */
gboolean gfsm_stateset_has_final_state(gfsmWStateSet *sset, gfsmAutomaton *fsm);

/** Lookup sum of final weights in \a fsm of states \a id in \a sset
 *  Returns TRUE iff at least one state in \a sset is final, and
 *  sets \a *wp to the sum of final weights.
 */
gboolean gfsm_stateset_lookup_final_weight(gfsmWStateSet *sset, gfsmAutomaton *fsm, gfsmWeight *wp);

//@}

//-- inline definitions
#ifdef GFSM_INLINE_ENABLED
# include <gfsmWStateSet.hi>
#endif

#endif /* _GFSM_STATESET_H */
