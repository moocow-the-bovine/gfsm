/*
   gfsm-utils : finite state automaton utilities
   Copyright (C) 2014 by Bryan Jurish <moocow.bovine@gmail.com>

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

#include "gfsmencode_cmdparser.h"

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *progname = "gfsmencode";

//-- options
struct gengetopt_args_info args;

//-- files
const char *infilename  = "-";
const char *keyfilename = NULL;
const char *outfilename = "-";

//-- global structs
gfsmAutomaton   *fsm=NULL, *keyfsm=NULL;
gfsmArcLabelKey *key=NULL;

/*--------------------------------------------------------------------------
 * Option Processing
 *--------------------------------------------------------------------------*/
void get_my_options(int argc, char **argv)
{
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(1);

  //-- mode
  if (args.encode_flag) args.decode_flag = 0;

  //-- output
  if (args.inputs_num>0) infilename  = args.inputs[0];
  if (args.inputs_num>1) keyfilename = args.inputs[1];
  if (args.output_arg) outfilename = args.output_arg;

  //-- load environmental defaults
  //cmdline_parser_envdefaults(&args);

  //-- initialize automaton
  fsm = gfsm_automaton_new();
}


/*--------------------------------------------------------------------------
 * MAIN
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  gfsmError *err = NULL;
  get_my_options(argc,argv);

  //-- load base automaton
  if (!gfsm_automaton_load_bin_filename(fsm,infilename,&err)) {
    g_printerr("%s: load failed for '%s': %s\n", progname, infilename, err->message);
    exit(255);
  }

  //-- load key automaton 
  if (args.decode_flag || (!args.encode_flag && keyfilename!=NULL && g_file_test(keyfilename, G_FILE_TEST_EXISTS))) {
    keyfsm = gfsm_automaton_new();
    if (keyfilename==NULL) keyfilename = "-";
    if (!gfsm_automaton_load_bin_filename(keyfsm, keyfilename, &err)) {
      g_printerr("%s: load failed for key automaton from '%s': %s\n", progname, keyfilename, err->message);
      exit(255);
    }
    key = gfsm_arclabel_fsm_to_key(keyfsm, key);
  }

  if (args.decode_flag) {
    //-- guts: decode
    gfsm_automaton_decode(fsm, key, args.labels_flag, args.costs_flag);
  }
  else {
    //-- guts: encode
    key = gfsm_automaton_encode(fsm, key, args.labels_flag, args.costs_flag);

    //-- encode: dump key
    keyfsm = gfsm_arclabel_key_to_fsm(key, keyfsm);
    if (!gfsm_automaton_save_bin_filename(keyfsm,keyfilename,args.compress_arg,&err)) {
      g_printerr("%s: store failed for key fsm to  '%s': %s\n", progname, keyfilename, err->message);
      exit(4);
    }
  }

  //-- dump recoded automaton
  if (!gfsm_automaton_save_bin_filename(fsm,outfilename,args.compress_arg,&err)) {
    g_printerr("%s: store failed to '%s': %s\n", progname, outfilename, err->message);
    exit(4);
  }

  //-- cleanup
  if (keyfsm) gfsm_automaton_free(keyfsm);
  if (key) gfsm_arclabel_key_free(key);
  if (fsm) gfsm_automaton_free(fsm);

  return 0;
}
