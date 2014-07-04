
/*=============================================================================*\
 * File: gfsmMem.h
 * Author: Bryan Jurish <moocow.bovine@gmail.com>
 * Description: finite state machine library: memory utilities (currently unused)
 *
 * Copyright (c) 2004-2011 Bryan Jurish.
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

/** \file gfsmMem.h
 *  \brief Memory utilities
 */

#ifndef _GFSM_MEM_H
#define _GFSM_MEM_H

#include <glib.h>
#include <gfsmConfig.h>

//======================================================================
///\name GAllocator allocation (deprecated)
//@{
/* - these aren't used by default!
 * - for gfsm2.10 they shouldn't be used at all
 */

//-- new for v0.0.10 : chuck deprecated GAllocator
#ifdef GFSM_USE_GALLOCATOR

/** Default GNode allocator */
extern GAllocator *gfsm_node_allocator;

/** Default GSList allocator */
extern GAllocator *gfsm_slist_allocator;

/** Default GList allocator */
extern GAllocator *gfsm_list_allocator;

/** Whether gfsm allocators are currently enabled */
extern gboolean gfsm_allocators_enabled;

/** Ensure that gfsm allocators are defined and non-NULL */
GFSM_INLINE
void gfsm_allocators_init(void);

/** Push gfsm allocators to the stack */
GFSM_INLINE
void gfsm_allocators_enable(void);

/** Pop gfsm allocators from the stack */
GFSM_INLINE
void gfsm_allocators_disable(void);

/** Free all memory allocated by the gfsm allocators */
GFSM_INLINE
void gfsm_allocators_free(void);

#endif /* GFSM_USE_GALLOCATOR */

//======================================================================
///\name Slice Allocators
//@{

/** Allocate a constant-size block of memory using g_slice_alloc() */
GFSM_INLINE
gpointer gfsm_slice_alloc(guint block_size);

/** Allocate and zero a constant-size block of memory using g_slice_alloc0() */
GFSM_INLINE
gpointer gfsm_slice_alloc0(guint block_size);

/** Allocate constant-size block of memory and copy data from \a src into it */
GFSM_INLINE
gpointer gfsm_slice_copy(guint block_size, gconstpointer src);

/** Free a constant-size block of memory allocated with gfsm_slice_alloc() */
GFSM_INLINE
void gfsm_slice_free1(gsize block_size, gpointer mem_block);

/** Wrapper for g_slice_new(type) */
#ifdef GFSM_USE_GSLICE
# define gfsm_slice_new(type) g_slice_new(type)
#else
# define gfsm_slice_new(type) g_new(type,1)
#endif

/** Wrapper for g_slice_alloc(sizeof(type)*n) */
#ifdef GFSM_USE_GSLICE
# define gfsm_slice_new_n(type,n) (type*)g_slice_alloc(sizeof(type)*(n))
#else
# define gfsm_slice_new_n(type,n) (type*)g_malloc(sizeof(type)*(n))
#endif

/** Wrapper for g_slice_new0(type) */
#ifdef GFSM_USE_GSLICE
# define gfsm_slice_new0(type) g_slice_new0(type)
#else
# define gfsm_slice_new0(type) g_new0(type,1)
#endif

/** Wrapper for g_slice_alloc0(sizeof(type)*n) */
#ifdef GFSM_USE_GSLICE
# define gfsm_slice_new0_n(type,n) (type*)g_slice_alloc0(sizeof(type)*(n))
#else
# define gfsm_slice_new0_n(type,n) (type*)g_malloc0(sizeof(type)*(n))
#endif

/** Wrapper for g_slice_dup(type,mem) */
#ifdef GFSM_USE_GSLICE
# define gfsm_slice_dup(type,mem) g_slice_dup(type,(mem))
#else
# define gfsm_slice_dup(type,mem) g_memdup((mem),sizeof(type))
#endif

/** Wrapper for g_slice_free(type,mem) */
#ifdef GFSM_USE_GSLICE
# define gfsm_slice_free(type,mem) g_slice_free(type,(type*)(mem))
#else
# define gfsm_slice_free(type,mem) g_free((mem))
#endif

/** Wrapper for g_slice_free1(sizeof(type)*n,mem) */
#ifdef GFSM_USE_GSLICE
# define gfsm_slice_free_n(type,mem,n) g_slice_free1(sizeof(type)*(n),(gpointer)(mem))
#else
# define gfsm_slice_free_n(type,mem,n) g_free((gpointer)(mem))
#endif

//@}

//======================================================================
///\name Allocation Copying utiltities (using g_new())
//@{

/** Abstract copy function */
typedef gpointer    (*gfsmDupNFunc)    (gconstpointer src, gsize size);

/** Abstract duplication function */
typedef gpointer    (*gfsmDupFunc)     (gconstpointer src);

/** Wrapper for g_new() */
#define gfsm_new(struct_type, n_structs) g_new(struct_type, (n_structs))

/** Wrapper for g_new0() */
#define gfsm_new0(struct_type, n_structs) g_new0(struct_type, (n_structs))

/** Wrapper for g_free() */
GFSM_INLINE
void gfsm_free(gpointer mem);

/** String copy function for NUL-terminated strings */
GFSM_INLINE
gpointer gfsm_string_dup_n (gconstpointer src, gsize size);

/** size-based copy function */
GFSM_INLINE
gpointer gfsm_mem_dup_n (gconstpointer src, gsize size);

/** String duplication function for NUL-terminated strings */
#define gfsm_string_dup g_strdup

/** String duplication function for GString*s */
GFSM_INLINE
GString *gfsm_gstring_dup (GString *gstr);

/** Byte-assignment for GString*s */
GFSM_INLINE
void gfsm_gstring_assign_bytes (GString *gstr, const gchar *src, gsize len);

/** Byte-vector creation for GString*s */
GFSM_INLINE
GString *gfsm_gstring_new_bytes (const gchar *src, gsize len);

//@}

//-- inline definitions
#ifdef GFSM_INLINE_ENABLED
# include <gfsmMem.hi>
#endif

#endif /* _GFSM_MEM_H */
