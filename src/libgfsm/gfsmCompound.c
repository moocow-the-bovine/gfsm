/*=============================================================================*\
 * File: gfsmCompound.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: common definitions
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

#include <gfsmCompound.h>
#include <gfsmMem.h>
#include <stdlib.h>
#include <gfsmUtils.h>


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
void gfsm_statepair_free(gfsmStatePair *sp) { g_free(sp); }
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
 * Methods: gfsmComposeState
 */

/*--------------------------------------------------------------
 * compose_state_new()
 */
gfsmComposeState *gfsm_compose_state_new(gfsmStateId id1, gfsmStateId id2, gfsmComposeFilterState idf)
{
  gfsmComposeState *sp = g_new(gfsmComposeState,1);
  sp->id1 = id1;
  sp->id2 = id2;
  sp->idf = idf;
  return sp;
}

/*--------------------------------------------------------------
 * compose_state_clone()
 */
gfsmComposeState *gfsm_compose_state_clone(gfsmComposeState *sp)
{
  return (gfsmComposeState*)gfsm_mem_dup_n(sp, sizeof(gfsmComposeState));
}

/*--------------------------------------------------------------
 * compose_state_free()
 */
/*
void gfsm_compose_state_free(gfsmComposeState *sp) { g_free(sp); }
*/

/*--------------------------------------------------------------
 * compose_state_hash()
 */
guint gfsm_compose_state_hash(gfsmComposeState *sp)
{ return 7949*sp->id1 + sp->id2 + 7963*sp->idf; } 


/*--------------------------------------------------------------
 * compose_state_compare()
 */
gint gfsm_compose_state_compare(const gfsmComposeState *sp1, const gfsmComposeState *sp2)
{
  return (sp1->id1 < sp2->id1 ? -1
	  : (sp1->id1 > sp2->id1 ? 1
	     : (sp1->id2 < sp2->id2 ? -1
		: (sp1->id2 > sp2->id2 ? 1
		   : (sp1->idf < sp2->idf ? -1
		      : (sp1->idf > sp2->idf ? 1
			 : 0))))));
}

/*--------------------------------------------------------------
 * compose_state_equal()
 */
gboolean gfsm_compose_state_equal(const gfsmComposeState *sp1, const gfsmComposeState *sp2)
{ return sp1->id1==sp2->id1 && sp1->id2==sp2->id2 && sp1->idf==sp2->idf; }



/*======================================================================
 * Methods: gfsmStateWeightPair
 */

//--------------------------------------------------------------
gfsmStateWeightPair *gfsm_state_weight_pair_new(gfsmStateId id, gfsmWeight w)
{
  gfsmStateWeightPair *swp = g_new(gfsmStateWeightPair,1);
  swp->id = id;
  swp->w  = w;
  return swp;
}

//--------------------------------------------------------------
gfsmStateWeightPair *gfsm_state_weight_pair_clone(const gfsmStateWeightPair *swp)
{
  return gfsm_state_weight_pair_new(swp->id,swp->w);
}

//--------------------------------------------------------------
guint gfsm_state_weight_pair_hash(gfsmStateWeightPair *swp)
{
  return swp->id;
}

//--------------------------------------------------------------
gint gfsm_state_weight_pair_compare(const gfsmStateWeightPair *swp1, const gfsmStateWeightPair *swp2, gfsmSemiring *sr)
{
  gint rc = gfsm_uint_compare(GINT_TO_POINTER(swp1->id),GINT_TO_POINTER(swp2->id));
  if (!rc) rc = gfsm_sr_compare(sr,swp1->w,swp2->w);
  return rc;
}

//--------------------------------------------------------------
gboolean gfsm_state_weight_pair_equal(const gfsmStateWeightPair *swp1, const gfsmStateWeightPair *swp2)
{
  return swp1->id==swp2->id && swp1->w==swp2->w;
}


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


/*======================================================================
 * Methods: gfsmComposeStateEnum
 */

/*--------------------------------------------------------------
 * compose_state_enum_new()
 */
gfsmComposeStateEnum *gfsm_compose_state_enum_new(void)
{
  return gfsm_enum_new_full((gfsmDupFunc)gfsm_compose_state_clone,
			    (GHashFunc)gfsm_compose_state_hash,
			    (GEqualFunc)gfsm_compose_state_equal,
			    (GDestroyNotify)g_free);
}

