
/*=============================================================================*\
 * File: gfsmEnum.c
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

#include <glib.h>
#include <gfsmEnum.h>
#include <gfsmUtils.h>
/*======================================================================
 * Constants
 */
const guint gfsmEnumNone = ((guint)(-1));

/*======================================================================
 * Methods: Constructors etc.
 */

/*--------------------------------------------------------------
 * new_full()
 */
gfsmEnum *gfsm_enum_new_full(gfsmDupFunc key_dup_func,
			     GHashFunc   key_hash_func,
			     GEqualFunc  key_equal_func,
			     GDestroyNotify key_destroy_func)
{
  gfsmEnum *en = g_new(gfsmEnum,1);
  en->table    = g_hash_table_new_full(key_hash_func,
				       key_equal_func,
				       key_destroy_func,
				       NULL);
  en->nxtval   = 0;
  en->key_dup  = key_dup_func;
  return en;
}

/*--------------------------------------------------------------
 * clear()
 */
void gfsm_enum_clear(gfsmEnum *en)
{
  g_hash_table_foreach_remove(en->table, gfsm_hash_clear_func, NULL);
  en->nxtval = 0;
}

/*--------------------------------------------------------------
 * free()
 */
void gfsm_enum_free(gfsmEnum *en)
{
  g_hash_table_destroy(en->table);
  g_free(en);
}


/*======================================================================
 * Methods: Accessors
 */

/*--------------------------------------------------------------
 * lookup_extended()
 */
gboolean gfsm_enum_lookup_extended(gfsmEnum      *en,
				   gconstpointer  lookup_key,
				   gpointer      *stored_key,
				   gpointer      *stored_val)
{
  return g_hash_table_lookup_extended(en->table, lookup_key, stored_key, stored_val);
}

/*--------------------------------------------------------------
 * lookup()
 */
guint gfsm_enum_lookup(gfsmEnum *en, gconstpointer key)
{
  guint s_val;
  if (g_hash_table_lookup_extended(en->table, key, NULL, (gpointer*)(&s_val))) return s_val;
  return gfsmEnumNone;
}


/*--------------------------------------------------------------
 * insert_full()
 */
guint gfsm_enum_insert_full(gfsmEnum *en, gpointer key, guint val)
{
  gpointer s_key;
  guint    s_val;
  if (g_hash_table_lookup_extended(en->table, key, &s_key, ((gpointer)(&s_val)))) {
    //-- already present: steal it & replace
    if (val == gfsmEnumNone) {
      val = s_val;
    } else {
      g_hash_table_steal(en->table, s_key);
      g_hash_table_insert(en->table, s_key, (gpointer)val);
    }
  }
  else {
    //-- no key already present: insert a new one
    if (en->key_dup) s_key = (*(en->key_dup))(key);
    else             s_key = key;
    if (val == gfsmEnumNone) val = en->nxtval;
    g_hash_table_insert(en->table, s_key, (gpointer)val);
  }

  //-- keep track of available values
  if (val >= en->nxtval) en->nxtval = val+1;

  return val;
}
