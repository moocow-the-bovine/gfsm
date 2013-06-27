/*=============================================================================*\
 * File: gfsmProduct.c
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
 * Methods: algebra: product
 */

/*--------------------------------------------------------------
 * product() (single-destructive)
 */
gfsmAutomaton *gfsm_automaton_product(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  gfsmAutomaton *fsm2_tmp = gfsm_automaton_clone(fsm2);
  gfsm_automaton_product2(fsm1,fsm2_tmp);
  gfsm_automaton_free(fsm2_tmp);
  return fsm1;
}

/*--------------------------------------------------------------
 * _product() (dual-destructive)
 */
gfsmAutomaton *gfsm_automaton_product2(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  gfsmStateId  qid;
  gfsmState   *qp;
  gfsmArcIter  ai;
  gfsmArc     *a;

  //-- chuck out all upper-labels from fsm1
  for (qid=0; qid < fsm1->states->len; qid++) {
    qp = gfsm_automaton_find_state(fsm1,qid);
    if (!qp || !qp->is_valid) continue;
    for (gfsm_arciter_open_ptr(&ai,fsm1,qp); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      a = gfsm_arciter_arc(&ai);
      a->upper = gfsmEpsilon;
    }
  }

  //-- chuck out all upper-labels from fsm2
  for (qid=0; qid < fsm2->states->len; qid++) {
    qp = gfsm_automaton_find_state(fsm2,qid);
    if (!qp || !qp->is_valid) continue;
    for (gfsm_arciter_open_ptr(&ai,fsm2,qp); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      a = gfsm_arciter_arc(&ai);
      a->lower = gfsmEpsilon;
    }
  }

  //-- concatenate
  gfsm_automaton_concat(fsm1,fsm2);
  
  //-- mark output fsm as transducer
  fsm1->flags.is_transducer = 1;

  return fsm1;
}
