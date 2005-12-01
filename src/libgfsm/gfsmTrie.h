
/*=============================================================================*\
 * File: gfsmTrie.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library
 *
 * Copyright (c) 2005 Bryan Jurish.
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

/** \file gfsmTrie.h
 *  \brief Deterministic prefix tree automata
 */

#ifndef _GFSM_TRIE_H
#define _GFSM_TRIE_H

#include <gfsmAutomaton.h>
#include <gfsmAlphabet.h>

/*======================================================================
 * Types: Trie
 */
/** Alias for gfsmAutomaton */
typedef gfsmAutomaton gfsmTrie;

/*======================================================================
 * Constants
 */
/** Default initial Trie flags */
extern const gfsmAutomatonFlags gfsmTrieDefaultFlags;

/** Default initial Trie semiring */
extern const gfsmSRType gfsmTrieDefaultSRType;


/*======================================================================
 * Methods: Constructors etc.
 */
///\name Constructors etc.
//@{

//------------------------------
/** Create a new Trie automaton */
#define gfsm_trie_new() \
  gfsm_automaton_new_full(gfsmTrieDefaultFlags, gfsmTrieDefaultSRType, gfsmAutomatonDefaultSize)

//@}


/*======================================================================
 * Methods: Manipulation
 */
///\name Manipulation
//@{

//------------------------------
/** Add a string-pair \a (lo,hi) to the trie with weight \a w
 *  \param trie Trie
 *  \param lo   lower string (NULL for epsilon)
 *  \param trie upper string (NULL for epsilon)
 *  \param w    weight which is added (gfsm_sr_plus) to all arcs for this pair
 *  \returns Id of the final state of the added path
 */
gfsmStateId gfsm_trie_add_paths(gfsmTrie        *trie,
				gfsmLabelVector *lo,
				gfsmLabelVector *hi,
				gfsmWeight       w);

/** Find an arc from state \a qid with lower label \a lab in trie \a trie.
 *  \param trie Trie
 *  \param qid   outgoing state qid
 *  \param lab   lower label
 *  \returns gfsmArc* or NULL on failure
 */
gfsmArc* gfsm_trie_find_arc_lower(gfsmTrie *trie, gfsmStateId qid, gfsmLabelVal lab);

/** Find an arc from state \a qid with upper label \a lab in trie \a trie.
 *  \param trie  Trie
 *  \param qid    outgoing state qid
 *  \param lab   upper label id
 *  \returns gfsmArc* or NULL on failure
 */
gfsmArc* gfsm_trie_find_arc_upper(gfsmTrie *trie, gfsmStateId qid, gfsmLabelVal lab);

/** Find an arc from state \a qid with lower label \a lo and upper label \a hi in trie \a trie.
 *  \param trie  Trie
 *  \param qid  outgoing state qid
 *  \param lo   lower label id
 *  \param hi   upper label id
 *  \returns gfsmArc* or NULL on failure
 */
gfsmArc* gfsm_trie_find_arc_both(gfsmTrie *trie, gfsmStateId qid, gfsmLabelVal lo, gfsmLabelVal hi);


/** Find or insert an arc from state \a qid with lower label \a lab in trie \a trie;
 *  adding weight \a w.
 *  \param trie Trie
 *  \param qid   outgoing state qid
 *  \param lab   lower label
 *  \param w    arc weight.
 *  \returns gfsmStateId of the (unique) destination state
 */
gfsmStateId gfsm_trie_get_arc_lower(gfsmTrie *trie, gfsmStateId qid, gfsmLabelVal lab, gfsmWeight w);

/** Find or insert an arc from state \a qid with upper label \a lab in trie \a trie;
 *  adding weight \a w.
 *  \param trie Trie
 *  \param qid   outgoing state qid
 *  \param lab   upper label
 *  \param w    arc weight
 *  \returns gfsmStateId of the (unique) destination state
 */
gfsmStateId gfsm_trie_get_arc_upper(gfsmTrie *trie, gfsmStateId qid, gfsmLabelVal lab, gfsmWeight w);

/** Find or insert an arc from state \a qid with lower label \a lo and upper label \a hi
 *  with weight \a w in trie \a trie.
 *  \param trie  Trie
 *  \param qid  outgoing state qid
 *  \param lo   lower label id
 *  \param hi   upper label id
 *  \param w    arc weight
 *  \returns gfsmStateId of the (unique) destination state
 */
gfsmStateId gfsm_trie_get_arc_both(gfsmTrie *trie, gfsmStateId qid, gfsmLabelVal lo, gfsmLabelVal hi, gfsmWeight w);

//@}


#endif /* _GFSM_LOOKUP_H */
