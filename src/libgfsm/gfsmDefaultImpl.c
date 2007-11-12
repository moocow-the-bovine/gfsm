/*=============================================================================*\
 * File: gfsmDefaultImpl.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: implementation: default wrappers
 *
 * Copyright (c) 2007 Bryan Jurish.
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

#include <gfsmDefaultImpl.h>


/*--------------------------------------------------------------
 * "pure-virtual" error message
 */
#define gfsm_die_no_impl(fnc,itype) \
  g_error("libgfsm ERROR: API function `" # fnc "()' is not implemented for automaton class `%s'\n", gfsm_class_name(itype))

/*======================================================================
 * Methods: Constructors etc.
 */

/*--------------------------------------------------------------
 * init()
 */
void gfsm_automaton_init_default(gfsmAutomaton *fsm, guint n_states, guint n_arcs)
{
  gfsm_die_no_impl(init,fsm->itype);
}

/*--------------------------------------------------------------
 * clone()
 */
gfsmAutomaton *gfsm_automaton_clone_default(gfsmAutomaton *src)
{
  return gfsm_automaton_copy(gfsm_automaton_classed_new_full(src->type,
							     src->flags,
							     src->sr->type,
							     0, 0),
			     src);
}


/*--------------------------------------------------------------
 * copy()
 */
gfsmAutomaton *gfsm_automaton_copy_default(gfsmAutomaton *dst, gfsmAutomaton *src)
{
  gfsmStateId qid;
  gfsmArcIter ai;
  gfsmWeight  fw;

  gfsm_automaton_clear(dst);
  gfsm_automaton_copy_shallow(dst,src);

  //-- copy: reserve space
  gfsm_automaton_reserve_states(dst, gfsm_automaton_n_states(src));
  //gfsm_automaton_reserve_arcs(dst, gfsm_automaton_n_arcs(src));

  for (qid=0; qid < gfsm_automaton_n_states(src); qid++) {
    if (!gfsm_automaton_has_state(src,qid)) continue;
    gfsm_automaton_add_state_full(dst,qid);

    if (gfsm_automaton_lookup_final(src,qid,&fw))
      gfsm_automaton_set_final_state_full(dst,qid,TRUE,fw);

    for (gfsm_arciter_open(&ai,src,qid); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      gfsm_automaton_add_arc(dst,qid,a->target,a->lower,a->upper,a->weight);
    }
    gfsm_arciter_close(&ai);
  }

  return dst;
}


/*--------------------------------------------------------------
 * clear()
 */
void gfsm_automaton_clear_default(gfsmAutomaton *fsm)
{
  gfsm_die_no_impl(clear,fsm->itype);
}

/*--------------------------------------------------------------
 * free()
 */
void gfsm_automaton_free_default(gfsmAutomaton *fsm)
{
  gfsm_die_no_impl(free,fsm->itype);
}




/*======================================================================
 * Methods: Automaton Structure
 */

/*--------------------------------------------------------------
 * reserve_states()
 */
void gfsm_automaton_reserve_states_default(gfsmAutomaton *fsm, gfsmStateId n_states)
{
  gfsm_die_no_impl(reserve_states,fsm->itype);
}

/*--------------------------------------------------------------
 * reserve_arcs()
 */
void gfsm_automaton_reserve_arcs_default(gfsmAutomaton *fsm, gfsmArcId n_arcs)
{
  gfsm_die_no_impl(reserve_arcs,fsm->itype);
}

/*--------------------------------------------------------------
 * n_states()
 */
guint gfsm_automaton_n_states_default(gfsmAutomaton *fsm)
{
  gfsmStateId qid;
  for (qid=0; qid < gfsmNoState; qid++) {
    if (!gfsm_automaton_has_state(fsm,qid)) return qid;
  }
  return gfsmNoState;
}

/*--------------------------------------------------------------
 * n_arcs_default()
 */
guint gfsm_automaton_n_arcs_default(gfsmAutomaton *fsm)
{
  return gfsm_automaton_n_arcs_full(fsm,NULL,NULL,NULL);
}

/*--------------------------------------------------------------
 * n_final_states_default()
 */
guint gfsm_automaton_n_final_states_default(gfsmAutomaton *fsm)
{
  gfsmStateId qid;
  guint n_final=0;
  for (qid=0; qid < gfsm_automaton_n_states(fsm); qid++) {
    if (gfsm_automaton_is_final_state(qid)) ++n_final;
  }
  return n_final;
}

/*--------------------------------------------------------------
 * get_root()
 */
gfsmStateId gfsm_automaton_get_root_default(gfsmAutomaton *fsm)
{
  return 0;
}

/*--------------------------------------------------------------
 * set_root()
 */
gfsmStateId gfsm_automaton_set_root_default(gfsmAutomaton *fsm, gfsmStateId qid)
{
  gfsm_die_no_impl(set_root,fsm->itype);
}

/*--------------------------------------------------------------
 * renumber_states_full()
 */
void gfsm_automaton_renumber_states_full_default(gfsmAutomaton *fsm, GArray *old2new, gfsmStateId n_new_states);
{
  gfsm_die_no_impl(renumber_states_full,fsm->itype);
}



/*======================================================================
 * API: Automaton States
 */

/*--------------------------------------------------------------
 * has_state()
 */
gboolean gfsm_automaton_has_state_default(gfsmAutomaton *fsm, gfsmStateId qid)
{
  return FALSE;
}

/*--------------------------------------------------------------
 * add_state_full()
 */
gfsmStateIdVal gfsm_automaton_add_state_full_default(gfsmAutomaton *fsm, gfsmStateIdVal qid)
{
  gfsm_die_no_impl(add_state_full,fsm->itype);
}

/*--------------------------------------------------------------
 * remove_state()
 */
void gfsm_automaton_remove_state_default(gfsmAutomaton *fsm, gfsmStateId qid)
{
  gfsm_die_no_impl(remove_state,fsm->itype);
}

/*--------------------------------------------------------------
 * open_state()
 */
gfsmState *gfsm_automaton_open_state_default(gfsmAutomaton *fsm, gfsmStateId qid)
{
  gfsm_die_no_impl(open_state,fsm->itype);
}

/*--------------------------------------------------------------
 * close_state()
 */
void gfsm_automaton_close_state_default(gfsmAutomaton *fsm, gfsmStateId qid)
{
  gfsm_die_no_impl(close_state,fsm->itype);
}



/*--------------------------------------------------------------
 * lookup_final
 */
gboolean gfsm_automaton_lookup_final_default(gfsmAutomaton *fsm, gfsmStateId id, gfsmWeight *wp)
{
  *wp = fsm->sr->zero;
  return FALSE;
}

/*--------------------------------------------------------------
 * set_final_state_full
 */
void gfsm_automaton_set_final_state_full_default(gfsmAutomaton *fsm,
						 gfsmStateIdVal qid,
						 gboolean       is_final,
						 gfsmWeight     final_weight)
{
  gfsm_die_no_impl(set_final_state_full,fsm->itype);
}


/*--------------------------------------------------------------
 * out_degree()
 */
guint gfsm_automaton_out_degree_default(gfsmAutomaton *fsm, gfsmStateId qid)
{
  guint deg;
  gfsmArcIter ai;
  if (!gfsm_automaton_has_state(fsm,qid)) return 0;
  for (deg=0,gfsm_arciter_open(&ai,fsm,qid); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai),++deg) { ; }
  return deg;
}


/*======================================================================
 * Methods: Accessors: Automaton Arcs
 */

/*--------------------------------------------------------------
 * add_arc()
 */
void gfsm_automaton_add_arc_default(gfsmAutomaton *fsm,
				    gfsmStateId q1,
				    gfsmStateId q2,
				    gfsmLabelId lo,
				    gfsmLabelId hi,
				    gfsmWeight  w)
{
  gfsm_die_no_impl(add_arc,fsm->itype);
}

/*--------------------------------------------------------------
 * arcsort_full()
 */
void gfsm_automaton_arcsort_full_default(gfsmAutomaton *fsm, GCompareDataFunc cmpfunc, gpointer data)
{
  gfsm_die_no_impl(arcsort_full,fsm->itype);
}
