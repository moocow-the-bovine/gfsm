
/*=============================================================================*\
 * File: gfsmArclist.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: arc lists
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

#include <gfsmArcList.h>
#include <stdlib.h>

/*======================================================================
 * Methods: Arc lists
 */

/*--------------------------------------------------------------
 * arclist_insert_sorted()
 */
gfsmArcList *gfsm_arclist_insert_sorted(gfsmArcList *al, gfsmArc *a, gfsmArcSortData *sdata)
{
  gfsmArcList *al_first=al;
  gfsmArcList *al_prev=NULL;
  
  for (; al != NULL; al_prev=al, al=al->next) {
    if (gfsm_arc_compare_inline(a, (gfsmArc*)(al->data), sdata) >= 0) break;
  }
  if (al_prev==NULL) al_first      = gfsm_arclist_prepend(al_first, a);
  else               al_prev->next = gfsm_arclist_prepend(al, a);
  return al_first;
}

/*--------------------------------------------------------------
 * arclist_insert_link_sorted()
 */
gfsmArcList *gfsm_arclist_insert_link_sorted(gfsmArcList *al, gfsmArcList *link, gfsmArcSortData *sdata)
{
  gfsmArcList *al_first=al;
  gfsmArcList *al_prev=NULL;
  gfsmArc     *a = gfsm_arclist_arc(link); //(gfsmArc*)(link->data);

  for (; al != NULL; al_prev=al, al=al->next) {
    if (gfsm_arc_compare_inline(a, (gfsmArc*)(al->data), sdata) <= 0) break;
  }
  if (al_prev == NULL) return g_slist_concat(link,al);
  al_prev->next = g_slist_concat(link, al);

  return al_first;
}

/*--------------------------------------------------------------
 * arclist_clone()
 */
gfsmArcList *gfsm_arclist_clone(gfsmArcList *src)
{
  gfsmArcList *al;
  gfsmArcList *dst = g_slist_copy(src);
  for (al=dst; al != NULL; al=al->next) {
    al->data = gfsm_arc_clone(al->data);
  }
  return dst;
}

/*--------------------------------------------------------------
 * arclist_free()
 */
void gfsm_arclist_free(gfsmArcList *al)
{
  while (al != NULL) {
    gfsm_arc_free((gfsmArc*)al->data);
    al = g_slist_delete_link(al,al);
  }
}
