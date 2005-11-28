
/*=============================================================================*\
 * File: gfsmArcIter.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: arc iterators
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

/** \file gfsmArcIter.h
 *  \brief Iterate over outgoing arcs of an automaton state.
 */

#ifndef _GFSM_ARCITER_H
#define _GFSM_ARCITER_H

#include <gfsmAutomaton.h>

/*======================================================================
 * Types: Arc iterators
 */
/// Abstract type for arc iterators
typedef struct _gfsmArcIter {
  gfsmAutomaton *fsm;    /**< fsm holding these arcs */
  gfsmState     *state;  /**< state holding these arcs */
  gfsmArcList   *arcs;   /**< pointer to node for current arc */
} gfsmArcIter;

/*======================================================================
 * Methods: Arc iterators: open/close
 */
///\name Arc Iterators: Constructors etc.
//@{
/** "Open" an arc iterator for the outgoing arcs from state with
 *  id \a stateid in \a fsm */
void gfsm_arciter_open(gfsmArcIter *aip, gfsmAutomaton *fsm, gfsmStateId stateid);

/** "Open" an arc iterator for the outgoing arcs from a state pointer
 *  into \a fsm */
void gfsm_arciter_open_ptr(gfsmArcIter *aip, gfsmAutomaton *fsm, gfsmState *stateptr);

/** Reset an arc iterator to the first outgoing arc */
void gfsm_arciter_reset(gfsmArcIter *aip);

/** close an arc-iterator pointer \a aip */
void gfsm_arciter_close(gfsmArcIter *aip);
/* does nothing */

//@}

/*======================================================================
 * Methods: Arc iterators: Accessors
 */
///\name Arc Iterators: Accessors
//@{

/** get a pointer to the arc associated with \a aip, or NULL
 *  if no such arc exists */
//gfsmArc *gfsm_arciter_arc(gfsmArcIter *aip);
#define gfsm_arciter_arc(aip) \
  (((aip)->arcs) ? ((gfsmArc*)((aip)->arcs->data)) : NULL)

/** check validity of an arc iterator pointer \a aip */
//gboolean gfsm_arciter_ok(gfsmArcIter *aip);
#define gfsm_arciter_ok(aip) ((aip) != NULL && (aip)->arcs != NULL)

/** Increment an arc iterator */
#define gfsm_arciter_next(aip) \
  ((aip)->arcs ? ((aip)->arcs = (aip)->arcs->next) : NULL)

/** Position an arc-iterator to the next arc with lower label \a lo */
void gfsm_arciter_seek_lower(gfsmArcIter *aip, gfsmLabelVal lo);

/** Position an arc-iterator to the next arc with upper label \a hi */
void gfsm_arciter_seek_upper(gfsmArcIter *aip, gfsmLabelVal hi);

/** Position an arc-iterator to the next arc
 *  with lower label \a lo and upper label \a hi.
 *  If either \a lo or \a hi is gfsmNoLabel, the corresponding label(s)
 *  will be ignored.
 */
void gfsm_arciter_seek_both(gfsmArcIter *aip, gfsmLabelVal lo, gfsmLabelVal hi);

/// Typedef for user-seek functions
typedef gboolean (*gfsmArcIterSeekFunc) (gfsmArcIter *aip, gpointer data);

/** Position an arc-iterator to the next arc for
 *  which (*seekfunc)(arciter,data)
 *  returns TRUE.
 */
void gfsm_arciter_seek_user(gfsmArcIter *aip,
			    gfsmArcIterSeekFunc seekfunc,
			    gpointer data);


/** Remove the arc referred to by a gfsmArcIter, and positions the iterator to the
 *  next arc, if any.
 */
void gfsm_arciter_remove(gfsmArcIter *aip);

//@}

#endif /* _GFSM_ARCITER_H */

