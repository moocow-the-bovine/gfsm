
/*=============================================================================*\
 * File: gfsmMem.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: memory utilities (currently unused)
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

#ifndef _GFSM_MEM_H
#define _GFSM_MEM_H

#include <glib.h>

/*----------------------------------------------------------------------
 * Copying
 */
/** Abstract copy function */
typedef gpointer    (*gfsmDupNFunc)    (gconstpointer src, gsize size);

/** Abstract duplication function */
typedef gpointer    (*gfsmDupFunc)     (gconstpointer src);

/** String copy function for NUL-terminated strings */
gpointer gfsm_string_dup_n (gconstpointer src, gsize size);

/** size-based copy function */
gpointer gfsm_mem_dup_n (gconstpointer src, gsize size);

/** String duplication function for NUL-terminated strings */
#define gfsm_string_dup g_strdup

#endif /* _GFSM_MEM_H */
