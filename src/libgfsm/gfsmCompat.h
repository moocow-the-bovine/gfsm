
/*=============================================================================*\
 * File: gfsmCompat.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: compatibility
 *
 * Copyright (c) 2004 Bryan Jurish.
 *
 * For information on usage and redistribution, and for a DISCLAIMER
 * OF ALL WARRANTIES, see the file "COPYING" in this distribution.
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

/** \file gfsmCompat.h
 *  \brief Compatibility routines for non-GNU systems
 */

#ifndef _GFSM_COMPAT_H
#define _GFSM_COMPAT_H

#ifdef HAVE_CONFIG_H
#include <gfsmConfig.h>
#endif /* HAVE_CONFIG_H */

#ifndef HAVE_GETLINE
#include <stdio.h>

/** Compatibility function for non-GNU systems */
extern ssize_t getdelim(char **linebuf, size_t *linebufsz, int delimiter, FILE *file);

/** Compatibility function for non-GNU systems */
extern ssize_t getline(char **linebuf, size_t *n, FILE *file);

#endif /* HAVE_GETLINE */

#endif /* _GFSM_COMPAT_H */
