/*=============================================================================*\
 * File: gfsmArray.c
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

#include <gfsmArray.h>
#include <string.h> //-- for memmove()

/*======================================================================
 * Methods: GArray
 */

//--------------------------------------------------------------
gpointer gfsm_array_lower_bound(GArray *array, guint element_size, gconstpointer key, GCompareDataFunc compare_func, gpointer data)
{
  //guint element_size = g_array_get_element_size(array); //-- requires glib > 2.16
  gchar *min  = array->data;
  gchar *max  = array->data + array->len*element_size;
  while (/*min &&*/ min < max) {
    gchar *mid = min + ((max-min)/2)*element_size;
    gint   cmp = (*compare_func)(mid, key, data);
    if (cmp < 0) { min=mid+element_size; }
    else         { max=mid; }
  }
  return min >= (array->data + array->len*element_size) ? NULL : min;
}

/*======================================================================
 * Methods: GPtrArray
 */

//--------------------------------------------------------------
gpointer* gfsm_ptr_array_lower_bound(GPtrArray *parray, gconstpointer key, GCompareDataFunc compare_func, gpointer data)
{
  gpointer *min = parray->pdata;
  gpointer *max = parray->pdata + parray->len;
  while (/*min &&*/ min < max) {
    gpointer *mid = min + (max-min)/2;
    gint      cmp = (*compare_func)(*mid, key, data);
    if (cmp < 0) { min=mid+1; }
    else         { max=mid; }
  }
  return min >= parray->pdata+parray->len ? NULL : min;
}

//--------------------------------------------------------------
gpointer* gfsm_ptr_array_insert_sorted(GPtrArray *parray, gpointer elt, GCompareDataFunc compare_func, gpointer data)
{
  gpointer *lb, *end;
  g_ptr_array_set_size(parray,parray->len+1);
  parray->len--;
  end = parray->pdata+parray->len;

  lb = gfsm_ptr_array_lower_bound(parray, elt, compare_func, data);
  if (lb==NULL) {
    *end = elt;
    parray->len++;
    return end;
  }

  //-- we need to shovel some data around
  memmove(lb+1, lb, (end-lb)*sizeof(gpointer));
  *lb = elt;
  parray->len++;
  return lb;
}
