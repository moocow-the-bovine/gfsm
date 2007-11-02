
/*=============================================================================*\
 * File: gfsmIndexed.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: arc indices
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

/** \file gfsmIndexed.h
 *  \brief First stab at indexed automata
 */

#ifndef _GFSM_INDEXED_H
#define _GFSM_INDEXED_H

#include <gfsmAutomaton.h>

/*======================================================================
 * Types
 */

/// Final-weight index type for gfsmIndexedAutomaton
/** GArray of ::gfsmWeight, indexed by state id: final weight or sr->zero if state is non-final */
typedef GArray gfsmWeightArray;

/// Arc storage type for gfsmIndexedAutomaton
/** GArray of ::gfsmArc, indexed by absolute arc offset */
typedef GArray gfsmArcArray;

/// Type for an arc offset in a gfsmIndexedAutomaton
typedef guint gfsmArcId;

/// Type for for an array of arc offsets
/** GArray of gfsmArcId, indexed by whatever */
typedef GArray gfsmArcIdArray;

/// Type for an indexed automaton.
typedef struct {
  //-- gfsmAutomaton compatibility
  gfsmAutomatonFlags  flags;              /**< automaton flags, for ::gfsmAutomaton compatibility */
  gfsmSemiring       *sr;                 /**< semiring used for arc weight computations */
  gfsmStateId         root_id;            /**< id of root state, or gfsmNoState if not defined */
  //
  //-- Basic data
  gfsmWeightArray    *state_final_weight; /**< State final weight, or sr->zero */
  gfsmArcArray       *arcs;               /**< Arc storage (sorted primarily by source state) */
  gfsmArcIdArray     *state_first_arc;    /**< ID of first outgoing arc, indexed by state */
  //
  //-- Forward indices
  gfsmArcIdArray     *arcix_lower;        /**< arc-ids (state_first_arc applies) sorted by lower label */
  gfsmArcIdArray     *arcix_upper;        /**< arc-ids (state_first_arc applies) sorted by upper label */
  //
#if 0
  //-- Reverse indices
  gfsmArcIdArray     *rarcs;              /**< arc-ids sorted primarily by target state */
  gfsmArcIdArray     *state_first_rarc;   /**< ID of first incoming arc in rarcs, indexed by target state */
  gfsmArcIdArray     *rarcix_lower;       /**< arc-ids (state_first_rarc applies) sorted by lower label */
  gfsmArcIdArray     *rarcix_upper;       /**< arc-ids (state_first_rarc applies) sorted by upper label */
#endif
  //
  //-- .... more here?
} gfsmIndexedAutomaton;

/// sort data passed to gfsm_indexed_automaton_build_indices()
typedef struct {
  gfsmArc       *arcs; ///< pointer to populate arc array
  gfsmSemiring  *sr;   ///< pointer to fsm semiring
} gfsmIndexedAutomatonSortData;

/*======================================================================
 * Constants
 */

/// Constant indicating an invalid arc (or state)
extern const gfsmArcId gfsmNoArc;

/*======================================================================
 * Methods: gfsmIndexedAutomaton: constructors, etc.
 */
/// \name Constructors etc.
//@{

/** Create a new ::gfsmIndexedAutomaton, specifying some basic automaton structure */
gfsmIndexedAutomaton *gfsm_indexed_automaton_new_full(gfsmAutomatonFlags flags,
						      gfsmSRType         srtype,
						      gfsmStateId        n_states,
						      gfsmArcId          n_arcs);

/** Create a new indexed automaton, using some default values */
#define gfsm_indexed_automaton_new() \
    gfsm_indexed_automaton_new_full(gfsmAutomatonDefaultFlags,\
                                    gfsmAutomatonDefaultSRType,\
                                    gfsmAutomatonDefaultSize,\
                                    gfsmAutomatonDefaultSize,\
                                    TRUE);

/** Clear a ::gfsmIndexedAutomaton */
void gfsm_indexed_automaton_clear(gfsmIndexedAutomaton *xfsm);

/** Free a ::gfsmIndexedAutomaton */
void gfsm_indexed_automaton_free(gfsmIndexedAutomaton *xfsm);

//@}

/*======================================================================
 * Methods: Import & Export
 */
/// \name Import & Export
//@{

/** Populate a ::gfsmIndexedAutomaton from a ::gfsmAutomaton
 *  \param fsm source automaton
 *  \param xfsm destination indexed automaton,
 *         may be passed as NULL to create a new ::gfsmIndexedAutomaton
 *  \returns (new) indexed automaton \a xfsm
 *  \note implicitly clears \a xfsm
 */
gfsmIndexedAutomaton *gfsm_automaton_to_indexed(gfsmAutomaton *fsm, gfsmIndexedAutomaton *xfsm);

/** (low-level) populate xfsm->arcix_lower and xfsm->arcix_upper  */
void gfsm_indexed_automaton_build_indices(gfsmIndexedAutomaton *xfsm);

/** Comparison function for gfsm_indexed_automaton_build_indices() */
gint gfsm_indexed_automaton_build_cmp_lo(const gfsmArcId *aidp1,
					 const gfsmArcId *aidp2,
					 gfsmIndexedAutomatonSortData *sdata);

/** Comparison function for gfsm_indexed_automaton_build_indices() */
gint gfsm_indexed_automaton_build_cmp_hi(const gfsmArcId *aidp1,
					 const gfsmArcId *aidp2,
					 gfsmIndexedAutomatonSortData *sdata);


/** Export a ::gfsmIndexedAutomaton to a ::gfsmAutomaton
 *  \param xfsm source indexed automaton
 *  \param fsm destination :.gfsmAutomaton
 *         may be passed as NULL to create a new ::gfsmAutomaton
 *  \returns (new) automaton \a fsm
 *  \note implicitly clears \a fsm
 */
gfsmAutomaton *gfsm_indexed_to_automaton(gfsmAutomaton *fsm, gfsmIndexedAutomaton *xfsm);

//@}

/*======================================================================
 * Methods: Accessors: gfsmIndexedAutomaton
 */
/// \name Accessors: gfsmIndexedAutomaton
//@{

/** Reserve space for at least \a n_states states */
gfsmStateId gfsm_indexed_automaton_reserve_states(gfsmIndexedAutomaton *fsm, gfsmStateId n_states);

/** Reserve space for at least \a n_arcs arcs */
gfsmArcId gfsm_indexed_automaton_reserve_arcs(gfsmIndexedAutomaton *fsm, gfsmArcId n_arcs);

//@}

/*======================================================================
 * Methods: Accessors: gfsmAutomaton API: Automaton
 */
/// \name Accessors: gfsmAutomaton API: Automaton
//@{

/** Get pointer to the semiring associated with this automaton */
#define gfsm_indexed_automaton_get_semiring(xfsm) (xfsm->sr)

/** Set the semiring associated with this automaton */
gfsmSemiring *gfsm_indexed_automaton_set_semiring(gfsmIndexedAutomaton *fsm, gfsmSemiring *sr);

/** Set the semiring associated with this automaton by semiring-type */
void gfsm_indexed_automaton_set_semiring_type(gfsmIndexedAutomaton *fsm, gfsmSRType srtype);

/** True iff automaton is a transducer */
#define gfsm_indexed_automaton_is_transducer(fsm) gfsm_automaton_is_transducer(fsm)

/** True iff automaton is weighted */
#define gfsm_indexed_automaton_is_weighted(fsm) gfsm_automaton_is_weighted(fsm)

/** Get number of states (constant time) */
#define gfsm_indexed_automaton_n_states(fsm) ((fsm)->state_final_weight->len)

/** Get total number of arcs (constant time) */
#define gfsm_indexed_automaton_n_arcs(fsm) ((fsm)->arcs->len)

/** Get Id of root node, or gfsmNoState if undefined */
#define gfsm_indexed_automaton_get_root(fsm) gfsm_automaton_get_root(fsm)

/** Set Id of root node, creating state if necessary */
#define gfsm_indexed_automaton_set_root(fsm,id) gfsm_automaton_set_root((fsm),(id))

/** Alias for gfsm_indexed_automaton_reserve_states */
#define gfsm_indexed_automaton_reserve(fsm,n_states) gfsm_indexed_automaton_reserve_states((fsm),(n_states))

//@}

/*======================================================================
 * Methods: Accessors: gfsmAutomaton API: States
 */
/// \name Accessors: gfsmAutomaton API: States
//@{

/** Check whether state might possibly exist in automaton (low-level) */
#define gfsm_indexed_automaton_check_state(fsm,qid) \
    ((qid) < gfsm_indexed_automaton_n_states(fsm))

/** Check whether automaton has a state with ID \a qid. */
#define gfsm_indexed_automaton_has_state(fsm,qid) \
    ((qid) < gfsm_indexed_automaton_n_states(fsm))
//    gfsm_indexed_automaton_check_state((fsm),(id))
//    && g_array_index((fsm)->state_first_arc,gfsmArcId,(id)) != gfsmNoArc)


/** Ensures that state \a id exists \returns \a qid */
gfsmStateId gfsm_indexed_automaton_ensure_state(gfsmIndexedAutomaton *xfsm, gfsmStateId qid);


/* Remove the state with id \a qid, if any.
 *  Note that any incoming arcs for state \a qid are NOT removed,
 *  although any outgoing arcs are removed and freed.
 */
//void gfsm_indexed_automaton_remove_state(gfsmIndexedAutomaton *fsm, gfsmStateId qid);


/** Check whether a state is final.  \returns gboolean */
#define gfsm_indexed_automaton_is_final_state(fsm,qid) \
  (gfsm_indexed_automaton_has_state((fsm),(qid)) \
   && g_array_index((fsm)->state_final_weight,gfsmWeight,(qid)) != (fsm)->sr->zero)

/** Set boolean final-state flag. \returns (void) */
#define gfsm_indexed_automaton_set_final_state(fsm,qid,is_final) \
  gfsm_indexed_automaton_set_final_state_full(fsm,qid,is_final,fsm->sr->one)

/** Set final weight. \returns (void) */
void gfsm_indexed_automaton_set_final_state_full(gfsmIndexedAutomaton *fsm,
						 gfsmStateId    qid,
						 gboolean       is_final,
						 gfsmWeight     final_weight);

/** Get final weight. \returns final weight if state \a qid is final, else \a fsm->sr->zero */
#define gfsm_indexed_automaton_get_final_weight(fsm,qid) \
  (gfsm_indexed_automaton_check_state((fsm),(qid)) \
   ? g_array_index((fsm)->state_final_weight,gfsmWeight,(qid)) \
   : (fsm)->sr->zero)

/** Lookup final weight. \returns TRUE iff state \a id is final, and sets \a *wp to its final weight. */
gboolean gfsm_indexed_automaton_lookup_final(gfsmIndexedAutomaton *fsm, gfsmStateId id, gfsmWeight *wp);

/** Get number of outgoing arcs. \returns guint */
guint gfsm_indexed_automaton_out_degree(gfsmIndexedAutomaton *fsm, gfsmStateId qid);

//@}


#endif /* _GFSM_INDEXED_H */
