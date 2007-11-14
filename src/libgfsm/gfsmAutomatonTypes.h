
/*=============================================================================*\
 * File: gfsmAutomatonTypes.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: typedefs for various automaton flavors
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

/** \file gfsmAutomatonTypes.h
 *  \brief Generic automaton API: structures and typedefs
 */

#ifndef _GFSM_AUTOMATON_TYPES_H
#define _GFSM_AUTOMATON_TYPES_H

#include <gfsmImplOldTypes.h>
#include <gfsmImplBasicTypes.h>

/*======================================================================
 * Implementation Dispatch
 */
///\name Implementation Dispatch
//@{

/// Known automaton implementation classes
/** Used to resolve \a impl.data for a ::gfsmAutomatonImpl \a impl */
typedef enum {
  gfsmACUnknown,            /**< unknown class; should never happen in practice */
  gfsmACOld,                /**< backwards-compatible implementation, see ::gfsmImplOld \b TODO */
  gfsmACBasic,              /**< basic implementation; see ::gfsmBasicImpl \b TODO */
#if 0
  gfsmACArcTable,           /**< (read-only) arc table implementation; see ::gfsmImplArcTable \b TODO */
  gfsmACFullTable,          /**< (read-only) full table implementation; see ::gfsmImplFullTable \b TODO */
  gfsmACVirtual,            /**< user-defined full-virtual implementation; see ::gfsmImplVirtual \b TODO */
#endif
  gfsmACNClasses            /**< not really an implementation class at all */
} gfsmAutomatonClass;

/// Automaton implementation data union type, for comfortable typecasting
/** Use the ::gfsmAutomaton.itype field to determine which of these union fields is the 'right' one */
typedef union {
  gpointer             p;   /**< Generic (void*) */
  gfsmImplOld       *old;   /**< backwards-compatible implementation */
  gfsmImplBasic       *b;   /**< basic implementation */
#if 0
  gfsmImplArcTable   *at;   /**< (read-only) arc table implementation */
  gfsmImplFulTable   *ft;   /**< (read-only) full table implementation */
  gfsmImplVirtual     *v;   /**< user-defined full-virtual implementation */
#endif
} gfsmAutomatonImpl;

//@}


/*======================================================================
 * Generic Automata
 */
///\name Generic Automata
//@{

/// Status flags for generic automata
/** \todo multiply binary property flags out: add 'xKnown' flag for each property 'x' */
typedef struct {
  guint32 is_transducer     : 1;       /**< whether this automaton is a transducer */
  guint32 is_weighted       : 1;       /**< whether this automaton is weighted */
  guint32 sort_mode         : 4;       /**< sort-mode (cast to gfsmArcSortMode) */
  guint32 is_deterministic  : 1;       /**< whether fsm is known to be deterministic */
  guint32 unused            : 25;      /**< reserved */
} gfsmAutomatonFlags;

/// Generic automaton type
typedef struct
{
  gfsmAutomatonFlags   flags;    /**< automaton flags */
  gfsmAutomatonClass   itype;    /**< implementation class */
  gfsmAutomatonImpl    impl;     /**< underlying implementation, cast by \a switch{} over \a itype  */
} gfsmAutomaton;

//@}

/*======================================================================
 * Generic Arc Iterators
 */
///\name Generic Arc Iterators
//@{

/// Arc-iterator implementation data union type, for comfortable typecasting
/** Use the ::gfsmAutomaton.itype field to determine which of these union fields is the 'right' one */
typedef union {
  struct {
    gpointer p1;                /**< some data */
    gpointer p2;                /**< some more data */
  }                       pp;   /**< for user-defined iterator types? */
  gfsmArcIterDataOld     old;   /**< for ::gfsmImplOld */
  gfsmArcIterDataBasic     b;   /**< for ::gfsmImplBasic */
#if 0
  gfsmArcIterDataTable    at;   /**< for ::gfsmImplTable (TODO) */
  gfsmArcIterDataVirtual   v;   /**< for ::gfsmImplVirtual (TODO) */
#endif
} gfsmArcIterData;

/// Generic arc iterator type
struct {
  gfsmAutomaton   *fsm;           /**< automaton which 'owns' these arcs */
  gfsmStateId      qid;           /**< state in \a fsm which 'owns' these arcs */
  gfsmArcIterData data;           /**< implementation-dependent data */
} gfsmArcIter;

/// Typedef for user-defined seek functions for ::gfsmArcIter
typedef gboolean (*gfsmArcIterSeekFunc) (gfsmArcIter *aip, gpointer data);

//@}


#endif /* _GFSM_AUTOMATON_TYPES_H */
