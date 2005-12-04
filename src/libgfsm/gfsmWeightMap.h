
/*=============================================================================*\
 * File: gfsmWeightMap.h
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

/** \file gfsmWeightMap.h
 *  \brief Abstract map from gpointers to gfsmWeights using GTree
 */

#ifndef _GFSM_WEIGHTMAP_H
#define _GFSM_WEIGHTMAP_H

#include <glib.h>
#include <gfsmMem.h>
#include <gfsmSet.h>
#include <gfsmCommon.h>

/*======================================================================
 * Types
 */
/** \brief Type for sets of weighted pointers
 *  \detail really just an ugly wrapper for GTree
 */
typedef GTree gfsmWeightMap;


/*======================================================================
 * Constructors etc.
 */
///\name gfsmWeightMap: Constructors etc.
//@{
/** gfsm_weightmap_new(key_compare_func,key_compare_data,key_destroy_func):
 * create and return a new set
 */
#define gfsm_weightmap_new_full(key_cmp_f,key_cmp_d,key_free_f) \
   g_tree_new_full((key_cmp_f),(key_cmp_d),(key_free_f),NULL)

/** gfsm_weightmap_new(key_compare_func): create and return a new weightmap
 *  (returned weightmap will not free elements)
 */
#define gfsm_weightmap_new(key_cmp_f) g_tree_new(key_cmp_f)

/** Copy weightmap \a src to \a dst. \returns \a dst */
#define gfsm_weightmap_copy(dst,src) gfsm_set_copy((dst),(src))

/** clear a weightmap */
#define gfsm_weightmap_clear(weightmap) gfsm_set_clear(weightmap)

/** destroy a weightmap */
#define gfsm_weightmap_free(weightmap) gfsm_set_free(weightmap)
//@}


/*======================================================================
 * Accessors
 */
///\name gfsmWeightmap: Accessors
//@{

/** Macro to convert gpointer->gfsmWeight */
#define gfsm_ptr2weight(p) (*((gfsmWeight*)(&(p))))

/** Macro to convert gfsmWeight->gpointer */
#define gfsm_weight2ptr(w) ((gpointer)(*((int*)(&(w)))))

/** lookup: check weightmap membership */
gboolean gfsm_weightmap_contains(gfsmWeightMap *weightmap, gconstpointer key);

/** extended lookup: get weight associated with key */
gboolean gfsm_weightmap_lookup(gfsmWeightMap *weightmap, gconstpointer key, gfsmWeight *wp);

/** insert a new key->weight mapping into the weightmap */
#define _gfsm_weightmap_insert(weightmap,key,w) \
   g_tree_insert((weightmap),((gpointer)(key)),gfsm_weight2ptr(w))

/** insert a new key->weight mapping into the weightmap */
void gfsm_weightmap_insert(gfsmWeightMap *weightmap, gconstpointer key, gfsmWeight w);

/** get size of weightmap */
#define gfsm_weightmap_size(weightmap) g_tree_nnodes(weightmap)

/** Remove an element from a weightmap */
#define gfsm_weightmap_remove(weightmap,key) g_tree_remove((weightmap),((gpointer)(key)))

/** Traversal (see g_tree_foreach) */
#define gfsm_weightmap_foreach(weightmap,func,data) g_tree_foreach((weightmap),(func),(data))
//@}

#endif /* _GFSM_WEIGHTMAP_H */
