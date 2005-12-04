/*=============================================================================*\
 * File: gfsmArith.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library
 *
 * Copyright (c) 2005 Bryan Jurish.
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

#include <glib.h>
#include <gfsmArith.h>

/*======================================================================
 * Methods: arithmetic: final
 */

/*--------------------------------------------------------------
 * weight_X()
 */
typedef struct {
  gfsmAutomaton *fsm;
  gfsmWeight     w;
} _gfsm_automaton_weight_op_data;

/*--------------------------------------------------------------
 * final_weight_plus()
 */
gboolean _gfsm_automaton_final_weight_plus_foreach_func(gfsmStateId id, gpointer pw,
							_gfsm_automaton_weight_op_data *data)
{
  gfsmWeight w = gfsm_sr_plus(data->fsm->sr, gfsm_ptr2weight(pw), data->w);
  gfsm_weightmap_insert(data->fsm->finals, (gpointer)id, w);
  return FALSE;
}


gfsmAutomaton *gfsm_automaton_final_weight_plus(gfsmAutomaton *fsm, gfsmWeight w)
{
  _gfsm_automaton_weight_op_data data = { fsm, w };
  g_tree_foreach(fsm->finals, (GTraverseFunc)_gfsm_automaton_final_weight_plus_foreach_func, &data);
  return fsm;
}

/*--------------------------------------------------------------
 * final_weight_times()
 */
gboolean _gfsm_automaton_final_weight_times_foreach_func(gfsmStateId id, gpointer pw,
							 _gfsm_automaton_weight_op_data *data)
{
  gfsmWeight w = gfsm_sr_times(data->fsm->sr, gfsm_ptr2weight(pw), data->w);
  gfsm_weightmap_insert(data->fsm->finals, (gpointer)id, w);
  return FALSE;
}

gfsmAutomaton *gfsm_automaton_final_weight_times(gfsmAutomaton *fsm, gfsmWeight w)
{
  _gfsm_automaton_weight_op_data data = { fsm, w };
  g_tree_foreach(fsm->finals, (GTraverseFunc)_gfsm_automaton_final_weight_times_foreach_func, &data);
  return fsm;
}
