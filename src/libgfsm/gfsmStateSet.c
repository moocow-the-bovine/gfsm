
/*=============================================================================*\
 * File: gfsmStateSet.c
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

#include <gfsmStateSet.h>

/*======================================================================
 * Constants
 */
const guint gfsmStateSetDefaultSize = 2;

/*======================================================================
 * Methods: Constructors etc.
 */
//-- inlined

/*======================================================================
 * Methods: Accessors
 */
//-- inlined

/*======================================================================
 * Methods: iterators
 */
//-- inlined

/*======================================================================
 * Methods: Utilities
 */

/*--------------------------------------------------------------
 * hash()
 */
guint gfsm_stateset_hash(gfsmStateSet *sset)
{
  guint hv = 0;
  gfsmStateSetIter sseti;
  gfsmStateId      iid;
  for (sseti = gfsm_stateset_iter_begin(sset);
       (iid=gfsm_stateset_iter_id(sseti)) != gfsmNoState;
       sseti = gfsm_stateset_iter_next(sset,sseti))
    {
      hv += 5*iid;
    }
  return hv;
}


/*======================================================================
 * Methods: Automaton access
 */

/*--------------------------------------------------------------
 * populate()
 */
void gfsm_stateset_populate(gfsmStateSet *sset,
			    gfsmAutomaton *fsm,
			    gfsmStateId     id,
			    gfsmLabelVal    lo,
			    gfsmLabelVal    hi)
{
  gfsmArcIter ai;
  if (gfsm_stateset_insert(sset,id)) return;
  
  for (gfsm_arciter_open(&ai,fsm,id), gfsm_arciter_seek_both(&ai,lo,hi);
       gfsm_arciter_ok(&ai);
       gfsm_arciter_next(&ai), gfsm_arciter_seek_both(&ai,lo,hi))
    {
      gfsm_stateset_populate(sset,fsm,gfsm_arciter_arc(&ai)->target,lo,hi);
    }
  gfsm_arciter_close(&ai);
}

/*--------------------------------------------------------------
 * has_final_state()
 */
//--inlined

/*--------------------------------------------------------------
 * lookup_final_weight()
 */
//--inlined
