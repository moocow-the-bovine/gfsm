
/*=============================================================================*\
 * File: gfsmMem.c
 * Author: Bryan Jurish <moocow.bovine@gmail.com>
 * Description: finite state machine library: memory utilities
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

#include <gfsmMem.h>

#ifndef GFSM_INLINE_ENABLED
# include <gfsmMem.hi>
#endif

/*----------------------------------------------------------------------
 * Allocators
 */
#ifdef GFSM_USE_GALLOCATOR
GAllocator *gfsm_node_allocator  = NULL;
GAllocator *gfsm_slist_allocator = NULL;
GAllocator *gfsm_list_allocator  = NULL;
gboolean    gfsm_allocators_enabled = FALSE;
#endif /* GFSM_USE_GALLOCATOR */
