
/*=============================================================================*\
 * File: gfsmEncode.h
 * Author: Bryan Jurish <moocow.bovine@gmail.com>
 * Description: finite state machine library: debugging
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

/** \file gfsmEncode.h
 *  \brief Arc-label and/or -weight encoding utilities
 */

#ifndef _GFSM_ENCODE_H
#define _GFSM_ENCODE_H

#include <gfsmAutomaton.h>
#include <gfsmAlphabet.h>


/*======================================================================
 * Types
 */
///\name Types
//@{

/// Weighted pair of ::gfsmLabelId , used by gfsm_automaton_encode()
typedef struct {
  gfsmLabelId  lo;  /**< lower (input) label */
  gfsmLabelId  hi;  /**< upper (output) label */
  gfsmWeight    w;  /**< weight */
} gfsmArcLabel;

/// Typedef for mapping (::gfsmArcLabel)s to and/or from atomic (::gfsmLabelVal)s,
/// used by gfsm_automaton_encode().  Really implemented as a ::gfsmPointerKey
typedef gfsmAlphabet gfsmArcLabelKey;

//@}

/*======================================================================
 * gfsmArcLabel: Methods
 */
///\name gfsmArcLabel Methods
//@{

/** Create a ::gsfmArcLabel */
GFSM_INLINE
gfsmArcLabel *gfsm_arclabel_new(gfsmLabelVal lo, gfsmLabelVal hi, gfsmWeight w);

/** Assign values to a ::gsfmArcLabel , like *al={lo,hi,w} */
GFSM_INLINE
void gfsm_arclabel_set(gfsmArcLabel *al, gfsmLabelVal lo, gfsmLabelVal hi, gfsmWeight w);

/** Clone an existing ::gfsmArcLabel */
GFSM_INLINE
gfsmArcLabel *gfsm_arclabel_clone(gfsmArcLabel *al);

/** Free a ::gfsmArcLabel */
GFSM_INLINE
void gfsm_arclabel_free(gfsmArcLabel *al);

/** Hash function alias for ::gfsmArcLabel */
guint gfsm_arclabel_hash(gfsmArcLabel *al);

/** Equal function alias for ::gfsmArcLabel */
guint gfsm_arclabel_equal(const gfsmArcLabel *al1, const gfsmArcLabel *al2);

//@}

/*======================================================================
 * gfsmArcLabelKey
 */
///\name gfsmArcLabelKey Methods
//@{

/** create a new ::gfsmArcLabelKey (copies & frees keys)
 *  \see gfsmPointerAlphabet
 */
GFSM_INLINE
gfsmArcLabelKey *gfsm_arclabel_key_new(void);

/** Alias \see gfsm_alphabet_clear() */
#define gfsm_arclabel_key_clear gfsm_alphabet_clear

/** Alias; \see gfsm_alphabet_free() */
#define gfsm_arclabel_key_free  gfsm_alphabet_free

/** Convert a ::gfsmArcLabelKey to a ::gfsmAutomaton "key", a la fsmencode(1)
 *  \param key label-pair enumeration to be converted
 *  \param fsm output automaton, may be passed as NULL to create a new automaton
 *  \returns fsm representing \a key
 */
gfsmAutomaton *gfsm_arclabel_key_to_fsm(gfsmArcLabelKey *key, gfsmAutomaton *fsm);

/** Convert a ::gfsmAutomaton "key" to a ::gfsmArcLabelKey , a la fsmencode(1) in decoding mode.
 *  \param fsm input "key" automaton
 *  \param key output label-pair enumeration, may be passed as NULL to create a new one
 *  \returns key represented by \a fsm
 */
gfsmArcLabelKey *gfsm_arclabel_fsm_to_key(gfsmAutomaton *fsm, gfsmArcLabelKey *key);

//@}

/*======================================================================
 * Top-Level Methods
 */
///\name Top-Level Methods
//@{

/** Encode arc-labels and/or weights to a ::gfsmArcLabelKey
 * \param fsm automaton to encode (destructively altered)
 * \param key stores label-pair enumeration, may be passed as NULL to allocate a new enum
 * \param encode_labels whether to encode labels
 * \param encode_weights whether to encode weights
 * \returns key or new enum
 */
gfsmArcLabelKey *gfsm_automaton_encode(gfsmAutomaton *fsm, gfsmArcLabelKey *key, gboolean encode_labels, gboolean encode_weights);

/** Decode arc-labels and/or weights as specified in a ::gfsmArcLabelKey in an automaton \a fsm
 * \param fsm encoded automaton to decode (destructively altered)
 * \param key stores label-pair enumeration
 * \param decode_labels whether to decode labels
 * \param decode_weights whether to decode weights
 * \returns decoded \a fsm
 */
gfsmAutomaton *gfsm_automaton_decode(gfsmAutomaton *fsm, gfsmArcLabelKey *key, gboolean encode_labels, gboolean encode_weights);

//@}

//-- inline definitions
#ifdef GFSM_INLINE_ENABLED
# include <gfsmEncode.hi>
#endif

#endif /* _GFSM_ENCODE_H */
