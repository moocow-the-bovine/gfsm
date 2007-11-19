
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

/*======================================================================
 * Types: Lists
 */
///\name ArcList variants
//@{

/// GSLis* of ::gfsmArc
typedef GSList gfsmArcListOld;

/// Typecast-safe 'hard-linked' list of ::gfsmArc
/** \todo implement ::gfsmArcListLinked_ methods in gfsmArcList.h */
typedef struct gfsmArcListLinked_ {
  gfsmArc                      arc;    /**< first (current) arc */
  struct gfsmArcListLinked_  *next;    /**< pointer to next arc */
} gfsmArcListLinked;

//@}

/*======================================================================
 * Methods: gfsmArcListOld
 */
/// \name gfsmArcListOld: Constructors etc.
//@{

/** Create and return a new ::gfsmArcListOld node, including arc data
 *  \param src ID of source state
 *  \param dst ID of target state
 *  \param lo  Lower label
 *  \param hi  Upper label
 *  \param wt  Arc weight
 *  \param nxt next arc list node
 *  \returns head of a new ::gfsmArcListOld for new arc data
 */
static inline
gfsmArcListOld *gfsm_arclist_new_full_old(gfsmStateId     src,
					  gfsmStateId     dst,
					  gfsmLabelVal    lo,
					  gfsmLabelVal    hi,
					  gfsmWeight      wt,
					  gfsmArcListOld *nxt);

/** Create a deep exact copy of a ::gfsmArcListOld \a src.
 *  \param src arc list to copy
 *  \returns head of a deep exact copy of \a src
 */
static inline
gfsmArcListOld *gfsm_arclist_clone_old(gfsmArcListOld *src);

/** Clear an arc list, freeing arc data
 * \returns NULL (new head of list)
 */
static inline
gfsmArcListOld *gfsm_arclist_free_old(gfsmArcListOld *al);

/** Get length of an arc-list */
static inline
guint gfsm_arclist_length_old(gfsmArcListOld *al);

/** (low-level): insert a link into a (possibly sorted) arc-list
 *  \returns new head of the list
 */
static inline
gfsmArcListOld *gfsm_arclist_insert_link_old(gfsmArcListOld *al, gfsmArcListOld *link, gfsmArcSortData *sdata);

/** Sort an arc list with user-specified comparison function and data
 *  \returns new head of the list
 */
static inline
gfsmArcListOld *gfsm_arclist_sort_full_old(gfsmArcListOld *al, GCompareDataFunc cmpfunc, gpointer data);

/** Sort an arc list using one of the builtin comparisons
 * \returns new head of the list
 */
static inline
gfsmArcListOld *gfsm_arclist_sort_old(gfsmArcListOld *al, gfsmArcSortData *sdata);

//@}


//-- inline definitions
#include <gfsmArcList.def>

#endif /* _GFSM_ARC_LIST_H */
