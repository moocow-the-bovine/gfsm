/*
   gfsm-utils : finite state automaton utilities
   Copyright (C) 2004 by Bryan Jurish <moocow@ling.uni-potsdam.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
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

#include "gfsmconcat_cmdparser.h"

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *progname = "gfsmconcat";

//-- options
struct gengetopt_args_info args;

//-- files
const char *infilename;
const char *outfilename = "-";

//-- global structs etc.
gfsmError *err = NULL;
gfsmAutomaton *fsmConcat=NULL, *fsmIn=NULL;

/*--------------------------------------------------------------------------
 * Option Processing
 *--------------------------------------------------------------------------*/
void get_my_options(int argc, char **argv)
{
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(1);

  //-- require at least one file argument
  if (args.inputs_num < 1) {
    cmdline_parser_print_help();
    exit(2);
  }

  //-- output
  if (args.output_arg) outfilename = args.output_arg;

  //-- load environmental defaults
  //cmdline_parser_envdefaults(&args);

  //-- initialize automaton
  fsmIn = gfsm_automaton_new();
}

/*--------------------------------------------------------------------------
 * compute_concat()
 *  + utility routine
 */
void compute_concat(const char *infilename)
{
  //-- load automaton
  if (!gfsm_automaton_load_bin_filename(fsmIn,infilename,&err)) {
    g_printerr("%s: load failed for '%s': %s\n", progname, infilename, err->message);
    exit(255);
  }

  //-- compute concat
  if (fsmConcat == NULL) {
    fsmConcat = gfsm_automaton_clone(fsmIn);
  } else {
    gfsm_automaton_concat(fsmConcat,fsmIn);
  }
}

/*--------------------------------------------------------------------------
 * MAIN
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  int i;
  get_my_options(argc,argv);

  for (i = 0; i < args.inputs_num; i++) {
    compute_concat(args.inputs[i]);
  }
  if (args.inputs_num == 1) compute_concat("-");

  //-- spew automaton
  if (!gfsm_automaton_save_bin_filename(fsmConcat,outfilename,&err)) {
    g_printerr("%s: store failed to '%s': %s\n", progname, outfilename, err->message);
    exit(4);
  }

  //-- cleanup
  if (fsmIn) gfsm_automaton_free(fsmIn);
  if (fsmConcat) gfsm_automaton_free(fsmConcat);

  return 0;
}
