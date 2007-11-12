
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

/// "Heavy" arc-list structure, data is a (gfsmArc*)
typedef GSList gfsmArcList;
/*
typedef struct _gfsmArcList {
gfsmArc               arc;     ///< current arc 
struct _gfsmArcList  *next;    ///< pointer to next arc 
} gfsmArcList;
*/

/// enum type for arc sorting
typedef enum {
  gfsmASMNone,  ///< unsorted
  gfsmASMLower, ///< sort by (lower,upper,target)
  gfsmASMUpper, ///< sort by (upper,lower,target)
  gfsmASMWeight ///< sort by weight (refers to semiring)
} gfsmArcSortMode;

/// typedef for arc-sorting parameters
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
#define gfsm_arc_new() g_new0(gfsmArc,1)

/** Initialize a ::gfsmArc (generic) */
gfsmArc *gfsm_arc_init(gfsmArc *a,
		       gfsmStateId src,
		       gfsmStateId dst,
		       gfsmLabelId lo,
		       gfsmLabelId hi,
		       gfsmWeight wt);

/** Convenience macro to create and initialize a ::gfsmArc in a single call */
#define gfsm_arc_new_full(src,dst,lo,hi,wt) \
  gfsm_arc_init(g_malloc(sizeof(gfsmArc)),(src),(dst),(lo),(hi),(wt))

/** Create an exact of a ::gfsmArc \a src  */
gfsmArc *gfsm_arc_copy(gfsmArc *src);

/** Destroy a ::gfsmArc \a a */
#define gfsm_arc_free(a) g_free(a)
//@}

/*======================================================================
 * Methods: Arc Lists: Constructors etc.
 */
/// \name Arc Lists: Constructors etc.
//@{
/** Allocate space for a new gfsmArcList node. */
#define gfsm_arclist_alloc() g_slist_alloc()

/** Create a new arc-list node for \a arc, prepending it to \a nxt
 *  \returns a pointer to the new 1st element of the arclist
 */
#define gfsm_arclist_prepend(al,arc) g_slist_prepend(al,arc)

/** Insert an arc into a (possibly sorted) arclist.
 *  \returns a pointer to the (possibly new) 1st element of the arclist */
gfsmArcList *gfsm_arclist_insert(gfsmArcList *al,
				 gfsmStateId  src,
				 gfsmStateId  dst,
				 gfsmLabelVal lo,
				 gfsmLabelVal hi,
				 gfsmWeight   wt,
				 gfsmArcSortData *sdata);

/** Insert a single arc-link into a (possibly sorted) arclist.
 *  \returns a pointer to the (possibly new) 1st element of the arclist */
gfsmArcList *gfsm_arclist_insert_link(gfsmArcList *al,
				      gfsmArcList *link,
				      gfsmArcSortData *sdata);

/** Create a new arc-list node (full) */
#define gfsm_arclist_new_full(src,dst,lo,hi,wt,nxt) \
   (gfsm_arclist_prepend(nxt,gfsm_arc_new_full(src,dst,lo,hi,wt)))

/** Create and return a (deep) copy of an existing arc-list */
gfsmArcList *gfsm_arclist_copy(gfsmArcList *src);

/** Destroy an arc-list node and all subsequent nodes */
void gfsm_arclist_free(gfsmArcList *al);

/** Free one node of an arc-list */
#define gfsm_arclist_free1(al) g_free(al)
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

/* Get pointer to next arc -- may be NULL */
//#define gfsm_arc_next(arcptr) ((arcptr)->next)

//@}


/*======================================================================
 * Methods: Arc Lists: Utilities
 */
///\name ArcList: Utilities
//@{
/** Get length of an arc-list \a al (linear time) */
#define gfsm_arclist_length(al) g_slist_length(al)

/** 3-way comparison on arcs */
gint gfsm_arc_compare(gfsmArc *a1, gfsmArc *a2, gfsmArcSortData *sdata);

/** Sort an arclist \a al with data \a sdata. */
#define gfsm_arclist_sort(al,sdata) \
  g_slist_sort_with_data(al, (GCompareDataFunc)gfsm_arc_compare, sdata)
//@}


/*======================================================================
 * Methods: String utilities
 */
///\name String utilities
//@{
/** Return symbolic name of an arc-sort mode */
gchar *gfsm_arc_sortmode_to_name(gfsmArcSortMode mode);
//@}


#endif /* _GFSM_ARC_H */
