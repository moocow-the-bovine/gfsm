
/*=============================================================================*\
 * File: gfsmCompound.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: basic compound types
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

#ifndef _GFSM_COMPOUND_H
#define _GFSM_COMPOUND_H

#include <glib.h>
#include <gfsmCommon.h>

/*======================================================================
 * Compound Types
 */
/// Typedef for (lower,upper) label pairs: really just a wrapper for some bit operations
typedef guint gfsmLabelPair;

/// Typedef for matched pairs of state-ids (used by algebraic operations)
typedef struct {
  gfsmStateId  id1;
  gfsmStateId  id2;
} gfsmStatePair;


/*======================================================================
 * LabelPair: Methods
 */
///\name gfsmLabelPair Methods
//@{
/** Create a "new" gsfmLabelPair */
#define gfsm_labelpair_new(lo,hi) ((((guint)(hi))<<16)|((guint)(lo)))

/** Get lower label of of a label-pair */
#define gfsm_labelpair_lower(lp) ((gfsmLabelId)((lp)&0xffff))

/** Get upper label of of a label-pair */
#define gfsm_labelpair_upper(lp) ((gfsmLabelId)(((lp)>>16)&0xffff))

/** hash function for gfsmLabelPair */
#define gfsm_labelpair_hash g_direct_hash

/** equal function for gfsmLabelPair */
#define gfsm_labelpair_equal g_direct_equal

/** comparison function for gfsmLabelPair */
gint gfsm_labelpair_compare(gfsmLabelPair lp1, gfsmLabelPair lp2);

/** dummy data-comparison function for gfsmLabelPair */
gint gfsm_labelpair_compare_with_data(gfsmLabelPair lp1, gfsmLabelPair lp2, gpointer data);
//@}

/*======================================================================
 * Methods: gfsmStatePair
 */
///\name gfsmStatePair Methods
//@{

/** Create a new gfsmStatePair */
gfsmStatePair *gfsm_statepair_new(gfsmStateId id1, gfsmStateId id2);

/** Clone an existing gfsmStatePair */
gfsmStatePair *gfsm_statepair_clone(gfsmStatePair *sp);

/** Free a gfsmStatePair */
#define gfsm_statepair_free(sp) g_free(sp)
//gfsmStatePair *gfsm_statepair_free(gfsmStatePair *sp);

/** Get a more or less sensible hash value from a state pair */
guint gfsm_statepair_hash(gfsmStatePair *sp);

/** Comparison function for gfsmStatePair */
gint gfsm_statepair_compare(const gfsmStatePair *sp1, const gfsmStatePair *sp2);

/** Equality predicate for gfsmStatePair */
gboolean gfsm_statepair_equal(const gfsmStatePair *sp1, const gfsmStatePair *sp2);

//@}


#endif /* _GFSM_COMPOUND_H */
