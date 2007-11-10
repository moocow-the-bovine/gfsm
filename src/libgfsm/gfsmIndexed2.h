
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

#ifndef _GFSM_INDEXED2_H
#define _GFSM_INDEXED2_H

#include <gfsmAutomaton.h>

/*======================================================================
 * Types
 */

/// Final-weight index type for gfsmIndexedAutomaton2
/** GArray of ::gfsmWeight, indexed by state id: final weight or sr->zero if state is non-final */
//typedef GArray gfsmWeightArray;

/// Arc storage type for gfsmIndexedAutomaton2
/** GArray of ::gfsmArc, indexed by absolute arc offset */
//typedef GArray gfsmArcArray;

/// Type for an arc offset in a gfsmIndexedAutomaton2
//typedef guint gfsmArcId;

/// Type for for an array of arc offsets
/** GArray of gfsmArcId, indexed by whatever */
//typedef GArray gfsmArcIdArray;

/// Type for an indexed automaton
typedef struct {
  //-- gfsmAutomaton compatibility
  gfsmAutomatonFlags  flags;              /**< automaton flags, for ::gfsmAutomaton compatibility */
  gfsmSemiring       *sr;                 /**< semiring used for arc weight computations */
  gfsmStateId         root_id;            /**< id of root state, or gfsmNoState if not defined */
  //
  //-- Basic data
  gfsmBitVector      *state_is_valid;     /**< per-state validity flags */
  gfsmWeightArray    *state_final_weight; /**< State final weight, or sr->zero */
  gfsmArcArray       *arcs;               /**< Arc storage (sorted primarily by source state) */
  gfsmArcIdArray     *state_first_arc;    /**< ID of first outgoing arc, indexed by state */
  //
  //-- Forward indices
  GPtrArray          *arcix_lower;        /**< (gfsmArc*)s (state_first_arc applies) sorted by lower label */
  GPtrArray          *arcix_upper;        /**< (gfsmArc*)s (state_first_arc applies) sorted by upper label */
  //
  //-- .... more here?
} gfsmIndexedAutomaton2;

/// sort data passed to gfsm_indexed_automaton2_build_indices()
/*typedef struct {
  gfsmArc       *arcs; ///< pointer to populate arc array
  gfsmSemiring  *sr;   ///< pointer to fsm semiring
} gfsmIndexedAutomaton2SortData;
*/

/*======================================================================
 * Constants
 */

/// Constant indicating an invalid arc (or state)
extern const gfsmArcId gfsmNoArc;

/*======================================================================
 * Methods: gfsmIndexedAutomaton2: constructors, etc.
 */
/// \name Constructors etc.
//@{

/** Create a new ::gfsmIndexedAutomaton2, specifying some basic automaton structure */
gfsmIndexedAutomaton2 *gfsm_indexed_automaton2_new_full(gfsmAutomatonFlags flags,
						      gfsmSRType         srtype,
						      gfsmStateId        n_states,
						      gfsmArcId          n_arcs);

/** Create a new indexed automaton, using some default values */
#define gfsm_indexed_automaton2_new() \
    gfsm_indexed_automaton2_new_full(gfsmAutomatonDefaultFlags,\
                                    gfsmAutomatonDefaultSRType,\
                                    gfsmAutomatonDefaultSize,\
                                    gfsmAutomatonDefaultSize)

/** Clear a ::gfsmIndexedAutomaton2 */
void gfsm_indexed_automaton2_clear(gfsmIndexedAutomaton2 *xfsm);

/** Free a ::gfsmIndexedAutomaton2 */
void gfsm_indexed_automaton2_free(gfsmIndexedAutomaton2 *xfsm);

//@}

/*======================================================================
 * Methods: Import & Export
 */
/// \name Import & Export
//@{

/** Populate a ::gfsmIndexedAutomaton2 from a ::gfsmAutomaton
 *  \param fsm source automaton
 *  \param xfsm destination indexed automaton,
 *         may be passed as NULL to create a new ::gfsmIndexedAutomaton2
 *  \returns (new) indexed automaton \a xfsm
 *  \note implicitly clears \a xfsm
 */
gfsmIndexedAutomaton2 *gfsm_automaton_to_indexed2(gfsmAutomaton *fsm, gfsmIndexedAutomaton2 *xfsm);

/** (low-level) populate xfsm->arcix_lower and xfsm->arcix_upper  */
void gfsm_indexed_automaton2_build_indices(gfsmIndexedAutomaton2 *xfsm);

/** Comparison function for gfsm_indexed_automaton2_build_indices() */
gint gfsm_indexed_automaton2_build_cmp_lo(const gfsmArc **app1,
					 const gfsmArc **app2,
					 gfsmSemiring   *sr);

/** Comparison function for gfsm_indexed_automaton2_build_indices() */
gint gfsm_indexed_automaton2_build_cmp_hi(const gfsmArc **app1,
					 const gfsmArc **app2,
					 gfsmSemiring   *sr);


/** Export a ::gfsmIndexedAutomaton2 to a ::gfsmAutomaton
 *  \param xfsm source indexed automaton
 *  \param fsm destination :.gfsmAutomaton
 *         may be passed as NULL to create a new ::gfsmAutomaton
 *  \returns (new) automaton \a fsm
 *  \note implicitly clears \a fsm
 */
gfsmAutomaton *gfsm_indexed_to_automaton2(gfsmIndexedAutomaton2 *xfsm, gfsmAutomaton *fsm);

//@}

/*======================================================================
 * Methods: Accessors: gfsmIndexedAutomaton2
 */
/// \name Accessors: gfsmIndexedAutomaton2
//@{

/** Reserve space for at least \a n_states states */
gfsmStateId gfsm_indexed_automaton2_reserve_states(gfsmIndexedAutomaton2 *fsm, gfsmStateId n_states);

/** Reserve space for at least \a n_arcs arcs */
gfsmArcId gfsm_indexed_automaton2_reserve_arcs(gfsmIndexedAutomaton2 *fsm, gfsmArcId n_arcs);

//@}

/*======================================================================
 * Methods: Accessors: gfsmAutomaton API: Automaton
 */
/// \name Accessors: gfsmAutomaton API: Automaton
//@{

/** Get pointer to the semiring associated with this automaton */
#define gfsm_indexed_automaton2_get_semiring(xfsm) (xfsm->sr)

/** Set the semiring associated with this automaton */
gfsmSemiring *gfsm_indexed_automaton2_set_semiring(gfsmIndexedAutomaton2 *fsm, gfsmSemiring *sr);

/** Set the semiring associated with this automaton by semiring-type */
void gfsm_indexed_automaton2_set_semiring_type(gfsmIndexedAutomaton2 *fsm, gfsmSRType srtype);

/** True iff automaton is a transducer */
#define gfsm_indexed_automaton2_is_transducer(fsm) gfsm_automaton_is_transducer(fsm)

/** True iff automaton is weighted */
#define gfsm_indexed_automaton2_is_weighted(fsm) gfsm_automaton_is_weighted(fsm)

/** Get number of states (constant time) */
#define gfsm_indexed_automaton2_n_states(fsm) ((fsm)->state_final_weight->len)

/** Get total number of arcs (constant time) */
#define gfsm_indexed_automaton2_n_arcs(fsm) ((fsm)->arcs->len)

/** Get Id of root node, or gfsmNoState if undefined */
#define gfsm_indexed_automaton2_get_root(fsm) gfsm_automaton_get_root(fsm)

/** Set Id of root node, creating state if necessary */
#define gfsm_indexed_automaton2_set_root(fsm,id) gfsm_automaton_set_root((fsm),(id))

/** Alias for gfsm_indexed_automaton2_reserve_states */
#define gfsm_indexed_automaton2_reserve(fsm,n_states) gfsm_indexed_automaton2_reserve_states((fsm),(n_states))

//@}

/*======================================================================
 * Methods: Accessors: gfsmAutomaton API: States
 */
/// \name Accessors: gfsmAutomaton API: States
//@{

/** Check whether state might possibly exist in automaton (low-level) */
#define gfsm_indexed_automaton2_check_state(fsm,qid) \
    ((qid) < gfsm_indexed_automaton2_n_states(fsm))

/** Check whether automaton has a state with ID \a qid. */
#define gfsm_indexed_automaton2_has_state(fsm,qid) \
    ((qid) < gfsm_indexed_automaton2_n_states(fsm) \
     && gfsm_bitvector_get((fsm)->state_is_valid, (qid)))

//    gfsm_indexed_automaton2_check_state((fsm),(id))
//    && g_array_index((fsm)->state_first_arc,gfsmArcId,(id)) != gfsmNoArc)


/** Ensures that state \a id exists \returns \a qid */
gfsmStateId gfsm_indexed_automaton2_ensure_state(gfsmIndexedAutomaton2 *xfsm, gfsmStateId qid);


/* Remove the state with id \a qid, if any.
 * Currently, this just clears the bit for \a qid in \a fsm->state_is_valid.
 */
void gfsm_indexed_automaton2_remove_state(gfsmIndexedAutomaton2 *fsm, gfsmStateId qid);


/** Check whether a state is final.  \returns gboolean */
#define gfsm_indexed_automaton2_is_final_state(fsm,qid) \
  (gfsm_indexed_automaton2_has_state((fsm),(qid)) \
   && g_array_index((fsm)->state_final_weight,gfsmWeight,(qid)) != (fsm)->sr->zero)

/** Set boolean final-state flag. \returns (void) */
#define gfsm_indexed_automaton2_set_final_state(fsm,qid,is_final) \
  gfsm_indexed_automaton2_set_final_state_full(fsm,qid,is_final,fsm->sr->one)

/** Set final weight. \returns (void) */
void gfsm_indexed_automaton2_set_final_state_full(gfsmIndexedAutomaton2 *fsm,
						 gfsmStateId    qid,
						 gboolean       is_final,
						 gfsmWeight     final_weight);

/** Get final weight. \returns final weight if state \a qid is final, else \a fsm->sr->zero */
#define gfsm_indexed_automaton2_get_final_weight(fsm,qid) \
  (gfsm_indexed_automaton2_has_state((fsm),(qid)) \
   ? g_array_index((fsm)->state_final_weight,gfsmWeight,(qid)) \
   : (fsm)->sr->zero)

/** Lookup final weight. \returns TRUE iff state \a id is final, and sets \a *wp to its final weight. */
gboolean gfsm_indexed_automaton2_lookup_final(gfsmIndexedAutomaton2 *fsm, gfsmStateId id, gfsmWeight *wp);

/** Get number of outgoing arcs. \returns guint */
guint gfsm_indexed_automaton2_out_degree(gfsmIndexedAutomaton2 *fsm, gfsmStateId qid);

//@}


#endif /* _GFSM_INDEXED_H */
