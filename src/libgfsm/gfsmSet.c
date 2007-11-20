/*=============================================================================*\
 * File: gfsmSet.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library
 *
 * Copyright (c) 2004-2007 Bryan Jurish.
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

#include <gfsmSet.h>



/*======================================================================
 * Constructors etc.
 */

/*--------------------------------------------------------------
 * copy_foreach_func()
 */
gboolean gfsm_set_copy_foreach_func(gpointer key, gpointer value, gfsmSet *data)
{
  if (data) g_tree_insert(data,key,value);
  return FALSE; // don't stop iterating
}


/*======================================================================
 * Algebra
 */

/*--------------------------------------------------------------
 * union_func()
 */
gboolean gfsm_set_union_func(gpointer key, gpointer value, gfsmSetUnionData *data)
{
  if (!data->dupfunc) {
    //-- no memory hairiness: just insert
    gfsm_set_insert(data->dst, key);
  } else {
    if (!gfsm_set_contains(data->dst, key))
      gfsm_set_insert(data->dst, (*data->dupfunc)(key));
  }
  return FALSE;
}

/*--------------------------------------------------------------
 * difference_func()
 */
gboolean gfsm_set_difference_func(gpointer key, gpointer value, gfsmSet *set1)
{
  gfsm_set_remove(set1,key);
  return FALSE;
}


/*======================================================================
 * Converters
 */

/*--------------------------------------------------------------
 * to_slist_foreach_func()
 */
gboolean gfsm_set_to_slist_foreach_func(gpointer key, gpointer value, GSList **dst)
{
  *dst = g_slist_prepend(*dst, key);
  return FALSE; //-- don't stop iterating
}

/*--------------------------------------------------------------
 * to_ptr_array_foreach_func()
 */
gboolean gfsm_set_to_ptr_array_foreach_func(gpointer key, gpointer value, GPtrArray *dst)
{
  g_ptr_array_add(dst,key);
  return FALSE;
}

/*======================================================================
 * Debugging
 */
#ifdef GFSM_DEBUG_ENABLED

gboolean gfsm_set_print_foreach_func(gpointer key, gpointer data, FILE *f)
{
  fprintf(f, " %u", GPOINTER_TO_UINT(key));
  return FALSE;
}

void gfsm_set_print_uint(gfsmSet *set, FILE *f)
{
  fputc('{',f);
  g_tree_foreach(set, (GTraverseFunc)gfsm_set_print_foreach_func, f);
  fputs(" }", f);
}

#endif /*GFSM_DEBUG_ENABLED*/
