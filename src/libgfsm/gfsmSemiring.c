/*=============================================================================*\
 * File: gfsmSemiring.c
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
#include <gfsmSemiring.h>
#include <float.h>
#include <math.h>
#include <string.h>

/*======================================================================
 * Semiring: methods: constructors etc.
 */

/*--------------------------------------------------------------
 * new()
 */
gfsmSemiring *gfsm_semiring_new(gfsmSRType type)
{
  gfsmSemiring *sr = (gfsmSemiring*)g_new(gfsmSemiring,1);
  gfsm_semiring_init(sr,type);
  return sr;
}

/*--------------------------------------------------------------
 * init()
 */
void gfsm_semiring_init(gfsmSemiring *sr, gfsmSRType type)
{
  memset(sr, 0, type==gfsmSRTUser ? sizeof(gfsmUserSemiring) : sizeof(gfsmSemiring));
  sr->type = type;
  switch (type) {
  case gfsmSRTBoolean:
    sr->zero = 0;
    sr->one  = 1;
    break;
  case gfsmSRTLog:
    sr->zero = FLT_MAX;
    sr->one  = 0;
    break;
  case gfsmSRTTrivial:
    sr->zero = 0;
    sr->one  = 0;
    break;
  case gfsmSRTTropical:
    sr->zero = FLT_MAX;
    sr->one  = 0;
    break;
  case gfsmSRTReal:
  default:
    sr->zero = 0;
    sr->one  = 1;
    break;
  }
}

/*--------------------------------------------------------------
 * user_new()
 */
gfsmUserSemiring *gfsm_user_semiring_new(gfsmSRBinaryPredicate equal_func,
					 gfsmSRBinaryPredicate less_func,
					 gfsmSRBinaryOp        plus_func,
					 gfsmSRBinaryOp        times_func)
{
  gfsmUserSemiring *sr = g_new(gfsmUserSemiring, 1);
  gfsm_semiring_init((gfsmSemiring*)sr,gfsmSRTUser);
  sr->equal_func = equal_func;
  sr->less_func = less_func;
  sr->plus_func = plus_func;
  sr->times_func = times_func;
  return sr;
}

/*--------------------------------------------------------------
 * copy()
 */
gfsmSemiring *gfsm_semiring_copy(gfsmSemiring *sr)
{
  if (sr->type==gfsmSRTUser)
    return (gfsmSemiring*)gfsm_user_semiring_new(((gfsmUserSemiring*)sr)->equal_func,
						 ((gfsmUserSemiring*)sr)->less_func,
						 ((gfsmUserSemiring*)sr)->plus_func,
						 ((gfsmUserSemiring*)sr)->times_func);
  return gfsm_semiring_new(sr->type);
}

/*--------------------------------------------------------------
 * free()
 */
void gfsm_semiring_free(gfsmSemiring *sr)
{
  if (sr->type==gfsmSRTUser) g_free((gfsmUserSemiring*)sr);
  else g_free(sr);
}
//@}


/*--------------------------------------------------------------
 * equal()
 */
//(macro)

/*--------------------------------------------------------------
 * less()
 */
gboolean gfsm_sr_less(gfsmSemiring *sr, gfsmWeight x, gfsmWeight y)
{
  switch (sr->type) {
  case gfsmSRTLog:
  case gfsmSRTTropical: return (x < y);

  case gfsmSRTTrivial:  return (x != y);

  case gfsmSRTUser:
    if (((gfsmUserSemiring*)sr)->less_func)
      return (*((gfsmUserSemiring*)sr)->less_func)(sr,x,y);

  case gfsmSRTBoolean:
  case gfsmSRTReal:
  default:             return (x > y);
  }
  return FALSE; //-- should never happen
}

/*--------------------------------------------------------------
 * compare()
 */
gint gfsm_sr_compare(gfsmSemiring *sr, gfsmWeight x, gfsmWeight y)
{
  switch (sr->type) {
  case gfsmSRTLog:
  case gfsmSRTTropical: 
  case gfsmSRTTrivial:  return (x < y ? -1 : (x > y ? 1 : 0));

  case gfsmSRTUser:
    return (gfsm_sr_compare(sr,x,y) ? -1 : (gfsm_sr_compare(sr,y,x) ? 1 : 0));
    if (((gfsmUserSemiring*)sr)->less_func) {
      if ((*((gfsmUserSemiring*)sr)->less_func)(sr,x,y)) return -1;
      if ((*((gfsmUserSemiring*)sr)->less_func)(sr,y,x)) return  1;
      return 0;
    }

  case gfsmSRTBoolean:
  case gfsmSRTReal:
  default:             return (x > y ? -1 : (x < y ? 1 : 0));
  }
  return 0; //-- should never happen
}

/*--------------------------------------------------------------
 * plus()
 */
gboolean gfsm_sr_plus(gfsmSemiring *sr, gfsmWeight x, gfsmWeight y)
{ 
  switch (sr->type) {
  case gfsmSRTBoolean:  return x || y;
  case gfsmSRTLog:      return -log(exp(-x) + exp(-y));
  case gfsmSRTTropical: return (x < y ? x : y);
  case gfsmSRTTrivial:

  case gfsmSRTUser:
    if (((gfsmUserSemiring*)sr)->plus_func)
      return (*((gfsmUserSemiring*)sr)->plus_func)(sr,x,y);

  case gfsmSRTReal:
  default:             return x + y;
  }
  return sr->zero; //-- should never happen
}

/*--------------------------------------------------------------
 * times()
 */
gboolean gfsm_sr_times(gfsmSemiring *sr, gfsmWeight x, gfsmWeight y)
{
  switch (sr->type) {
  case gfsmSRTBoolean:  return x && y;
  case gfsmSRTLog:      
  case gfsmSRTTrivial:
  case gfsmSRTTropical: return x + y;

  case gfsmSRTUser:
    if (((gfsmUserSemiring*)sr)->times_func)
      return (*((gfsmUserSemiring*)sr)->times_func)(sr,x,y);

  case gfsmSRTReal:
  default:             return x * y;
  }
  return sr->zero; //-- should never happen
}

/*--------------------------------------------------------------
 * div()
 */
/*gboolean gfsm_sr_div(gfsmSemiring *sr, gfsmWeight x, gfsmWeight y)
  { return (sr->div_func ? ((*sr->div_func)(sr,x,y)) : (x/y)); }
*/
//@}
