/*=============================================================================*\
 * File: gfsmSemiring.c
 * Author: Bryan Jurish <moocow.bovine@gmail.com>
 * Description: finite state machine library
 *
 * Copyright (c) 2004-2011 Bryan Jurish.
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

#include <gfsmSemiring.h>

//-- no-inline definitions
#ifndef GFSM_INLINE_ENABLED
# include <gfsmSemiring.hi>
#endif

/*======================================================================
 * Semiring: methods: constructors etc.
 */

//--------------------------------------------------------------
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
  case gfsmSRTPLog:
    sr->zero = -FLT_MAX;
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
  case gfsmSRTArctic:
    sr->zero = -FLT_MAX;
    sr->one  = 0;
    break;
  case gfsmSRTFuzzy:
  case gfsmSRTProb:
  case gfsmSRTReal:
  default:
    sr->zero = 0;
    sr->one  = 1;
    break;
  }
}


/*======================================================================
 * Semiring: methods: general accessors & operations
 */

/*--------------------------------------------------------------
 * compare()
 */
gint gfsm_sr_compare(gfsmSemiring *sr, gfsmWeight x, gfsmWeight y)
{
  switch (gfsm_sr_type(sr)) {
  case gfsmSRTTrivial:    return 0;

  case gfsmSRTLog:
  case gfsmSRTTropical:
  case gfsmSRTReal:       return (x < y ? -1 : (x > y ? 1 : 0));

  case gfsmSRTBoolean:
  case gfsmSRTPLog: 
  case gfsmSRTArctic:
  case gfsmSRTFuzzy:
  case gfsmSRTProb:       return (x > y ? -1 : (x < y ? 1 : 0));

  case gfsmSRTUser:
    //return (gfsm_sr_less(sr,x,y) ? -1 : (gfsm_sr_less(sr,y,x) ? 1 : 0));
    if (((gfsmUserSemiring*)sr)->less_func) {
      if ((*((gfsmUserSemiring*)sr)->less_func)(sr,x,y)) return -1;
      if ((*((gfsmUserSemiring*)sr)->less_func)(sr,y,x)) return  1;
      return 0;
    }

  default:
    return (x < y ? -1 : (x > y ? 1 : 0));
  }
  return 0; //-- should never happen
}

//--------------------------------------------------------------
gfsmWeight gfsm_sr_pow_n(gfsmSemiring *sr, gfsmWeight x, gint n)
{
  gfsmWeight xn = sr->one;
  for ( ; n > 0; n--) {
    xn = gfsm_sr_times(sr, xn, x);
  }
  return xn;
}

//--------------------------------------------------------------
gfsmWeight gfsm_sr_star_n(gfsmSemiring *sr, gfsmWeight x, guint n)
{
  gfsmWeight xn=sr->one, xstar=xn;
  for ( ; n > 0; n--) {
    xn    = gfsm_sr_times(sr, xn, x);
    xstar = gfsm_sr_plus(sr, xstar, xn);
  }
  return xstar;
}

/*======================================================================
 * Semiring: string utilities
 */

/*--------------------------------------------------------------
 * name_to_type()
 */
gfsmSRType gfsm_sr_name_to_type(const char *name)
{
  if      (strcmp(name,"boolean")  ==0) return gfsmSRTBoolean;
  else if (strcmp(name,"log")      ==0) return gfsmSRTLog;
  else if (strcmp(name,"plog")     ==0) return gfsmSRTPLog;
  else if (strcmp(name,"real")     ==0) return gfsmSRTReal;
  else if (strcmp(name,"trivial")  ==0) return gfsmSRTTrivial;
  else if (strcmp(name,"tropical") ==0) return gfsmSRTTropical;
  else if (strcmp(name,"arctic")   ==0) return gfsmSRTArctic;
  else if (strcmp(name,"fuzzy")    ==0) return gfsmSRTFuzzy;
  else if (strcmp(name,"prob")     ==0) return gfsmSRTProb;
  else if (strcmp(name,"user")     ==0) return gfsmSRTUser;
  return gfsmSRTUnknown;
}

/*--------------------------------------------------------------
 * type_to_name()
 */
gchar *gfsm_sr_type_to_name(gfsmSRType type)
{
  switch (type) {
  case gfsmSRTBoolean:  return "boolean";
  case gfsmSRTLog:      return "log";
  case gfsmSRTPLog:     return "plog";
  case gfsmSRTTrivial:  return "trivial";
  case gfsmSRTTropical: return "tropical";
  case gfsmSRTReal:     return "real";
  case gfsmSRTArctic:   return "arctic";
  case gfsmSRTFuzzy:    return "fuzzy";
  case gfsmSRTProb:     return "prob";
  default:              return "unknown";
  }
}

/*======================================================================
 * Semiring: general utilities
 */
