
/*=============================================================================*\
 * File: gfsmAutomaton.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: automata
 *
 * Copyright (c) 2004-2007 Bryan Jurish.
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

/** \file gfsmAutomaton.h
 *  \brief Automaton definitions and low-level access
 */

#ifndef _GFSM_AUTOMATON_H
#define _GFSM_AUTOMATON_H

#include <glib.h>
#include <gfsmCommon.h>
#include <gfsmAlphabet.h>
#include <gfsmSemiring.h>
#include <gfsmArc.h>
#include <gfsmState.h>
#include <gfsmSet.h>
#include <gfsmWeightMap.h>
#include <gfsmBitVector.h>

/*======================================================================
 * Types
 */

/// Automaton status flags
typedef struct _gfsmAutomatonFlags {
  guint32 is_transducer    : 1;       /**< whether this automaton is a transducer */
  guint32 is_weighted      : 1;       /**< whether this automaton is weighted */
  guint32 sort_mode        : 4;       /**< sort-mode (cast to gfsmArcSortMode) */
  guint32 is_deterministic : 1;       /**< whether fsm is known to be deterministic */
  guint32 unused           : 25;      /**< reserved */
} gfsmAutomatonFlags;

/** \brief "Heavy" automaton type
 *  
 *  All automata are stored as weighted transducers.
 */
typedef struct
{
  gfsmAutomatonFlags  flags;     /**< automaton flags */
  gfsmSemiring       *sr;        /**< semiring used for arc weight computations */
  GArray             *states;    /**< vector of automaton states */
  gfsmWeightMap      *finals;    /**< map from final state-Ids to final weights */
  gfsmStateId         root_id;   /**< ID of root node, or gfsmNoState if not defined */
} gfsmAutomaton;

/*======================================================================
 * Constants
 */
/** Default initial automaton size */
extern const guint gfsmAutomatonDefaultSize;

/** Default initial automaton flags */
extern const gfsmAutomatonFlags gfsmAutomatonDefaultFlags;

/** Default semiring for automaton arc weights */
extern const gfsmSRType gfsmAutomatonDefaultSRType;

/*======================================================================
 * Methods: constructors, etc.
 */
/// \name Constructors etc.
//@{
/** Create a new gfsmAutomaton, preallocating \a size states */
gfsmAutomaton *gfsm_automaton_new_full(gfsmAutomatonFlags flags, gfsmSRType srtype, guint size);

/** Create a new gfsmAutomaton */
#define gfsm_automaton_new() \
   gfsm_automaton_new_full(gfsmAutomatonDefaultFlags,\
                           gfsmAutomatonDefaultSRType,\
                           gfsmAutomatonDefaultSize)

/** Create a new gfsmAutomaton by cloning an existing one */
#define gfsm_automaton_clone(fsm) \
  gfsm_automaton_copy(gfsm_automaton_new_full(gfsmAutomatonDefaultFlags,fsm->sr->type,0),fsm)

/** Assign non-structural contents of \a src to \a dst. \returns \a dst */
gfsmAutomaton *gfsm_automaton_copy_shallow(gfsmAutomaton *dst, gfsmAutomaton *src);

/** Assign the contents of fsm \a src to fsm \a dst \returns \a dst */
gfsmAutomaton *gfsm_automaton_copy(gfsmAutomaton *dst, gfsmAutomaton *src);

/** Create a new FSM whose non-structural contents match those of \a fsm */
#define gfsm_automaton_shadow(fsm) \
  gfsm_automaton_copy_shallow(gfsm_automaton_new(), fsm)

/** Swap the contents of fsms \a fsm1 and \a fsm2 */
void gfsm_automaton_swap(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2);

/** Clear an automaton */
void gfsm_automaton_clear(gfsmAutomaton *fsm);

/** Destroy an automaton: all associated states and arcs will be freed. */
void gfsm_automaton_free(gfsmAutomaton *fsm);
//@}

/*======================================================================
 * Methods: Accessors
 */
/// \name Accessors: Automaton
//@{
/** Get pointer to the semiring associated with this automaton */
#define gfsm_automaton_get_semiring(fsm) (fsm->sr)

/** Set the semiring associated with this automaton */
gfsmSemiring *gfsm_automaton_set_semiring(gfsmAutomaton *fsm, gfsmSemiring *sr);

/** Set the semiring associated with this automaton by semiring-type */
void gfsm_automaton_set_semiring_type(gfsmAutomaton *fsm, gfsmSRType srtype);

/** True iff automaton is a transducer */
#define gfsm_automaton_is_transducer(fsm) (fsm->flags.is_transducer)

/** True iff automaton is weighted */
#define gfsm_automaton_is_weighted(fsm) (fsm->flags.is_weighted)

/** Get current automaton arc-sort mode */
#define gfsm_automaton_sortmode(fsm) ((gfsmArcSortMode)(fsm->flags.sort_mode))

/** Get number of states (constant time) */
#define gfsm_automaton_n_states(fsm) (fsm->states->len)

/** Get number of final states (linear time w/ number of final states) */
#define gfsm_automaton_n_final_states(fsm) (gfsm_weightmap_size(fsm->finals))

/** Get total number of arcs (linear time w/ number of arcs) */
//guint gfsm_automaton_n_arcs(gfsmAutomaton *fsm);
#define gfsm_automaton_n_arcs(fsm) gfsm_automaton_n_arcs_full(fsm,NULL,NULL,NULL)

/** Get verbose summary arc information (linear time w/ number of arcs)
 *  \returns total number of arcs
 */
guint gfsm_automaton_n_arcs_full(gfsmAutomaton *fsm,
				 guint *n_lo_epsilon,
				 guint *n_hi_epsilon,
				 guint *n_both_epsilon);

/** Get Id of root node, or gfsmNoState if undefined */
#define gfsm_automaton_get_root(fsm) (fsm->root_id)

/** Set Id of root node, creating state if necessary */
#define gfsm_automaton_set_root(fsm,id) \
   (fsm->root_id = gfsm_automaton_ensure_state(fsm,id))

/** Reserve space for at least \a n states */
void gfsm_automaton_reserve(gfsmAutomaton *fsm, gfsmStateId nstates);

/** Utility function for gfsm_automaton_is_cyclic() */
gboolean gfsm_automaton_is_cyclic_state(gfsmAutomaton *fsm,
					gfsmStateId    id,
					gfsmBitVector *visited,
					gfsmBitVector *completed);

/** Test whether automaton is cyclic */
gboolean gfsm_automaton_is_cyclic(gfsmAutomaton *fsm);

/** Test whether automaton is acyclic */
#define gfsm_automaton_is_acyclic(fsm) (!gfsm_automaton_is_cyclic(fsm))

/** Extract automaton-internal labels to \a alph.  If \a alph is NULL,
 *  a new default alphabet will be created and returned (you will need to
 *  free it yourself).
 *
 *  The alphabet should be able to match literal label values to themselved
 *  (i.e. don't pass a string alphabet)
 *
 *  \a which determines which side to extract.
 *
 * \returns \a alph
 */
gfsmAlphabet *gfsm_automaton_get_alphabet(gfsmAutomaton *fsm,
					  gfsmLabelSide  which,
					  gfsmAlphabet  *alph);
//@}

/*======================================================================
 * Methods: Accessors: Automaton: States
 */
/// \name Accessors: Automaton States
//@}
/** Add a new state, specifying id.  If \a id is gfsmNoState,
 *  first available state id will be selected.
 *  Implicitly sets root_id
 * \returns Id of the new state
 */
gfsmStateId gfsm_automaton_add_state_full(gfsmAutomaton *fsm, gfsmStateId id);

/** Ensures that state \a id exists \returns \a id */
#define gfsm_automaton_ensure_state(fsm,id) \
   gfsm_automaton_add_state_full(fsm,id)

/** Add a new state. */
#define gfsm_automaton_add_state(fsm) \
   gfsm_automaton_add_state_full(fsm,gfsmNoState)

/** Get a (possibly new) state with id \a id.
 * \returns pointer to the state
 */
#define gfsm_automaton_get_state(fsm,id) \
    (((gfsmState*)((fsm)->states->data))+gfsm_automaton_ensure_state(fsm,(id)))
/*  (&g_array_index(fsm->states, gfsmState, gfsm_automaton_ensure_state(fsm,id)))*/

/** Try to get a pre-cast pointer to the state with id \a id,
 *  or NULL if so such state exists.
 *  \returns gfsmState* for \a id in \a fsm or NULL
 *  \warning
 *  The pointer returned may be invalidated if states are added
 *  or removed.
 */
#define gfsm_automaton_find_state(fsm,id) \
    ((id) < (fsm)->states->len ? (((gfsmState*)((fsm)->states->data))+(id)) : NULL)

/** Try to get a constant pointer to the state with id \a id,
 *  or NULL if so such state exists.
 *  \returns const gfsmState* for \a id in \a fsm
 *  \warning
 *  The pointer returned may be invalidated if states are added
 *  or removed.
 */
#define gfsm_automaton_find_state_const(fsm,id) \
    ((id) < fsm->states->len ? (((const gfsmState*)((fsm)->states->data))+(id)) : NULL)

/** Check whether automaton has a state with ID \a id. */
#define gfsm_automaton_has_state(fsm,id) \
    ((id) < (fsm)->states->len && (((gfsmState*)((fsm)->states->data))+(id))->is_valid)

/** Remove the state with id \a id, if any.
 *  Note that any incoming arcs for state \a id are NOT removed,
 *  although any outgoing arcs are removed and freed.
 */
void gfsm_automaton_remove_state(gfsmAutomaton *fsm, gfsmStateId id);


/** Check whether a state is final.  \returns gboolean */
#define gfsm_automaton_is_final_state(fsm,id) \
  gfsm_state_is_final(gfsm_automaton_find_state_const(fsm,id))

/** Set boolean final-state flag. \returns (void) */
//void gfsm_automaton_set_final_state(gfsmAutomaton *fsm, gfsmStateId id, gboolean is_final);
#define gfsm_automaton_set_final_state(fsm,id,is_final) \
  gfsm_automaton_set_final_state_full(fsm,id,is_final,fsm->sr->one)

/** Set final weight. \returns (void) */
void gfsm_automaton_set_final_state_full(gfsmAutomaton *fsm,
					 gfsmStateId    id,
					 gboolean       is_final,
					 gfsmWeightU    final_weight);

/** Get final weight. \returns final weight if state \a id is final, else \a fsm->sr->zero */
gfsmWeightU gfsm_automaton_get_final_weight(gfsmAutomaton *fsm, gfsmStateId id);

/** Lookup final weight. \returns TRUE iff state \a id is final, and sets \a *wp to its final weight. */
gboolean gfsm_automaton_lookup_final(gfsmAutomaton *fsm, gfsmStateId id, gfsmWeightU *wp);

/** Get number of outgoing arcs. \returns guint */
#define gfsm_automaton_out_degree(fsm,id) \
  ((id) < (fsm)->states->len ? gfsm_state_out_degree(gfsm_automaton_get_state((fsm),(id))) : 0)

/** Renumber states of an FSM */
void gfsm_automaton_renumber_states(gfsmAutomaton *fsm);
//@}

/*======================================================================
 * Methods: Accessors: Automaton Arcs
 */
/// \name Accessors: Automaton Arcs
//@{
/** Add an arc from state \a q1 to state \a q2 with label (\a lo,\a hi) and weight \a w
 *  Missing states are implicitly created.
 */
void gfsm_automaton_add_arc(gfsmAutomaton *fsm,
			    gfsmStateId q1,
			    gfsmStateId q2,
			    gfsmLabelId lo,
			    gfsmLabelId hi,
			    gfsmWeightU w);

/** Add an arc given pointers \a sp to the state and \a link to a
 *  single-element arclist to be added.  No states are
 *  implicitly created.
 */
void gfsm_automaton_add_arc_link(gfsmAutomaton *fsm,
				 gfsmState     *sp,
				 gfsmArcList   *link);

/** Sort all arcs in the automaton */
gfsmAutomaton *gfsm_automaton_arcsort(gfsmAutomaton *fsm, gfsmArcSortMode mode);

//@}

#endif /* _GFSM_AUTOMATON_H */
