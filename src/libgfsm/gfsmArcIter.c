
/*=============================================================================*\
 * File: gfsmArcIter.c
 * Author: Bryan Jurish <moocow.bovine@gmail.com>
 * Description: finite state machine library: arc iterators
 *
 * Copyright (c) 2004-2011 Bryan Jurish.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
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

#include <gfsmArcIter.h>

//-- no-inline definitions
#ifndef GFSM_INLINE_ENABLED
# include <gfsmArcIter.hi>
#endif

/*======================================================================
 * Methods: Arc iterators: open/close
 */

/*======================================================================
 * Methods: Arc iterators: Accessors
 */


//--------------------------------------------------------------
// seek_lower()
void gfsm_arciter_seek_lower(gfsmArcIter *aip, gfsmLabelVal lo)
{
  for ( ; gfsm_arciter_ok(aip); gfsm_arciter_next(aip)) {
    if (gfsm_arciter_arc(aip)->lower == lo) break;
  }
}

//--------------------------------------------------------------
// seek_upper()
void gfsm_arciter_seek_upper(gfsmArcIter *aip, gfsmLabelVal hi)
{
  for ( ; gfsm_arciter_ok(aip); gfsm_arciter_next(aip)) {
    if (gfsm_arciter_arc(aip)->upper == hi) break;
  }
}

//--------------------------------------------------------------
// seek_both()
void gfsm_arciter_seek_both(gfsmArcIter *aip, gfsmLabelVal lo, gfsmLabelVal hi)
{
  for ( ; gfsm_arciter_ok(aip); gfsm_arciter_next(aip)) {
    gfsmArc *a = gfsm_arciter_arc(aip);
    if ((lo==gfsmNoLabel || a->lower==lo) && (hi==gfsmNoLabel || a->upper==hi)) break;
  }
}

//--------------------------------------------------------------
// seek_all()
void gfsm_arciter_seek_all(gfsmArcIter *aip, gfsmLabelVal lo, gfsmLabelVal hi, gfsmStateId dst)
{
  for ( ; gfsm_arciter_ok(aip); gfsm_arciter_next(aip)) {
    gfsmArc *a = gfsm_arciter_arc(aip);
    if ((lo==gfsmNoLabel || a->lower==lo) && (hi==gfsmNoLabel || a->upper==hi) && (dst==gfsmNoState || a->target==dst)) break;
  }
}


//--------------------------------------------------------------
// seek_user()
void gfsm_arciter_seek_user(gfsmArcIter *aip,
			    gfsmArcIterSeekFunc seekfunc,
			    gpointer data)
{
  for ( ; gfsm_arciter_ok(aip); gfsm_arciter_next(aip)) {
    if ((*seekfunc)(aip,data)) break;
  }
}
