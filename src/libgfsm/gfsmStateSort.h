
/*=============================================================================*\
 * File: gfsmStateSort.h
 * Author: Bryan Jurish <moocow.bovine@gmail.com>
 * Description: finite state machine library
 *
 * Copyright (c) 2004-2012 Bryan Jurish.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
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

/** \file gfsmStateSort.h
 *  \brief sort states of a gfsmAutomaton
 */

#ifndef _GFSM_STATESORT_H
#define _GFSM_STATESORT_H

#include <gfsmAutomaton.h>

/** Type for maps from (old) gfsmStateId to new gfsmStateId , used by state-sorting functions.
 *   GArray of ::gfsmStateId such that \code qid_new==old2new[qid_old] \enccode .
 *   \a qid_new may be ::gfsmNoState to ignore the corresponding \a qid_old
 */
typedef GArray gfsmStateIdMap;

/*======================================================================*/
/// \name Methods: High-level state sorting
//@{

/** Sort \a fsm according to an 'affine' enumeration.
 *  Basically just a wrapper for
 *  \code gfsm_statemap_apply(fsm,gfsm_statemap_aff(fsm,old2new),0); \endcode
 *  Caller is responsible for freeing \a old2new if it is specified and non-NULL.
 *  \param fsm automaton to be sorted
 *  \param old2new target ::gfsmStateIdMap , or NULL to use a temporary map
 *  \sa gfsm_renumber_states(), gfsm_statemap_aff(), gfsm_statemap_apply()
 */
void gfsm_statesort_aff(gfsmAutomaton *fsm, gfsmStateIdMap *old2new);


/** Sort \a fsm according to a depth-first enumeration.
 *  Basically just a wrapper for
 *  \code gfsm_statemap_apply(fsm,gfsm_statemap_dfs(fsm,old2new),0); \endcode
 *  Caller is responsible for freeing \a old2new if it is specified and non-NULL.
 *  \param fsm automaton to be sorted
 *  \param old2new target ::gfsmStateIdMap , or NULL to use a temporary map
 *  \sa gfsm_statemap_dfs(), gfsm_statemap_apply()
 */
void gfsm_statesort_dfs(gfsmAutomaton *fsm, gfsmStateIdMap *old2new);

/** Sort \a fsm according to a breadth-first enumeration.
 *  Basically just a wrapper for
 *  \code gfsm_statemap_apply(fsm,gfsm_statemap_bfs(fsm,old2new),0); \endcode
 *  Caller is responsible for freeing \a old2new if it is specified and non-NULL.
 *  \param fsm automaton to be sorted
 *  \param old2new target ::gfsmStateIdMap , or NULL to use a temporary map
 *  \sa gfsm_statemap_bfs(), gfsm_statemap_apply()
 */
void gfsm_statesort_bfs(gfsmAutomaton *fsm, gfsmStateIdMap *old2new);

//@}

/*======================================================================*/
/// \name Methods: gfsmStateIdMap application 
//@{

/** Renumber states of \a fsm according to the ::gfsmStateIdMap \a old2new .
 *  \param fsm automaton whose states are to be mapped
 *  \param old2new state-mapping to apply
 *  \param n_new_states number of target states to allocate; may be 0 (zero) to auto-compute
 */
void gfsm_statemap_apply(gfsmAutomaton *fsm, gfsmStateIdMap *old2new, gfsmStateId n_new_states);

//@}

/*======================================================================*/
/// \name Methods: gfsmStateIdMap acquisition
//@{

/** Populate and return an 'affine' ::gfsmStateIdMap for \a fsm. The
 *  returns ::gfsmStateIdMap is such that the target state enumeration has
 *  no 'gaps', and the root state is mapped to the 0 (zero).
 *  \param fsm automaton to be mapped
 *  \param old2new destination ::gfsmStateIdMap , or NULL to create and return a new map
 *  \returns \a old2new , or a new ::gfsmStateIdMap
 *  \sa gfsm_renumber_states()
 */
gfsmStateIdMap* gfsm_statemap_aff(gfsmAutomaton *fsm, gfsmStateIdMap *old2new);


/** Populate and return a depth-first ::gfsmStateIdMap for \a fsm
 *  \param fsm automaton to be mapped
 *  \param old2new destination ::gfsmStateIdMap , or NULL to create and return a new map
 *  \returns \a old2new , or a new ::gfsmStateIdMap
 */
gfsmStateIdMap* gfsm_statemap_dfs(gfsmAutomaton *fsm, gfsmStateIdMap *old2new);

/** Populate and return a breadth-first ::gfsmStateIdMap for \a fsm
 *  \param fsm automaton to be mapped
 *  \param old2new destination ::gfsmStateIdMap , or NULL to create and return a new map
 *  \returns \a old2new , or a new ::gfsmStateIdMap
 */
gfsmStateIdMap* gfsm_statemap_bfs(gfsmAutomaton *fsm, gfsmStateIdMap *old2new);

/** Get depth information for each state; depths[q]==minim-path-length(q0,q) or gfsmNoState if non-accessible
 *  \param fsm automaton to be investigated
 *  \param depths destination ::gfsmStateIdMap , or NULL to create and return a new map
 *  \returns \a depths or a new gfsmStateIdMap
 */
gfsmStateIdMap* gfsm_statemap_depths(gfsmAutomaton *fsm, gfsmStateIdMap *depths);

//@}

/*======================================================================*/
/// \name Low-level fsmStateIdMap utilities
//@{

/** Initialize a ::gfsmStateIdMap \a smap of size \sz .
 *  \li a new ::gfsmStateIdMap will be allocated if \a smap is passed as NULL
 *  \li all target values in \a sm are initialized to \a gfsmNoState.
 *  \param smap map to initialize , or NULL
 *  \param sz new size of map
 *  \returns \a smap , or a new ::gfsmStateIdMap
 */
gfsmStateIdMap *gfsm_statemap_init(gfsmStateIdMap *smap, gfsmStateId sz);

//@}


#endif /* _GFSM_STATESORT_H */
