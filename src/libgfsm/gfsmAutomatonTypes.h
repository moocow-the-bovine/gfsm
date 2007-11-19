
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

#include <gfsmVersion.h>
#include <gfsmState.h>
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
  gfsmACArcTable,           /**< (read-only) arc table implementation; see ::gfsmImplArcTable \b TODO */
  gfsmACFullTable,          /**< (read-only) full table implementation; see ::gfsmImplFullTable \b TODO */
  gfsmACVirtual,            /**< user-defined full-virtual implementation; see ::gfsmImplVirtual \b TODO */
  gfsmACNClasses            /**< not really an implementation class at all */
} gfsmAutomatonClass;

/// Automaton implementation data union type, for comfortable typecasting
/** Use the ::gfsmAutomaton.itype field to determine which of these union fields is the 'right' one */
typedef union {
  gpointer             p;   /**< Generic (void*) */
  gfsmImplOld       *old;   /**< backwards-compatible implementation */
  gfsmImplBasic       *b;   /**< basic implementation */
  struct gfsmImplArcTable_   *at;   /**< (read-only) arc table implementation */
  struct gfsmImplFulTable_   *ft;   /**< (read-only) full table implementation */
  struct gfsmImplVirtual_     *v;   /**< user-defined full-virtual implementation */
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
  gfsmSemiring        *sr;       /**< underlying semiring */
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
typedef struct {
  gfsmAutomaton   *fsm;           /**< automaton which 'owns' these arcs */
  gfsmStateId      qid;           /**< state in \a fsm which 'owns' these arcs */
  gfsmArcIterData data;           /**< implementation-dependent data */
} gfsmArcIter;

/// Typedef for user-defined seek functions for ::gfsmArcIter
typedef gboolean (*gfsmArcIterSeekFunc) (gfsmArcIter *aip, gpointer data);

//@}

/*======================================================================
 * Binary Automaton I/O
 */
///\name Binary Automaton I/O
//@{

/// Common header info struct for binary automaton I/O
typedef struct {
  gchar              magic[16];      /**< magic header string "gfsm_automaton" */
  gfsmVersionInfo    version;        /**< gfsm version which created the stored file */
  gfsmVersionInfo    version_min;    /**< minimum libgfsm version required to load the file (optional) */
  gfsmAutomatonFlags flags;          /**< automaton flags (optional) */
  gfsmStateId        root_id;        /**< Id of root node (optional) */
  gfsmStateId        n_states;       /**< number of stored states (optional) */
  gfsmStateId        n_arcs;         /**< number of stored arcs (v0.0.2 .. v0.0.7) (optional) */
  guint32            srtype;         /**< semiring type (cast to ::gfsmSRType, recommended) */
  guint32            itype     :  8; /**< automaton implementation class (cast to ::gfsmAutomatonClass, required) */
  guint32            reserved1 : 24; /**< reserved */
  guint32            reserved2;      /**< reserved */
  guint32            reserved3;      /**< reserved */
} gfsmAutomatonHeader;

/// Default type for a stored state
typedef struct {
  guint32  is_valid : 1;  /**< valid flag */
  guint32  is_final : 1;  /**< final flag */
  guint32  unused   : 30; /**< reserved */
  guint32  n_arcs;        /**< number of stored arcs for this state */
} gfsmStoredState;

/// Default type for a stored arc (no 'source' field)
typedef struct {
  gfsmStateId       target;  /**< ID of target node */
  gfsmLabelId       lower;   /**< Lower label */
  gfsmLabelId       upper;   /**< Upper label */
  gfsmWeight        weight;  /**< arc weight */
} gfsmStoredArc;

//@}

#endif /* _GFSM_AUTOMATON_TYPES_H */
