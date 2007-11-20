
/*=============================================================================*\
 * File: gfsmArcList.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: arc lists
 *  + formerly defined in gfsmArc.h
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

/** \file gfsmArcList.h
 *  \brief Definitions & utilities for arc lists, <b>Deprecated</b>
 *  \detail
 *  \deprecated
 *    in favor of gfsm_automaton_add_arc() and ::gfsmArcIter interface
 *  \see gfsmAutomaton.h, gfsmArcIter.h
 */

#ifndef _GFSM_ARC_LIST_H
#define _GFSM_ARC_LIST_H

#include <gfsmArc.h>

/// "Heavy" arc-list structure using glib GSList, data is a ::gfsmArc*
typedef GSList gfsmArcList;


/*======================================================================
 * Methods: Arc List: Constructors etc.
 */
/// \name Arc List: Constructors etc.
//@{

/** Create a new arc-list node for \a arc, prepending it to \a nxt
 *  \returns a pointer to the new 1st element of the arclist
 *  \deprecated in favor of gfsm_automaton_add_arc(), gfsm_arciter_insert()
 */
static inline
gfsmArcList *gfsm_arclist_prepend(gfsmArcList *al, gfsmArc *a);


/** Insert an arc into a (possibly sorted) arclist.
 *  \param al ::gfsmArcList into which a new arc is to be inserted
 *  \param src source state id for the new arc
 *  \param dst target state id for the new arc
 *  \param lo  lower label for the new arc
 *  \param hi  upper label for the new arc
 *  \parm  wt  weight for the new arc
 *  \param sdata sort data for 'smart' sorted insertion
 *  \returns a pointer to the (possibly new) 1st node of the arc list
 *  \deprecated in favor of gfsm_automaton_add_arc(), gfsm_arciter_insert()
 */
static inline
gfsmArcList *gfsm_arclist_insert(gfsmArcList *al,
				 gfsmStateId  src,
				 gfsmStateId  dst,
				 gfsmLabelVal lo,
				 gfsmLabelVal hi,
				 gfsmWeight   wt,
				 gfsmArcSortData *sdata);

/** Allocate and return a new arc-list node */
static inline
gfsmArcList *gfsm_arclist_new_full(gfsmStateId  src,
				   gfsmStateId  dst,
				   gfsmLabelVal lo,
				   gfsmLabelVal hi,
				   gfsmWeight   wt,
				   gfsmArcList  *nxt);

/** Insert a single arc-link into a (possibly sorted) arclist.
 *  \param al   arc list into which \a link is to be inserted
 *  \param link arc list node to insert
 *  \param sdata sort data for 'smart' sorted insertion
 *  \returns a pointer to the (possibly new) 1st element of the arclist
 *  \deprecated in favor of gfsm_automaton_add_arc(), gfsm_arciter_insert()
 */
static inline
gfsmArcList *gfsm_arclist_insert_link(gfsmArcList *al,
				      gfsmArcList *link,
				      gfsmArcSortData *sdata);


/** Create and return a (deep) copy of an existing arc-list */
static inline
gfsmArcList *gfsm_arclist_clone(gfsmArcList *src);

/** Destroy an arc-list node and all subsequent nodes */
static inline
void gfsm_arclist_free(gfsmArcList *al);

/** Free one node of an arc-list */
//#define gfsm_arclist_free1(al) g_free(al)
//@}

/*======================================================================
 * Methods: Arc List: Accessors
 */
///\name Arc List: Accessors
//@{

/** Get the arc pointer for an arclist -- may be \c NULL
 *  \deprecated in favor of ::gfsmArcIter interface
 *  \see gfsmArcIter.h
 */
#define gfsm_arclist_arc(al) \
  ((al) ? ((gfsmArc*)((al)->data)) : NULL)

//@}


/*======================================================================
 * Methods: Arc List: Utilities
 */
///\name Arc List: Utilities
//@{

/** Get length of an arc-list \a al (linear time) */
#define gfsm_arclist_length(al) g_slist_length(al)

/** Sort an arclist \a al using one of the builtin sort modes as specified by \a sdata.
 *  \param al    arc list to sort
 *  \param sdata sort data for builtin comparison
 *  \returns pointer to the new head of the sorted arc list
 */
static inline
gfsmArcList *gfsm_arclist_sort(gfsmArcList *al, gfsmArcSortData *sdata);

/** Sort an arclist \a al using a user-defined arc comparison function.
 *  \param al       arc list to sort
 *  \param cmpfunc  3-way comparison function on ::gfsmArc* for sorting
 *  \param data     additional data for \a cmpfunc
 *  \returns pointer to the new head of the sorted arc list
 */
static inline
gfsmArcList *gfsm_arclist_sort_full(gfsmArcList *al, GCompareDataFunc cmpfunc, gpointer data);
//@}

#include <gfsmArcList.def>

#endif /* _GFSM_ARC_LIST_H */
