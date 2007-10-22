/*=============================================================================*\
 * File: gfsmSemiring.h
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

/** \file gfsmSemiring.h
 *  \brief semiring types & operations
 */

#ifndef _GFSM_SEMIRING_H
#define _GFSM_SEMIRING_H

#include <glib.h>
#include <gfsmCommon.h>

#include <float.h>

/*======================================================================
 * Semiring: types
 */
/** Builtin semiring types
 *  \see fsmcost(3)
 */
typedef enum _gfsmSRType {
  gfsmSRTUnknown  = 0,  ///< unknown semiring (should never happen)
  gfsmSRTBoolean  = 1,  ///< boolean semiring <set:{0,1}, plus:||, times:&&, less:>, zero:0, one:1>
  gfsmSRTLog      = 2,  ///< negative log semiring <set:[-inf,inf], plus:-log(e^-x+e^-y), times:+, less:<, zero:inf, one:0>
  gfsmSRTReal     = 3,  ///< real semiring: <set:[0,inf], plus:+, times:*, less:<, zero:0, one:1>
  gfsmSRTTrivial  = 4,  ///< trivial semiring <set:{0}, plus:+, times:+, less:!=, zero:0, one:0>
  gfsmSRTTropical = 5,  ///< tropical semiring: <set:[-inf,inf], plus:min, times:+, less:<, zero:inf, one:0>
  gfsmSRTPLog     = 6,  ///< positive log semiring <set:[-inf,inf], plus:log(e^x+e^y), times:+, less:>, zero:-inf, one:0>
  gfsmSRTUser     = 256 ///< user-defined semiring
} gfsmSRType;

/*======================================================================
 * Semiring: types: structs
 */
/// struct to represent a builtin semi-ring for gfsm arc weights
typedef struct _gfsmSemiring {
  gfsmSRType type;    /**< type of this semiring */
  gfsmWeightU zero;    /**< nil element of this semiring (identity for '+') */
  gfsmWeightU one;     /**< unity element of this semiring (idendity for '*') */
} gfsmSemiring;

/*======================================================================
 * Semiring: types: functions
 */
/** Type for user-defined semiring unary predicates (i.e. member) */
typedef gboolean (*gfsmSRUnaryPredicate) (gfsmSemiring *sr, gfsmWeightU x);

/// Type for user-defined semiring binary predicates (i.e. equal) */
typedef gboolean (*gfsmSRBinaryPredicate) (gfsmSemiring *sr, gfsmWeightU x, gfsmWeightU y);

/// Type for user-defined semiring unary operations */
typedef gfsmWeightU (*gfsmSRUnaryOp) (gfsmSemiring *sr, gfsmWeightU x);

/// Type for user-defined semiring binary operations */
typedef gfsmWeightU (*gfsmSRBinaryOp) (gfsmSemiring *sr, gfsmWeightU x, gfsmWeightU y);


/*======================================================================
 * Semiring: types: user structs
 */
/// User-defined semirings for gfsm operations
typedef struct _gfsmSemiringUser {
  gfsmSemiring sr;                  /**< inheritance magic */

  //-- user-defined semirings *must* set these functions
  gfsmSRBinaryPredicate equal_func; /**< equality predicate */
  gfsmSRBinaryPredicate less_func;  /**< order predicate */
  gfsmSRBinaryOp        plus_func;  /**< addition operation */
  gfsmSRBinaryOp        times_func; /**< multiplication operation */
} gfsmUserSemiring;

/*======================================================================
 * Semiring: methods: constructors etc.
 */
///\name Constructors etc.
//@{

/** Create, initialize (for builtin types), and return new semiring of type \a type */
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

/** Get 'zero' element of the semiring \a sr */
#define gfsm_sr_zero(sr) ((sr) ? (sr)->zero : ((gfsmWeightU){0}))

/** Get 'one' element of the semiring \a sr */
#define gfsm_sr_one(sr) ((sr) ? (sr)->one : ((gfsmWeightU){1}))

/** Check semiring element equality */
#define gfsm_sr_equal(sr,x,y) \
  ((sr)->type == gfsmSRTUser && ((gfsmUserSemiring*)(sr))->equal_func \
   ? ((*((gfsmUserSemiring*)(sr))->equal_func)((sr),(x),(y))) \
   : ((x).f==(y).f))

/** Check semiring element order */
gboolean gfsm_sr_less(gfsmSemiring *sr, gfsmWeightU x, gfsmWeightU y);

/** 3-way comparison for semiring values */
gint gfsm_sr_compare(gfsmSemiring *sr, gfsmWeightU x, gfsmWeightU y);

/** Semiring addition */
gfsmWeightU gfsm_sr_plus(gfsmSemiring *sr, gfsmWeightU x, gfsmWeightU y);

/** Semiring multiplication */
gfsmWeightU gfsm_sr_times(gfsmSemiring *sr, gfsmWeightU x, gfsmWeightU y);
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

/*======================================================================
 * Semiring: methods: general utilities
 */
///\name General utilities
//@{
/** stable log addition.
 *  \returns log(exp(x)+exp(y))
 */
gfloat gfsm_log_add(gfloat x, gfloat y);
//@}

#endif /* _GFSM_SEMIRING_H */
