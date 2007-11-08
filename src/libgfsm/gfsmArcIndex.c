
/*=============================================================================*\
 * File: gfsmArcIndex.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: arc indices
 *
 * Copyright (c) 2006-2007 Bryan Jurish.
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

#include <gfsmArcIndex.h>
#include <gfsmArcIter.h>

/*======================================================================
 * Methods: ReverseArcIndex
 */

/*--------------------------------------------------------------
 * automaton_reverse_arc_index()
 */
gfsmReverseArcIndex *gfsm_automaton_reverse_arc_index(gfsmAutomaton *fsm, gfsmReverseArcIndex *rarcs)
{
  gfsmStateId idfrom;
  gfsmArcIter ai;
  gfsmArc *arc, *rarc;

  if (!rarcs) {
    rarcs = gfsm_reverse_arc_index_sized_new(fsm->states->len);
  }
  g_ptr_array_set_size(rarcs,fsm->states->len);

  for (idfrom=0; idfrom < fsm->states->len; idfrom++) {
    for (gfsm_arciter_open(&ai,fsm,idfrom); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      arc  = gfsm_arciter_arc(&ai);
      /*rarc = gfsm_arc_copy(arc);
	rarc->target = idfrom;
      g_ptr_array_index(rarcs,arc->target)
	= gfsm_arclist_prepend(g_ptr_array_index(rarcs,arc->target), rarc);
      */
      g_ptr_array_index(rarcs,arc->target)
	= gfsm_arclist_prepend(g_ptr_array_index(rarcs,arc->target), arc);
    }
  }

  return rarcs;
}

/*--------------------------------------------------------------
 * reverse_arc_index_free()
 */
void gfsm_reverse_arc_index_free(gfsmReverseArcIndex *rarcs, gboolean free_lists, gboolean ignored)
{
  guint i;
  if (free_lists) {
    //-- +free_lists, -free_arcs
    for (i=0; i < rarcs->len; i++) { g_slist_free(g_ptr_array_index(rarcs,i)); }
  }

  //-- free index array
  g_ptr_array_free(rarcs,TRUE);
}
