
/*=============================================================================*\
 * File: gfsmAutomatonIO.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: automata: I/O
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

#ifndef _GFSM_AUTOMATON_IO_H
#define _GFSM_AUTOMATON_IO_H

#include <gfsmAlphabet.h>
#include <gfsmAutomaton.h>
#include <gfsmVersion.h>
#include <gfsmError.h>
#include <stdio.h>

/*======================================================================
 * Types
 */
/// Header info for binary files
typedef struct {
  gchar              magic[16];    ///< magic header string "gfsm_automaton"
  gfsmVersionInfo    version;      ///< gfsm version which created the stored file
  gfsmVersionInfo    version_min;  ///< minimum gfsm versionr required to load the file
  gfsmAutomatonFlags flags;        ///< automaton flags
  gfsmStateId        root_id;      ///< Id of root node
  gfsmStateId        n_states;     ///< number of stored states
  gfsmStateId        n_arcs;       ///< number of stored arcs
  gfsmSRType         srtype;       ///< semiring type
  guint              unused2;      ///< reserved
  guint              unused3;      ///< reserved
  guint              unused4;      ///< reserved
} gfsmAutomatonHeader;

/// Type for a stored state
typedef struct {
  gboolean is_valid : 1; ///< valid flag
  gboolean is_final : 1; ///< final flag
  guint    n_arcs;       ///< number of stored arcs
  guint    min_arc;      ///< index of stored minimum arc
} gfsmStoredState;

/// Type for a stored arc
typedef gfsmArc gfsmStoredArc;

/*======================================================================
 * Constants
 */
/** Scanner config for gfsm_automaton_compile() */
//extern const GScannerConfig gfsm_automaton_scanner_config;

/** Magic header string for stored gfsm files */
extern const gchar gfsm_header_magic[16];

/** Minor version required for loading files stored by this version of libgfsm */
extern const gfsmVersionInfo gfsm_version_bincompat_min;

/*======================================================================
 * Methods: Binary I/O
 */
/// \name Automaton Methods: Binary I/O
//@{
/** Load an automaton header from a stored binary file.
 *  Returns TRUE iff the header looks valid. */
gboolean gfsm_automaton_load_header(gfsmAutomatonHeader *hdr, FILE *f, gfsmError **errp);

/** Load an automaton from a stored binary file (implicitly clear()s @fsm) */
gboolean gfsm_automaton_load_bin_file(gfsmAutomaton *fsm, FILE *f, gfsmError **errp);

/** Load an automaton from a named binary file (implicitly clear()s @fsm) */
gboolean gfsm_automaton_load_bin_filename(gfsmAutomaton *fsm, const gchar *filename, gfsmError **errp);

/** Store an automaton from to a binary file */
gboolean gfsm_automaton_save_bin_file(gfsmAutomaton *fsm, FILE *f, gfsmError **errp);

/** Store an automaton to a named binary file */
gboolean gfsm_automaton_save_bin_filename(gfsmAutomaton *fsm, const gchar *filename, gfsmError **errp);
//@}

/*======================================================================
 * Automaton Methods: Text I/O
 */
/// \name Automaton Methods: Text I/O
//@{
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

/** Load an automaton in Ma-Bell-compatible text-format from a named file */
gboolean gfsm_automaton_compile_filename_full (gfsmAutomaton *fsm,
					       const gchar   *filename,
					       gfsmAlphabet  *lo_alphabet,
					       gfsmAlphabet  *hi_alphabet,
					       gfsmAlphabet  *state_alphabet,
					       gfsmError     **errp);

/** Convenience macro for compiling all-numeric-id named text files */
#define gfsm_automaton_compile_filename(fsm,filename,errp) \
   gfsm_automaton_compile_filename_full(fsm,filename,NULL,NULL,NULL,errp)

/*-----------------------*/

/** Print an automaton in Ma-Bell-compatible text-format to a FILE*  */
gboolean gfsm_automaton_print_file_full (gfsmAutomaton *fsm,
					 FILE          *f,
					 gfsmAlphabet  *lo_alphabet,
					 gfsmAlphabet  *hi_alphabet,
					 gfsmAlphabet  *state_alphabet,
					 gfsmError     **errp);

/** Convenience macro for printing to all-numeric-id text streams */
#define gfsm_automaton_print_file(fsm,f,errp) \
  gfsm_automaton_print_file_full(fsm,f,NULL,NULL,NULL,errp)

/** Print an automaton in Ma-Bell-compatible text-format to a named file */
gboolean gfsm_automaton_print_filename_full (gfsmAutomaton *fsm,
					     const gchar   *filename,
					     gfsmAlphabet  *lo_alphabet,
					     gfsmAlphabet  *hi_alphabet,
					     gfsmAlphabet  *state_alphabet,
					     gfsmError     **errp);

/** Convenience macro for printing to all-numeric-id named text files */
#define gfsm_automaton_print_filename(fsm,f,errp) \
  gfsm_automaton_print_filename_full(fsm,f,NULL,NULL,NULL,errp)
//@}

#endif /* _GFSM_AUTOMATON_IO_H */
