
/*=============================================================================*\
 * File: gfsmState.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: states
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

#include <gfsmState.h>
#include <gfsmMem.h>

/*======================================================================
 * Methods: Constructors etc.
 */

/*--------------------------------------------------------------
 * new_full()
 */
gfsmState *gfsm_state_new_full(gboolean is_final, gfsmArcList *arcs)
{
  gfsmState *s = g_new(gfsmState,1);
  s->is_valid = TRUE;
  s->is_final = is_final;
  s->arcs     = arcs;
  return s;
}

/*--------------------------------------------------------------
 * clone()
 */
gfsmState *gfsm_state_copy(gfsmState *dst, const gfsmState *src)
{
  gfsm_state_clear(dst);
  if (!src->is_valid) return dst;
  dst->is_valid = src->is_valid;
  dst->is_final = src->is_final;
  //dst->arcs     = gfsm_arclist_copy(src->arcs);
  dst->arcs     = g_slist_concat(gfsm_arclist_copy(src->arcs), dst->arcs);
  return dst;
}

/*--------------------------------------------------------------
 * clear()
 */
void gfsm_state_clear(gfsmState *s)
{
  gfsm_arclist_free(s->arcs);
  s->is_valid = FALSE;
  s->is_final = FALSE;
  s->arcs     = NULL;
}

/*--------------------------------------------------------------
 * free()
 */
void gfsm_state_free(gfsmState *s, gboolean free_arcs)
{
  if (free_arcs && s->arcs) gfsm_arclist_free(s->arcs);
  g_free(s);
}

/*======================================================================
 * Methods: Accessors
 */
//(none)

