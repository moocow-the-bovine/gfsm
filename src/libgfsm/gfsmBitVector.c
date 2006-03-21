
/*=============================================================================*\
 * File: gfsmBitVector.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library
 *
 * Copyright (c) 2004-2006 Bryan Jurish.
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
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

#include <glib.h>
#include <gfsmBitVector.h>

/*======================================================================
 * Constructors etc.
 */

/*--------------------------------------------------------------
 * new()
 */
//gfsmBitVector *gfsm_bitvector_new(void) { return g_byte_array_new(); }

/*--------------------------------------------------------------
 * sized_new()
 */
gfsmBitVector *gfsm_bitvector_sized_new(guint nbits)
{
  gfsmBitVector *bv = g_array_sized_new(FALSE,TRUE,1,_gfsm_bitvector_bits2bytes(nbits)+1);
  gfsm_bitvector_set(bv, nbits-1, 0);
  return bv;
}

/*--------------------------------------------------------------
 * zero()
 */
gfsmBitVector *gfsm_bitvector_zero(gfsmBitVector *bv)
{
  guint i;
  for (i=0; i < bv->len; i++) { g_array_index(bv,guint8,i) = 0; }
  return bv;
}

/*--------------------------------------------------------------
 * one()
 */
gfsmBitVector *gfsm_bitvector_one(gfsmBitVector *bv)
{
  guint i;
  for (i=0; i < bv->len; i++) { g_array_index(bv,guint8,i) = 0xff; }
  return bv;
}


/*======================================================================
 * Accessors
 */

