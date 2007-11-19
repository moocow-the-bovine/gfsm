
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
  gfsmArc *arc;

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
    gfsm_arciter_close(&ai);
  }

  return rarcs;
}

/*--------------------------------------------------------------
 * reverse_arc_index_free()
 */
void gfsm_reverse_arc_index_free(gfsmReverseArcIndex *rarcs, gboolean free_lists)
{
  guint i;
  if (free_lists) {
    //-- +free_lists, -free_arcs
    for (i=0; i < rarcs->len; i++) { g_slist_free(g_ptr_array_index(rarcs,i)); }
  }

  //-- free index array
  g_ptr_array_free(rarcs,TRUE);
}

/*======================================================================
 * Methods: gfsmFinalWeightIndex
 */

/*--------------------------------------------------------------
 * automaton_final_weight_index()
 */
gfsmFinalWeightIndex *gfsm_automaton_final_weight_index(gfsmAutomaton *fsm, gfsmFinalWeightIndex *ix)
{
  gfsmStateId qid;
  gfsmWeight  fw, srzero = fsm->sr->zero;

  if (ix==NULL) {
    ix = gfsm_final_weight_index_sized_new(gfsm_automaton_n_states(fsm));
  } else {
    g_array_set_size(ix,gfsm_automaton_n_states(fsm));
    ix->len = 0;
  }

  for (qid=0; qid < gfsm_automaton_n_states(fsm); qid++) {
    if (gfsm_automaton_lookup_final(fsm,qid,&fw)) {
      g_array_index(ix,gfsmWeight,qid) = fw;
    } else {
      g_array_index(ix,gfsmWeight,qid) = srzero;
    }
  }

  return ix;
}

/*======================================================================
 * Methods: gfsmArcLabelIndex
 */

/*--------------------------------------------------------------
 * new()
 */
gfsmArcLabelIndex *gfsm_arc_label_index_new(void) {
  gfsmArcLabelIndex *ix = g_new(gfsmArcLabelIndex,1);
  ix->arcs  = g_ptr_array_new();
  ix->first = g_ptr_array_new();
  return ix;
}

/*--------------------------------------------------------------
 * new_full()
 */
gfsmArcLabelIndex *gfsm_arc_label_index_new_full(gfsmStateId n_states, guint n_arcs) {
  gfsmArcLabelIndex *ix = g_new(gfsmArcLabelIndex,1);
  ix->arcs  = g_ptr_array_sized_new(n_arcs);
  ix->first = g_ptr_array_sized_new(n_states);
  return ix;
}

/*--------------------------------------------------------------
 * automaton_lower_label_index()
 */
gfsmArcLabelIndex *gfsm_automaton_lower_label_index(gfsmAutomaton *fsm, gfsmArcLabelIndex *ix)
{
  gfsmStateId qid;
  gfsmArcIter ai;

  //-- maybe allocate
  if (ix==NULL) {
    ix = gfsm_arc_label_index_new_full(gfsm_automaton_n_states(fsm), gfsm_automaton_n_arcs(fsm));
  } else {
    g_ptr_array_set_size(ix->arcs,  gfsm_automaton_n_arcs(fsm));
    g_ptr_array_set_size(ix->first, gfsm_automaton_n_states(fsm));
  }

  //-- populate arcs
  for (qid=0; qid < gfsm_automaton_n_states(fsm); qid++) {
    for (gfsm_arciter_open(&ai,fsm,qid); FALSE; ) { ; } //-- CONTINUE HERE
  }
}

/*--------------------------------------------------------------
 * automaton_upper_label_index()
 */
gfsmArcLabelIndex *gfsm_automaton_upper_label_index(gfsmAutomaton *fsm, gfsmArcLabelIndex *ix)
{
  gfsmStateId qid;

  //-- maybe allocate
  if (ix==NULL) {
    ix = gfsm_arc_label_index_new_full(gfsm_automaton_n_states(fsm), gfsm_automaton_n_arcs(fsm));
  } else {
    g_ptr_array_set_size(ix->arcs,  gfsm_automaton_n_arcs(fsm));
    g_ptr_array_set_size(ix->first, gfsm_automaton_n_states(fsm));
  }
}
