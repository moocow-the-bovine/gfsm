/*=============================================================================*\
 * File: gfsmDifference.c
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
 * Methods: algebra: difference
 */

/*--------------------------------------------------------------
 * difference()
 */
gfsmAutomaton *gfsm_automaton_difference(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2)
{
  gfsmAutomaton *fsm = gfsm_automaton_difference_full(fsm1,fsm2,NULL);
  gfsm_automaton_swap(fsm1,fsm);
  gfsm_automaton_free(fsm);
  return fsm1;
}

/*--------------------------------------------------------------
 * difference_full()
 */
gfsmAutomaton *gfsm_automaton_difference_full(gfsmAutomaton *fsm1,
					      gfsmAutomaton *fsm2,
					      gfsmAutomaton *diff)
{
  gfsmAutomaton *not_fsm2;
  gfsmAlphabet *alph1 = gfsm_identity_alphabet_new();

  gfsm_automaton_get_alphabet(fsm1, gfsmLSLower, alph1);
  not_fsm2 = gfsm_automaton_clone(fsm2);
  gfsm_automaton_complement_full(not_fsm2, alph1);
  diff = gfsm_automaton_intersect_full(fsm1, not_fsm2, diff, NULL);

  gfsm_automaton_free(not_fsm2);
  gfsm_alphabet_free(alph1);

  return diff;
}
