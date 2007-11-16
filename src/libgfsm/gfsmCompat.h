/*=============================================================================*\
 * File: gfsmCompat.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: generic automata
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

/** \file gfsmCompat.h
 *  \brief Backwards-compatibility definitions
 */

#ifndef _GFSM_COMPAT_H
#define _GFSM_COMPAT_H

/** Type for extended state-identifiers (parameters and return values)
 *  \todo ensure that we can safely fit a pointer in ::gfsmStateIdVal , ::gfsmLabelIdVal , etc.
 */
typedef guint gfsmStateIdVal;

/** Alias for state identifiers */
typedef gfsmStateId gfsmNodeId;

/** Constant indicating missing weight
 *  \warning Deprecated: prefer gfsm_sr_one()
 */
extern const gfsmWeight gfsmNoWeight;


/// Automaton status flags (gfsm <= v0.1.0)
typedef struct {
  guint32 is_transducer     : 1;       /**< whether this automaton is a transducer */
  guint32 is_weighted       : 1;       /**< whether this automaton is weighted */
  guint32 sort_mode         : 4;       /**< sort-mode (cast to gfsmArcSortMode) */
  guint32 is_deterministic  : 1;       /**< whether fsm is known to be deterministic */
  guint32 unused            : 25;      /**< reserved */
} gfsmAutomatonFlags_009;

/** Backwards-compatible alias for gfsmAutomatonDefaultNStates */
#define gfsmAutomatonDefaultSize gfsmAutomatonDefaultNStates

/** Allocate and return a new ::gfsmAutomaton, backwards-compatible */
gfsmAutomaton *gfsm_automaton_new_full_compat(gfsmAutomatonFlags flags, gfsmSRType srtype, gfsmStateId size);

/** Backwards-compatible alias for gfsm_automaton_reserve_states()
 *  \param fsm automaton to modify
 *  \param n_states number of states to reserve
 */
#define gfsm_automaton_reserve(fsm,n_states) \
   gfsm_automaton_reserve_states((fsm),(n_states))

/** Backwards-compatible alias for gfsm_automaton_open_state() */
#define gfsm_automaton_find_state(fsm,id) \
   gfsm_automaton_open_state((fsm),(id))

/** Backwards-compatible alias for gfsm_automaton_open_state() */
#define gfsm_automaton_find_state_const(fsm,id) \
   ((const gfsmState*)gfsm_automaton_open_state((fsm),(id)))

/** Backwards-compatible alias for gfsm_automaton_open_state_force() */
#define gfsm_automaton_get_state(fsm,id) \
   gfsm_automaton_open_state_force((fsm),(id))

/** Set boolean final-state flag: Backwards-compatible alias for gfsm_automaton_set_final_state_full() */
#define gfsm_automaton_set_final_state(fsm,id,is_final) \
   gfsm_automaton_set_final_state_full((fsm),(id),(is_final),(fsm)->sr->one)


/** Add an arc given pointers \a sp to the state and \a link to a
 *  single-element arclist to be added.  No states are
 *  implicitly created.
 *
 *  \deprecated { prefer gfsm_automaton_add_arc() }
 */
void gfsm_automaton_add_arc_link(gfsmAutomaton *fsm,
				 gfsmState     *sp,
				 gfsmArcList   *link);

#endif /* _GFSM_COMPAT_H */
