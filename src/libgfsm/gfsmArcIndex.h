
/*=============================================================================*\
 * File: gfsmArcIndex.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: arc indices
 *
 * Copyright (c) 2006 Bryan Jurish.
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

#include <glib.h>
#include <gfsmAutomaton.h>

/*======================================================================
 * Types
 */
/// Reverse arc-index type
/**  \a element at \a qto is a gfsmArcList*
 *    which contains an element \a gfsmArc* \a {lo,hi,qfrom,w}
 *    whenever source \a fsm contains an arc \a {lo,hi,qto,w} from \a qfrom.
 */
typedef GPtrArray gfsmReverseArcIndex;


/*======================================================================
 * Methods: ReverseArcIndex
 */

//------------------------------
/** Create a new reversed arc index */
#define gfsm_reverse_arc_index_new() g_ptr_array_new()

//------------------------------
/** Create a new reversed arc index, given size */
#define gfsm_reverse_arc_index_sized_new(nelts) g_ptr_array_sized_new(nelts)

//------------------------------
/** Populate a reversed arc index for \a fsm.
 * \param fsm Automaton
 * \param rarcs
 *   Reverse arc index.
 *   May be passed as NULL to create a new arc index.
 * \returns \a rarcs if non-NULL, otherwise a new reverse arc index for \a fsm.
 */
gfsmReverseArcIndex *gfsm_automaton_reverse_arc_index(gfsmAutomaton *fsm, gfsmReverseArcIndex *rarcs);

//------------------------------
/** Free a reverse arc index.
 *  \param
 *  \param free_lists
 *    If true, associated arc-lists will be freed.
 *  \param free_arcs
 *    If true, arcs in associated arc-lists will be freed.
 */
void gfsm_reverse_arc_index_free(gfsmReverseArcIndex *rarcs, gboolean free_lists, gboolean free_arcs);


#endif /* _GFSM_ARCINDEX_H */
