
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
//--inline

/*--------------------------------------------------------------
 * compare()
 */
gint gfsm_arc_compare(gfsmArc *a1, gfsmArc *a2, gfsmArcSortData *sdata)
{ return gfsm_arc_compare_inline(a1,a2,sdata); }

/*--------------------------------------------------------------
 * sortmode_to_name()
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
