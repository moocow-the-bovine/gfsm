/*
   gfsm-utils : finite state automaton utilities
   Copyright (C) 2005 by Bryan Jurish <moocow.bovine@gmail.com>

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

#include "gfsmreplace_cmdparser.h"

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *progname = "gfsmreplace";

//-- options
struct gengetopt_args_info args;

//-- files
const char *replfilename = "-";
const char *infilename  = "-";
const char *outfilename = "-";

//-- global structs
gfsmAutomaton *repl = NULL, *fsm=NULL;
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
  if (args.inputs_num)         infilename  = args.inputs[0];
  if (args.replacement_given) replfilename = args.replacement_arg;
  outfilename = args.output_arg;

  //-- load replacement automaton
  repl = gfsm_automaton_new();
  if (!gfsm_automaton_load_bin_filename(repl, replfilename, &err)) {
    g_printerr("%s: load failed for FST file '%s': %s\n", progname, replfilename, err->message);
    exit(255);
  }
}

/*--------------------------------------------------------------------------
 * MAIN
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  GFSM_INIT
  get_my_options(argc,argv);

  //-- load input automaton
  fsm = gfsm_automaton_new();
  if (!gfsm_automaton_load_bin_filename(fsm,infilename,&err)) {
    g_printerr("%s: load failed for '%s': %s\n", progname, infilename, err->message);
    exit(255);
  }

  //-- perform replacement
  gfsm_automaton_replace(fsm,
			 (args.lower_given ? args.lower_arg : gfsmNoLabel),
			 (args.upper_given ? args.upper_arg : gfsmNoLabel),
			 repl);

  //-- save output
  if (!gfsm_automaton_save_bin_filename(fsm,outfilename,args.compress_arg,&err)) {
    g_printerr("%s: store failed to '%s': %s\n", progname, outfilename, err->message);
    exit(4);
  }

  //-- cleanup
  if (repl) gfsm_automaton_free(repl);
  if (fsm)  gfsm_automaton_free(fsm);

  GFSM_FINISH
  return 0;
}
