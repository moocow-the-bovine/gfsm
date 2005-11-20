/*
   gfsm-utils : finite state automaton utilities
   Copyright (C) 2005 by Bryan Jurish <moocow@ling.uni-potsdam.de>

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
#include <errno.h>
#include <ctype.h>

#include <gfsm.h>

#include "gfsmlabels_cmdparser.h"

/*--------------------------------------------------------------------------
 * Globals
 *--------------------------------------------------------------------------*/
char *progname = "gfsmlabels";

//-- options
struct gengetopt_args_info args;

//-- files
const char *infilename = "-";
const char *outfilename = "-";

FILE  *outfile = NULL;

//-- global structs
gfsmAlphabet  *labels=NULL;
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

  //-- open output file
  if (args.output_given) {
    outfilename = args.output_arg;
    outfile = gfsm_open_filename(outfilename,"w",&err);
    if (!outfile) {
      g_printerr("%s: open failed for output file '%s': %s\n",
		 progname, outfilename, strerror(errno));
    }
    exit(2);
  }
  else {
    outfile = stdout;
  } 

  //-- labels
  if (args.labels_given) {
    labels = gfsm_string_alphabet_new();
    if (!gfsm_alphabet_load_filename(labels, args.labels_arg, &err)) {
      g_printerr("%s: load failed for labels file '%s': %s\n",
		 progname, args.labels_arg, err->message);
      exit(3);
    }
  }
}

/*--------------------------------------------------------------------------
 * apply_labels_file()
 */
void apply_labels_file(gfsmAlphabet *labels, FILE *infile, FILE *outfile)
{
  char            *str = NULL;
  ssize_t          buflen = 0;
  ssize_t          linelen = 0;
  ssize_t          i;
  gfsmLabelVal     lab;
  char             cs[2] = {'\0', '\0'};

  while (!feof(infile)) {
    linelen = getline(&str,&buflen,infile);
    for (i=0; i < linelen; i++) {
      if (isspace(str[i])) continue;
      cs[0] = str[i];
      lab = gfsm_alphabet_find_label(labels,cs);

      if (lab==gfsmNoLabel) {
	g_printerr("%s: Warning: no label for character '%c' -- skipping.\n",
		   progname, cs[0]);
	continue;
      }

      fprintf(outfile, "%d ", lab);
    }
    fputs("\n", outfile);
  }

  if (str) free(str);
}

/*--------------------------------------------------------------------------
 * MAIN
 *--------------------------------------------------------------------------*/
int main (int argc, char **argv)
{
  int i;

  GFSM_INIT
  get_my_options(argc,argv);

  //-- process input(s)
  if (args.inputs_num==0) {
    apply_labels_file(labels,stdin,outfile);
  }
  for (i=0; i < args.inputs_num; i++) {
    FILE *infile = (strcmp(args.inputs[i],"-")==0 ? stdin : fopen(args.inputs[i], "r"));
    if (!infile) {
      g_printerr("%s: load failed for input file '%s': %s\n", progname, args.inputs[i], strerror(errno));
      exit(255);
    }
    apply_labels_file(labels,infile,outfile);
    if (infile != stdin) fclose(infile);
  }


  //-- cleanup
  if (labels) gfsm_alphabet_free(labels);

  GFSM_FINISH
  return 0;
}
