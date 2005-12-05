/*=============================================================================*\
 * File: gfsmWeightmap.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library
 *
 * Copyright (c) 2004 Bryan Jurish.
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

#include <gfsmWeightMap.h>
#include <gfsmUtils.h>

/*======================================================================
 * gfsmWeightMap: Constructors etc.
 */

/*======================================================================
 * gfsmWeightMap: Accessors
 */

//--------------------------------------------------------------
void gfsm_weightmap_insert(gfsmWeightMap *weightmap, gconstpointer key, gfsmWeight w)
{
  g_tree_insert(weightmap, (gpointer)key, gfsm_weight2ptr(w));
}

//--------------------------------------------------------------
gboolean gfsm_weightmap_contains(gfsmWeightMap *weightmap, gconstpointer key)
{
  gfsmWeight w;
  return gfsm_weightmap_lookup(weightmap, key, &w);
}

//--------------------------------------------------------------
gboolean gfsm_weightmap_lookup(gfsmWeightMap *weightmap, gconstpointer key, gfsmWeight *wp)
{
  gpointer orig_key;
  return g_tree_lookup_extended(weightmap, key, &orig_key, (gpointer*)wp);
}



/*======================================================================
 * gfsmWeighHash: Constructors etc.
 */
//--------------------------------------------------------------
gfsmWeightHash *gfsm_weighthash_new_full(gfsmDupFunc key_dup_func,
					 GHashFunc   key_hash_func,
					 GEqualFunc  key_equal_func,
					 GDestroyNotify key_destroy_func)
{
  gfsmWeightHash *wh = g_new(gfsmWeightHash,1);
  wh->table   = g_hash_table_new_full(key_hash_func, key_equal_func, key_destroy_func, NULL);
  wh->key_dup = key_dup_func;
  return wh;
}


//--------------------------------------------------------------
void gfsm_weighthash_clear(gfsmWeightHash *wh)
{
  g_hash_table_foreach_remove(wh->table, gfsm_hash_clear_func,NULL);
}

//--------------------------------------------------------------
void gfsm_weighthash_free(gfsmWeightHash *wh)
{
  g_hash_table_destroy(wh->table);
  g_free(wh);
}


/*======================================================================
 * gfsmWeightHash: Accessors
 */

//--------------------------------------------------------------
gboolean gfsm_weighthash_lookup(gfsmWeightHash *wh, gconstpointer key, gfsmWeight *wp)
{
  gpointer s_key;
  return g_hash_table_lookup_extended(wh->table, key, &s_key, (gpointer*)wp);
}

//--------------------------------------------------------------
void gfsm_weighthash_insert(gfsmWeightHash *wh, gconstpointer key, gfsmWeight w)
{
  gpointer s_key;
  gpointer s_val;
  if (wh->key_dup && g_hash_table_lookup_extended(wh->table, key, &s_key, &s_val)) {
    //-- already present: steal & replace
    g_hash_table_steal(wh->table, s_key);
    g_hash_table_insert(wh->table, s_key, gfsm_weight2ptr(w));
  }
  else {
    //-- not yet present: insert new mapping
    if (wh->key_dup) s_key = (*(wh->key_dup))(key);
    else             s_key = (gpointer)key;
    g_hash_table_insert(wh->table, s_key, gfsm_weight2ptr(w));
  }
}

//--------------------------------------------------------------
gboolean gfsm_weighthash_insert_if_less(gfsmWeightHash *wh, gconstpointer key, gfsmWeight w, gfsmSemiring *sr)
{
  gpointer s_key;
  gpointer s_val;
  if (wh->key_dup && g_hash_table_lookup_extended(wh->table, key, &s_key, &s_val)) {
    //-- already present 
    gfsmWeight s_w = gfsm_ptr2weight(s_val);
    if (!gfsm_sr_less(sr, w, s_w)) return FALSE; //-- (s_w) <= (w) : no update required

    //-- steal & update
    g_hash_table_steal(wh->table, s_key);
    g_hash_table_insert(wh->table, s_key, gfsm_weight2ptr(w));
  }
  else {
    //-- not yet present: insert new mapping
    if (wh->key_dup) s_key = (*(wh->key_dup))(key);
    else             s_key = (gpointer)key;
    g_hash_table_insert(wh->table, s_key, gfsm_weight2ptr(w));
  }

  return TRUE; //-- update performed
}

//--------------------------------------------------------------
gboolean gfsm_weighthash_insert_sum_if_less(gfsmWeightHash *wh, gconstpointer key, gfsmWeight w, gfsmSemiring *sr)
{
  gpointer s_key;
  gpointer s_val;
  if (wh->key_dup && g_hash_table_lookup_extended(wh->table, key, &s_key, &s_val)) {
    //-- already present
    gfsmWeight s_w = gfsm_ptr2weight(s_val);
    w = gfsm_sr_plus(sr,w,s_w);
    if (!gfsm_sr_less(sr,w,s_w)) return FALSE; //-- (s_w) <= (w+s_w) : no update required

    //-- steal & update
    g_hash_table_steal(wh->table, s_key);
    g_hash_table_insert(wh->table, s_key, gfsm_weight2ptr(w));
  }
  else {
    //-- not yet present: insert new mapping
    if (wh->key_dup) s_key = (*(wh->key_dup))(key);
    else             s_key = (gpointer)key;
    g_hash_table_insert(wh->table, s_key, gfsm_weight2ptr(w));
  }

  return TRUE; //-- update performed
}
