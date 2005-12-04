
/*=============================================================================*\
 * File: gfsmArith.h
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

#ifndef _GFSM_ARITH_H
#define _GFSM_ARITH_H

#include <glib.h>
#include <gfsmSemiring.h>
#include <gfsmAutomaton.h>

/** \file gfsmArith.h
 *  \brief Arithmetic operations on automata
 */

/*======================================================================
 * Methods: arithmetic
 */
///\name Arithmetic
//@{


//------------------------------
/** Collect final weights in \a fsm with \a w using gfsm_sr_plus().
 *  \note Destructively alters \a fsm.
 *
 *  \param fsm1 Automaton
 *  \param w    weight.
 *  \returns modified \a fsm1
 */
gfsmAutomaton *gfsm_automaton_final_weight_plus(gfsmAutomaton *fsm, gfsmWeight w);

//------------------------------
/** Extend final weights in \a fsm by \a w using gfsm_sr_times().
 *  \note Destructively alters \a fsm.
 *
 *  \param fsm1 Automaton
 *  \param w    weight.
 *  \returns modified \a fsm
 */
gfsmAutomaton *gfsm_automaton_final_weight_times(gfsmAutomaton *fsm, gfsmWeight w);


//@}
#endif /* _GFSM_ARITH_H */
