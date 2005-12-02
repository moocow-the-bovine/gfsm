
/*=============================================================================*\
 * File: gfsmLookup.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library
 *
 * Copyright (c) 2005 Bryan Jurish.
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

/** \file gfsmLookup.h
 *  \brief Linear composition
 */

#ifndef _GFSM_LOOKUP_H
#define _GFSM_LOOKUP_H

#include <gfsmAlphabet.h>
#include <gfsmAutomaton.h>

/*======================================================================
 * Types: lookup
 */
/** Type for gfsmLookup computation state */
typedef struct _gfsmLookupConfig {
  gfsmStateId qt;  /**< current state in transducer */
  gfsmStateId qr;  /**< current state in result acceptor */
  guint32     i;   /**< current position in input vector */
} gfsmLookupConfig;

/** Type for a StateId map */
typedef GPtrArray gfsmStateIdVector;

/*======================================================================
 * Constants
 */

/** Number of states to pre-allocate when extending state-map vector on lookup_full() (>= 1) */
extern const guint gfsmLookupStateMapGet;


/*======================================================================
 * Methods: lookup
 */
///\name Lookup
//@{

//------------------------------
/** Compose linear automaton specified by \a input with the transducer
 *  \a fst and project the output tape to \a result.
 *  \param fst transducer (lower-upper)
 *  \param input input labels (lower)
 *  \param result output transducer or NULL
 *  \returns \a result if non-NULL, otherwise a new automaton.
 */
#define gfsm_automaton_lookup(fst,input,result) \
  gfsm_automaton_lookup_full((fst),(input),(result),NULL)

//------------------------------
/** Compose linear automaton specified by \a input with the transducer
 *  \a fst and project the output tape to \a result, storing state-translation map \a qenum.
 *  \param fst transducer (lower-upper)
 *  \param input input labels (lower)
 *  \param result output transducer or NULL
 *  \param statemap if non-NULL, maps \a result StateIds (indices) to \a fst StateIds (values) on return.
 *                  Not implicitly created or cleared.
 *  \returns \a result if non-NULL, otherwise a new automaton.
 */
gfsmAutomaton *gfsm_automaton_lookup_full(gfsmAutomaton     *fst,
					  gfsmLabelVector   *input,
					  gfsmAutomaton     *result,
					  gfsmStateIdVector *statemap);
					  

//@}


#endif /* _GFSM_LOOKUP_H */
