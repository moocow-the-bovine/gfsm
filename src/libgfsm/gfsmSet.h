
/*=============================================================================*\
 * File: gfsmSet.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library
 *
 * Copyright (c) 2004 Bryan Jurish.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See file LICENSE for further informations on licensing terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *=============================================================================*/

#ifndef _GFSM_SET_H
#define _GFSM_SET_H

#include <glib.h>
#include <gfsmMem.h>
#include <stdio.h>

/*======================================================================
 * Types
 */
/// type for sets of pointers: really just an ugly wrapper for GTree
typedef GTree gfsmSet;



/*======================================================================
 * Constructors etc.
 */
///\name gfsmSet: Constructors etc.
//@{
/** gfsm_set_new(key_compare_func,key_compare_data,key_destroy_func):
 * create and return a new set
 */
#define gfsm_set_new_full(key_cmp_f,key_cmp_d,key_free_f) \
   g_tree_new_full(key_cmp_f,key_cmp_d,key_free_f,NULL)

/** gfsm_set_new(key_compare_func): create and return a new set
 *  (returned set will not free elements)
 */
#define gfsm_set_new(key_cmp_f) g_tree_new(key_cmp_f)

/** Copy set @src to @dst. \returns @dst */
gfsmSet *gfsm_set_copy(gfsmSet *dst, gfsmSet *src);

/** Utilitiy for gfsm_set_copy() */
gboolean gfsm_set_copy_foreach_func(gpointer key, gpointer value, gfsmSet *dst);

/** clear a set */
void gfsm_set_clear(gfsmSet *set);

/** destroy a set */
#define gfsm_set_free(set) g_tree_destroy(set)
//@}


/*======================================================================
 * Accessors
 */
///\name gfsmSet: Accessors
//@{
/** check set membersgip */
#define gfsm_set_contains(set,key) g_tree_lookup(set,key)

/** insert a new key into the set */
#define gfsm_set_insert(set,key) g_tree_insert(set,key,(gpointer)1)

/** get size of set */
#define gfsm_set_size(set) g_tree_nnodes(set)

/** Remove an element from a set */
#define gfsm_set_remove(set,key) g_tree_remove(set,key)

/** Traversal (see g_tree_foreach) */
#define gfsm_set_foreach(set,func,data) g_tree_foreach(set,func,data)
//@}

/*======================================================================
 * set: Algebra
 */
///\name gfsmSet: Algebra
//@{

/** Add all elements of set @set2 to @set1.
 *  If @dupfunc is non-NULL, it will be used to copy elements from @set2,
 *  otherwise elements will be copied as literal gpointer values.
 * \returns altered @set1
 */
gfsmSet *gfsm_set_union(gfsmSet *set1, gfsmSet *set2, gfsmDupFunc dupfunc);

/** Remove all elements in @set2 from @set1.
 * \returns altered @set1 */
gfsmSet *gfsm_set_difference(gfsmSet *set1, gfsmSet *set2);

/** Remove all elements from @set1 which are not also in @set2.
 * \returns altered @set1 */
gfsmSet *gfsm_set_intersection(gfsmSet *set1, gfsmSet *set2);

//@}


/*======================================================================
 * Converters
 */
///\name gfsmSet: converters
//@{

/** Get a GSList of a set's elements */
GSList *gfsm_set_to_slist(gfsmSet *set);

/** Foreach utilitity for set_to_slist() */
gboolean gfsm_set_to_slist_foreach_func(gpointer key, gpointer value, GSList **dst);

/** Append a set's elements to a GPtrArray */
void gfsm_set_to_ptr_array(gfsmSet *set, GPtrArray *array);

/** Foreach utilitity for set_to_array() */
gboolean gfsm_set_to_ptr_array_foreach_func(gpointer key, gpointer value, GPtrArray *dst);
//@}

/*======================================================================
 * Debugging
 */
///\name gfsmSet: debugging
//@{

/** Dump contents of a gfsmSet using '%u' to a FILE* */
void gfsm_set_print_uint(gfsmSet *set, FILE *f);

//@}

#endif /* _GFSM_SET_H */
