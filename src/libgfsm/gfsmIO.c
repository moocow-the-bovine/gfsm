
/*=============================================================================*\
 * File: gfsmIO.c
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

#include <glib.h>
#include <gfsmIO.h>
#include <gfsmCompat.h>
#include <gfsmConfig.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef GFSM_ZLIB_ENABLED
# include <zlib.h>
# define GFSM_DEFAULT_COMPRESSION Z_DEFAULT_COMPRESSION
#endif

/*======================================================================
 * I/O: Handles: Methods: Read
 */

/*--------------------------------------------------------------*/
gboolean gfsmio_read(gfsmIOHandle *io, void *buf, size_t nbytes)
{
  switch (io->ioclass) {

  case gfsmIOTCFile:
    //-- C FILE*
    return (fread(buf, nbytes, 1, (FILE*)io->handle) == 1);
    break;

#ifdef GFSM_ZLIB_ENABLED
  case gfsmIOTZFile:
    //-- zlib gzFile
    return (gzread((gzFile)io->handle, buf, nbytes) == nbytes);
    break;
#endif

  case gfsmIOTGString:
    //-- glib GString*
    if ( io->pos+nbytes <= ((GString*)io->handle)->len ) {
      memcpy(buf, ((GString*)io->handle)->str, nbytes);
      io->pos += nbytes;
      return TRUE;
    }
    break;

  case gfsmIOTUser:
    return (*((gfsmIOUserHandle*)io)->read_func)(io, buf, nbytes);
    break;

  default:
    g_printerr("gfsmio_read(): failed for unknown handle type %u\n", io->ioclass);
    break;
  }
  return FALSE;
}

/** wrapper for getline() */
size_t gfsmio_getline(gfsmIOHandle *io, char **linebuf, size_t *n)
{
  return getdelim(io, linebuf, n, '\n');
}

/** wrapper for getdelim() */
size_t gfsmio_getdelim(gfsmIOHandle *io, char **linebuf, size_t *n, int delim)
{
  switch (io->ioclass) {

  case gfsmIOTCFile:
    //-- C FILE*
    return getdelim(linebuf, n, delim, (FILE*)io->handle);
    break;

#ifdef GFSM_ZLIB_ENABLED
  case gfsmIOTZFile:
    {
      ssize_t i;
      GString *gs;
      gzFile   zf = (gzFile)io->handle;
      for (i=0; i < *n && !gzeof(zf); i++) {
	(*linebuf)[i] = gzgetc(zf);
	if ((*linebuf)[i] == delim) return i;
      }
      if (gzeof(zf)) return i;

      //-- oops: buffer overflow
      gs = g_string_new(i>0 ? *linebuf : "");
      for ( ; !gzeof(zf); i++) {
	g_string_append_c(gs, gzgetc(zf));
	if (gs->str[gs->len-1] == delim) break;
      }

      if (linebuf && *linebuf) free(*linebuf);

      //-- put new buffer into linebuf
      *linebuf = gs->str;
      *n       = gs->allocated_len;
      g_string_free(gs,FALSE);

      return i;
    }
    break;
#endif

  case gfsmIOTGString:
    {
      ssize_t i;
      GString *gs;
      GString *bufgs = (GString*)io->handle;
      for (i=0; i < *n && io->pos < bufgs->len; i++) {
	(*linebuf)[i] = bufgs->str[io->pos++];
	if ((*linebuf)[i] == delim) return i;
      }
      if (io->pos >= bufgs->len) return i;

      //-- oops: buffer overflow
      gs = g_string_new(i>0 ? *linebuf : "");
      for ( ; io->pos < bufgs->str; i++) {
	g_string_append_c(gs, bufgs->str[io->pos++]);
	if (gs->str[gs->len-1] == delim) break;
      }

      if (linebuf && *linebuf) free(*linebuf);

      //-- put new buffer into linebuf
      *linebuf = gs->str;
      *n       = gs->allocated_len;
      g_string_free(gs,FALSE);

      return i;
    }
    break;

  case gfsmIOTUser:
    {
      ssize_t i;
      char    c;
      gboolean rc;
      GString *gs;
      gfsmIOUserHandle *uio = (gfsmIOUserHandle*)io;
      gfsmIOReadFunc    rf  = uio->read_func;

      for (i=0; i < *n && (rc=(*rf)(io,&c,1)); i++) {
	(*linebuf)[i] = c;
	if (c == delim) return i;
      }
      if (!rc) return i;

      //-- oops: buffer overflow
      gs = g_string_new(i>0 ? *linebuf : "");
      for ( ; (rc=(*rf)(io,&c,1)); i++) {
	g_string_append_c(gs, c);
	if (c == delim) break;
      }

      if (linebuf && *linebuf) free(*linebuf);

      //-- put new buffer into linebuf
      *linebuf = gs->str;
      *n       = gs->allocated_len;
      g_string_free(gs,FALSE);

      return i;
    }
    break;

  default:
    g_printerr("gfsmio_getdelim(): failed for unknown handle type %u\n", io->ioclass);
    break;
  }

  return FALSE;
}


/*======================================================================
 * I/O: Handles: Methods: Write
 */

/** write \a nbytes of data from \a buf into \a io, as \a fwrite() */
gboolean gfsmio_write(gfsmIOHandle *io, void *buf, size_t nbytes)
{
  switch (io->ioclass) {

  case gfsmIOTCFile:
    return (fwrite(buf, nbytes, 1, (FILE*)io->handle) == 1);
    break;

#ifdef GFSM_ZLIB_ENABLED
  case gfsmIOTZFile:
    return (gzwrite((gzFile)io->handle, buf, nbytes) == nbytes);
    break;
#endif

  case gfsmIOTGString:
    g_string_append_len((GString*)io->handle, buf, nbytes);
    return TRUE;
    break;

  case gfsmIOTUser:
    return (*((gfsmIOUserHandle*)io)->write_func)(io, buf, nbytes);
    break;

  default:
    g_printerr("gfsmio_write(): failed for unknown handle type %u\n", io->ioclass);
    break;
  }
  return FALSE;
}

/** wrapper for puts() */
gboolean gfsmio_puts(gfsmIOHandle *io, const char *s)
{
  return gfsmio_write(io,s,strlen(s));
}

/** wrapper for putc() */
gboolean gfsmio_putc(gfsmIOHandle *io, int c)
{
  return gfsmio_write(io,\&c,1);
}


/** wrapper for printf() */
gboolean gfsmio_printf(gfsmIOHandle *io, const char *fmt, ...)
{
  gboolean rc;
  va_list ap;

  va_start(ap,fmt);
  rc = gfsmio_vprintf(io, fmt, &ap);
  va_end(ap);

  return rc;
}

/** wrapper for vprintf() */
gboolean gfsmio_printf(gfsmIOHandle *io, const char *fmt, va_list *ap)
{
  switch (io->ioclass) {
  case gfsmIOTCFile:
    vfprintf(buf, fmt, *ap);
    return TRUE;
    break;

#ifdef GFSM_ZLIB_ENABLED
  case gfsmIOTZFile:
#endif
  case gfsmIOTGString:
  case gfsmIOTUser:
    {
      char *obuf = NULL;
      size_t len = 0;
      gboolean rc;
      len = vasprintf(&obuf, fmt, *ap);
      rc  = gfsmio_write(io, obuf, len);
      if (obuf) free(obuf);
      return rc;
    }
    break;

  default:
    g_printerr("gfsmio_write(): failed for unknown handle type %u\n", io->ioclass);
    break;
  }

  return FALSE;
}

#endif /* _GFSM_IO_H */
