
/*=============================================================================*\
 * File: gfsmLookup.h
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

/** \file gfsmLookup.h
 *  \brief Linear composition
 */

#ifndef _GFSM_LOOKUP_H
#define _GFSM_LOOKUP_H

#include <gfsmAlphabet.h>
#include <gfsmAutomaton.h>
#include <gfsmUtils.h>

/*======================================================================
 * Types: lookup
 */
/** Type for gfsm_automaton_lookup() computation state */
typedef struct {
  gfsmStateId qt;  /**< current state in transducer */
  gfsmStateId qr;  /**< current state in result acceptor */
  guint32     i;   /**< current position in input vector */
} gfsmLookupConfig;

/** Type for a StateId map */
typedef GPtrArray gfsmStateIdVector;




/** Type for gfsm_automaton_lookup_viterbi(): Trellis (1 per call) */
typedef GPtrArray gfsmViterbiTable;

/** Type for gfsm_automaton_lookup_viterbi(): Trellis columns (1 per input index)
 *  Key-type is gfsmViterbiNodeKey
 *  Value-type is gfsmViterbiNodeValue
 */
typedef GTree gfsmViterbiColumnMap;

/** Type for gfsm_automaton_lookup_viterbi(): Trellis columns (1 per input index)
 *  Data-type is (gfsmViterbiNode*)
 */
typedef GSList gfsmViterbiColumn;

/** Type for gfsm_automaton_lookup_viterbi(): Trellis nodes: keys (1 per (input_index,fst_state)-pair) */
typedef gfsmStateId gfsmViterbiNodeKey;

/** Type for gfsm_automaton_lookup_viterbi(): Trellis nodes: values (1 per (input_index,fst_state)-pair) */
typedef struct {
  gfsmStateId qtrellis;  /**< current state in trellis */
  gfsmStateId pqtrellis; /**< best previous state in trellis */
  gfsmLabelVal label;    /**< label of best arc to qtrellis */
  gfsmWeight  w;         /**< weight of best path to qtrellis in fst */
} gfsmViterbiNodeValue;
/*---> HEY: could we use a simple gfsmStateId (in trellis) for this?
 * node = qtrellis
 * node->pqtrellis = (gfsmArc*)(gfsm_automaton_get_state(qtrellis)->arcs->data)->target
 * node->label     = (gfsmArc*)(gfsm_automaton_get_state(qtrellis)->arcs->data)->upper
 * node->w         = (gfsmArc*)(gfsm_automaton_get_state(qtrellis)->arcs->data)->weight
 *
 *--> this implies building a REVERSE Viterbi trellis (which is what we have to do anyways),
 *    and we can always use gfsm_automaton_reverse() to get the nice-looking stringifiable-version...
 */


/** Type for a full ViterbiNode */
typedef struct {
  gfsmViterbiNodeKey    key; /**< node key */
  gfsmViterbiNodeValue *val; /**< node value */
} gfsmViterbiNode;

/*======================================================================
 * Constants
 */

/** Number of states to pre-allocate when extending state-map vector on lookup_full() (>= 1) */
extern const guint gfsmLookupStateMapGet;


/*======================================================================
 * Methods: lookup
 */
///\name Lookup
//@{

//------------------------------
/** Compose linear automaton specified by \a input with the transducer
 *  \a fst and project the output tape to \a result.
 *  \param fst transducer (lower-upper)
 *  \param input input labels (lower)
 *  \param result output transducer or NULL
 *  \returns \a result if non-NULL, otherwise a new automaton.
 */
#define gfsm_automaton_lookup(fst,input,result) \
  gfsm_automaton_lookup_full((fst),(input),(result),NULL)

//------------------------------
/** Compose linear automaton specified by \a input with the transducer
 *  \a fst and project the output tape to \a result, storing state-translation map \a qenum.
 *  \param fst transducer (lower-upper)
 *  \param input input labels (lower)
 *  \param result output transducer or NULL
 *  \param statemap if non-NULL, maps \a result StateIds (indices) to \a fst StateIds (values) on return.
 *                  Not implicitly created or cleared.
 *  \returns \a result if non-NULL, otherwise a new automaton.
 */
gfsmAutomaton *gfsm_automaton_lookup_full(gfsmAutomaton     *fst,
					  gfsmLabelVector   *input,
					  gfsmAutomaton     *result,
					  gfsmStateIdVector *statemap);

//@}


/*======================================================================
 * Methods: Viterbi
 */
///\name Viterbi Lookup
//@{

//------------------------------
/** Get the best path for input \a input in the transducer \a fst using the Viterbi algorithm.
 *  \param fst transducer (lower-upper)
 *  \param input input labels (lower)
 *  \param trellis output fsm or NULL
 *  \returns \a trellis if non-NULL, otherwise a new automaton representing the (reversed) Viterbi trellis.
 *           lower&upper labels in \a trellis represent upper labels of \a fst,
 *           and arc-weights in \a trellis represent Viterbi algorithm weights (gamma).
 */
gfsmAutomaton *gfsm_automaton_lookup_viterbi(gfsmAutomaton     *fst,
					     gfsmLabelVector   *input,
					     gfsmAutomaton     *trellis);

//------------------------------
/** Expand lower-epsilon arcs from \a fst into \a col.
 *  Implicitly clears \a cmap.
 */
void _gfsm_viterbi_expand_column(gfsmAutomaton        *fst,
				 gfsmAutomaton        *trellis,
				 gfsmViterbiColumn    *col,
				 gfsmViterbiColumnMap *cmap);

//------------------------------
/** Utility: Create a new gfsmViterbiColumnMap: copies but does NOT free keys! */
#define gfsm_viterbi_column_map_new() \
  g_tree_new_full((GCompareDataFunc)gfsm_uint_compare, NULL, NULL, NULL)

//------------------------------
/** Utility: Free a gfsmViterbiColumnMap */
#define gfsm_viterbi_column_map_free(col) g_tree_destroy(col)

//------------------------------
/** Utility: Lookup stored value in a gfsmViterbiColumnMap
 *  \returns gpointer to the stored value for \a key in \a col
 */
#define gfsm_viterbi_column_map_lookup(col,key) g_tree_lookup((col),(key))

//------------------------------
/** insert a literal value into a gfsmViterbiColumnMap */
void gfsm_viterbi_column_map_insert(gfsmViterbiColumnMap *cmap,
				    gfsmViterbiNodeKey    key,
				    gfsmViterbiNodeValue *val);

//------------------------------
/** Possibly insert a key->value mapping into the gfsmViterbiColumnMap.
 *  The mapping \a (key=>val) is inserted if no mapping for \a key exists in \a col.
 *  Otherwise, the weight of the stored value \a (stored_val->w) for \a key is set to \a (w)
 *  just in case \a (w)
 *    is strictly less than
 *  \a (stored_val->w) for \a key according to \a sr.
 *
 *  \returns pointer to the stored value if update occurred, otherwise NULL
 */
gfsmViterbiNodeValue *gfsm_viterbi_column_map_insert_if_less(gfsmViterbiColumnMap *cmap,
							     gfsmViterbiNodeKey    key,
							     gfsmWeight            w,
							     gfsmSemiring         *sr);

//------------------------------
/** Possibly insert a key->value mapping into the gfsmViterbiColumnMap.
 *  The mapping \a (key=>val) is inserted if no mapping for \a key exists in \a col.
 *  Otherwise, the weight of the stored value \a (stored_val->w) for \a key is set to \a (w+stored_val->w)
 *  just in case \a (w+stored_val->w)
 *    is strictly less than
 *  \a (stored_val->w) for \a key according to \a sr.
 *
 *  \returns pointer to the newly inserted value if one was created, otherwise NULL
 */
gfsmViterbiNodeValue *gfsm_viterbi_column_map_insert_sum_if_less(gfsmViterbiColumnMap *cmap,
								 gfsmViterbiNodeKey    key,
								 gfsmWeight            w,
								 gfsmSemiring         *sr);


//@}

#endif /* _GFSM_LOOKUP_H */
