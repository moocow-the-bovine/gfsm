
/*=============================================================================*\
 * File: gfsmBitVector.h
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

/** \file gfsmBitVector.h
 *  \brief Bit vector utilities using GPtrArray
 */

#ifndef _GFSM_BITVECTOR_H
#define _GFSM_BITVECTOR_H

#include <glib.h>

/*======================================================================
 * Types
 */
/// bit vector type: really just a wrapper for GArray
typedef GArray gfsmBitVector;

/*======================================================================
 * Utilities
 */
///\name Utilities
//@{
/**  Utility macro to convert bit-indices to byte-indices (starting from 0) */
#define _gfsm_bitvector_bits2bytes(nbits)  ( (nbits)/8 )


/** Utility macro to convert byte-sizes to bit-sizes */
#define _gfsm_bitvector_bytes2bits(nbytes) ( (nbytes)*8 )
//@}

/*======================================================================
 * Constructors etc.
 */
///\name Constructors etc.
//@{
/** Create a new bit vector of length 0 */
#define gfsm_bitvector_new() g_array_new(FALSE,TRUE,1)

/** Create a new bit vector of length 0 with reserved space for at least \a nbits bits */
gfsmBitVector *gfsm_bitvector_sized_new(guint nbits);

/** Resize a bit vector \a bv to at least \a nbits bits (rounded to next byte boundary) */
#define gfsm_bitvector_resize(bv,nbits) \
   g_array_set_size((bv), _gfsm_bitvector_bits2bytes(nbits)+1)

/** Get current size (in bits) of a bit vector \a bv */
#define gfsm_bitvector_size(bv) (_gfsm_bitvector_bytes2bits((bv)->len))

/** Clear a bit vector */
#define gfsm_bitvector_clear(bv) g_array_set_size((bv),0)

/** Set all bits to zero. \returns altered bv */
gfsmBitVector *gfsm_bitvector_zero(gfsmBitVector *bv);

/** Set all bits to one. \returns altered bv */
gfsmBitVector *gfsm_bitvector_one(gfsmBitVector *bv);

/** Destroy a bit vector */
#define gfsm_bitvector_free(bv) g_array_free((bv),TRUE)
//@}

/*======================================================================
 * Accessors
 */
///\name Accessors
//@{

/** Get the value (0 or 1) of the bit at index \a i in vector \a bv
 *  \note Implemented as a macro which evaluates its arguments multiple times.
 */
#define gfsm_bitvector_get(bv,i) \
  ( (i) < gfsm_bitvector_size(bv) \
    ? ( (g_array_index((bv), guint8, _gfsm_bitvector_bits2bytes(i)) & (1<<((i)%8))) ? 1 : 0 ) \
    : 0 )

/** Set the value (0 or 1) of the bit at index \a i to boolean value \a v in vector \a bv.
 *  Formerly implemented as a macro which evaluates its arguments multiple times.
 */
void gfsm_bitvector_set(gfsmBitVector *bv, guint i, gboolean v);

#define gfsm_bitvector_set_macro(bv,i,v) \
  ( ((i) >= gfsm_bitvector_size(bv) ? gfsm_bitvector_resize((bv),(i)) : 0), \
    ((v) ? ( g_array_index((bv), guint8, _gfsm_bitvector_bits2bytes(i)) |=  (1<<((i)%8)) ) \
         : ( g_array_index((bv), guint8, _gfsm_bitvector_bits2bytes(i)) &= ~(1<<((i)%8)) ) ) )

//@}


#endif /* _GFSM_BITVECTOR_H */
