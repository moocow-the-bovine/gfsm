/*=============================================================================*\
 * File: gfsmImplOldTypes.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: automaton implementation: old: types
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

/** \file gfsmImplOldTypes.h
 *  \brief Typedefs for backwards-compatible automaton implementation
 */

#ifndef _GFSM_IMPL_OLD_TYPES_H
#define _GFSM_IMPL_OLD_TYPES_H

#include <gfsmArc.h>
#include <gfsmWeightMap.h>

/*======================================================================
 * Old Basic implementation type(s)
 */

/// "Heavy" automaton implementation using GSList linked-lists of arcs (backwards-compatible)
typedef struct {
  GArray             *states;      /**< vector of automaton states as ::gfsmState structs */
  gfsmWeightMap      *finals;      /**< map from final state-Ids to final weights */
  gfsmStateId         root_id;     /**< ID of root node, or gfsmNoState if not defined */
} gfsmImplOld;

/// Arc iterator data for ::gfsmImplOld automaton implementations
//typedef gfsmArcListOld* gfsmArcIterDataOld;
typedef struct {
  gfsmState          *state;       /**< pointer to state holding these arcs */
  gfsmArcListOld     *arcs;        /**< current node of this iterator */
} gfsmArcIterDataOld;


/// Type for a stored state
typedef struct {
  guint32  is_valid : 1;  /**< valid flag */
  guint32  is_final : 1;  /**< final flag */
  guint32  reserved : 30; /**< reserved */
  guint32  n_arcs;        /**< number of stored arcs for this state */
} gfsmStoredStateOld;

/// Type for a stored arc (no 'source' field)
typedef struct {
  gfsmStateId       target;  /**< ID of target node */
  gfsmLabelId       lower;   /**< Lower label */
  gfsmLabelId       upper;   /**< Upper label */
  gfsmWeight        weight;  /**< arc weight */
} gfsmStoredArcOld;

#endif /* _GFSM_IMPL_OLD_TYPES_H */
