
/*=============================================================================*\
 * File: gfsmArcIndex.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: arc indices
 *
 * Copyright (c) 2006-2007 Bryan Jurish.
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

/** \file gfsmArcIndex.h
 *  \brief Arc (transition) index utilities
 */

#ifndef _GFSM_ARCINDEX_H
#define _GFSM_ARCINDEX_H

#include <gfsmAutomaton.h>

/*======================================================================
 * Types
 */

/// Reverse arc-index type
/**  \a element at \a qto is a gfsmArcList*
 *    which contains an element \a gfsmArc* \a {qfrom,qto,lo,hi,w}
 *    whenever source \a fsm contains an arc \a {qfrom,qto,lo,hi,w}
 *    from \a qfrom.
 *
 *  \warning arc data pointed to is shared by source automaton
 *           and the ::gfsmReverseArcIndex!
 */
typedef GPtrArray gfsmReverseArcIndex;


/// Final-weight index type
/** GArray of ::gfsmWeight, indexed by ::gfsmStateId : final weight or sr->zero if state is non-final */
typedef GArray gfsmFinalWeightIndex;

/// Generic arc-label index type
typedef struct {
  GPtrArray *arcs;      /**< arc-pointers sorted by state id, label, weight */
  GPtrArray *first;     /**< first[q] is address of first element of  \a arcs->pdata for state \a q (a :gfsmArc **) */
} gfsmArcLabelIndex;


/*======================================================================
 * Methods: ReverseArcIndex
 */
///\name gfsmReverseArcIndex Utilities
//@{

/** Create a new reversed arc index */
#define gfsm_reverse_arc_index_new() g_ptr_array_new()

/** Create a new reversed arc index, given size */
#define gfsm_reverse_arc_index_sized_new(nelts) g_ptr_array_sized_new(nelts)

/** Populate a reversed arc index for \a fsm.
 * \param fsm source automaton
 * \param rarcs
 *   Reverse arc index.
 *   May be passed as NULL to create a new arc index.
 * \returns \a rarcs if non-NULL, otherwise a new reverse arc index for \a fsm.
 */
gfsmReverseArcIndex *gfsm_automaton_reverse_arc_index(gfsmAutomaton *fsm, gfsmReverseArcIndex *rarcs);

/** Free a reverse arc index.
 *  \param rarcs
 *    reverse arc-index to be freed
 *  \param free_lists
 *    If true, associated arc-lists will be freed.
 *  \param ignored
 *    Ignored.  (Formerly if true, arcs in associated arc-lists will be freed)
 */
void gfsm_reverse_arc_index_free(gfsmReverseArcIndex *rarcs, gboolean free_lists, gboolean ignored);

//@}

/*======================================================================
 * Methods: gfsmFinalWeightIndex
 */
///\name gfsmFinalWeightIndex Utilities
//@{

/** Create a new (empty) ::gfsmFinalWeightIndex */
#define gfsm_final_weight_index_new() \
  g_array_new(FALSE,FALSE,sizeof(gfsmWeight))

/** Create a new (empty) ::gfsmFinalWeightIndex, specifying initial size */
#define gfsm_final_weight_index_sized_new(size) \
  g_array_sized_new(FALSE,FALSE,sizeof(gfsmWeight),(size))

/** Populate a ::gfsmFinalWeightIndex from a ::gfsmAutomaton
 * \param fsm source automaton
 * \param ix
 *   Final weight index
 *   May be passed as NULL to create a new index.
 * \returns \a ix if non-NULL, otherwise a new final weight index for \a fsm.
 */
gfsmFinalWeightIndex *gfsm_automaton_final_weight_index(gfsmAutomaton *fsm, gfsmFinalWeightIndex *ix);

/** Free a ::gfsmFinalWeightIndex */
#define gfsm_final_weight_index_free(ix) \
  g_array_free((ix),TRUE)

//@}

/*======================================================================
 * Methods: gfsmArcLabelIndex
 */
///\name gfsmArcLabelIndex Utilities
//@{

/** Create and return a new (empty) ::gfsmArcLabelIndex */
gfsmArcLabelIndex *gfsm_arc_label_index_new(void);

/** Create and return a new (empty) ::gfsmArcLabelIndex, specifying sizes */
gfsmArcLabelIndex *gfsm_arc_label_index_new_full(gfsmStateId n_states, guint n_arcs);

/** Populate a ::gfsmArcLabelIndex over lower labels from a ::gfsmAutomaton
 * \param fsm source automaton
 * \param ix
 *   Arc-label index for lower labels.
 *   May be passed as NULL to create a new index.
 * \returns \a ix if non-NULL, otherwise a new lower-label index for \a fsm.
 */
gfsmArcLabelIndex *gfsm_automaton_lower_label_index(gfsmAutomaton *fsm, gfsmArcLabelIndex *ix);

/** Populate a ::gfsmArcLabelIndex over upper labels from a ::gfsmAutomaton
 * \param fsm source automaton
 * \param ix
 *   Arc-label index for upper labels.
 *   May be passed as NULL to create a new index.
 * \returns \a ix if non-NULL, otherwise a new upper-label index for \a fsm.
 */
gfsmArcLabelIndex *gfsm_automaton_upper_label_index(gfsmAutomaton *fsm, gfsmArcLabelIndex *ix);

//@}

/*======================================================================
 * END
 */
#endif /* _GFSM_ARCINDEX_H */
