
/*=============================================================================*\
 * File: gfsmEnum.h
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

#ifndef _GFSM_ENUM_H
#define _GFSM_ENUM_H

#include <glib.h>
#include <gfsmMem.h>

/*======================================================================
 * Types
 */
///\name Types and constants
//@{

/// Structure for mapping symbolic names to numeric IDs
typedef struct {
  GHashTable  *table;   ///< hash table which does the dirty work
  guint        nxtval;  ///< next id to assign
  gfsmDupFunc  key_dup; ///< key copying function
} gfsmEnum;

/** Constant indicating failed gfsmEnum value-lookup */
extern const guint gfsmEnumNone;

//@}

/*======================================================================
 * Methods: Constructors etc.
 */
/// \name Constructors etc.
//@{

/** create a new gfsmEnum (full version) */
gfsmEnum *gfsm_enum_new_full(gfsmDupFunc key_dup_func,
			     GHashFunc   key_hash_func,
			     GEqualFunc  key_equal_func,
			     GDestroyNotify key_destroy_func);

/** create a new gfsmEnum (no copying) */
#define gfsm_enum_new(key_hash_f) gfsm_enum_new_full(NULL,key_hash_f,NULL,NULL)

/** Clear a gfsmEnum */
void gfsm_enum_clear(gfsmEnum *en);

/** Free a gfsmEnum */
void gfsm_enum_free(gfsmEnum *en);
//@}

/*======================================================================
 * Methods: Accessors
 */
/// \name Accessors
//@{

/** Get next available value */
#define gfsm_enum_next_value(en) ((en)->nxtval)

/** Lookup the numeric value associated with @lookup_key.
 *  On return, *@stored_key points to the original (stored) key, if any;
 *  and *@stored_value points to the stored value.
 *  \returns true iff a value for @key was already stored
 */
gboolean gfsm_enum_lookup_extended(gfsmEnum      *en,
				   gconstpointer  lookup_key,
				   gpointer      *stored_key,
				   gpointer      *stored_val);

/** Lookup the numeric value associated with @key.
 *  \returns the value associated with @key, or gfsmEnumNone
 *  if no such value was found
 */
guint gfsm_enum_lookup(gfsmEnum *en, gconstpointer key);


/** Insert or overwrite new value @val for @key.
 *  If @val is gfsmEnumNone, the next available value will be used.
 *  \returns new value for @key
 */
guint gfsm_enum_insert_full(gfsmEnum *en, gpointer key, guint val);

/** Insert a (possibly new) value @val for @key */
#define gfsm_enum_insert(en,key) gfsm_enum_insert_full(en,key,gfsmEnumNone)

/** Really just an alias for gfsm_enum_insert(en,key) */
#define gfsm_enum_get(en,key) gfsm_enum_insert(en,key)

#endif /* _GFSM_ENUM_H */
