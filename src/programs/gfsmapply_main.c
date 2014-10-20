/*
   gfsm-utils : finite state automaton utilities
   Copyright (C) 2005-2014 by Bryan Jurish <moocow.bovine@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include <gfsm.h>

#include "gfsmapply_cmdparser.h"

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *progname = "gfsmapply";

//-- options
struct gengetopt_args_info args;

//-- files
const char *fstfilename = "-";
const char *outfilename = "-";

FILE  *outfile = NULL;

//-- global structs
gfsmAlphabet  *ilabels=NULL, *olabels=NULL, *qlabels=NULL;
gfsmAutomaton *fst = NULL, *result=NULL;
gfsmError     *err = NULL;
gfsmLabelVector *labvec = NULL;

gboolean       att_mode = FALSE;
gboolean       map_mode = FALSE;
gboolean       warn_on_undef = TRUE;
gboolean       align_mode = FALSE;
gfsmStateId    max_states = 0; //-- ~ gfsmNoState
gfsmArcPathToStringOptions p2sopts;

/*--------------------------------------------------------------------------
 * Option Processing
 *--------------------------------------------------------------------------*/
void get_my_options(int argc, char **argv)
{
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(1);

  //-- load environmental defaults
  //cmdline_parser_envdefaults(&args);

  //-- filenames
  if (!args.fst_given && !args.inputs_num) {
    g_printerr("%s: Error: -fst (-f) option required but not specified!", progname);
    exit(2);
  }
  fstfilename = args.fst_arg;
  outfilename = args.output_arg;

  //-- scalar flags
  att_mode = args.att_mode_flag;
  warn_on_undef = !args.quiet_flag;
  align_mode = args.align_flag;
  max_states = args.maxq_arg==0 ? gfsmNoState : args.maxq_arg;

  //-- labels: input + output
  if (args.labels_given) {
    if (!args.ilabels_given) {
      args.ilabels_given = 1;
      args.ilabels_arg   = args.labels_arg;
    }
    if (!args.olabels_given) {
      args.olabels_given = 1;
      args.olabels_arg   = args.labels_arg;
    }
  }
  //-- labels: input
  if (args.ilabels_given) {
    ilabels = gfsm_string_alphabet_new();
    ilabels->utf8 = args.utf8_flag;
    if (!gfsm_alphabet_load_filename(ilabels,args.ilabels_arg,&err)) {
      g_printerr("%s: load failed for input-labels file '%s': %s\n",
		 progname, args.ilabels_arg, (err ? err->message : "?"));
      exit(2);
    }
  }
  //-- labels: output
  if (args.olabels_given) {
    olabels = gfsm_string_alphabet_new();
    olabels->utf8 = args.utf8_flag;
    if (!gfsm_alphabet_load_filename(olabels,args.olabels_arg,&err)) {
      g_printerr("%s: load failed for output-labels file '%s': %s\n",
		 progname, args.olabels_arg, (err ? err->message : "?"));
      exit(2);
    }
  }
#if 0
  //-- labels: states
  if (args.qlabels_given) {
    qlabels = gfsm_string_alphabet_new();
    qlabels->utf8 = args.utf8_flag;
    if (!gfsm_alphabet_load_filename(qlabels,args.qlabels_arg,&err)) {
      g_printerr("%s: load failed for state-labels file '%s': %s\n",
		 progname, args.qlabels_arg, (err ? err->message : "?"));
      exit(2);
    }
  }
#endif

  //-- output file
  if (args.output_given) {
    outfilename = args.output_arg;
    outfile = gfsm_open_filename(outfilename,"w",&err);
    if (!outfile) {
      g_printerr("%s: open failed for output file '%s': %s\n",
		 progname, outfilename, strerror(errno));
      exit(2);
    }
  }
  else {
    outfile = stdout;
  } 

  //-- load FST
  fst = gfsm_automaton_new();
  if (!gfsm_automaton_load_bin_filename(fst, fstfilename, &err)) {
    g_printerr("%s: load failed for FST file '%s': %s\n", progname, fstfilename, err->message);
    exit(255);
  }

  //-- setup p2sopts
  memset(&p2sopts, 0, sizeof(p2sopts));
  p2sopts.abet_q  = qlabels;
  p2sopts.abet_lo = ilabels;
  p2sopts.abet_hi = olabels;
  p2sopts.warn_on_undefined = warn_on_undef;
  p2sopts.att_style   = att_mode;
  p2sopts.compress_id = TRUE;
  p2sopts.show_states = FALSE;
  //p2sopts.q0 = gfsm_automaton_get_root(fsm);
  //p2sopts.sr = gfsm_automaton_get_semiring(fsm);


  //-- setup label-vector
  labvec = g_ptr_array_sized_new(512);
}

/*--------------------------------------------------------------------------
 * lookup_labels()
 */
gfsmAutomaton *lookup_labels(gfsmAutomaton *fst, int argc, char **argv)
{
  gfsmLabelVector *vec = g_ptr_array_sized_new(argc);
  char            *s=NULL, *tail=NULL;
  gfsmLabelVal     lab;
  gfsmAutomaton   *result = NULL;
  int              i;

  //-- fill input vector
  for (i=0; i < argc; i++) {
    for (s=argv[i], lab=strtol(s,&tail,0); s != tail; s=tail, lab=strtol(s,&tail,0)) {
      g_ptr_array_add(vec, (gpointer)lab);
    }
  }

  //-- state limit
  if (max_states==0) max_states = gfsmNoState;

  //-- actual lookup
  result = gfsm_automaton_lookup_full(fst, vec, result, NULL, max_states);

  //-- cleanup
  g_ptr_array_free(vec,TRUE);

  return result;
}


//--------------------------------------------------------------------------
void apply_word(const char *w)
{
  gfsmSet *paths = NULL;
  GSList  *arcpaths = NULL;
  GSList  *strings = NULL;

  //-- ouput prefix
  fputs(w, outfile);
  fputc('\t', outfile);

  //-- lookup guts
  labvec = gfsm_alphabet_generic_string_to_labels(ilabels, w,labvec, warn_on_undef, att_mode);
  result = gfsm_automaton_lookup_full(fst, labvec, result, NULL, max_states);

  //-- stringification
  if (args.align_flag) {
    //-- aligned paths
    p2sopts.q0 = gfsm_automaton_get_root(result);
    p2sopts.sr = gfsm_automaton_get_semiring(result);

    arcpaths = gfsm_automaton_arcpaths(result);
    strings  = gfsm_arcpaths_to_strings(arcpaths, &p2sopts);
  }
  else {
    //-- non-aligned paths; serialize "normal" automaton
    paths   = gfsm_automaton_paths_full(result, NULL, gfsmLSUpper); //gfsmLSBoth
    strings = gfsm_paths_to_strings(paths, NULL,olabels, result->sr,TRUE, att_mode, NULL);
  }

  //-- dump strings
  while (strings) {
    //-- pop first datum
    char *s = (char *)strings->data;
    strings = g_slist_delete_link(strings,strings);

    //-- print string
    fputc('\t', outfile);
    fputs(s, outfile);
    g_free(s);
  }
  fputc('\n', outfile);

  //-- cleanup
  if (paths)    gfsm_set_free(paths);
  if (arcpaths) gfsm_arcpath_list_free(arcpaths);
}

//--------------------------------------------------------------------------
void apply_file(FILE *infile)
{
  char            *buf = NULL;
  size_t           buflen = 0;
  ssize_t          linelen = 0;

  while (!feof(infile)) {
    linelen = getdelim(&buf,&buflen,'\n',infile);
    if (linelen<0) { break; } //-- EOF

    //-- truncate terminating newline character(s)
    while (linelen > 0 && (buf[linelen-1]=='\n' || buf[linelen-1]=='\r')) {
      buf[--linelen]=0;
    }

    apply_word(buf);
  }

  if (buf) free(buf);
}

//--------------------------------------------------------------------------
void apply_words(char **argv, int argc)
{
  int i;
  for (i=0; i < argc; ++i) {
    apply_word(argv[i]);
  }
}

/*--------------------------------------------------------------------------
 * MAIN
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  GFSM_INIT
  get_my_options(argc,argv);

  if (args.words_flag) {
    //-- main: apply to user-specified words
    apply_words(args.inputs, args.inputs_num);
  }
  else if (args.inputs_num) {
    //-- main: apply to user-specified files
    int i;
    for (i=0; i < args.inputs_num; ++i) {
      FILE *infile = (strcmp(args.inputs[i],"-")==0 ? stdin : fopen(args.inputs[i], "r"));
      if (!infile) {
	g_printerr("%s: load failed for input file '%s': %s\n", progname, args.inputs[i], strerror(errno));
	exit(255);
      }
      apply_file(infile);
      if (infile != stdin) fclose(infile);
    }
  }
  else {
    //-- main: apply to stdin
    apply_file(stdin);
  }

  //-- cleanup
  if (outfile != stdout) fclose(outfile);
  if (fst)    gfsm_automaton_free(fst);
  if (result) gfsm_automaton_free(result);
  if (ilabels)  gfsm_alphabet_free(ilabels);
  if (olabels)  gfsm_alphabet_free(olabels);
  if (qlabels)  gfsm_alphabet_free(qlabels);

  GFSM_FINISH
  return 0;
}
