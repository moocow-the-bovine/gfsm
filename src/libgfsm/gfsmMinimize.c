/*=============================================================================*\
 * File: gfsmMinimize.c
 * Author: Bryan Jurish <moocow.bovine@gmail.com>
 * Description: finite state machine library
 *
 * Copyright (c) 2011 Bryan Jurish.
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

#include <gfsmAlgebra.h>
#include <gfsmEncode.h>

/*======================================================================
 * Methods: algebra: minimization
 *  + simple implementation of Brzozowski (1963) algorithm
 *    Brzozowski, J. A. (1963), "Canonical regular expressions and minimal
 *    state graphs for definite events", Proc. Sympos. Math. Theory of
 *    Automata (New York, 1962), Polytechnic Press of Polytechnic Inst.
 *    of Brooklyn, Brooklyn, N.Y., pp. 529–561, MR0175719.
 */

//--------------------------------------------------------------
gfsmAutomaton *gfsm_automaton_minimize_full(gfsmAutomaton *fsm, gboolean rmeps)
{
  gfsmAutomaton *tmp=gfsm_automaton_shadow(fsm);

  //-- ensure forward dfa: rmeps + determinize
  if (!fsm->flags.is_deterministic) {
    if (rmeps) gfsm_automaton_rmepsilon(fsm);
    gfsm_automaton_determinize_full(fsm,tmp);
    gfsm_automaton_swap(fsm,tmp);
  }

  //-- reverse-dfa
  gfsm_automaton_reverse(fsm);
  if (rmeps) gfsm_automaton_rmepsilon(fsm);
  gfsm_automaton_determinize_full(fsm,tmp);

  //-- re-reverse & determinize
  gfsm_automaton_reverse(tmp);
  if (rmeps) gfsm_automaton_rmepsilon(tmp);
  gfsm_automaton_determinize_full(tmp,fsm);

  //-- cleanup & return
  gfsm_automaton_free(tmp);
  return fsm;
}

//--------------------------------------------------------------
gfsmAutomaton *gfsm_automaton_minimize(gfsmAutomaton *fsm)
{
  return gfsm_automaton_minimize_full(fsm,TRUE);
}


/*--------------------------------------------------------------
 * compact()
 */
gfsmAutomaton *gfsm_automaton_compact_full(gfsmAutomaton *fsm, gboolean rmeps)
{
  gfsmArcLabelKey *key;

  //-- guts
  key = gfsm_automaton_encode(fsm, NULL, TRUE,TRUE);
  gfsm_automaton_minimize_full(fsm, rmeps);
  gfsm_automaton_decode(fsm, key, TRUE,TRUE);

  //-- cleanup
  if (key) gfsm_arclabel_key_free(key);

  return fsm;
}

//--------------------------------------------------------------
gfsmAutomaton *gfsm_automaton_compact(gfsmAutomaton *fsm)
{
  return gfsm_automaton_compact_full(fsm,TRUE);
}



