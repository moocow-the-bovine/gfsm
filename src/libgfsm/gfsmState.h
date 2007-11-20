
/*=============================================================================*\
 * File: gfsmState.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: states
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

/** \file gfsmState.h
 *  \brief Automaton state definitions & utilities
 */

#ifndef _GFSM_STATE_H
#define _GFSM_STATE_H

#include <gfsmArcList.h>

/*======================================================================
 * Types
 */

/// Automaton state structure
typedef struct {
  guint32       is_valid      : 1;  /**< whether this is a valid state */
  guint32       is_final      : 1;  /**< whether this is a final state */
  guint32       is_temp       : 1;  /**< whether this state should be freed on gfsm_state_close() */
  guint32       arc_list_temp : 1;  /**< whether arc list should be freed on gfsm_state_close() */
  guint32       arc_data_temp : 1;  /**< whether arc data should be freed on gfsm_state_close(): implies arc_list_temp=1 */
  guint32       unused        : 27; /**< reserved */
  gfsmArcList  *arcs;             /**< list of outgoing arcs */
} gfsmState;


/*======================================================================
 * Methods: gfsmState: Constructors etc.
 */
/// \name gfsmState: Constructors etc.
//@{
/** Create a new state (generic) */
static inline
gfsmState *gfsm_state_new_full(gboolean is_final, gfsmArcList *arcs);

/** Create a new state (empty) */
static inline
gfsmState *gfsm_state_new(void);

/** Copy an existing state */
static inline
gfsmState *gfsm_state_copy(gfsmState *dst, const gfsmState *src);

/** Clear an existing state */
static inline
void gfsm_state_clear(gfsmState *s);

/** Destroy a state */
static inline
void gfsm_state_free(gfsmState *s, gboolean free_arcs);

/** Close a state (generic) */
static inline
void gfsm_state_close(gfsmState *s);

//@}

/*======================================================================
 * Methods: gfsmState: Accessors
 */
///\name gfsmState: Accessors
//@{

/** Check if state is valid */
static inline
gboolean gfsm_state_is_ok(const gfsmState *s);

/** Check for final state */
static inline
gboolean gfsm_state_is_final(const gfsmState *s);

/** Set final state flag */
static inline
void gfsm_state_set_final(gfsmState *s, gboolean is_final);

/** Get number of outgoing arcs */
static inline
guint gfsm_state_out_degree(const gfsmState *s);

//@}

#include <gfsmState.def>

#endif /* _GFSM_STATE_H */
