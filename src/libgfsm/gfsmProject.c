/*=============================================================================*\
 * File: gfsmProject.c
 * Author: Bryan Jurish <moocow.bovine@gmail.com>
 * Description: finite state machine library
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

#include <glib.h>
#include <gfsmAlgebra.h>
#include <gfsmAssert.h>
#include <gfsmArcIter.h>
#include <gfsmEnum.h>
#include <gfsmUtils.h>
#include <gfsmCompound.h>

/*======================================================================
 * Methods: algebra: projection
 */

/*--------------------------------------------------------------
 * invert()
 */
gfsmAutomaton *gfsm_automaton_invert(gfsmAutomaton *fsm)
{
  gfsmStateId id;
  gfsmArcIter ai;
  gfsmArcCompMask acmask_old=fsm->flags.sort_mode, acmask_new=gfsmACNone;;
  gint aci;

  //-- invert arcs
  for (id=0; id < fsm->states->len; id++) {
    for (gfsm_arciter_open(&ai,fsm,id); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      gfsmLabelId tmp = a->lower;
      a->lower        = a->upper;
      a->upper        = tmp;
    }
  }

  //-- adjust sort mask (translate "lower"<->"upper")
  for (aci=0; aci < gfsmACMaxN; aci++) {
    gfsmArcCompMask cmp = gfsm_acmask_nth(acmask_old,aci);
    switch (cmp) {
    case gfsmACLower:  cmp=gfsmACUpper; break;
    case gfsmACUpper:  cmp=gfsmACLower; break;
    case gfsmACLowerR: cmp=gfsmACUpperR; break;
    case gfsmACUpperR: cmp=gfsmACLowerR; break;
    default: break;
    }
    acmask_new |= gfsm_acmask_new(cmp,aci);
  }
  fsm->flags.sort_mode = acmask_new;
  
  return fsm;
}

/*--------------------------------------------------------------
 * project()
 */
gfsmAutomaton *gfsm_automaton_project(gfsmAutomaton *fsm, gfsmLabelSide which)
{
  gfsmStateId id;
  gfsmArcIter ai;
  gfsmArcCompMask acmask_old=fsm->flags.sort_mode, acmask_new=gfsmACNone;;
  guint aci;

  if (which==gfsmLSBoth) return fsm;

  for (id=0; id < fsm->states->len; id++) {
    for (gfsm_arciter_open(&ai,fsm,id); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      if (which==gfsmLSLower) a->upper = a->lower;
      else                    a->lower = a->upper;
    }
  }

  //-- adjust flags
  fsm->flags.is_transducer = FALSE;

  for (aci=0; aci < gfsmACMaxN; aci++) {
    gfsmArcCompMask cmp = gfsm_acmask_nth(acmask_old,aci);
    switch (cmp) {

    case gfsmACLower:
    case gfsmACLowerR:
      if (which != gfsmLSLower) cmp=gfsmACNone;
      break;

    case gfsmACUpper:
    case gfsmACUpperR:
      if (which != gfsmLSUpper) cmp=gfsmACNone;
      break;

    default:
      break;
    }
    acmask_new |= gfsm_acmask_new(cmp,aci);
  }
  fsm->flags.sort_mode = acmask_new;


  return fsm;
}
