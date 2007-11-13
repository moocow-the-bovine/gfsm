
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


/// Typecast-safe 'hard-linked' list of ::gfsmArc
typedef struct gfsmArcListLinked_ {
  gfsmArc                      arc;    /**< first (current) arc */
  struct gfsmArcListLinked_  *next;    /**< pointer to next arc */
} gfsmArcListLinked;

/// Array of ::gfsmArc as a pair of pointer bounds into a ::gfsmArc array.
typedef struct {
  gfsmArc  *beg;   /**< pointer to first arc in this list */
  gfsmArc  *end;   /**< pointer to the first address \b not containing an arc in this list (last+1) */
} gfsmArcListArray;

/// Enum of arc-list types
typedef enum {
  gfsmALTLinked, /**< data.l is ::gfsmArcListLinked* */
  gfsmALTArray   /**< data.a is ::gfsmArcListArray   */
} gfsmArcListType;

/// Generic arc-list data type
typedef union {
  gfsmArcListLinked *l;
  gfsmArcListArray   a;
} gfsmArcListData;

/// Generic arc-list flags
typedef struct {
  guint32 altype      :  4;   /**< type of this arc-list, cast to ::gfsmArcListType */
  guint32 sort_mode   :  4;   /**< sort mode of this arc-list, cast to ::gfsmArcSortMode */
  guint32 local_arcs  :  1;   /**< whether this arc-list owns its own arc data */
  guint32 unused      : 22;   /**< reserved */
} gfsmArcListFlags;

/// Generic arc-list type
typedef struct {
  gfsmArcListFlags flags; /**< list-local flags */
  gfsmArcListData   data; /**< list data */
} gfsmArcList;

//@}

/*======================================================================
 * Types: ArcList (alt)
 */

/// Enum of arc-list types
typedef enum {
  gfsmALTLinked, /**< data.l is ::gfsmArcListLinked* */
  gfsmALTArray   /**< data.a is ::gfsmArcListArray   */
} gfsmArcListType;

/// base type
typedef struct {
  guint32 type        :  4;   /**< type of this arc-list, cast to ::gfsmArcListType */
  guint32 sort_mode   :  4;   /**< sort mode of this arc-list, cast to ::gfsmArcSortMode */
  guint32 local_arcs  :  1;   /**< whether this arc-list owns its own arc data */
  guint32 unused      : 23;   /**< reserved */
} gfsmArcListBase;

/// arc linked list: node
typedef struct gfsmArcNode_ {
  gfsmArc                 a;
  struct gfsmArcNode_ *next;
} gfsmArcNode;

/// arc array: struct
typedef struct {
  gfsmArc *begin;
  gfsmArc *end;
} gfsmArcArray;


/// arc list: generic
typedef struct {
  gfsmArcListBase base;
  union {
    gfsmArcListNode *l;
    gfsmArcArray     a;
  } arcs;
  union {
    gfsmArcListNode *l;
    gfsmArc         *a;
  } cur;
  struct gfsmAutomaton_ *fsm;
  gfsmStateId            qid;
}

/*======================================================================
 * Types: ArcIter
 */
///\name ArcIter variants
//@{

/// Type for generic arc iteration data
typedef union {
  gfsmArcListLinked *l;  /**< for ::gfsmArcListLinked */
  gfsmArc           *a;  /**< for ::gfsmArcListArray  */
} gfsmArcIterData;

/// Type for generic arc iterators
typedef struct {
  gfsmArcList           arcs;  /**< underlying arc-list */
  gfsmArcIterData        cur;  /**< current position in the arc-list */
  struct gfsmAutomaton_ *fsm;  /**< automaton from which this ArcIter was drawn, or NULL */
  gfsmStateId            qid;  /**< ID of the state from which these arcs were drawn, or ::gfsmNoState */
} gfsmArcIter;

//@}

/*======================================================================
 * Methods: Arc Lists: Constructors etc.
 */
/// \name Arc Lists: Constructors etc.
//@{
/** Create and return a new empty ::gfsmArcList */
static inline
gfsmArcList *gfsm_arclist_new(void);

/** Create and return a new ::gfsmArcList, specifying initial flags */
static inline
gfsmArcList *gfsm_arclist_new_with_flags(gfsmArcListFlags flags);

/** Create and return a new ::gfsmArcList, specifying initial flags and data */
static inline
gfsmArcList *gfsm_arclist_new_with_data(gfsmArcListFlags flags, gfsmArcListData data);

/** Copy all arcs in \a src to \a dst, without changing type of \a dst.
 *  May set \a dst.flags.is_tmp to \c TRUE if copying was required.
 *  \param src source arc list
 *  \param dst destination arc list
 *  \returns modified \a dst
 */
static inline
gfsmArcList *gfsm_arclist_copy(gfsmArcList *dst, gfsmArcList *src);

/** Create and return a (deep) copy of an existing arc-list */
static inline
gfsmArcList *gfsm_arclist_clone(gfsmArcList *src);

/** Destroy an arc-list node and all subsequent nodes */
void gfsm_arclist_free(gfsmArcList *al);


/** Insert a new arc into a (possibly sorted) arclist. */
void gfsm_arclist_insert(gfsmArcList *al,
			 gfsmStateId  src,
			 gfsmStateId  dst,
			 gfsmLabelVal lo,
			 gfsmLabelVal hi,
			 gfsmWeight   wt,
			 gfsmArcSortData *sdata);

//@}


/*======================================================================
 * Methods: Arc Lists: Utilities
 */
///\name ArcList: Utilities
//@{

/** Get the 'current' arc pointer for an arclist -- may be NULL */
gfsmArc *gfsm_arclist_arc(gfsmArcList *al);

/** Get length of a ::gfsmArcList \a al */
guint gfsm_arclist_length(gfsmArcList *al);

/** Sort a ::gfsmArcList \a al with ::gfsmArcSortData \a sdata */
void gfsm_arclist_sort(gfsmArcList *al, gfsmArcSortData *sdata);

/** Sort a ::gfsmArcList \a al with user-defined comparison and data */
void gfsm_arclist_sort_full(gfsmArcList *al, GCompareDataFunc cmpfunc, gpointer data);

//@}

#endif /* _GFSM_ARC_H */
