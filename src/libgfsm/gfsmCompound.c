/*=============================================================================*\
 * File: gfsmCompound.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: common definitions
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

#include <gfsmCompound.h>
#include <gfsmMem.h>
#include <stdlib.h>


/*======================================================================
 * Label Pair: Methods
 */
/*--------------------------------------------------------------
 * labelpair_compare()
 */
gint gfsm_labelpair_compare(gfsmLabelPair lp1, gfsmLabelPair lp2)
{
  gfsmLabelId
    lo1 = gfsm_labelpair_lower(lp1),
    lo2 = gfsm_labelpair_lower(lp2),
    hi1 = gfsm_labelpair_upper(lp1),
    hi2 = gfsm_labelpair_upper(lp2);
  return (lo1 < lo2          ? -1
	  : (lo1 > lo2       ?  1
	     : (hi1 < hi2    ? -1
		: (hi1 > hi2 ?  1
		   :            0))));
}

/*--------------------------------------------------------------
 * labelpair_compare_with_data()
 */
gint gfsm_labelpair_compare_with_data(gfsmLabelPair lp1, gfsmLabelPair lp2, gpointer data)
{ return gfsm_labelpair_compare(lp1,lp2); }


/*======================================================================
 * Methods: gfsmStatePair
 */

/*--------------------------------------------------------------
 * statepair_new()
 */
gfsmStatePair *gfsm_statepair_new(gfsmStateId id1, gfsmStateId id2)
{
  gfsmStatePair *sp = g_new(gfsmStatePair,1);
  sp->id1 = id1;
  sp->id2 = id2;
  return sp;
}

/*--------------------------------------------------------------
 * statepair_clone()
 */
gfsmStatePair *gfsm_statepair_clone(gfsmStatePair *sp)
{
  return (gfsmStatePair*)gfsm_mem_dup_n(sp, sizeof(gfsmStatePair));
}

/*--------------------------------------------------------------
 * statepair_free()
 */
/*
gfsmStatePair *gfsm_statepair_free(gfsmStatePair *sp)
{ g_free(sp); }
*/

/*--------------------------------------------------------------
 * statepair_hash()
 */
guint gfsm_statepair_hash(gfsmStatePair *sp)
{ return 7*sp->id1 + sp->id2; }


/*--------------------------------------------------------------
 * statepair_compare()
 */
gint gfsm_statepair_compare(const gfsmStatePair *sp1, const gfsmStatePair *sp2)
{
  return (sp1->id1 < sp2->id1 ? -1
	  : (sp1->id1 > sp2->id1 ? 1
	     : (sp1->id2 < sp2->id2 ? -1
		: (sp1->id2 > sp2->id2 ? 1
		   : 0))));
}

/*--------------------------------------------------------------
 * statepair_equal()
 */
gboolean gfsm_statepair_equal(const gfsmStatePair *sp1, const gfsmStatePair *sp2)
{ return sp1->id1==sp2->id1 && sp1->id2==sp2->id2; }


/*======================================================================
 * Methods: gfsmStatePairEnum
 */

/*--------------------------------------------------------------
 * statepair_enum_new()
 */
gfsmStatePairEnum *gfsm_statepair_enum_new(void)
{
  return gfsm_enum_new_full((gfsmDupFunc)gfsm_statepair_clone,
			    (GHashFunc)gfsm_statepair_hash,
			    (GEqualFunc)gfsm_statepair_equal,
			    (GDestroyNotify)g_free);
}




