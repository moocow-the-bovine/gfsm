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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include <gfsm.h>

#include "gfsmtrain_cmdparser.h"

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *progname = "gfsmtrain";

//-- options
struct gengetopt_args_info args;

//-- files
const char *fstfilename = "-";
const char *outfilename = "-";
const char *input_default = "-";

//-- global structs and options
gfsmTrainer *trainer=NULL;
gfsmAutomaton *fst  =NULL;
gfsmAlphabet  *ilabels=NULL, *olabels=NULL;
gfsmError     *err = NULL;

gboolean       att_mode = FALSE;
gboolean       warn_on_undef = TRUE;


/*--------------------------------------------------------------------------
 * Option Processing
 *--------------------------------------------------------------------------*/
void get_my_options(int argc, char **argv)
{
  if (cmdline_parser(argc, argv, &args) != 0)
    exit(1);

  //-- default input: stdin
  if (args.inputs_num < 1) {
    args.inputs_num = 1;
    args.inputs     = (char**)&input_default;
  }

  //-- load environmental defaults
  //cmdline_parser_envdefaults(&args);

  //-- filenames
  if (args.fst_given) fstfilename = args.fst_arg;
  if (args.output_given)  outfilename = args.output_arg;

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

  //-- load FST
  fst = gfsm_automaton_new();
  if (!gfsm_automaton_load_bin_filename(fst, fstfilename, &err)) {
    g_printerr("%s: load failed for FST file '%s': %s\n", progname, fstfilename, err->message);
    exit(255);
  }

  //-- setup trainer
  trainer = gfsm_trainer_new(fst);
  trainer->bestPathsOnly         =  args.best_flag;
  trainer->prunePathPermutations = !args.ordered_flag;
  trainer->distributeOverPaths   =  args.distribute_by_path_flag;
  trainer->distributeOverArcs    =  args.distribute_by_arc_flag;

  //-- mode flags
  att_mode = args.att_mode_flag;
  warn_on_undef = !args.quiet_flag;
}

/*--------------------------------------------------------------------------
 * guts
 */
void train_pairfile(FILE *pairfile)
{
  char            *str = NULL, *istr,*ostr;
  size_t           buflen = 0;
  ssize_t          linelen = 0;
  gfsmLabelVector  *ivec = g_ptr_array_new();
  gfsmLabelVector  *ovec = g_ptr_array_new();

  while (!feof(pairfile)) {
    linelen = getdelim(&str,&buflen,'\n',pairfile);
    if (linelen<0) { break; } //-- EOF

    //-- truncate terminating newline character(s)
    while (str[linelen-1] == '\n' || str[linelen-1] == '\r') {
      str[linelen-1]=0;
      --linelen;
    }

    //-- skip comments and blank lines
    if (linelen==0 || (linelen>=2 && str[0]=='%' && str[1]=='%')) continue;

    //-- separate pairs
    istr = str;
    ostr = strchr(str, '\t');
    while (ostr && *ostr=='\t') {
      *ostr = '\0';
      ++ostr;
    }

    //-- convert to labels
    ivec = gfsm_alphabet_generic_string_to_labels(ilabels, istr, ivec, warn_on_undef, att_mode);
    ovec = gfsm_alphabet_generic_string_to_labels(olabels, ostr, ovec, warn_on_undef, att_mode);

    //-- training guts
    gfsm_trainer_train(trainer, ivec, ovec);
  }

  //-- cleanup
  if (str)  free(str);
  if (ivec) g_ptr_array_free(ivec,TRUE);
  if (ovec) g_ptr_array_free(ovec,TRUE);

  return;
}


/*--------------------------------------------------------------------------
 * MAIN
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  int i;

  GFSM_INIT
  get_my_options(argc,argv);

  //-- churn pair files
  for (i=0; i < args.inputs_num; i++) {
    const char *pairfile = args.inputs[i];
    FILE       *pairfh   = strcmp(pairfile,"-")==0 ? stdin : fopen(pairfile,"r");
    if (!pairfh) {
      g_printerr("%s: open failed for input file '%s': %s\n", progname, pairfile, strerror(errno));
      exit(255);
    }
    train_pairfile(pairfh);
    if (pairfh != stdin) fclose(pairfh);
  }

  //-- save output
  fst = gfsm_trainer_automaton(trainer, TRUE);
  if (!gfsm_automaton_save_bin_filename(fst, outfilename, args.compress_arg, &err)) {
    g_printerr("%s: store failed to '%s': %s\n", progname, outfilename, err->message);
    exit(4);
  }

  //-- cleanup
  if (trainer) gfsm_trainer_free(trainer,TRUE);
  if (fst)     gfsm_automaton_free(fst);
  if (olabels) gfsm_alphabet_free(olabels);
  if (ilabels) gfsm_alphabet_free(ilabels);

  GFSM_FINISH
  return 0;
}
