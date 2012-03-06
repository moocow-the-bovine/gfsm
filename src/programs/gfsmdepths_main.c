/*
   gfsm-utils : finite state automaton utilities
   Copyright (C) 2012 by Bryan Jurish <moocow.bovine@gmail.com>

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

#include "gfsmdepths_cmdparser.h"

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *progname = "gfsminfo";

//-- options
struct gengetopt_args_info args;

//-- files
const char *infilename = "-";

//-- global structs
gfsmAutomaton *fsm;

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

  //-- initialize fsm
  fsm = gfsm_automaton_new();
}

/*--------------------------------------------------------------------------
 * dump
 */
void dump_depths(FILE *f, gfsmAutomaton *fsm, gfsmStateIdMap *depths)
{
  gfsmStateId  qid;
  gfsmStateId *dp = (gfsmStateId*)(depths->data);
  for (qid=0; qid < gfsm_automaton_n_states(fsm); ++qid, ++dp) {
    if (!gfsm_automaton_has_state(fsm,qid)) continue;
    fprintf(f, "%u\t%u\n", qid, *dp);
  }
}

/*--------------------------------------------------------------------------
 * MAIN
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  gfsmError *err = NULL;
  gfsmStateIdMap *depths = NULL;
  GFSM_INIT

  get_my_options(argc,argv);

  //-- load automaton
  if (!gfsm_automaton_load_bin_filename(fsm,infilename,&err)) {
    g_printerr("%s: load failed for '%s': %s\n", progname, infilename, err->message);
    exit(2);
  }

  //-- get depth-map
  depths = gfsm_statemap_depths(fsm,NULL);
  dump_depths(stdout, fsm, depths);

  //-- cleanup
  g_array_free(depths,TRUE);
  gfsm_automaton_free(fsm);

  GFSM_FINISH

  return 0;
}
