
/*=============================================================================*\
 * File: gfsmArcIter.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: arc iterators
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
void gfsm_arciter_reset(gfsmArcIter *aip) {
  if (aip->state && gfsm_state_is_ok(aip->state)) {
    aip->arcs = aip->state->arcs;
  } else {
    aip->arcs = NULL;
  }
}

/*--------------------------------------------------------------
 * close()
 */
void gfsm_arciter_close(gfsmArcIter *aip) {
  if (!aip) return;
  aip->fsm   = NULL;
  aip->state = NULL;
  aip->arcs  = NULL;
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

/*--------------------------------------------------------------
 * remove()
 */
void gfsm_arciter_remove(gfsmArcIter *aip)
{
  if (aip && aip->arcs) {
    gfsmArcList *next = aip->arcs->next;
    gfsmArc *arc = (gfsmArc*)aip->arcs->data;
    if (arc) gfsm_arc_free(arc);
    aip->state->arcs = g_slist_delete_link(aip->state->arcs, aip->arcs);
    aip->arcs = next;
  }
}
