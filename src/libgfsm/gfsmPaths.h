/*=============================================================================*\
 * File: gfsmPaths.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library
 *
 * Copyright (c) 2005 Bryan Jurish.
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

#ifndef _GFSM_PATHS_H
#define _GFSM_PATHS_H

#include <gfsmAutomaton.h>
#include <gfsmAlphabet.h>
#include <gfsmSet.h>
#include <gfsmSemiring.h>

#include <glib.h>

/*======================================================================
 * Types: paths
 */

/** Type for a full automaton path */
typedef struct _gfsmPath {
  gfsmLabelVector *lo;  /**< lower label sequence */
  gfsmLabelVector *hi;  /**< upper label sequence */
  gfsmWeight       w;   /**< weight attached to this path */
} gfsmPath;



/*======================================================================
 * Methods: Path Utilities
 */

///\name Path Utilities
//@{

/** Copy gfsmLabelVector. \returns @dst */
gfsmLabelVector *gfsm_label_vector_copy(gfsmLabelVector *dst, gfsmLabelVector *src);

/** Duplicate a gfsmLabelVector. \returns @dst */
#define gfsm_label_vector_dup(src) \
  gfsm_label_vector_copy(g_ptr_array_sized_new(src->len), src)

/** Create and return a new gfsmPath, specifying components
 *  If either of @lo or @hi are NULL, a new vector will be created.
 */
gfsmPath *gfsm_path_new_full(gfsmLabelVector *lo, gfsmLabelVector *hi, gfsmWeight w);

/** Create and return a new empty gfsmPath, specifying semiring. */
#define gfsm_path_new(sr) \
  gfsm_path_new_full(NULL,NULL,gfsm_sr_one(sr))

/** Create and return a new gfsmPath as a copy of an existing gfsmPath */
gfsmPath *gfsm_path_new_copy(gfsmPath *p1);

/** Create and return a new gfsmPath, appending to an existing path */
gfsmPath *gfsm_path_new_append(gfsmPath *p1, gfsmLabelVal lo, gfsmLabelVal hi, gfsmWeight w, gfsmSemiring *sr);
/** Append an arc to a gfsmPath */
void gfsm_path_push(gfsmPath *p, gfsmLabelVal lo, gfsmLabelVal hi, gfsmWeight w, gfsmSemiring *sr);

/** Pop an arc from a gfsmPath */
     void gfsm_path_pop(gfsmPath *p, gfsmLabelVal lo, gfsmLabelVal hi);

/** 3-way path comparison function. */
gint gfsm_path_compare_data(const gfsmPath *p1, const gfsmPath *p2, gfsmSemiring *sr);

/** Destroy a gfsmPath */
void gfsm_path_free(gfsmPath *p);
//@}

/*======================================================================
 * Methods: Automaton Serialization
 */

///\name Automaton Serialization
//@{

/** Serialize a gfsmAutomaton to a set of (gfsmPath*)s.
 *
 *  Causes deep recursion for cyclic automata.
 *  Returns a gfsmSet whose elements are (gfsmPath*)s.
 *  allocated with g_new().  It is the caller's responsibility to free the
 *  returned objects.
 *  \returns @set if non-NULL, otherwise a new gfsmSet*.
 */
gfsmSet *gfsm_automaton_paths(gfsmAutomaton *fsm, gfsmSet *paths);

/** Recursive guts for gfsm_automaton_paths() */
gfsmSet *_gfsm_automaton_paths_r(gfsmAutomaton *fsm, gfsmSet *paths, gfsmStateId q, gfsmPath *path);

/** Convert a gfsmPathSet to a list of (char*)s.
 *  @abet_lo and @abet_hi should be (gfsmStringAlphabet*)s.
 */
GSList *gfsm_paths_to_strings(gfsmSet *paths,
			      gfsmAlphabet *abet_lo,
			      gfsmAlphabet *abet_hi,
			      gfsmSemiring       *sr,
			      gboolean warn_on_undefined,
			      gboolean att_style,
			      GSList *strings);

/** Utility struct for gfsm_paths_to_strings() */
struct _gfsm_paths_to_strings_options {
  gfsmAlphabet *abet_lo;  ///< should be a gfsmStringAlphabet*
  gfsmAlphabet *abet_hi;  ///< should be a gfsmStringAlphabet*
  gfsmSemiring       *sr; ///< semiring for weight-based set sorting
  gboolean warn_on_undefined;  ///< warn on undefined symbols?
  gboolean att_style;          ///< use ATT-style output?
  GSList *strings;             ///< output list
};

/** Utility for gfsm_paths_to_strings() */
gboolean gfsm_paths_to_strings_foreach_func(gfsmPath *path,
					    gpointer value_dummy,
					    struct _gfsm_paths_to_strings_options *opts);

//@}


#endif /* _GFSM_PATHS_H */
