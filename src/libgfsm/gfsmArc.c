
/*=============================================================================*\
 * File: gfsmArc.c
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

#include <gfsmArc.h>
#include <stdlib.h>

/*======================================================================
 * Methods: Arcs: Constructors etc.
 */
//-- INLINED


/*======================================================================
 * Methods: Arc Lists: Utilities
 */

/*--------------------------------------------------------------
 * compare()
 */
#define GFSM_ARC_COMPARE_COMPAT 1
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
    //else if (a1->source < a2->source) return -1;
    //else if (a1->source > a2->source) return  1;
    else if (a1->target < a2->target) return -1;
    else if (a1->target > a2->target) return  1;
    return 0;
			
  case gfsmASMUpper:
    if      (a1->upper < a2->upper)   return -1;
    else if (a1->upper > a2->upper)   return  1;
    else if (a1->lower < a2->lower)   return -1;
    else if (a1->lower > a2->lower)   return  1;
    //else if (a1->source < a2->source) return -1;
    //else if (a1->source > a2->source) return  1;
    else if (a1->target < a2->target) return -1;
    else if (a1->target > a2->target) return  1;
    return 0;

  case gfsmASMWeight:
    return gfsm_sr_compare(sdata->sr, a1->weight, a2->weight);

  case gfsmASMNone:
  default:
    return (GPOINTER_TO_INT(a2)-GPOINTER_TO_INT(a1));
  }

  return 0;
}


/*======================================================================
 * Methods: String utilities
 */
const gchar *gfsm_arc_sortmode_to_name(gfsmArcSortMode mode)
{
  switch (mode) {
  case gfsmASMNone:   return "none";
  case gfsmASMLower:  return "lower";
  case gfsmASMUpper:  return "upper";
  case gfsmASMWeight: return "weight";
  default:            return "unknown";
  }
}
