
/*=============================================================================*\
 * File: gfsmAutomatonIO.h
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: automata: I/O
 *
 * Copyright (c) 2004-2007 Bryan Jurish.
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

/** \file gfsmAutomatonIO.h
 *  \brief Librarian routines for automata.
 */

#ifndef _GFSM_AUTOMATON_IO_H
#define _GFSM_AUTOMATON_IO_H

#include <gfsmAutomaton.h>
#include <gfsmVersion.h>

/*======================================================================
 * Constants
 */
/* Scanner config for gfsm_automaton_compile() */
//extern const GScannerConfig gfsm_automaton_scanner_config;

/** Magic header string for stored gfsm files */
extern const gchar gfsm_header_magic[16];

/*======================================================================
 * Methods: Binary I/O
 */
/// \name Automaton Methods: Binary I/O
//@{
/** Load an automaton header from a stored binary file.
 *  Returns TRUE iff the header looks valid. */
gboolean gfsm_automaton_load_header(gfsmAutomatonHeader *hdr, gfsmIOHandle *ioh, gfsmError **errp);

/** Load an automaton from a named binary file (implicitly clear()s \a fsm) */
//gboolean gfsm_automaton_load_bin_handle(gfsmAutomaton *fsm, gfsmIOHandle *ioh, gfsmError **errp);

/** Load an automaton from a stored binary file (implicitly clear()s \a fsm) */
gboolean gfsm_automaton_load_bin_file(gfsmAutomaton *fsm, FILE *f, gfsmError **errp);

/** Load an automaton from a named binary file (implicitly clear()s \a fsm) */
gboolean gfsm_automaton_load_bin_filename(gfsmAutomaton *fsm, const gchar *filename, gfsmError **errp);

/** Load an automaton from an in-memory buffer */
gboolean gfsm_automaton_load_bin_gstring(gfsmAutomaton *fsm, GString *gs, gfsmError **errp);


/** Store an automaton in binary form to a gfsmIOHandle* */
//gboolean gfsm_automaton_save_bin_handle(gfsmAutomaton *fsm, gfsmIOHandle *ioh, gfsmError **errp);

/** Store an automaton in binary form to a file */
gboolean gfsm_automaton_save_bin_file(gfsmAutomaton *fsm, FILE *f, gfsmError **errp);

/** Store an automaton to a named binary file (no compression) */
gboolean gfsm_automaton_save_bin_filename_nc(gfsmAutomaton *fsm, const gchar *filename, gfsmError **errp);

/** Store an automaton to a named binary file, possibly compressing.
 *  Set \a zlevel=-1 for default compression, and
 *  set \a zlevel=0  for no compression, otherwise should be as for zlib (1 <= zlevel <= 9)
 */
gboolean gfsm_automaton_save_bin_filename(gfsmAutomaton *fsm, const gchar *filename, int zlevel, gfsmError **errp);

/** Append an uncompressed binary automaton to an in-memory buffer */
gboolean gfsm_automaton_save_bin_gstring(gfsmAutomaton *fsm, GString *gs, gfsmError **errp);

//@}

/*======================================================================
 * Automaton Methods: Text I/O
 */
/// \name Automaton Methods: Text I/O
//@{

/** Load an automaton in Ma-Bell-compatible text-format from a gfsmIOHandle*  */
gboolean gfsm_automaton_compile_handle (gfsmAutomaton *fsm,
					gfsmIOHandle  *ioh,
					gfsmAlphabet  *lo_alphabet,
					gfsmAlphabet  *hi_alphabet,
					gfsmAlphabet  *state_alphabet,
					gfsmError     **errp);


/** Load an automaton in Ma-Bell-compatible text-format from a FILE*  */
gboolean gfsm_automaton_compile_file_full (gfsmAutomaton *fsm,
					   FILE          *f,
					   gfsmAlphabet  *lo_alphabet,
					   gfsmAlphabet  *hi_alphabet,
					   gfsmAlphabet  *state_alphabet,
					   gfsmError     **errp);

/** Convenience macro for compiling all-numeric-id text streams */
#define gfsm_automaton_compile_file(fsm,file,errp) \
   gfsm_automaton_compile_file_full(fsm,f,NULL,NULL,NULL,errp)

/** Load an automaton in Ma-Bell-compatible text-format from a named file, possibly compressed. */
gboolean gfsm_automaton_compile_filename_full (gfsmAutomaton *fsm,
					       const gchar   *filename,
					       gfsmAlphabet  *lo_alphabet,
					       gfsmAlphabet  *hi_alphabet,
					       gfsmAlphabet  *state_alphabet,
					       gfsmError     **errp);

/** Convenience macro for compiling all-numeric-id named text files */
#define gfsm_automaton_compile_filename(fsm,filename,errp) \
   gfsm_automaton_compile_filename_full(fsm,filename,NULL,NULL,NULL,errp)

/** Load an automaton in Ma-Bell-compatible text-format from an in-memory buffer */
gboolean gfsm_automaton_compile_gstring_full (gfsmAutomaton *fsm,
					      GString       *gs,
					      gfsmAlphabet  *lo_alphabet,
					      gfsmAlphabet  *hi_alphabet,
					      gfsmAlphabet  *state_alphabet,
					      gfsmError     **errp);


/*-----------------------*/

/** Print an automaton in Ma-Bell-compatible text-format to a gfsmIOHandle* */
gboolean gfsm_automaton_print_handle (gfsmAutomaton *fsm,
				      gfsmIOHandle  *ioh,
				      gfsmAlphabet  *lo_alphabet,
				      gfsmAlphabet  *hi_alphabet,
				      gfsmAlphabet  *state_alphabet,
				      gfsmError     **errp);


/** Print an automaton in Ma-Bell-compatible text-format to a FILE*  */
gboolean gfsm_automaton_print_file_full (gfsmAutomaton *fsm,
					 FILE          *f,
					 gfsmAlphabet  *lo_alphabet,
					 gfsmAlphabet  *hi_alphabet,
					 gfsmAlphabet  *state_alphabet,
					 int            zlevel,
					 gfsmError     **errp);

/** Convenience macro for printing to uncompresed all-numeric-id text streams  */
#define gfsm_automaton_print_file(fsm,f,errp) \
  gfsm_automaton_print_file_full(fsm,f,NULL,NULL,NULL,0,errp)

/** Print an automaton in Ma-Bell-compatible text-format to a named file */
gboolean gfsm_automaton_print_filename_full (gfsmAutomaton *fsm,
					     const gchar   *filename,
					     gfsmAlphabet  *lo_alphabet,
					     gfsmAlphabet  *hi_alphabet,
					     gfsmAlphabet  *state_alphabet,
					     int            zlevel,
					     gfsmError     **errp);

/** Convenience macro for printing to uncompressed all-numeric-id named text files */
#define gfsm_automaton_print_filename(fsm,f,errp) \
  gfsm_automaton_print_filename_full(fsm,f,NULL,NULL,NULL,0,errp)

/** Print an automaton in Ma-Bell-compatible text-format to an in-memory buffer */
gboolean gfsm_automaton_print_gstring_full (gfsmAutomaton *fsm,
					    GString       *gs,
					    gfsmAlphabet  *lo_alphabet,
					    gfsmAlphabet  *hi_alphabet,
					    gfsmAlphabet  *state_alphabet,
					    gfsmError     **errp);

//@}

#endif /* _GFSM_AUTOMATON_IO_H */
