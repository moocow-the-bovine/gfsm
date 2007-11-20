
/*=============================================================================*\
 * File: gfsmCompound.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: basic compound types
 *
 * Copyright (c) 2004-2007 Bryan Jurish.
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

/** \file gfsmCompound.h
 *  \brief Utilities for "compound" states and labels
 */

#ifndef _GFSM_COMPOUND_H
#define _GFSM_COMPOUND_H

#include <gfsmEnum.h>
#include <gfsmWeightMap.h>

/*======================================================================
 * Compound Types
 */
/// Typedef for (lower,upper) label pairs: really just a wrapper for some bit operations
typedef guint32 gfsmLabelPair;

/// Matched pair of state-ids (used for automaton intersection)
typedef struct {
  gfsmStateId  id1;  /**< Id of first component */
  gfsmStateId  id2;  /**< Id of second component */
} gfsmStatePair;

/// Typedef for composition filter states
/**
 * as described in Morhi, Pereira, & Riley (1996) "Weighted Automata in Text and Speech Processing",
 * Proc. ECAI 96, John Wiley & Sons.
 */
typedef guint8 gfsmComposeFilterState;

/// Matched pair of (::gfsmStateId)s with an additional filter state (used for transducer composition)
/**
 * for details, see
 * Morhi, Pereira, & Riley (1996) "Weighted Automata in Text and Speech Processing",
 * Proc. ECAI 96, John Wiley & Sons.
 */
typedef struct {
  gfsmStateId            id1;  /**< Id of first component */
  gfsmStateId            id2;  /**< Id of second component */
  gfsmComposeFilterState idf;  /**< Filter state (0|1|2) */
} gfsmComposeState;

/// Type for a (::gfsmStateId,::gfsmWeight) pair (used by algebraic operations)
typedef struct {
  gfsmStateId id; /**< state-id */
  gfsmWeight   w; /**< weight */
} gfsmStateWeightPair;


/// Typedef for mapping (::gfsmStatePair)s to single (::gfsmStateId)s,
/// used by gfsm_automaton_intersection()
typedef gfsmEnum gfsmStatePairEnum;

/// Typedef for mapping (::gfsmComposeState)s to single (::gfsmStateId)s,
/// used by gfsm_automaton_compose()
typedef gfsmEnum gfsmComposeStateEnum;

/// Typedef for mapping (::gfsmStatePair)s to single (::gfsmWeight)s,
/// used by gfsm_automaton_rmepsilon()
typedef gfsmWeightHash gfsmStatePair2WeightHash;


/*======================================================================
 * LabelPair: Methods
 */
///\name gfsmLabelPair Methods
//@{

/** Create a "new" gsfmLabelPair */
static inline
gfsmLabelPair gfsm_labelpair_new(guint32 lo, guint32 hi);

/** Get lower label of of a label-pair */
static inline
gfsmLabelId gfsm_labelpair_lower(gfsmLabelPair lp);

/** Get upper label of of a label-pair */
static inline
gfsmLabelId gfsm_labelpair_upper(gfsmLabelPair lp);

/** Hash function alias for ::gfsmLabelPair */
#define gfsm_labelpair_hash g_direct_hash

/** Equal function alias for ::gfsmLabelPair */
#define gfsm_labelpair_equal g_direct_equal

/** Comparison function for ::gfsmLabelPair */
gint gfsm_labelpair_compare(gfsmLabelPair lp1, gfsmLabelPair lp2);

/** Dummy data-comparison function for gfsmLabelPair */
gint gfsm_labelpair_compare_with_data(gfsmLabelPair lp1, gfsmLabelPair lp2, gpointer data);
//@}

/*======================================================================
 * Methods: gfsmStatePair
 */
///\name gfsmStatePair Methods
//@{

/** Create a new ::gfsmStatePair */
static inline
gfsmStatePair *gfsm_statepair_new(gfsmStateId id1, gfsmStateId id2);

/** Clone an existing ::gfsmStatePair */
static inline
gfsmStatePair *gfsm_statepair_clone(gfsmStatePair *sp);

/** Free a ::gfsmStatePair */
static inline
void gfsm_statepair_free(gfsmStatePair *sp);

/** Get a more or less sensible hash value from a state pair */
guint gfsm_statepair_hash(gfsmStatePair *sp);

/** Comparison function for ::gfsmStatePair */
gint gfsm_statepair_compare(const gfsmStatePair *sp1, const gfsmStatePair *sp2);

/** Equality predicate for ::gfsmStatePair */
gboolean gfsm_statepair_equal(const gfsmStatePair *sp1, const gfsmStatePair *sp2);

//@}

/*======================================================================
 * Methods: gfsmComposeState
 */
///\name gfsmComposeState Methods
//@{

/** Create a new ::gfsmComposeState */
static inline
gfsmComposeState *gfsm_compose_state_new(gfsmStateId id1, gfsmStateId id2, gfsmComposeFilterState idf);

/** Clone an existing ::gfsmComposeState */
static inline
gfsmComposeState *gfsm_compose_state_clone(gfsmComposeState *sp);

/** Free a ::gfsmComposeState */
static inline
void gfsm_compose_state_free(gfsmComposeState *sp);

/** Get a (more or less sensible) hash value from a ::gfsmComposeState */
guint gfsm_compose_state_hash(gfsmComposeState *sp);

/** Comparison function for gfsmComposeState */
gint gfsm_compose_state_compare(const gfsmComposeState *sp1, const gfsmComposeState *sp2);

/** Equality predicate for ::gfsmComposeState */
gboolean gfsm_compose_state_equal(const gfsmComposeState *sp1, const gfsmComposeState *sp2);

//@}


/*======================================================================
 * Methods: gfsmStateWeightPair
 */
///\name gfsmStateWeightPair Methods
//@{

/** Create a new ::gfsmStateWeightPair */
static inline
gfsmStateWeightPair *gfsm_state_weight_pair_new(gfsmStateId id, gfsmWeight w);

/** Clone an existing ::gfsmStateWeightPair */
static inline
gfsmStateWeightPair *gfsm_state_weight_pair_clone(const gfsmStateWeightPair *swp);

/** Free a ::gfsmStateWeightPair */
static inline
void gfsm_state_weight_pair_free(gfsmStateWeightPair *swp);

/** Get a (more or less sensible) hash value from a ::gfsmStateWeightPair (really just hashes id) */
guint gfsm_state_weight_pair_hash(gfsmStateWeightPair *sp);

/** Comparison function for ::gfsmStateWeightPair (id << w) */
gint gfsm_state_weight_pair_compare(const gfsmStateWeightPair *swp1, const gfsmStateWeightPair *swp2, gfsmSemiring *sr);

/** Equality predicate for ::gfsmStateWeightPair */
gboolean gfsm_state_weight_pair_equal(const gfsmStateWeightPair *swp1, const gfsmStateWeightPair *swp2);

//@}

/*======================================================================
 * Methods: gfsmStatePairEnum
 */
///\name gfsmStatePairEnum Methods
//@{

/** create a new ::gfsmStatePairEnum (copies & frees keys)
 *  \see gfsmEnum
 */
static inline
gfsmStatePairEnum *gfsm_statepair_enum_new(void);

/** Alias \see gfsm_enum_clear() */
#define gfsm_statepair_enum_clear gfsm_enum_clear

/** Alias; \sa gfsm_enum_clear() */
#define gfsm_statepair_enum_free  gfsm_enum_free

//@}

/*======================================================================
 * Methods: gfsmComposeStateEnum
 */
///\name gfsmComposeStateEnum Methods
//@{

/** create a new ::gfsmComposeStateEnum (copies & frees keys)
 *  \see gfsmEnum
 */
static inline
gfsmComposeStateEnum *gfsm_compose_state_enum_new(void);

/** Alias; \sa gfsm_enum_clear() */
#define gfsm_compose_state_enum_clear gfsm_enum_clear

/** Alias; \sa gfsm_enum_clear() */
#define gfsm_compose_state_enum_free  gfsm_enum_free

//@}


/*======================================================================
 * Methods: gfsmStatePair2WeightHash
 */
///\name gfsmStatePair2WeightHash Methods
//@{

/** create a new ::gfsmStatePair2WeightHash (copies & frees keys)
 *  \see gfsmWeightHash
 */
static inline
gfsmStatePair2WeightHash *gfsm_statepair2weighthash_new(void);

//@}

//-- inline definitions
#include <gfsmCompound.def>

#endif /* _GFSM_COMPOUND_H */
