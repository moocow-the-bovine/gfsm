
/*=============================================================================*\
 * File: gfsmState.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: states
 *
 * Copyright (c) 2004 Bryan Jurish.
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

/** \file gfsmState.h
 *  \brief Automaton state definitions & utilities
 */

#ifndef _GFSM_STATE_H
#define _GFSM_STATE_H

#include <gfsmCommon.h>
#include <gfsmArc.h>

/*======================================================================
 * Types
 */

/// Automaton state structure
typedef struct {
  gboolean      is_valid : 1; /**< whether this is a valid state */
  gboolean      is_final : 1; /**< whether this is a final state */
  gfsmArcList  *arcs;         /**< list of outgoing arcs */
} gfsmState;

/*======================================================================
 * Methods: gfsmState: Constructors etc.
 */
/// \name gfsmState: Constructors etc.
//@{
/** Create a new state (generic) */
gfsmState *gfsm_state_new_full(gboolean is_final, gfsmArcList *arcs);

/** Create a new state (empty) */
#define gfsm_state_new() gfsm_state_new_full(FALSE, NULL)

/** Copy an existing state */
gfsmState *gfsm_state_copy(gfsmState *dst, const gfsmState *src);

/** Clear an existing state */
void gfsm_state_clear(gfsmState *s);

/** Destroy a state */
void gfsm_state_free(gfsmState *s, gboolean free_arcs);
//@}

/*======================================================================
 * Methods: gfsmState: Accessors
 */
///\name gfsmState: Accessors
//@{
/** Check if state is valid */
#define gfsm_state_is_ok(s) (s && s->is_valid)

/** Check for final state */
#define gfsm_state_is_final(s) (s && s->is_final)

/** Set final state flag */
#define gfsm_state_set_final(s,b) (s->is_final = b)

/** Get number of outgoing arcs */
//#define gfsm_state_out_degree(s) g_slist_length(s->arcs)
#define gfsm_state_out_degree(s) gfsm_arclist_length(s->arcs)

//@}


#endif /* _GFSM_STATE_H */
