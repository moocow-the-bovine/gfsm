
/*=============================================================================*\
 * File: gfsmMem.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: memory utilities
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

#include <gfsmMem.h>
#include <string.h>

/*----------------------------------------------------------------------
 * Allocators
 */
GAllocator *gfsm_node_allocator  = NULL;
GAllocator *gfsm_slist_allocator = NULL;
GAllocator *gfsm_list_allocator  = NULL;
gboolean    gfsm_allocators_enabled = FALSE;


/*----------------------------------------------------------------------
 * init_allocators()
 */
void gfsm_allocators_init(void)
{
  if (!gfsm_node_allocator) gfsm_node_allocator = g_allocator_new("gfsm_node_allocator",128);
  if (!gfsm_slist_allocator) gfsm_slist_allocator = g_allocator_new("gfsm_slist_allocator",128);
  if (!gfsm_list_allocator) gfsm_list_allocator  = g_allocator_new("gfsm_list_allocator",128);
}

/*----------------------------------------------------------------------
 * allocators_enable()
 */
void gfsm_allocators_enable(void)
{
  if (!gfsm_allocators_enabled) {
    gfsm_allocators_init();
    g_node_push_allocator(gfsm_node_allocator);
    g_slist_push_allocator(gfsm_slist_allocator);
    g_list_push_allocator(gfsm_list_allocator);
    gfsm_allocators_enabled = TRUE;
  }
}

/*----------------------------------------------------------------------
 * allocators_disable()
 */
void gfsm_allocators_disable(void)
{
  if (gfsm_allocators_enabled) {
    g_node_pop_allocator();
    g_slist_pop_allocator();
    g_list_pop_allocator();
    gfsm_allocators_enabled = FALSE;
  }
}


/*----------------------------------------------------------------------
 * allocators_free()
 */
void gfsm_allocators_free(void)
{
  gfsm_allocators_disable();

  if (gfsm_node_allocator) g_allocator_free(gfsm_node_allocator);
  if (gfsm_slist_allocator) g_allocator_free(gfsm_slist_allocator);
  if (gfsm_list_allocator) g_allocator_free(gfsm_list_allocator);

  gfsm_node_allocator=NULL;
  gfsm_slist_allocator=NULL;
  gfsm_list_allocator=NULL;
}


/*----------------------------------------------------------------------
 * string_dup_n()
 */
gpointer gfsm_string_dup_n(gconstpointer src, gsize size)
{ return g_strndup(src,size); };

/*----------------------------------------------------------------------
 * mem_dup_n()
 */
gpointer gfsm_mem_dup_n(gconstpointer src, gsize size)
{ return g_memdup(src,size); };

/*----------------------------------------------------------------------
 * gstring_dup()
 */
GString *gfsm_gstring_dup (GString *gstr)
{
  GString *dst = g_string_sized_new(gstr->len);
  g_string_append_len(dst, gstr->str, gstr->len);
  return dst;
}

/*----------------------------------------------------------------------
 * gstring_asign_bytes()
 */
void gfsm_gstring_assign_bytes (GString *gstr, const gchar *src, gsize len)
{
  g_string_truncate(gstr, 0);
  g_string_append_len(gstr, src, gstr->len);
}

/*----------------------------------------------------------------------
 * gstring_new_bytes()
 */
GString *gfsm_gstring_new_bytes (const gchar *src, gsize len)
{
  GString *dst = g_string_sized_new(src[len]=='\0' ? len : (len+1));
  g_string_append_len(dst,src,len);
  return dst;
}
