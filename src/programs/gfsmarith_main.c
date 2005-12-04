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
#include <stdlib.h>
#include <string.h>

#include <gfsm.h>

#include "gfsmarith_cmdparser.h"

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *progname = "gfsmarith";

//-- options
struct gengetopt_args_info args;

//-- files
const char *infilename = "-";
const char *outfilename = "-";

//-- global structs
gfsmAutomaton *fsm;
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
  if (args.inputs_num > 0) infilename = args.inputs[0];
  outfilename = args.output_arg;

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

  //-- perform final-weight aritmetic
  if (args.multiply_given) {
    gfsm_automaton_final_weight_times(fsm,args.multiply_arg);
  }
  else if (args.add_given) {
    gfsm_automaton_final_weight_plus(fsm,args.add_arg);
  }

  //-- store automaton
  if (!gfsm_automaton_save_bin_filename(fsm,outfilename,&err)) {
    g_printerr("%s: store failed to '%s': %s\n", progname, outfilename, err->message);
    exit(4);
  }

  //-- cleanup
  gfsm_automaton_free(fsm);

  GFSM_FINISH
  return 0;
}
