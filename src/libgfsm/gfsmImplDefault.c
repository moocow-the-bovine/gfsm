
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

#include <gfsmAutomaton.h>
#include <gfsmIO.h>
#include <gfsmError.h>
#include <gfsmMem.h>
#include <stdlib.h>


/*--------------------------------------------------------------
 * "pure-virtual" error message
 */
#define gfsm_die_no_impl(fnc,itype) \
  g_error("libgfsm ERROR: API function `gfsm_" # fnc "()' is not implemented for automaton class `%s'\n", gfsm_class_name(itype))

/*======================================================================
 * API: Constructors etc.
 */

/*--------------------------------------------------------------
 * init()
 */
void gfsm_automaton_init_default(gfsmAutomaton *fsm, guint n_states, guint n_arcs)
{
  gfsm_die_no_impl(automaton_init,fsm->itype);
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
  gfsm_die_no_impl(automaton_clear,fsm->itype);
}

/*--------------------------------------------------------------
 * free()
 */
void gfsm_automaton_free_default(gfsmAutomaton *fsm)
{
  gfsm_die_no_impl(automaton_free,fsm->itype);
}




/*======================================================================
 * API: Automaton Structure
 */

/*--------------------------------------------------------------
 * reserve_states()
 */
void gfsm_automaton_reserve_states_default(gfsmAutomaton *fsm, gfsmStateId n_states)
{
  gfsm_die_no_impl(automaton_reserve_states,fsm->itype);
}

/*--------------------------------------------------------------
 * reserve_arcs()
 */
void gfsm_automaton_reserve_arcs_default(gfsmAutomaton *fsm, gfsmArcId n_arcs)
{
  gfsm_die_no_impl(automaton_reserve_arcs,fsm->itype);
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
 * n_arcs()
 */
guint gfsm_automaton_n_arcs_default(gfsmAutomaton *fsm)
{
  return gfsm_automaton_n_arcs_full(fsm,NULL,NULL,NULL);
}

/*--------------------------------------------------------------
 * n_final_states()
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
  gfsm_die_no_impl(automaton_set_root,fsm->itype);
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
gfsmStateId gfsm_automaton_add_state_full_default(gfsmAutomaton *fsm, gfsmStateId qid)
{
  gfsm_die_no_impl(automaton_add_state_full,fsm->itype);
}

/*--------------------------------------------------------------
 * remove_state()
 */
void gfsm_automaton_remove_state_default(gfsmAutomaton *fsm, gfsmStateId qid)
{
  gfsm_die_no_impl(automaton_remove_state,fsm->itype);
}

/*--------------------------------------------------------------
 * open_state()
 */
gfsmState *gfsm_automaton_open_state_default(gfsmAutomaton *fsm, gfsmStateId qid)
{
  gfsm_die_no_impl(automaton_open_state,fsm->itype);
}

/*--------------------------------------------------------------
 * close_state()
 */
void gfsm_automaton_close_state_default(gfsmAutomaton *fsm, gfsmStateId qid)
{
  gfsm_die_no_impl(automaton_close_state,fsm->itype);
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
						 gfsmStateId    qid,
						 gboolean       is_final,
						 gfsmWeight     final_weight)
{
  gfsm_die_no_impl(automaton_set_final_state_full,fsm->itype);
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
  gfsm_arciter_close(&ai);
  return deg;
}


/*======================================================================
 * API: Automaton Arcs
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
  gfsm_die_no_impl(automaton_add_arc,fsm->itype);
}

/*--------------------------------------------------------------
 * arcsort_full()
 */
void gfsm_automaton_arcsort_full_default(gfsmAutomaton *fsm, GCompareDataFunc cmpfunc, gpointer data)
{
  gfsm_die_no_impl(automaton_arcsort_full,fsm->itype);
}

/*======================================================================
 * API: Arc Iterators
 */

/*--------------------------------------------------------------
 * arciter_ok()
 */
gboolean gfsm_arciter_ok_default(gfsmArcIter *aip)
{
  return FALSE;
}

/*--------------------------------------------------------------
 * arciter_close()
 */
void gfsm_arciter_close_default(gfsmArcIter *aip)
{
  memset(aip,0,sizeof(gfsmArcIter));
}

/*--------------------------------------------------------------
 * arciter_open()
 */
void gfsm_arciter_open_default(gfsmArcIter *aip, gfsmAutomaton *fsm, gfsmStateId qid)
{
  gfsm_die_no_impl(arciter_open, fsm->itype);
}


/*--------------------------------------------------------------
 * arciter_next()
 */
void gfsm_arciter_next_default(gfsmArcIter *aip)
{
  gfsm_die_no_impl(arciter_next, (aip->fsm ? aip->fsm->itype : gfsmACUnknown));
}

/*--------------------------------------------------------------
 * arciter_reset()
 */
void gfsm_arciter_reset_default(gfsmArcIter *aip)
{
  gfsm_arciter_open(aip, aip->fsm, aip->qid);
}

/*--------------------------------------------------------------
 * arciter_sort()
 */
void gfsm_arciter_sort_default(gfsmArcIter *aip, GCompareDataFunc cmpfunc, gpointer data)
{
  gfsm_die_no_impl(arciter_sort, (aip->fsm ? aip->fsm->itype : gfsmACUnknown));
}

/*--------------------------------------------------------------
 * arciter_copy()
 */
void gfsm_arciter_copy_default(gfsmArcIter *dst, gfsmArcIter *src)
{
  *dst = *src;
}


/*--------------------------------------------------------------
 * arciter_clone()
 */
gfsmArcIter *gfsm_arciter_clone_default(const gfsmArcIter *src)
{
  return (gfsmArcIter*)gfsm_mem_dup_n(src,sizeof(gfsmArcIter));
}


/*--------------------------------------------------------------
 * arciter_arc()
 */
gfsmArc *gfsm_arciter_arc_default(gfsmArcIter *aip)
{
  gfsm_die_no_impl(arciter_arc, (aip->fsm ? aip->fsm->itype : gfsmACUnknown));
}

/*--------------------------------------------------------------
 * arciter_remove()
 */
void gfsm_arciter_remove_default(gfsmArcIter *aip)
{
  gfsm_die_no_impl(arciter_remove, (aip->fsm ? aip->fsm->itype : gfsmACUnknown));
}

/*--------------------------------------------------------------
 * arciter_seek_both()
 */
void gfsm_arciter_seek_both_default(gfsmArcIter *aip, gfsmLabelVal lo, gfsmLabelVal hi)
{
  for ( ; gfsm_arciter_ok(aip); gfsm_arciter_next(aip)) {
    gfsmArc *a = gfsm_arciter_arc(aip);
    if ((lo==gfsmNoLabel || a->lower==lo) && (hi==gfsmNoLabel || a->upper==hi)) break;
  }
}

/*--------------------------------------------------------------
 * arciter_seek_lower()
 */
void gfsm_arciter_seek_lower_default(gfsmArcIter *aip, gfsmLabelVal lo)
{
  for ( ; gfsm_arciter_ok(aip); gfsm_arciter_next(aip)) {
    if (gfsm_arciter_arc(aip)->lower == lo) break;
  }
}

/*--------------------------------------------------------------
 * arciter_seek_upper()
 */
void gfsm_arciter_seek_upper_default(gfsmArcIter *aip, gfsmLabelVal hi)
{
  for ( ; gfsm_arciter_ok(aip); gfsm_arciter_next(aip)) {
    if (gfsm_arciter_arc(aip)->upper == hi) break;
  }
}


/*======================================================================
 * API: Automaton I/O
 */

/*--------------------------------------------------------------
 * save_bin_handle()
 */
gboolean gfsm_automaton_save_bin_handle_default(gfsmAutomaton        *fsm,
						gfsmAutomatonHeader  *hdr,
						gfsmIOHandle         *ioh,
						gfsmError           **errp)
{
  g_set_error(errp,
	      g_quark_from_static_string("gfsm"),                         //-- domain
	      g_quark_from_static_string("automaton_save_bin:no_method"), //-- code
	      "no implementation defined for automaton type");
  return FALSE;
}

/*--------------------------------------------------------------
 * load_bin_handle()
 */
gboolean gfsm_automaton_load_bin_handle_default(gfsmAutomaton       *fsm,
						gfsmAutomatonHeader *hdr,
						gfsmIOHandle        *ioh,
						gfsmError          **errp)
{
  g_set_error(errp,
	      g_quark_from_static_string("gfsm"),                         //-- domain
	      g_quark_from_static_string("automaton_load_bin:no_method"), //-- code
	      "no implementation defined for automaton type");
  return FALSE;
}

