
/*=============================================================================*\
 * File: gfsmArc.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: arcs
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

#include <gfsmArc.h>
#include <stdlib.h>

/*======================================================================
 * Methods: Arcs: Constructors etc.
 */

/*--------------------------------------------------------------
 * arc_init()
 */
gfsmArc *gfsm_arc_init(gfsmArc *a,
		       gfsmStateId dst,
		       gfsmLabelId lo,
		       gfsmLabelId hi,
		       gfsmWeight wt)
{
  if (a) {
    a->target = dst;
    a->lower  = lo;
    a->upper  = hi;
    a->weight = wt;
  }
  return a;
}

/*--------------------------------------------------------------
 * arc_copy()
 */
gfsmArc *gfsm_arc_copy(gfsmArc *src)
{
  gfsmArc *dst = g_new(gfsmArc,1);
  *dst = *src;
  return dst;
}

/*======================================================================
 * Methods: Arc Lists: Constructors etc.
 */

/*--------------------------------------------------------------
 * arclist_alloc()
 */
//(macro)


/*--------------------------------------------------------------
 * arclist_prepend()
 */
//(macro)

/*--------------------------------------------------------------
 * arclist_insert()
 */
gfsmArcList *gfsm_arclist_insert(gfsmArcList *al,
				 gfsmStateId  dst,
				 gfsmLabelVal lo,
				 gfsmLabelVal hi,
				 gfsmWeight   wt,
				 gfsmArcSortData *sdata)
{
  gfsmArc *a = gfsm_arc_new_full(dst,lo,hi,wt);

  if (!sdata || sdata->mode == gfsmASMNone) return gfsm_arclist_prepend(al,a);
  else {
    gfsmArcList *al_first=al;
    gfsmArcList *al_prev=NULL;
  
    for (; al != NULL; al_prev=al, al=al->next) {
      if (gfsm_arc_compare(a, (gfsmArc*)(al->data), sdata) >= 0) break;
    }
    if (al_prev==NULL) al_first      = gfsm_arclist_prepend(al_first, a);
    else               al_prev->next = gfsm_arclist_prepend(al, a);
    return al_first;
  }
}

/*--------------------------------------------------------------
 * arclist_insert_link()
 */
gfsmArcList *gfsm_arclist_insert_link(gfsmArcList *al,
				      gfsmArcList *link,
				      gfsmArcSortData *sdata)
{
  gfsmArcList *al_first=al;
  gfsmArcList *al_prev=NULL;
  gfsmArc     *a = (gfsmArc*)(link->data);

  if (!sdata || sdata->mode == gfsmASMNone) return g_slist_concat(link,al);

  for (; al != NULL; al_prev=al, al=al->next) {
    if (gfsm_arc_compare(a, (gfsmArc*)(al->data), sdata) >= 0) break;
  }

  if (al_prev == NULL) return g_slist_concat(link,al);

  al_prev->next = g_slist_concat(link, al);
  return al_first;
}

/*--------------------------------------------------------------
 * arclist_init()
 */
gfsmArcList *gfsm_arclist_init(gfsmArcList *al,
			       gfsmStateId dst,
			       gfsmLabelId lo,
			       gfsmLabelId hi,
			       gfsmWeight wt,
			       gfsmArcList *nxt)
{
  if (al) {
    gfsm_arc_init((gfsmArc*)al, dst,lo,hi,wt);
    al->next = nxt;
  }
  return al;
}

/*--------------------------------------------------------------
 * arclist_copy()
 */
gfsmArcList *gfsm_arclist_copy(gfsmArcList *src)
{
  gfsmArcList *al;
  gfsmArcList *dst = g_slist_copy(src);
  for (al=dst; al != NULL; al=al->next) {
    al->data = gfsm_arc_copy(al->data);
  }
  return dst;
}

/*--------------------------------------------------------------
 * arclist_free()
 */
#include <stdio.h>
void gfsm_arclist_free(gfsmArcList *al)
{
  gfsmArcList *next = al;
  //fprintf(stderr, "<DEBUG>:gfsm_arclist_free() called; al=%p\n", al);
  for ( ; al != NULL; al = next) {
    next = g_slist_remove_link(al,al);
    //fprintf(stderr, "<DEBUG>:gfsm_arclist_free(): iter al=%p ; next=%p\n", al, next);
    gfsm_arc_free((gfsmArc*)(al->data));
    g_slist_free_1(al);
  }
}

/*======================================================================
 * Methods: Arc Lists: Utilities
 */
/*--------------------------------------------------------------
 * arclist_length()
 */
  /*
guint gfsm_arclist_length(gfsmArcList *al)
{
  guint n;
  for (n=0; al != NULL; al=al->next) ++n;
  return n;
}
*/

/*--------------------------------------------------------------
 * compare()
 */
gint gfsm_arc_compare(gfsmArc *a1, gfsmArc *a2, gfsmArcSortData *sdata)
{
  if (!a1) {
    if (!a2) return 0;
    return 1;
  }
  if (!a2) return -1;

  switch (sdata->mode) {
  case gfsmASMLower:
    if      (a1->lower < a2->lower)   return -1;
    else if (a1->lower > a2->lower)   return  1;
    else if (a1->upper < a2->upper)   return -1;
    else if (a1->upper > a2->upper)   return  1;
    else if (a1->target < a2->target) return -1;
    else if (a1->target > a2->target) return  1;
    return 0;
			
  case gfsmASMUpper:
    if      (a1->upper < a2->upper)   return -1;
    else if (a1->upper > a2->upper)   return  1;
    else if (a1->lower < a2->lower)   return -1;
    else if (a1->lower > a2->lower)   return  1;
    else if (a1->target < a2->target) return -1;
    else if (a1->target > a2->target) return  1;
    return 0;

  case gfsmASMWeight:
    return gfsm_sr_compare(sdata->sr, a1->weight, a2->weight);

  case gfsmASMNone:
  default:
    return ((gint)(a2))-((gint)a1);
  }

  return 0;
}


/*======================================================================
 * Methods: String utilities
 */
gchar *gfsm_arc_sortmode_to_name(gfsmArcSortMode mode)
{
  switch (mode) {
  case gfsmASMNone:   return "none";
  case gfsmASMLower:  return "lower";
  case gfsmASMUpper:  return "upper";
  case gfsmASMWeight: return "weight";
  default:            return "unknown";
  }
}
