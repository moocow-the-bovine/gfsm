
/*=============================================================================*\
 * File: gfsmIO.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: I/O
 *
 * Copyright (c) 2006 Bryan Jurish.
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

/** \file gfsmIO.h
 *  \brief Abstract I/O routines
 */

#ifndef _GFSM_IO_H
#define _GFSM_IO_H

#include <glib.h>
#include <stdarg.h>
#include <string.h>

/*======================================================================
 * I/O: types
 */
/** Builtin I/O classes */
typedef enum {
  gfsmIOTCFile,         ///< I/O on a C FILE*
  gfsmIOTZFile,         ///< I/O on a zlib gzFile*
  //gfsmIOCString,        ///< I/O on a C char*
  gfsmIOTGString,       ///< I/O on a GString*
  gfsmIOTUser = 255     ///< user I/O
} gfsmIOClass;

/*======================================================================
 * I/O: Handles: structs
 */
/** Generic I/O handle */
typedef struct {
  gfsmIOClass  ioclass; ///< class of this type
  void        *handle;  ///< underlying handle data
  size_t       pos;     ///< read position (really only for strings)
} gfsmIOHandle;

/*======================================================================
 * I/O: Handles: User: types
 */
typedef gboolean (*gfsmIOReadFunc)  (gfsmIOHandle *io, void *buf, size_t nbytes);
typedef gboolean (*gfsmIOWriteFunc) (gfsmIOHandle *io, void *buf, size_t nbytes);

/*======================================================================
 * I/O: Handles: User: struct
 */
/** User-defined I/O handle */
typedef struct {
  gfsmIOHandle h; ///< inheritance magic
  gfsmIOReadFunc  read_func;  ///< read function
  gfsmIOWriteFunc write_func; ///< write function
} gfsmIOUserHandle;

/*======================================================================
 * I/O: Handles: Methods: Read
 */

/** read \a nbytes of data from \a io into \a buf, as \a fread() */
gboolean gfsmio_read(gfsmIOHandle *io, void *buf, size_t nbytes);

/** wrapper for getline() */
size_t gfsmio_getline(gfsmIOHandle *io, char **linebuf, size_t *n);

/** wrapper for getdelim() */
size_t gfsmio_getdelim(gfsmIOHandle *io, char **linebuf, size_t *n, int delim);


/*======================================================================
 * I/O: Handles: Methods: Write
 */

/** write \a nbytes of data from \a buf into \a io, as \a fwrite() */
gboolean gfsmio_write(gfsmIOHandle *io, void *buf, size_t nbytes);

/** wrapper for puts() */
gboolean gfsmio_puts(gfsmIOHandle *io, const char *s);

/** wrapper for putc() */
gboolean gfsmio_putc(gfsmIOHandle *io, int c);

/** wrapper for printf() */
gboolean gfsmio_printf(gfsmIOHandle *io, const char *fmt, ...);

#endif /* _GFSM_IO_H */
