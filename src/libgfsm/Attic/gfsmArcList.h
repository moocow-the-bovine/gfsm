
/*=============================================================================*\
 * File: gfsmArcList.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: arc lists
 *  + formerly defined in gfsmArc.h
 *
 * Copyright (c) 2004-2007 Bryan Jurish
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

/** \file gfsmArcList.h
 *  \brief Arc-lists of various flavors
 */

#ifndef _GFSM_ARC_LIST_H
#define _GFSM_ARC_LIST_H

#include <gfsmArc.h>

/// List of arcs as a GSList whose \a data field is a ::gfsmArc*.
typedef GSList gfsmArcListGSList;

/// Typecast-safe "hard-linked" list of ::gfsmArc
typedef struct gfsmArcListLinked_ {
  gfsmArc                     arc;     /**< first (current) arc */
  struct gfsmArcListLinked_  *next;    /**< pointer to next arc */
} gfsmArcListLinked;

/// Array of ::gfsmArc as a GArray
typedef GArray gfsmArcListGArray;

/// Array of ::gfsmArc as a pair of pointer bounds into a ::gfsmArc array.
typedef struct {
  gfsmArc *begin;  /**< first (current) arc */
  gfsmArc *end;    /**< last arc + 1 */
} gfsmArcListPtrs;

/// Enum of arc-list types
typedef enum {
  gfsmALTGSList,
  gfsmALTLinked,
  gfsmALTGArray,
  gfsmALTPtrs
} gfsmArcListType;

/// Generic arc-list data type
typedef union {
  gfsmArcListGSList *gslist;
  gfsmArcListLinked *linked;
  gfsmArcListGArray *garray;
  gfsmArcListPtrs    ptrs;
} gfsmArcListData;

/// Generic arc-list flags
typedef struct {
  guint32 altype      :  4;   /**< type of this arc-list, cast to ::gfsmArcListType */
  guint32 sort_mode   :  4;   /**< sort mode of this arc-list, cast to ::gfsmArcSortMode */
  guint32 is_tmp      :  1;   /**< whether the data in this arc-list is a temporary */
  guint32 unused      : 23;   /**< reserved */
} gfsmArcListFlags;

/// Generic arc-list type
typedef struct {
  gfsmArcListFlags flags; /**< list-local flags */
  gfsmArcListData   data; /**< list data */
} gfsmArcList;

/*======================================================================
 * Methods: Arc Lists: Constructors etc.
 */
/// \name Arc Lists: Constructors etc.
//@{
/** Create and return a new empty ::gfsmArcList */
static inline gfsm_arclist_new(void) { return g_new0(gfsmArcList,1); }

/** Create and return a new ::gfsmArcList */
static inline gfsmArcList *gfsm_arclist_new_full(gfsmArcListFlags flags, gfsmArcListData data) {
  gfsmArcList *al = g_new0(gfsmArcList,1);
  al->flags = flags;
  al->data  = data;
  return al;
}

/** Insert a new arc into a (possibly sorted) arclist. */
void gfsm_arclist_insert(gfsmArcList *al,
			 gfsmStateId  src,
			 gfsmStateId  dst,
			 gfsmLabelVal lo,
			 gfsmLabelVal hi,
			 gfsmWeight   wt,
			 gfsmArcSortData *sdata);

/** Create and return a (deep) copy of an existing arc-list */
gfsmArcList *gfsm_arclist_clone(gfsmArcList *src);

/** Destroy an arc-list node and all subsequent nodes */
void gfsm_arclist_free(gfsmArcList *al);
//@}


/*======================================================================
 * Methods: Arc Lists: Utilities
 */
///\name ArcList: Utilities
//@{

/** Get the "current" arc pointer for an arclist -- may be NULL */
gfsmArc *gfsm_arclist_arc(gfsmArcList *al);

/** Get length of a ::gfsmArcList \a al */
guint gfsm_arclist_length(gfsmArcList *al);

/** Sort a ::gfsmArcList \a al with ::gfsmArcSortData \a sdata */
void gfsm_arclist_sort(gfsmArcList *al, gfsmArcSortData *sdata);

/** Sort a ::gfsmArcList \a al with user-defined comparison and data */
void gfsm_arclist_sort_full(gfsmArcList *al, GCompareDataFunc cmpfunc, gpointer data);

//@}

#endif /* _GFSM_ARC_H */
