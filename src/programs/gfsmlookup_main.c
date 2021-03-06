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

#include "gfsmlookup_cmdparser.h"

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *progname = "gfsmlookup";

//-- options
struct gengetopt_args_info args;

//-- files
const char *fstfilename = "-";
const char *outfilename = "-";

//-- global structs
gfsmAutomaton *fst = NULL;
gfsmError     *err = NULL;

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
  if (args.fst_given) fstfilename = args.fst_arg;
  outfilename = args.output_arg;

  //-- load FST
  fst = gfsm_automaton_new();
  if (!gfsm_automaton_load_bin_filename(fst, fstfilename, &err)) {
    g_printerr("%s: load failed for FST file '%s': %s\n", progname, fstfilename, err->message);
    exit(255);
  }
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
  gfsmStateId      max_states = args.maxq_arg;

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

/*--------------------------------------------------------------------------
 * MAIN
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  gfsmAutomaton *result;

  GFSM_INIT
  get_my_options(argc,argv);

  //-- process input
  result = lookup_labels(fst, args.inputs_num, args.inputs);

  //-- save output
  if (!gfsm_automaton_save_bin_filename(result,outfilename,args.compress_arg,&err)) {
    g_printerr("%s: store failed to '%s': %s\n", progname, outfilename, err->message);
    exit(4);
  }

  //-- cleanup
  if (fst)    gfsm_automaton_free(fst);
  if (result) gfsm_automaton_free(result);

  GFSM_FINISH
  return 0;
}
