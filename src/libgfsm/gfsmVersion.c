
/*=============================================================================*\
 * File: gfsmVersion.c
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

#include <gfsmVersion.h>

#ifdef HAVE_CONFIG_H
#include <gfsmConfig.h>
#endif

/*======================================================================
 * Constants
 */
const gfsmVersionInfo gfsm_version =
  {
    GFSM_VERSION_MAJOR,
    GFSM_VERSION_MINOR,
    GFSM_VERSION_MICRO
  };

const char *gfsm_version_string = PACKAGE_STRING;


