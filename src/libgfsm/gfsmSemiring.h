/*=============================================================================*\
 * File: gfsmSemiring.h
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

#ifndef _GFSM_SEMIRING_H
#define _GFSM_SEMIRING_H

#include <glib.h>
#include <gfsmCommon.h>

#include <float.h>

/*======================================================================
 * Semiring: types
 */
/// builtin semiring types
typedef enum _gfsmSRType {
  gfsmSRTUnknown  = 0,
  gfsmSRTBoolean  = 1,
  gfsmSRTLog      = 2,
  gfsmSRTReal     = 3,
  gfsmSRTTrivial  = 4,
  gfsmSRTTropical = 5,
  gfsmSRTUser     = 256
} gfsmSRType;

/*======================================================================
 * Semiring: types: structs
 */
/// struct to represent a builtin semi-ring for gfsm arc weights
typedef struct _gfsmSemiring {
  gfsmSRType type;    ///< type of this semiring
  gfsmWeight zero;    ///< nil element of this semiring (identity for '+')
  gfsmWeight one;     ///< unity element of this semiring (idendity for '*')
} gfsmSemiring;

/*======================================================================
 * Semiring: types: functions
 */
/** Type for semiring unary predicates (i.e. member) */
typedef gboolean (*gfsmSRUnaryPredicate) (gfsmSemiring *sr, gfsmWeight x);

/// Type for semiring binary predicates (i.e. equal) */
typedef gboolean (*gfsmSRBinaryPredicate) (gfsmSemiring *sr, gfsmWeight x, gfsmWeight y);

/// Type for semiring unary operations */
typedef gfsmWeight (*gfsmSRUnaryOp) (gfsmSemiring *sr, gfsmWeight x);

/// Type for semiring binary operations */
typedef gfsmWeight (*gfsmSRBinaryOp) (gfsmSemiring *sr, gfsmWeight x, gfsmWeight y);


/*======================================================================
 * Semiring: types: user structs
 */
/// Hacked struct to represent a semi-ring for gfsm arc weights
typedef struct _gfsmSemiringUser {
  gfsmSemiring sr;                  ///< inheritance magic

  //-- user-defined semirings must set these functions
  gfsmSRBinaryPredicate equal_func; ///< equality predicate
  gfsmSRBinaryPredicate less_func;  ///< order predicate
  gfsmSRBinaryOp        plus_func;  ///< addition operation
  gfsmSRBinaryOp        times_func; ///< multiplication operation
} gfsmUserSemiring;

/*======================================================================
 * Semiring: methods: constructors etc.
 */
///\name Constructors etc.
//@{

/** Create and return new semiring of type @type */
gfsmSemiring *gfsm_semiring_new(gfsmSRType type);

/** Initialize and return a builtin semiring */
void gfsm_semiring_init(gfsmSemiring *sr, gfsmSRType type);

/** Initialize and return a semiring */
gfsmUserSemiring *gfsm_user_semiring_new(gfsmSRBinaryPredicate equal_func,
					 gfsmSRBinaryPredicate less_func,
					 gfsmSRBinaryOp        plus_func,
					 gfsmSRBinaryOp        times_func);

/** Copy a semiring */
gfsmSemiring *gfsm_semiring_copy(gfsmSemiring *sr);

/** Destroy a gfsmSemiring */
void gfsm_semiring_free(gfsmSemiring *sr);
//@}

/*======================================================================
 * Semiring: general accessors
 */
///\name General Accessors
//@{

/** Get 'zero' element of the semiring @sr */
#define gfsm_sr_zero(sr) (sr ? sr->zero : 0)

/** Get 'one' element of the semiring @sr */
#define gfsm_sr_one(sr) (sr ? sr->one : 1)

/** Check semiring element equality */
//gboolean gfsm_sr_equal(gfsmSemiring *sr, gfsmWeight x, gfsmWeight y);
#define gfsm_sr_equal(sr,x,y) \
  (sr->type == gfsmSRTUser && sr->equal_func \
   ? ((*((gfsmUserSemiring*)sr)->equal_func)(sr,x,y)) \
   : (x==y))

/** Check semiring element order */
gboolean gfsm_sr_less(gfsmSemiring *sr, gfsmWeight x, gfsmWeight y);

/** 3-way comparison for semiring values */
gint gfsm_sr_compare(gfsmSemiring *sr, gfsmWeight x, gfsmWeight y);

/** Semiring addition */
gboolean gfsm_sr_plus(gfsmSemiring *sr, gfsmWeight x, gfsmWeight y);

/** Semiring multiplication */
gboolean gfsm_sr_times(gfsmSemiring *sr, gfsmWeight x, gfsmWeight y);
//@}

/*======================================================================
 * Semiring: methods: string utilities
 */
///\name String utilities
//@{
/** Convert symbolic name of a semiring to a gfsmSRType */
gfsmSRType gfsm_sr_name_to_type(const char *name);

/** Convert a gfsmSRType to a (constant) symbolic name */
gchar *gfsm_sr_type_to_name(gfsmSRType type);
//@}

#endif /* _GFSM_SEMIRING_H */
