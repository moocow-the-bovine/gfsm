/*=============================================================================*\
 * File: gfsmSigma.c
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
 * Methods: algebra: sigma
 */

/*--------------------------------------------------------------
 * sigma()
 */
gboolean gfsm_automaton_sigma_foreach_func_(GFSM_UNUSED gfsmAlphabet *abet, GFSM_UNUSED gpointer key, gfsmLabelVal lab, gfsmAutomaton *fsm)
{
  gfsm_automaton_add_arc(fsm,0,1,lab,lab,fsm->sr->one);
  return FALSE;
}

gfsmAutomaton *gfsm_automaton_sigma(gfsmAutomaton *fsm, gfsmAlphabet *abet)
{
  gfsm_automaton_clear(fsm);
  fsm->flags.sort_mode = gfsmASMNone; //-- avoid "smart" arc-insertion
  fsm->root_id = gfsm_automaton_add_state_full(fsm,0);
  gfsm_automaton_add_state_full(fsm,1);
  gfsm_automaton_set_final_state_full(fsm,1,TRUE,fsm->sr->one);
  gfsm_alphabet_foreach(abet, (gfsmAlphabetForeachFunc)gfsm_automaton_sigma_foreach_func_, fsm);
  return fsm;
}
