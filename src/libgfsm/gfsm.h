
/*=============================================================================*\
 * File: gfsm.h
 * Author: Bryan Jurish <moocow.bovine@gmail.com>
 * Description: finite state machine library: top-level
 *
 * Copyright (c) 2004-2025 Bryan Jurish.
 *
 * For information on usage and redistribution, and for a DISCLAIMER
 * OF ALL WARRANTIES, see the file "COPYING" in this distribution.
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

/** \file gfsm.h
 *  \brief Top-level header.
 */

#ifndef _GFSM_H
#define _GFSM_H

/* 2025-06-22: glib.h no longer likes to be included as extern "C" ... sigh
 * - see https://gitlab.gnome.org/GNOME/glib/-/issues/2331
 */
#include <glib.h>

G_BEGIN_DECLS
#include <gfsmArray.h>
#include <gfsmAssert.h>
#include <gfsmCommon.h>
#include <gfsmCompound.h>
#include <gfsmVersion.h>
#include <gfsmError.h>
#include <gfsmUtils.h>
#include <gfsmEnum.h>
#include <gfsmSet.h>
#include <gfsmWeightMap.h>
#include <gfsmBitVector.h>
#include <gfsmAlphabet.h>
#include <gfsmSemiring.h>
#include <gfsmArc.h>
#include <gfsmState.h>
#include <gfsmAutomaton.h>
#include <gfsmArcIter.h>
#include <gfsmArcIndex.h>
#include <gfsmStateSort.h>
//#include <gfsmWStateSet.h>
#include <gfsmIO.h>
#include <gfsmAutomatonIO.h>
#include <gfsmDraw.h>
#include <gfsmAlgebra.h>
#include <gfsmArith.h>
#include <gfsmEncode.h>
#include <gfsmLookup.h>
#include <gfsmPaths.h>
#include <gfsmTrain.h>
#include <gfsmTrie.h>
#include <gfsmScanner.h>
#include <gfsmRegexCompiler.h>
#include <gfsmIndexed.h>
#include <gfsmIndexedIO.h>
G_END_DECLS

#endif /* _GFSM_H */
