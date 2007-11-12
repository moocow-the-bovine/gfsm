
/*=============================================================================*\
 * File: gfsmDefaultImpl.h
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

/** \file gfsmDefaultImpl.h
 *  \brief Default methods for ::gfsmAutomaton API
 */

#ifndef _GFSM_DEFAULT_IMPL_H
#define _GFSM_DEFAULT_IMPL_H

#include <gfsmAutomatonAPI.h>

/*======================================================================
 * API: constructors, etc.
 */
///\name API: Constructors, etc.
//@{

/** Create and initialize a new implementation of \c fsm->itype in \c fsm->impl.
 *  Flags and semiring (type) should already have been intialized when this function is called.
 *  \warning Default implementation aborts with an error.
 */
void gfsm_automaton_init_default(gfsmAutomaton *fsm, guint n_states, guint n_arcs);

/** Create a new gfsmAutomaton by cloning an existing one.
 *
 *  \b Requirements on \c src:
 *  \li gfsm_automtaton_copy()
 *  \li gfsm_automaton_classed_new_full()
 *
 */
gfsmAutomaton *gfsm_automaton_clone_default(gfsmAutomaton *src);

/** Assign the contents of \a src to \a dst, without altering \a dst's implementation class.
 *
 *  \param dst target automaton
 *  \param src source automaton
 *  \returns modified \a dst
 *
 *  \b Requirements on \c dst:
 *  \li gfsm_automaton_clear()
 *  \li gfsm_automaton_reserve_states()
 *  \li (not called) gfsm_automaton_reserve_arcs()
 *  \li gfsm_automaton_add_state_full()
 *  \li gfsm_automaton_set_final_state_full()
 *  \li gfsm_automaton_add_arc()
 *
 *  \b Requirements on \c src:
 *  \li gfsm_automaton_n_states()
 *  \li gfsm_automaton_has_state()
 *  \li (not called) gfsm_automaton_n_arcs()
 *  \li gfsm_automaton_lookup_final()
 *  \li gfsm_arciter_open() & friends
 *
 */
gfsmAutomaton *gfsm_automaton_copy_default(gfsmAutomaton *dst, gfsmAutomaton *src);

/** Clear a ::gfsmAutomaton
 *  \warning Default implementation aborts with an error message.
 */
void gfsm_automaton_clear_default(gfsmAutomaton *fsm);

/** Destroy an automaton implementation
 *  \param fsm automaton whose implementation is to be destroyed
 *  \warning Default implementation aborts with an error.
 */
void gfsm_automaton_free_default(gfsmAutomaton *fsm);

//@}

/*======================================================================
 * API: Automaton Structure
 */
/// \name API: Automaton Structure
//@{

/** Reserve space for at least \a n_states states (may do nothing)
 *  \param fsm automaton to modify
 *  \param n_states number of states to reserve, if supported by implementation
 *
 *  \warning Default implementation aborts with an error.
 */
void gfsm_automaton_reserve_states_default(gfsmAutomaton *fsm, gfsmStateId n_states);

/** Reserve space for at least \a n_arcs arcs (may do nothing)
 *  \param fsm automaton to modify
 *  \param n_states number of arcs to reserve, if supported by implementation
 *
 *  \warning Default implementation aborts with an error.
 */
void gfsm_automaton_reserve_arcs_default(gfsmAutomaton *fsm, gfsmArcId n_arcs);

/** Get number of states.
 *  Default implementation just returns the minimum ::gfsmStateId \c qid for which
 *  gfsm_automaton_has_state(fsm,qid) returns FALSE: O(n_states) time.
 *
 */
guint gfsm_automaton_n_states_default(gfsmAutomaton *fsm);

/** Get number of arcs n \c fsm.
 *
 *  \b Requirements on \c fsm:
 *  \li gfsm_automaton_n_states()
 *  \li gfsm_arciter_open() and friends
 *
 *  \note Default implementation calls gfsm_automaton_n_arcs_full(): O(n_arcs) time.
 */
guint gfsm_automaton_n_arcs_default(gfsmAutomaton *fsm);

/** Get number of final states
 *  \note Default implementaion calls gfsm_automaton_state_is_final() on all states; O(n_states) time.
 */
guint gfsm_automaton_n_final_states_default(gfsmAutomaton *fsm);

/** Get ID of root node, or gfsmNoState if undefined
 *  \note Default implementation just returns 0 (zero).
 */
gfsmStateId gfsm_automaton_get_root_default(gfsmAutomaton *fsm);

/** Set ID of root node, creating state if necessary
 *  \warning Default implementation aborts with an error.
 */
void gfsm_automaton_set_root_default(gfsmAutomaton *fsm, gfsmStateId qid);

/** Renumber states of an automaton using a user-specified renumbering scheme.
 *  Destructively alters \a fsm.
 *  \param fsm
 *    automaton whose states are to be renumbered
 *  \param old2new
 *    ::GArray of ::gfsmStateId s.t. \a qid_new=old2new[qid_old], where \a newid may be ::gfsmNoState to ignore
 *  \param n_new_states
 *     number of new states, or 0 (zero) to auto-compute
 *  \note
 *    Should be guaranteed \b not to increase either the number of states nor the number of arcs in \a fsm.
 *  \warning Default implementation aborts with an error.
 */
void gfsm_automaton_renumber_states_full_default(gfsmAutomaton *fsm, GArray *old2new, gfsmStateId n_new_states);

//@}

/*======================================================================
 * API: Automaton States
 */
/// \name API: Automaton States
//@{

//----------------------------------------------
// States: ok/add/remove

/** Check whether automaton \a fsm has a valid state with ID \a qid.
 *  \param fsm automaton to examine
 *  \param qid ID of state to examine
 *  \returns TRUE if \a fsm has a state with ID \a id, FALSE otherwise.
 *
 *  \note Default implementation always returns FALSE.
 */
gboolean gfsm_automaton_has_state_default(gfsmAutomaton *fsm, gfsmStateId qid);

/** Add a new state, specifying qid.  If \a qid is gfsmNoState,
 *  first available state qid will be selected.
 *
 *  \param fsm fsm to modify
 *  \param qid  identifier of the new state, or gfsmNoState
 *  \returns ID of the new state, or gfsmNoState on failure
 *
 *  \warning Default implementation aborts with an error message.
 */
gfsmStateId gfsm_automaton_add_state_full_default(gfsmAutomaton *fsm, gfsmStateId qid);

/** Remove the state with ID \a qid, if any.
 *  May have no effect on incoming arcs for state \a qid,
 *  although any outgoing arcs should be removed and freed.
 *
 *  \param fsm automaton from which to remove a state
 *  \param qid ID of the state to be removed
 *
 *  \warning Default implementation aborts with an error message.
 */
void gfsm_automaton_remove_state_default(gfsmAutomaton *fsm, gfsmStateId qid);

//----------------------------------------------
// States: open/close

/** Get a pointer to the ::gfsmState object with the ID \a qid in \a fsm, if any.
 *  When you are done with the returned pointer, you must release it
 *  with gfsm_automaton_state_close().
 *
 *  \param fsm fsm from which to extract a state
 *  \param qid ID of the state to extract
 *  \returns a ::gfsmState* for \a qid, or NULL if \a qid not a state of \a fsm
 *
 *  \warning Default implementation aborts with an error message.
 */
gfsmState *gfsm_automaton_open_state_default(gfsmAutomaton *fsm, gfsmStateId qid);

/** Close a pointer to a ::gfsmState object which was previously opened by
 *  gfsm_automaton_open_state().
 *
 *  \param fsm fsm for which the state was opened
 *  \param sp  state pointer returned by gfsm_automaton_open_state()
 *
 *  \warning
 *     Bad things may happen if you change the implementation class of \a fsm
 *     while you have open state pointers for it hanging around.
 *
 *  \warning Default implementation aborts with an error message.
 */
void gfsm_automaton_close_state_default(gfsmAutomaton *fsm, gfsmState *sp);

//----------------------------------------------
// States: finality

/** Lookup final weight for state with ID \a qid in automaton \a fsm.
 *  \param fsm automaton to examine
 *  \param qid ID of state to examine
 *  \param  wp output parameter for final weight
 *  \returns
 *     TRUE if state \a id is final, FALSE otherwise,
 *
 *  \note Default implementation just sets \c *wp=fsm->sr->zero and returns FALSE.
 */
gboolean gfsm_automaton_lookup_final_default(gfsmAutomaton *fsm, gfsmStateId qid, gfsmWeight *wp);

/** Set final-weight and/or final-states membership flag for state with ID \a qid in \a fsm.
 * \param fsm automaton to modify
 * \param qid ID of state to modified
 * \param is_final whether state should be considered final
 * \param final_weight
 *    If \a is_final is true, final weight for state.  Otherwise
 *    final weight should implicitly be set to \a (fsm)->sr->zero
 *
 * \warning Default implementation aborts with an error.
 */
void gfsm_automaton_set_final_state_full_default(gfsmAutomaton *fsm,
						 gfsmStateId qid,
						 gboolean       is_final,
						 gfsmWeight     final_weight);


//----------------------------------------------
// States: properties

/** Get number of outgoing arcs for state with ID \a qid in \a fsm
 *  \param fsm automaton to examine
 *  \param qid ID of state to examine
 *  \returns output degree of \c qid in \c fsm
 *
 *  \note Default implementation uses ::gfsmArcIter interface, and is O(out_degree(qid)) time.
 */
guint gfsm_automaton_out_degree_default(gfsmAutomaton *fsm, gfsmStateId qid);

//@}


/*======================================================================
 * API: Automaton Arcs
 */
/// \name API: Automaton Arcs
//@{

/** Add an arc from state with ID \c qid1 to state with ID \c qid2
 *  on labels (\c lo,\c hi) with weight \c w.
 *  Missing states should be implicitly created.
 *  \warning may not be supported by all implementation classes
 *  \param fsm Automaton to modify
 *  \param qid1 ID of source state
 *  \param qid2 ID of target state
 *  \param lo   Lower label
 *  \param hi   Upper label
 *  \param w    Arc weight
 *
 *  \warning Default implementation aborts with an error message.
 */
void gfsm_automaton_add_arc_default(gfsmAutomaton *fsm,
				    gfsmStateId    qid1,
				    gfsmStateId    qid2,
				    gfsmLabelVal   lo,
				    gfsmLabelVal   hi,
				    gfsmWeight     w);

/** Sort all arcs in the automaton by a user-specified comparison function.
 *  \param fsm
 *    Automaton to modify
 *  \param cmpfunc
 *    3-way comparison function, called as \a (*cmpfunc)(gfsmArc *a1p, gfsmArc *a2p, gpointer data)
 *    to compare arcs a1p and a2p.
 *  \param data
 *    User data for \a cmpfunc
 *
 *  \warning default implementation aborts with an error message.
 */
void gfsm_automaton_arcsort_full_default(gfsmAutomaton *fsm, GCompareDataFunc cmpfunc, gpointer data);


/*-- todo:

+ list-wise arc methods:
  open_arcs_list(fsm,qid)
  set_arcs_list(fsm,qid,arcs_l)
  close_arcs_list(fsm,arcs_l)
  (?) sort_arcs_list_full(arcs_l, cmpfunc, data)

+ array-wise arc methods:
  open_arcs_array(fsm,qid)
  set_arcs_array(fsm,qid,arcs_a)
  close_arcs_array(fsm,arcs_a)
  (?) sort_arcs_array_full(arcs_a, cmpfunc, data)

+ arc-iterator methods:
  arciter_open(fsm,qid)
  arciter_open_ptr(fsm,qp)
  (?) arciter_seek_lower(aip,lab)
  (?) arciter_seek_upper(aip,lab)
  (?) arciter_seek_both(aip,lo,hi)
  (??) arciter_open_lower(fsm,qid,lab)
  (??) arciter_open_upper(fsm,qid,lab)
*/

//@}

#endif /* _GFSM_AUTOMATON_H */
