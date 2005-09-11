
/*=============================================================================*\
 * File: gfsmCompat.c
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

#include <gfsmCompat.h>
#include <stdlib.h>
#include <errno.h>

#ifndef HAVE_GETLINE

ssize_t getdelim(char **linebuf, size_t *linebufsz, int delimiter, FILE *file)
{
  static const int GROWBY = 80; /* how large we will grow strings by */

  int ch;
  int idx = 0;
    
    
  if ((file == NULL || linebuf==NULL || *linebuf == NULL || *linebufsz == 0)
      && !(*linebuf == NULL && *linebufsz ==0 )) {
    errno=EINVAL;
    return -1;
  }
                          
                          
  if (*linebuf == NULL && *linebufsz == 0){
    *linebuf = malloc(GROWBY);
    if (!*linebuf) {
      errno=ENOMEM;
      return -1;
    }
    *linebufsz += GROWBY;
  }
                                                          
                                                          
  while (1) {
    ch = fgetc(file);
    if (ch == EOF)
      break;
    /* grow the line buffer as necessary */
    while (idx > *linebufsz-2) {
      *linebuf = realloc(*linebuf, *linebufsz += GROWBY);
      if (!*linebuf) {
	errno=ENOMEM;
	return -1;
      }
    }
    (*linebuf)[idx++] = (char)ch;
    if ((char)ch == delimiter)
      break;
  }

  if (idx != 0)
    (*linebuf)[idx] = 0;
  else if ( ch == EOF )
    return -1;
  return idx;
}

ssize_t getline(char **linebuf, size_t *n, FILE *file)
{
  return (getdelim (linebuf, n, '\n', file));
}

#endif /* HAVE_GETLINE */
