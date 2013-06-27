/*=============================================================================*\
 * File: gfsmConcat.c
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
 * Methods: algebra: concatenation
 */

/*--------------------------------------------------------------
 * concat_final_func()
 *  + called for each final @id of @fsm during concat(@fsm,@fsm2)
 *  + during the call @fsm1->root_id should be set to the translated root of @fsm2
 */
static
gboolean gfsm_automaton_concat_final_func_(gfsmStateId id, gpointer pw, gfsmAutomaton *fsm)
{
  gfsmWeight w = gfsm_ptr2weight(pw);
  gfsm_automaton_add_arc(fsm, id, fsm->root_id, gfsmEpsilon, gfsmEpsilon, w);
  gfsm_automaton_find_state(fsm,id)->is_final = FALSE;
  return FALSE;
}

/*--------------------------------------------------------------
 * concat()
 */
gfsmAutomaton *gfsm_automaton_concat(gfsmAutomaton *fsm1, gfsmAutomaton *_fsm2)
{
  gfsmAutomaton *fsm2;
  gfsmStateId    offset;
  gfsmStateId    id2;
  gfsmStateId    size2;
  gfsmStateId    rootx;
  gfsmWeightMap *finals2 = NULL;

  //-- sanity check(s)
  if (!_fsm2 || _fsm2->root_id == gfsmNoState) return fsm1;
  if (_fsm2==fsm1) fsm2 = gfsm_automaton_clone(fsm1);
  else             fsm2 = _fsm2;

  if (fsm1->finals == fsm2->finals) {
    finals2 = gfsm_weightmap_new(gfsm_uint_compare);
    gfsm_weightmap_copy(finals2, fsm2->finals);
  }

  offset = fsm1->states->len;
  size2  = fsm2->states->len;
  gfsm_automaton_reserve(fsm1, offset + size2);

  //-- concatenative arcs
  if (fsm1->root_id != gfsmNoState) {
    //-- multiple final states: add epsilon arcs from old finals to mapped root2
    gfsmStateId root_tmp = fsm1->root_id;
    rootx                = fsm2->root_id+offset;
    fsm1->root_id        = rootx;
    gfsm_automaton_finals_foreach(fsm1, (GTraverseFunc)gfsm_automaton_concat_final_func_, fsm1);
    fsm1->root_id        = root_tmp;
  } else /*if (fsm2->root_id != gfsmNoState)*/ {
    fsm1->root_id = rootx = fsm2->root_id + offset;
  }
  gfsm_weightmap_clear(fsm1->finals);

  //-- adopt states from fsm2 into fsm1
  for (id2 = 0; id2 < size2; id2++) {
    gfsmStateId      id1;
    const gfsmState *s2;
    gfsmState       *s1;
    gfsmArcIter      ai;
    gfsmWeight       s2fw;

    s2 = gfsm_automaton_find_state_const(fsm2,id2);
    id1 = id2+offset;
    s1 = gfsm_automaton_find_state(fsm1, id1);

    //-- sanity check(s)
    if (!s1 || !s2 || !s2->is_valid) continue;

    //-- copy state
    gfsm_state_copy(s1,s2);

    //-- translate targets for adopted arcs
    for (gfsm_arciter_open_ptr(&ai,fsm1,s1); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai))
      {
	gfsmArc *a = gfsm_arciter_arc(&ai);
	a->target += offset;
      }

    //-- check for new final states: get weight & mark state is_final flag
    if ( (finals2 && gfsm_weightmap_lookup(finals2, GUINT_TO_POINTER(id2), &s2fw))
	 ||
	 (!finals2 && gfsm_weightmap_lookup(fsm2->finals, GUINT_TO_POINTER(id2), &s2fw)) )
      {
	s1->is_final = TRUE;
	gfsm_weightmap_insert(fsm1->finals, GUINT_TO_POINTER(id1), s2fw);
      }
  }

  //-- mark as unsorted
  fsm1->flags.sort_mode = gfsmASMNone;

  //-- cleanup
  if (finals2) gfsm_weightmap_free(finals2);
  if (fsm2 != _fsm2) gfsm_automaton_free(fsm2);

  return fsm1;
}

/*--------------------------------------------------------------
 * n_concat()
 */
gfsmAutomaton *gfsm_automaton_n_concat(gfsmAutomaton *fsm1, gfsmAutomaton *_fsm2, guint n)
{
  gfsmAutomaton *fsm2 = _fsm2;

  //-- sanity check(s)
  if (!_fsm2 || _fsm2->root_id == gfsmNoState) return fsm1;
  if (_fsm2==fsm1) fsm2 = gfsm_automaton_clone(fsm1);

  for ( ; n > 0; n--) { gfsm_automaton_concat(fsm1, fsm2); }

  if (fsm2 != _fsm2) gfsm_automaton_free(fsm2);

  return fsm1;
}
