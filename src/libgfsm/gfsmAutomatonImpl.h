
/*=============================================================================*\
 * File: gfsmAutomatonImpl.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: generic automaton implementation API
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

/** \file gfsmAutomatonImpl.h
 *  \brief Generic automaton implementation API
 */

#ifndef _GFSM_AUTOMATON_IMPL_H
#define _GFSM_AUTOMATON_IMPL_H

#include <gfsmState.h>

/*======================================================================
 * Types
 */

/// Known automaton implementation classes
/** Used to resolve \a impl.data for a ::gfsmAutomatonImpl \a impl */
typedef enum {
  gfsmACUnknown,            /**< unknown class; should never happen in practice */
  gfsmACOldBasic,           /**< backwards-compatible implementation, see ::gfsmOldBasicImpl */
  gfsmACBasic,              /**< basic implementation; see ::gfsmBasicImpl */
  gfsmACArcTable,           /**< (read-only) arc table implementation; see ::gfsmArcTableImpl */
  gfsmACFullTable,          /**< (read-only) full table implementation; see ::gfsmFullTableImpl */
  gfsmACVirtual             /**< user-defined full-virtual implementation; see ::gfsmVirtualImpl */
} gfsmAutomatonImplClass;

/// Automaton implementation data union type, for comfortable typecasting
typedef union {
  gpointer                    p;   /**< Generic (void*) */
  struct gfsmOldBasicImpl_  *ob;   /**< backwards-compatible implementation, see ::gfsmOldBasicImpl */
  struct gfsmBasicImpl_      *b;   /**< basic implementation; see ::gfsmBasicImpl */
  struct gfsmArcTableImpl_  *at;   /**< (read-only) arc table implementation; see ::gfsmArcTableImpl */
  struct gfsmFullTableImpl_ *ft;   /**< (read-only) full table implementation; see ::gfsmFullTableImpl */
  struct gfsmVirtualImpl_    *v;   /**< user-defined full-virtual implementation; see ::gfsmVirtualImpl */
} gfsmAutomatonImplData;

/// Generic automaton implementation type
typedef struct {
  gfsmAutomatonImplClass itype;    /**< implementation type */
  gfsmSemiring           *sr;      /**< semiring used for arc weight computations */
  gfsmAutomatonImplData  data;     /**< implementation data */
} gfsmAutomatonImpl;


/*======================================================================
 * API: constructors, etc.
 */
///\name API: Constructors, etc.
//@{

/** Initialize a ::gfsmAutomatonImpl of type \c itype.
 *  \note required
 *  \param fsmi     implementation to initialize (assumed empty but non-NULL)
 *  \param itype    type of implementation to initialize
 *  \param sr       semiring for arc weight computations
 *  \param n_states number of states to pre-allocate (may be ignored)
 *  \param n_arcs   number of arcs to pre-allocate (may be ignored)
 */
void gfsm_automaton_impl_init(gfsmAutomatonImpl       *fsmi,
			      gfsmAutomatonImplClass   itype,
			      gfsmSemiring            *sr,
			      gfsmStateIdVal           n_states,
			      gfsmArcIdVal             n_arcs
			      );

/** Copy a ::gfsmAutomatonImpl from \c src to \c dst.
 *  \note optional
 *  \param src
 *    source implementation
 *  \param dst
 *    target implementation, assumed allocated but empty.
 */
void gfsm_automaton_impl_copy(gfsmAutomatonImpl *dst, gfsmAutomatonImpl *src);

/** Clear contents of a ::gfsmAutomatonImpl
 *  \note optional, may produce error
 *  \param fsmi implementation to be cleared
 */
void gfsm_automaton_impl_clear(gfsmAutomatonImpl *fsmi);

/** Destroy a ::gfsmAutomatonImpl */
void gfsm_automaton_impl_free(gfsmAutomatonImpl *fsmi);

//@}

/*======================================================================
 * API: Implementation-Dependent
 */
/// \name API: Implementation-Dependent

//@{

/** Get number of states. \note recommended */
guint gfsm_automaton_impl_n_states(gfsmAutomatonImpl *fsmi);

/** Get number of final states \note recommended */
guint gfsm_automaton_impl_n_final_states(gfsmAutomaton *fsmi);

/** Get total number of arcs. \note optional */
guint gfsm_automaton_impl_n_arcs(gfsmAutomatonImpl *fsmi);

/** Get ID of root node, or gfsmNoState if undefined \note recommended */
gfsmStateIdVal gfsm_automaton_impl_get_root(gfsmAutomatonImpl *fsmi);

/** Set ID of root node, creating state if necessary \note recommended */
void gfsm_automaton_impl_set_root(gfsmAutomatonImpl *fsmi, gfsmStateIdVal qid);

/** Reserve space for at least \a n_states states \note optional
 *  \param fsm automaton to modify
 *  \param n_states number of states to reserve, if supported by implementation
 */
void gfsm_automaton_impl_reserve_states(gfsmAutomatonImpl *fsmi, gfsmStateIdVal n_states);

/** Reserve space for at least \a n_arcs arcs \note optional
 *  \param fsm automaton to modify
 *  \param n_states number of arcs to reserve, if supported by implementation
 */
void gfsm_automaton_impl_reserve_arcs(gfsmAutomatonImpl *fsmi, gfsmStateIdVal n_states);

//@}

/*======================================================================
 * API: Automaton States
 */
/// \name API: Automaton States
//@{

/** Add a new state, specifying qid.  If \a qid is gfsmNoState,
 *  first available state qid will be selected.
 *  \warning not supported by all implementations!
 *
 *  \param fsm fsm to modify
 *  \param qid  identifier of the new state, or gfsmNoState
 *  \returns ID of the new state, or gfsmNoState on failure
 */
gfsmStateIdVal gfsm_automaton_impl_add_state_full(gfsmAutomatonImpl *fsmi, gfsmStateIdVal qid);

/** Get a pointer to the ::gfsmState object with the ID \a qid in \a fsm, if any.
 *  When you are done with the returned pointer, you must release it
 *  with gfsm_automaton_state_close().
 *  \param fsm fsm from which to extract a state
 *  \param qid ID of the state to extract
 *  \returns a ::gfsmState* for \a qid, or NULL if \a qid not a state of \a fsm
 */
gfsmState *gfsm_automaton_impl_open_state(gfsmAutomatonImpl *fsmi, gfsmStateIdVal qid);

/** Close a pointer to a ::gfsmState object which was previously opened by
 *  gfsm_automaton_open_state().
 *  \param fsm fsm for which the state was opened
 *  \param sp  state pointer returned by gfsm_automaton_open_state()
 *  \warning
 *     Bad things may happen if you change the implementation class of \a fsm
 *     while you have open state pointers for it hanging around.
 */
void gfsm_automaton_impl_close_state(gfsmAutomatonImpl *fsmi, gfsmState *sp);

/** Check whether automaton \a fsm has a valid state with ID \a qid.
 *  \param fsm automaton to examine
 *  \param qid ID of state to examine
 *  \returns TRUE if \a fsm has a state with ID \a id, FALSE otherwise.
 */
gboolean gfsm_automaton_impl_has_state(gfsmAutomatonImpl *fsmi, gfsmStateIdVal qid);

/** Remove the state with ID \a qid, if any.
 *  \warning not supported by all implementations.
 *  \warning
 *     May have no effect on incoming arcs for state \a qid,
 *     although any outgoing arcs should be removed and freed.
 *  \param fsm automaton from which to remove a state
 *  \param qid ID of the state to be removed
 */
void gfsm_automaton_impl_remove_state(gfsmAutomatonImpl *fsmi, gfsmStateIdVal qid);


/** Check whether the state with ID \a qid is final in \a fsm  \note optional
 *  \param fsm automaton to examine
 *  \param qid ID of state to check for finality
 *  \returns TRUE if \a qid is final in \a fsm, FALSE otherwise.
 */
gboolean gfsm_automaton_impl_is_final_state(gfsmAutomatonImpl *fsmi, gfsmStateIdVal qid);

/** Set final-weight and/or final-states membership flag for state with ID \a qid in \a fsm.
 * \param fsm automaton to modify
 * \param qid ID of state to modified
 * \param is_final whether state should be considered final
 * \param final_weight
 *    If \a is_final is true, final weight for state.  Otherwise
 *    final weight should implicitly be set to \a (fsm)->sr->zero
 */
void gfsm_automaton_impl_set_final_state(gfsmAutomatonImpl *fsmi,
					 gfsmStateIdVal qid,
					 gboolean       is_final,
					 gfsmWeight     final_weight);

/** Lookup final weight for state with ID \a qid in automaton \a fsm.
 *  \param fsm automaton to examine
 *  \param qid ID of state to examine
 *  \param  wp output parameter for final weight
 *  \returns
 *     TRUE if state \a id is final, FALSE otherwise,
 */
gboolean gfsm_automaton_impl_lookup_final(gfsmAutomatonImpl *fsmi, gfsmStateIdVal qid, gfsmWeight *wp);

/** Get number of outgoing arcs for state with ID \a qid in \a fsm  \note optional
 *  \param fsm automaton to examine
 *  \param qid ID of state to examine
 *  \returns output degree of \c qid in \c fsm
 */
guint gfsm_automaton_impl_out_degree(gfsmAutomatonImpl *fsmi, gfsmStateIdVal qid);

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
 */
void gfsm_automaton_impl_add_arc(gfsmAutomatonImpl *fsmi,
				 gfsmStateIdVal qid1,
				 gfsmStateIdVal qid2,
				 gfsmLabelVal   lo,
				 gfsmLabelVal   hi,
				 gfsmWeight     w);

/** Sort all arcs in the automaton by a user-specified comparison function.
 *  \param fsm
 *    Automaton to modify
 *  \param cmpfunc
 *    3-way comparison function, called as \c (*cmpfunc)(gfsmArc *a1p, gfsmArc *a2p, gpointer data)
 *    to compare arcs a1p and a2p.
 *  \param data
 *    User data for \c cmpfunc
 *  \returns
 *    modified \c fsm
 */
gfsmAutomaton *gfsm_automaton_impl_arcsort(gfsmAutomaton *fsm, GCompareDataFunc cmpfunc, gpointer data);

/*-- todo: list-wise arc methods:
  open_arcs_list(fsm,qid)
  set_arcs_list(fsm,qid,arcs_l)
  close_arcs_list(fsm,arcs_l)
  (?) sort_arcs_list_user(arcs_l, cmpfunc, data)

  open_arcs_array(fsm,qid)
  set_arcs_array(fsm,qid,arcs_a)
  close_arcs_array(fsm,arcs_a)
  (?) sort_arcs_array_user(arcs_a, cmpfunc, data)
*/

//@}

#endif /* _GFSM_AUTOMATON_H */
