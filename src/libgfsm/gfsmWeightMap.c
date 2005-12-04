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

/*======================================================================
 * Constructors etc.
 */

/*======================================================================
 * Accessors
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
 * Algebra
 */

/*======================================================================
 * Converters
 */
