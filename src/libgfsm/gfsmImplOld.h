
/*=============================================================================*\
 * File: gfsmImplOld.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: automaton implementation: old: declarations
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

/** \file gfsmImplOldTypes.h
 *  \brief Declarations for backwards-compatible automaton implementation
 */

#ifndef _GFSM_AUTOMATON_H
#define _GFSM_AUTOMATON_H

#include <gfsmAutomatonTypes.h>
#include <gfsmAlphabet.h>
#include <gfsmState.h>
#include <gfsmIO.h>

/*======================================================================
 * Methods: constructors, etc.
 */
/// \name Constructors etc.

//@{

/** Implements gfsm_automaton_new(), gfsm_automaton_new_full() 
 *  \par Time:
 *   <b>O(g_array_sized_new(n_states))</b>
 */
static inline
void gfsm_automaton_init_old(gfsmAutomaton *fsm, gfsmStateId n_states, gfsmArcId n_arcs);

/** Implements gfsm_automaton_clone()
 *  \par Time: <b>O(n_states_src+n_arcs_src)</b>
 */
static inline
void gfsm_automaton_clone_impl_old(gfsmAutomaton *dst, gfsmAutomaton *src);

/** Implements gfsm_automaton_copy() */
//gfsmAutomaton *gfsm_automaton_copy_old(gfsmAutomaton *dst, gfsmAutomaton *src);
//--DEFAULT

/** Implements gfsm_automaton_clear()
 *  \par Time: O(n_arcs)
 */
static inline
void gfsm_automaton_clear_old(gfsmAutomaton *fsm);

/** Implements gfsm_automaton_free()
 *  \par Time: O(clear) + O(g_array_free(states))
 */
static inline
void gfsm_automaton_free_old(gfsmAutomaton *fsm);

//@}


/*======================================================================
 * API: Automaton Structure
 */
/// \name API: Automaton Structure
//@{

/** Implements gfsm_automaton_reserve_states()
 *
 * This implementation stores states in a \c GArray, so calling this function frequently
 * will gobble up lots of runtime.
 *
 * \par Time: O(n_states)
 */
static inline
void gfsm_automaton_reserve_states_old(gfsmAutomaton *fsm, gfsmStateId n_states);

/** Implements gfsm_automaton_reserve_arcs()
 *
 * Does nothing in this implementation.
 * \par Time: O(1)
 */
static inline
void gfsm_automaton_reserve_arcs_old(gfsmAutomaton *fsm, gfsmArcId n_arcs);

/** Implements gfsm_automaton_n_states()
 *  \par Time: O(1)
 */
static inline
gfsmStateId gfsm_automaton_n_states_old(gfsmAutomaton *fsm);

/** Implements gfsm_automaton_n_arcs() */
//static inline gfsmArcId gfsm_automaton_n_arcs_old(gfsmAutomaton *fsm);
//-- DEFAULT

/** Implements gfsm_automaton_n_final_states()
 *  \par Time: O(1)
 *    Actually O(g_tree_nnodes(fsm->impl.old->finals));
 *    consult GLib docs & sources to be certain!
 */
static inline
gfsmStateId gfsm_automaton_n_final_states_old(gfsmAutomaton *fsm);

/** Implements gfsm_automaton_get_root()
 *  \par Time: O(1)
 */
static inline
gfsmStateId gfsm_automaton_get_root_old(gfsmAutomaton *fsm);

/** Implements gfsm_automaton_set_root()
 * \par Time: O(1)
 */
static inline
void gfsm_automaton_set_root_old(gfsmAutomaton *fsm, gfsmStateId qid);

/** Implements gfsm_automaton_renumber_states_full()
 *  \par Time: O(n_states+n_arcs)
 */
void gfsm_automaton_renumber_states_full_old(gfsmAutomaton *fsm, GArray *old2new, gfsmStateId n_new_states);
//-- PURE FUNCTION?!

//@}

/*======================================================================
 * API: Automaton States
 */
/// \name API: Automaton States
//@{

//----------------------------------------------
// States: ok/add/remove

/** Implements gfsm_automaton_has_state()
 *  \par Time: O(1)
 */
static inline
gboolean gfsm_automaton_has_state_old(gfsmAutomaton *fsm, gfsmStateId qid);

/** Implements gfsm_automaton_add_state_full()
 * \par Time: O(1)
 */
static inline
gfsmStateId gfsm_automaton_add_state_old(gfsmAutomaton *fsm, gfsmStateId qid);

/** Implements gfsm_automaton_remove_state()
 * \par Time: O(out_degree(qid))
 */
static inline
void gfsm_automaton_remove_state_old(gfsmAutomaton *fsm, gfsmStateId qid);

//----------------------------------------------
// States: open/close
/** Implements *gfsm_automaton_open_state()
 * \par Time: O(1)
 */
gfsmState *gfsm_automaton_open_state_old(gfsmAutomaton *fsm, gfsmStateId qid);

/** Implements gfsm_automaton_close_state().
 * \par Time: O(1)
 */
void gfsm_automaton_close_state_old(gfsmAutomaton *fsm, gfsmState *sp);

//----------------------------------------------
// States: finality

/** Implements gfsm_automaton_lookup_final()
 * \par Time:
 *   \b O(1) if qid is not final, <b>O(log(n_finals))</b> otherwise.
 */
static inline
gboolean gfsm_automaton_lookup_final_old(gfsmAutomaton *fsm, gfsmStateId qid, gfsmWeight *wp);

/** Implements gfsm_automaton_set_final_state_full()
 * \par Time:
 *   Usually <b>O(log(n_finals))</b>; may trigger an implicit state-array reallocation.
 */
static inline
void gfsm_automaton_set_final_state_full_old(gfsmAutomaton *fsm,
					     gfsmStateId    qid,
					     gboolean       is_final,
					     gfsmWeight     final_weight);

//----------------------------------------------
// States: properties

/** Implements gfsm_automaton_out_degree()
 * \par Time: O(out_degree(qid))
 */
static inline
gfsmArcId gfsm_automaton_out_degree_old(gfsmAutomaton *fsm, gfsmStateId qid);

//@}


/*======================================================================
 * API: Automaton Arcs
 */
/// \name API: Automaton Arcs
//@{

/** Implements gfsm_automaton_add_arc()
 * \par Time:
 *   \b O(1) if states already exist; may trigger implicit state-array reallocation.
 */
static inline
void gfsm_automaton_add_arc_old(gfsmAutomaton *fsm,
				    gfsmStateId    qid1,
				    gfsmStateId    qid2,
				    gfsmLabelVal   lo,
				    gfsmLabelVal   hi,
				    gfsmWeight     w);

/** Implements gfsm_automaton_arcsort_full()
 * \par Time: O(n_states * O(g_slist_sort(avg_out_degree(fsm))))
 */
static inline
void gfsm_automaton_arcsort_full_old(gfsmAutomaton *fsm, GCompareDataFunc cmpfunc, gpointer data);

//@}

/*======================================================================
 * API: Arc Iterators
 */
///\name API: Arc Iterators
//@{

/** Implements gfsm_arciter_open()
 *  \par Time: O(1)
 */
static inline
void gfsm_arciter_init_old(gfsmArcIter *aip);

/* Implements gfsm_arciter_close() 
 * \par Time: O(1)
 */
static inline
void gfsm_arciter_close_old(gfsmArcIter *aip);

/** Implements gfsm_arciter_ok()
 *  \par Time: O(1)
 */
static inline
gboolean gfsm_arciter_ok_old(gfsmArcIter *aip);

/** Implements gfsm_arciter_next()
 *  \par Time: O(1)
 */
static inline
void gfsm_arciter_next(gfsmArcIter *aip);

/** Implements gfsm_arciter_reset()
 *  \par Time: O(1)
 */
static inline
void gfsm_arciter_reset_old(gfsmArcIter *aip);

/* Implements gfsm_arciter_copy() */
//void gfsm_arciter_copy_old(gfsmArcIter *dst, gfsmArcIter *src);
//-- DEFAULT


/* Implements gfsm_arciter_clone() */
//gfsmArcIter *gfsm_arciter_clone_old(const gfsmArcIter *src);
//-- DEFAULT

/** Implements gfsm_arciter_arc()
 *  \par Time: O(1)
 */
static inline
gfsmArc *gfsm_arciter_arc_old(gfsmArcIter *aip);

/** Implements gfsm_arciter_remove()
 *  \par Time: O(1)
 */
static inline
void gfsm_arciter_remove_old(gfsmArcIter *aip);

/* Implements gfsm_arciter_seek_both() */
//void gfsm_arciter_seek_both_default(gfsmArcIter *aip, gfsmLabelVal lo, gfsmLabelVal hi);
//--DEFAULT

/* Implements gfsm_arciter_seek_lower() */
//void gfsm_arciter_seek_lower_default(gfsmArcIter *aip, gfsmLabelVal lo);
//--DEFAULT

/* Implements gfsm_arciter_seek_upper() */
//void gfsm_arciter_seek_upper_default(gfsmArcIter *aip, gfsmLabelVal hi);
//--DEFAULT

//@}

/*======================================================================
 * API: Automaton I/O
 */
///\name API: Automaton I/O
//@{

/** Minimum libgfsm version required for loading files stored by this version of libgfsm */
extern const gfsmVersionInfo gfsm_version_bincompat_min_store_old;

/** Minimum libgfsm version whose binary files this version of libgfsm can read */
extern const gfsmVersionInfo gfsm_version_bincompat_min_check_old;

/** Implements gfsm_automaton_get_bin_header() */
void gfsm_automaton_get_bin_header_old(gfsmAutomaton *fsm, gfsmAutomatonHeader *hdr);

/** Implements gfsm_automaton_save_bin_handle() */
gboolean gfsm_automaton_save_bin_handle_old(gfsmAutomaton *fsm, gfsmIOHandle *ioh, gfsmError **errp);

/** Implementa gfsm_automaton_load_bin_handle() */
gboolean gfsm_automaton_load_bin_handle_old(gfsmAutomaton        *fsm,
					    gfsmAutomatonHeader  *hdr,
					    gfsmIOHandle         *ioh,
					    gfsmError           **errp);

//@}

#endif /* _GFSM_IMPL_OLD_H */
