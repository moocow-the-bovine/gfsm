
/*=============================================================================*\
 * File: gfsmArcIter.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: arc iterators
 *
 * Copyright (c) 2004 Bryan Jurish.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See file LICENSE for further informations on licensing terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *=============================================================================*/

#include <gfsmArcIter.h>


/*======================================================================
 * Methods: Arc iterators: open/close
 */

/*--------------------------------------------------------------
 * open()
 */
void gfsm_arciter_open(gfsmArcIter *aip, gfsmAutomaton *fsm, gfsmStateId stateid)
{
  aip->fsm = fsm;
  aip->state = gfsm_automaton_find_state(fsm,stateid);
  aip->arcs = NULL;
  gfsm_arciter_reset(aip);
}

/*--------------------------------------------------------------
 * open_ptr()
 */
void gfsm_arciter_open_ptr(gfsmArcIter *aip, gfsmAutomaton *fsm, gfsmState *stateptr)
{
  aip->fsm = fsm;
  aip->state = stateptr;
  aip->arcs = NULL;
  gfsm_arciter_reset(aip);
}

/*--------------------------------------------------------------
 * reset()
 */
/** Reset an arc iterator to the first outgoing arc */
void gfsm_arciter_reset(gfsmArcIter *aip) {
  if (aip->state && gfsm_state_is_ok(aip->state)) {
    aip->arcs = aip->state->arcs;
  } else {
    aip->arcs = NULL;
  }
}


/*======================================================================
 * Methods: Arc iterators: Accessors
 */
/*--------------------------------------------------------------
 * seek_lower()
 */
void gfsm_arciter_seek_lower(gfsmArcIter *aip, gfsmLabelVal lo)
{
  for ( ; gfsm_arciter_ok(aip); gfsm_arciter_next(aip)) {
    if (gfsm_arciter_arc(aip)->lower == lo) break;
  }
}

/*--------------------------------------------------------------
 * seek_upper()
 */
void gfsm_arciter_seek_upper(gfsmArcIter *aip, gfsmLabelVal hi)
{
  for ( ; gfsm_arciter_ok(aip); gfsm_arciter_next(aip)) {
    if (gfsm_arciter_arc(aip)->upper == hi) break;
  }
}

/*--------------------------------------------------------------
 * seek_both()
 */
void gfsm_arciter_seek_both(gfsmArcIter *aip, gfsmLabelVal lo, gfsmLabelVal hi)
{
  for ( ; gfsm_arciter_ok(aip); gfsm_arciter_next(aip)) {
    gfsmArc *a = gfsm_arciter_arc(aip);
    if ((lo==gfsmNoLabel || a->lower==lo) && (hi==gfsmNoLabel || a->upper==hi)) break;
  }
}


/*--------------------------------------------------------------
 * seek_user()
 */
void gfsm_arciter_seek_user(gfsmArcIter *aip,
			    gfsmArcIterSeekFunc seekfunc,
			    gpointer data)
{
  for ( ; gfsm_arciter_ok(aip); gfsm_arciter_next(aip)) {
    if ((*seekfunc)(aip,data)) break;
  }
}
