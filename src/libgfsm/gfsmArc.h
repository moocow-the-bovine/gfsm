
/*=============================================================================*\
 * File: gfsmArc.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: arcs
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

/** \file gfsmArc.h
 *  \brief Arc (transition) definitions & utilities
 */

#ifndef _GFSM_ARC_H
#define _GFSM_ARC_H

#include <gfsmSemiring.h>

/// "Heavy" arc structure
typedef struct _gfsmArc {
  gfsmStateId       source;  /**< ID of source node */
  gfsmStateId       target;  /**< ID of target node */
  gfsmLabelId       lower;   /**< Lower label */
  gfsmLabelId       upper;   /**< Upper label */
  gfsmWeight        weight;  /**< arc weight */
} gfsmArc;

/// Enum type for builtin arc sort modes
typedef enum {
  gfsmASMNone,  ///< unsorted
  gfsmASMLower, ///< sort by (lower,upper,target)
  gfsmASMUpper, ///< sort by (upper,lower,target)
  gfsmASMWeight ///< sort by weight (refers to semiring)
} gfsmArcSortMode;

/// Typedef for arc-sorting parameters
typedef struct {
  gfsmArcSortMode  mode; /**< sorting mode */
  gfsmSemiring    *sr;   /**< semiring for weight-mode, otherwise ignored */
} gfsmArcSortData;

/// Type for identifying transducer "sides" (lower vs. upper)
typedef enum {
  gfsmLSBoth  = 0, ///< Both sides (lower and upper)
  gfsmLSLower = 1, ///< Lower side only
  gfsmLSUpper = 2  ///< Upper side only
} gfsmLabelSide;

/*======================================================================
 * Methods: Arcs: Constructors etc.
 */
/// \name Arcs: Constructors etc.
//@{

/** Create a new (empty) ::gfsmArc */
static inline
gfsmArc *gfsm_arc_new(void);

/** Initialize a ::gfsmArc (generic) */
static inline
gfsmArc *gfsm_arc_init(gfsmArc *a,
		       gfsmStateId src,
		       gfsmStateId dst,
		       gfsmLabelId lo,
		       gfsmLabelId hi,
		       gfsmWeight wt);

/** Convenience function to create and initialize a ::gfsmArc in a single call */
static inline
gfsmArc *gfsm_arc_new_full(gfsmStateId src, gfsmStateId dst, gfsmLabelVal lo, gfsmLabelVal hi, gfsmWeight wt);

/** Create an exact of a ::gfsmArc \a src */
static inline
gfsmArc *gfsm_arc_clone(gfsmArc *src);

/** Backwards-compatible alias for gfsm_arc_clone() */
#define gfsm_arc_copy(a) gfsm_arc_clone(a)

/** Destroy a ::gfsmArc \a a */
static inline
void gfsm_arc_free(gfsmArc *a);
//@}

/*======================================================================
 * Methods: Arcs: Accessors
 */
///\name Arc Accessors
//@{

/** Get the arc pointer for an arclist -- may be NULL */
#define gfsm_arclist_arc(al) \
  ((al) ? ((gfsmArc*)((al)->data)) : NULL)

/** Get source node of an arc -- may be gfsmNoState */
#define gfsm_arc_source(arcptr) ((arcptr)->source)

/** Get target node of an arc -- may be gfsmNoState */
#define gfsm_arc_target(arcptr) ((arcptr)->target)

/** Get lower label of an arc -- may be gfsmNoLabel */
#define gfsm_arc_lower(arcptr) ((arcptr)->lower)

/** Get upper label of an arc -- may be gfsmNoLabel */
#define gfsm_arc_upper(arcptr) ((arcptr)->upper)

/** Get weight of an arc -- may be gfsmNoWeight */
#define gfsm_arc_weight(arcptr) ((arcptr)->weight)

//@}

/*======================================================================
 * Methods: Arcs: Sorting
 */
///\name Arc Sorting
//@{

/** 3-way comparison function for ::gfsmArc, castable to GCompareDataFunc */
gint gfsm_arc_compare(gfsmArc *a1, gfsmArc *a2, gfsmArcSortData *sdata);

//@}

/*======================================================================
 * Methods: String utilities
 */
///\name String utilities
//@{

/** Return symbolic name of an arc-sort mode */
const gchar *gfsm_arc_sortmode_to_name(gfsmArcSortMode mode);

//@}

#include <gfsmArc.def>

#endif /* _GFSM_ARC_H */
