
/*=============================================================================*\
 * File: gfsmVersion.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library
 *
 * Copyright (c) 2004 Bryan Jurish.
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

#ifndef _GFSM_VERSION_H
#define _GFSM_VERSION_H

#include <glib.h>

/*======================================================================
 * Types
 */
/// Version information
typedef struct {
  guint major;   ///< major version
  guint minor;   ///< minor version
  guint micro;   ///< micro version
} gfsmVersionInfo;

/*======================================================================
 * Constants
 */
/// current version information
extern const gfsmVersionInfo gfsm_version;

/// current version string
extern const char *gfsm_version_string;

#endif /* _GFSM_VERSION_H */
