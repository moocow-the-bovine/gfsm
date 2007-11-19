/*=============================================================================*\
 * File: gfsmImplBasicTypes.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: automaton implementation: basic: types
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

/** \file gfsmImplBasicTypes.h
 *  \brief Typedefs for basic automaton implementation
 */

#ifndef _GFSM_IMPL_BASIC_TYPES_H
#define _GFSM_IMPL_BASIC_TYPES_H

#include <gfsmArc.h>
#include <gfsmWeightMap.h>
#include <gfsmBitVector.h>

/*======================================================================
 * Implementation Types
 */

/// Basic automaton implementation using native linked-lists of arcs
typedef struct {
  GPtrArray      *state_arcs;      /**< array of ::gfsmArcNode* to list of outgoing arcs, indexed by ::gfsmStateId */
  gfsmBitVector  *state_is_valid;  /**< bit-vector of state-validity flags, indexed by ::gfsmStateId */
  gfsmBitVector  *state_is_final;  /**< bit-vector of state-finality flags, indexed by ::gfsmStateId */
  gfsmWeightMap  *finals;          /**< map from final ::gfsmStateId to final weights */
  gfsmStateId     root_id;         /**< ::gfsmStateId of root state, or ::gfsmNoState if no root is defined */
} gfsmImplBasic;

/// Arc iterator data for ::gfsmImplBasic automaton implementations
//typedef gfsmArcListNode* gfsmArcIterDataBasic;
typedef void* gfsmArcIterDataBasic;

#endif /* _GFSM_IMPL_BASIC_TYPES_H */
