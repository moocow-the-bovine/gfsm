/*
   gfsm-utils : finite state automaton utilities
   Copyright (C) 2004-2014 by Bryan Jurish <moocow.bovine@gmail.com>

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
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <gfsm.h>

#include "gfsmprint_cmdparser.h"

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *progname = "gfsmprint";

//-- options
struct gengetopt_args_info args;

//-- files
const char *infilename = "-";
const char *outfilename = "-";

//-- global structs
gfsmAutomaton *fsm;
gfsmAlphabet  *ilabels=NULL, *olabels=NULL, *slabels=NULL;
gfsmError *err = NULL;

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
  if (args.inputs_num > 0) infilename = args.inputs[0];
  if (args.inputs_num > 1) outfilename = args.inputs[1];

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
    if (!gfsm_alphabet_load_filename(ilabels,args.ilabels_arg,&err)) {
      g_printerr("%s: load failed for input-labels file '%s': %s\n",
		 progname, args.ilabels_arg, err->message);
      exit(2);
    }
  }
  //-- labels: output
  if (args.olabels_given) {
    olabels = gfsm_string_alphabet_new();
    if (!gfsm_alphabet_load_filename(olabels,args.olabels_arg,&err)) {
      g_printerr("%s: load failed for output-labels file '%s': %s\n",
		 progname, args.olabels_arg, err->message);
      exit(2);
    }
  }
  //-- labels: state
  if (args.slabels_given) {
    slabels = gfsm_string_alphabet_new();
    if (!gfsm_alphabet_load_filename(slabels,args.slabels_arg,&err)) {
      g_printerr("%s: load failed for state-labels file '%s': %s\n",
		 progname, args.slabels_arg, err->message);
      exit(2);
    }
  }

  //-- initialize fsm
  fsm = gfsm_automaton_new();
}

/*--------------------------------------------------------------------------
 * MAIN
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  GFSM_INIT
  get_my_options(argc,argv);

  //-- load automaton
  if (!gfsm_automaton_load_bin_filename(fsm,infilename,&err)) {
    g_printerr("%s: load failed for '%s': %s\n", progname, infilename, err->message);
    exit(3);
  }

  //-- print automaton
  if (!gfsm_automaton_print_filename_full(fsm,outfilename,ilabels,olabels,slabels,args.compress_arg,&err)) {
    g_printerr("%s: print failed to '%s': %s\n", progname, outfilename, err->message);
    exit(3);
  }

  //-- cleanup
  if (ilabels) gfsm_alphabet_free(ilabels);
  if (olabels) gfsm_alphabet_free(olabels);
  if (slabels) gfsm_alphabet_free(slabels);
  gfsm_automaton_free(fsm);

  GFSM_FINISH
  return 0;
}
