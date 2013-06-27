/*=============================================================================*\
 * File: gfsmClosure.c
 * Author: Bryan Jurish <moocow.bovine@gmail.com>
 * Description: finite state machine library
 *
 * Copyright (c) 2004-2008 Bryan Jurish.
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
 * Methods: algebra: closure
 */

/*--------------------------------------------------------------
 * closure_final_func()
 *  + called for each final @id of @fsm during closure(@fsm)
 */
static
gboolean gfsm_automaton_closure_final_func_(gfsmStateId id, gpointer pw, gfsmAutomaton *fsm)
{
  gfsmWeight w = gfsm_ptr2weight(pw);
  if (id != fsm->root_id)
    gfsm_automaton_add_arc(fsm, id, fsm->root_id, gfsmEpsilon, gfsmEpsilon, w);
  return FALSE;
}


/*--------------------------------------------------------------
 * closure()
 */
gfsmAutomaton *gfsm_automaton_closure(gfsmAutomaton *fsm, gboolean is_plus)
{
  //-- sanity check(s)
  if (!fsm || fsm->root_id == gfsmNoState) return fsm;

  //-- add epsilon arcs from old final states to translated new root
  gfsm_automaton_finals_foreach(fsm, (GTraverseFunc)gfsm_automaton_closure_final_func_, fsm);

  //-- reflexive+transitive or reflexive?
  if (!is_plus) gfsm_automaton_optional(fsm);

  return fsm;
}


/*--------------------------------------------------------------
 * n_closure()
 */
gfsmAutomaton *gfsm_automaton_n_closure(gfsmAutomaton *fsm, guint n)
{
  //-- sanity check(s)
  if (!fsm || fsm->root_id == gfsmNoState) return fsm;

  //-- check for simple closures
  if (n == 0)      return gfsm_automaton_closure(fsm, FALSE);
  else if (n == 1) return gfsm_automaton_closure(fsm, TRUE);
  else {
    gfsm_automaton_n_concat(fsm, fsm, n-1);
  }

  return gfsm_automaton_closure(fsm, TRUE);
}

/*--------------------------------------------------------------
 * optional()
 */
gfsmAutomaton *gfsm_automaton_optional(gfsmAutomaton *fsm)
{
  if (!gfsm_automaton_is_final_state(fsm,fsm->root_id))
    gfsm_automaton_set_final_state_full(fsm,fsm->root_id,TRUE,fsm->sr->one);
  return fsm;
}
