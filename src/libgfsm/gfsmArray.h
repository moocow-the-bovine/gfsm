/*=============================================================================*\
 * File: gfsmArray.h
 * Author: Bryan Jurish <moocow.bovine@gmail.com>
 * Description: finite state machine library
 *
 * Copyright (c) 2011 Bryan Jurish.
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

/** \file gfsmArray.h
 *  \brief various array utilities
 */

#ifndef _GFSM_ARRAY_H
#define _GFSM_ARRAY_H

#include <glib.h>

/*======================================================================
 * Methods: GArray
 */
/// \name GArray Methods
//@{

/** Search a ::GArray \a array of elements of size \a element_size sorted according to \a compare_func
 *  for the first element not less-than or equal to \a key, using \a data as comparison data.
 *
 * \param array ::GArray to search
 * \param element_size size of elements in \a array
 * \param key  key element to search for
 * \param compare_func comparison function for search (array should be sorted consistently with \a compare_func)
 * \param data comparison data
 * 
 * \returns a pointer to the desired element or \a NULL if no such element is found.
 */
gpointer gfsm_array_lower_bound(GArray *array, guint element_size, gconstpointer key, GCompareDataFunc compare_func, gpointer data);
//@}

/*======================================================================
 * Methods: GPtrArray
 */
/// \name GPtrArray Methods
//@{

/** Search a ::GPtrArray \a parray sorted according to \a compare_func for
 *  the first element not less-than or equal to \a key, using \a data as comparison data.
 *  \returns a pointer (gpointer*) to the desired element or \a NULL if no such element is found.
 */
gpointer* gfsm_ptr_array_lower_bound(GPtrArray *parray, gconstpointer key, GCompareDataFunc compare_func, gpointer data);

/** Insert an element into a sorted ::GPtrArray.
 */
gpointer* gfsm_ptr_array_insert_sorted(GPtrArray *parray, gpointer elt, GCompareDataFunc compare_func, gpointer data);

//@}


#endif /* _GFSM_ENUM_H */
