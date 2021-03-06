/* -*- Mode: C -*-
 *
 * File: gfsmdraw_cmdparser.c
 * Description: Code for command-line parser struct gengetopt_args_info.
 *
 * File autogenerated by optgen.perl version 0.07
 * generated with the following command:
 * /usr/local/bin/optgen.perl -u -l --no-handle-rcfile --nopod -F gfsmdraw_cmdparser gfsmdraw.gog
 *
 * The developers of optgen.perl consider the fixed text that goes in all
 * optgen.perl output files to be in the public domain:
 * we make no copyright claims on it.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

/* If we use autoconf/autoheader.  */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_PWD_H
# include <pwd.h>
#endif

/* Allow user-overrides for PACKAGE and VERSION */
#ifndef PACKAGE
#  define PACKAGE "PACKAGE"
#endif

#ifndef VERSION
#  define VERSION "VERSION"
#endif


#ifndef PROGRAM
# define PROGRAM "gfsmdraw"
#endif

/* #define cmdline_parser_DEBUG */

/* Check for "configure's" getopt check result.  */
#ifndef HAVE_GETOPT_LONG
# include "getopt.h"
#else
# include <getopt.h>
#endif

#include "gfsmdraw_cmdparser.h"


/* user code section */

/* end user  code section */


void
cmdline_parser_print_version (void)
{
  printf("gfsmdraw (%s %s) by Bryan Jurish <moocow.bovine@gmail.com>\n", PACKAGE, VERSION);
}

void
cmdline_parser_print_help (void)
{
  cmdline_parser_print_version ();
  printf("\n");
  printf("Purpose:\n");
  printf("  Generate graph specifications from binary gfsm files\n");
  printf("\n");
  
  printf("Usage: %s [OPTIONS]... BINFILE\n", "gfsmdraw");
  
  printf("\n");
  printf(" Arguments:\n");
  printf("   BINFILE  Stored binary gfsm file\n");
  
  printf("\n");
  printf(" Options:\n");
  printf("   -h         --help               Print help and exit.\n");
  printf("   -V         --version            Print version and exit.\n");
  printf("\n");
  printf(" Common Options:\n");
  printf("   -iLABELS   --ilabels=LABELS     Specify input (lower) labels file.\n");
  printf("   -oLABELS   --olabels=LABELS     Specify output (upper) labels file.\n");
  printf("   -lLABELS   --labels=LABELS      Set -i and -o labels simultaneously.\n");
  printf("   -sLABELS   --slabels=LABELS     Specify state labels file.\n");
  printf("   -tTITLE    --title=TITLE        Specify output title.\n");
  printf("   -v         --vertical           Draw vertically.\n");
  printf("   -FDOTFILE  --output=DOTFILE     Output graph specification.\n");
  printf("\n");
  printf(" dot Options:\n");
  printf("   -d         --dot                Request dot output (default)\n");
  printf("   -WFLOAT    --width=FLOAT        Specify output width (default=bbox).\n");
  printf("   -HFLOAT    --height=FLOAT       Specify output height (default=bbox).\n");
  printf("   -fPOINTS   --fontsize=POINTS    Set output font size.\n");
  printf("              --font=FONT          Set output font name.\n");
  printf("   -nFLOAT    --nodesep=FLOAT      Set min node separation.\n");
  printf("   -rFLOAT    --ranksep=FLOAT      Set min rank separation.\n");
  printf("   -p         --portrait           Generate in portrait mode.\n");
  printf("\n");
  printf(" VCG Options:\n");
  printf("   -g         --vcg                Request VCG output.\n");
  printf("   -xINT      --xspace=INT         Set xspace graph parameter.\n");
  printf("   -yINT      --yspace=INT         Set yspace graph parameter.\n");
  printf("              --state-shape=SHAPE  Set shape for state nodes.\n");
  printf("              --state-color=COLOR  Set default state color.\n");
  printf("              --final-color=COLOR  Set color for final states.\n");
}

#if defined(HAVE_STRDUP) || defined(strdup)
# define gog_strdup strdup
#else
/* gog_strdup(): automatically generated from strdup.c. */
/* strdup.c replacement of strdup, which is not standard */
static char *
gog_strdup (const char *s)
{
  char *result = (char*)malloc(strlen(s) + 1);
  if (result == (char*)0)
    return (char*)0;
  strcpy(result, s);
  return result;
}
#endif /* HAVE_STRDUP */

/* clear_args(args_info): clears all args & resets to defaults */
static void
clear_args(struct gengetopt_args_info *args_info)
{
  args_info->ilabels_arg = NULL; 
  args_info->olabels_arg = NULL; 
  args_info->labels_arg = NULL; 
  args_info->slabels_arg = NULL; 
  args_info->title_arg = NULL; 
  args_info->vertical_flag = 0; 
  args_info->output_arg = NULL; 
  args_info->dot_flag = 0; 
  args_info->width_arg = 8.5; 
  args_info->height_arg = 11; 
  args_info->fontsize_arg = 14; 
  args_info->font_arg = NULL; 
  args_info->nodesep_arg = 0.25; 
  args_info->ranksep_arg = 0.40; 
  args_info->portrait_flag = 0; 
  args_info->vcg_flag = 0; 
  args_info->xspace_arg = 40; 
  args_info->yspace_arg = 20; 
  args_info->state_shape_arg = gog_strdup("box"); 
  args_info->state_color_arg = gog_strdup("white"); 
  args_info->final_color_arg = gog_strdup("lightgrey"); 
}


int
cmdline_parser (int argc, char * const *argv, struct gengetopt_args_info *args_info)
{
  int c;	/* Character of the parsed option.  */
  int missing_required_options = 0;	

  args_info->help_given = 0;
  args_info->version_given = 0;
  args_info->ilabels_given = 0;
  args_info->olabels_given = 0;
  args_info->labels_given = 0;
  args_info->slabels_given = 0;
  args_info->title_given = 0;
  args_info->vertical_given = 0;
  args_info->output_given = 0;
  args_info->dot_given = 0;
  args_info->width_given = 0;
  args_info->height_given = 0;
  args_info->fontsize_given = 0;
  args_info->font_given = 0;
  args_info->nodesep_given = 0;
  args_info->ranksep_given = 0;
  args_info->portrait_given = 0;
  args_info->vcg_given = 0;
  args_info->xspace_given = 0;
  args_info->yspace_given = 0;
  args_info->state_shape_given = 0;
  args_info->state_color_given = 0;
  args_info->final_color_given = 0;

  clear_args(args_info);

  /* rcfile handling */
  
  /* end rcfile handling */

  optarg = 0;
  optind = 1;
  opterr = 1;
  optopt = '?';

  while (1)
    {
      int option_index = 0;
      static struct option long_options[] = {
	{ "help", 0, NULL, 'h' },
	{ "version", 0, NULL, 'V' },
	{ "ilabels", 1, NULL, 'i' },
	{ "olabels", 1, NULL, 'o' },
	{ "labels", 1, NULL, 'l' },
	{ "slabels", 1, NULL, 's' },
	{ "title", 1, NULL, 't' },
	{ "vertical", 0, NULL, 'v' },
	{ "output", 1, NULL, 'F' },
	{ "dot", 0, NULL, 'd' },
	{ "width", 1, NULL, 'W' },
	{ "height", 1, NULL, 'H' },
	{ "fontsize", 1, NULL, 'f' },
	{ "font", 1, NULL, 0 },
	{ "nodesep", 1, NULL, 'n' },
	{ "ranksep", 1, NULL, 'r' },
	{ "portrait", 0, NULL, 'p' },
	{ "vcg", 0, NULL, 'g' },
	{ "xspace", 1, NULL, 'x' },
	{ "yspace", 1, NULL, 'y' },
	{ "state-shape", 1, NULL, 0 },
	{ "state-color", 1, NULL, 0 },
	{ "final-color", 1, NULL, 0 },
        { NULL,	0, NULL, 0 }
      };
      static char short_options[] = {
	'h',
	'V',
	'i', ':',
	'o', ':',
	'l', ':',
	's', ':',
	't', ':',
	'v',
	'F', ':',
	'd',
	'W', ':',
	'H', ':',
	'f', ':',
	'n', ':',
	'r', ':',
	'p',
	'g',
	'x', ':',
	'y', ':',
	'\0'
      };

      c = getopt_long (argc, argv, short_options, long_options, &option_index);

      if (c == -1) break;	/* Exit from 'while (1)' loop.  */

      if (cmdline_parser_parse_option(c, long_options[option_index].name, optarg, args_info) != 0) {
	exit (EXIT_FAILURE);
      }
    } /* while */

  

  if ( missing_required_options )
    exit (EXIT_FAILURE);

  
  if (optind < argc) {
      int i = 0 ;
      args_info->inputs_num = argc - optind ;
      args_info->inputs = (char **)(malloc ((args_info->inputs_num)*sizeof(char *))) ;
      while (optind < argc)
        args_info->inputs[ i++ ] = gog_strdup (argv[optind++]) ; 
  }

  return 0;
}


/* Parse a single option */
int
cmdline_parser_parse_option(char oshort, const char *olong, const char *val,
			       struct gengetopt_args_info *args_info)
{
  if (!oshort && !(olong && *olong)) return 1;  /* ignore null options */

#ifdef cmdline_parser_DEBUG
  fprintf(stderr, "parse_option(): oshort='%c', olong='%s', val='%s'\n", oshort, olong, val);*/
#endif

  switch (oshort)
    {
      case 'h':	 /* Print help and exit. */
          if (args_info->help_given) {
            fprintf(stderr, "%s: `--help' (`-h') option given more than once\n", PROGRAM);
          }
          clear_args(args_info);
          cmdline_parser_print_help();
          exit(EXIT_SUCCESS);
        
          break;
        
        case 'V':	 /* Print version and exit. */
          if (args_info->version_given) {
            fprintf(stderr, "%s: `--version' (`-V') option given more than once\n", PROGRAM);
          }
          clear_args(args_info);
          cmdline_parser_print_version();
          exit(EXIT_SUCCESS);
        
          break;
        
        case 'i':	 /* Specify input (lower) labels file. */
          if (args_info->ilabels_given) {
            fprintf(stderr, "%s: `--ilabels' (`-i') option given more than once\n", PROGRAM);
          }
          args_info->ilabels_given++;
          if (args_info->ilabels_arg) free(args_info->ilabels_arg);
          args_info->ilabels_arg = gog_strdup(val);
          break;
        
        case 'o':	 /* Specify output (upper) labels file. */
          if (args_info->olabels_given) {
            fprintf(stderr, "%s: `--olabels' (`-o') option given more than once\n", PROGRAM);
          }
          args_info->olabels_given++;
          if (args_info->olabels_arg) free(args_info->olabels_arg);
          args_info->olabels_arg = gog_strdup(val);
          break;
        
        case 'l':	 /* Set -i and -o labels simultaneously. */
          if (args_info->labels_given) {
            fprintf(stderr, "%s: `--labels' (`-l') option given more than once\n", PROGRAM);
          }
          args_info->labels_given++;
          if (args_info->labels_arg) free(args_info->labels_arg);
          args_info->labels_arg = gog_strdup(val);
          break;
        
        case 's':	 /* Specify state labels file. */
          if (args_info->slabels_given) {
            fprintf(stderr, "%s: `--slabels' (`-s') option given more than once\n", PROGRAM);
          }
          args_info->slabels_given++;
          if (args_info->slabels_arg) free(args_info->slabels_arg);
          args_info->slabels_arg = gog_strdup(val);
          break;
        
        case 't':	 /* Specify output title. */
          if (args_info->title_given) {
            fprintf(stderr, "%s: `--title' (`-t') option given more than once\n", PROGRAM);
          }
          args_info->title_given++;
          if (args_info->title_arg) free(args_info->title_arg);
          args_info->title_arg = gog_strdup(val);
          break;
        
        case 'v':	 /* Draw vertically. */
          if (args_info->vertical_given) {
            fprintf(stderr, "%s: `--vertical' (`-v') option given more than once\n", PROGRAM);
          }
          args_info->vertical_given++;
         if (args_info->vertical_given <= 1)
           args_info->vertical_flag = !(args_info->vertical_flag);
          break;
        
        case 'F':	 /* Output graph specification. */
          if (args_info->output_given) {
            fprintf(stderr, "%s: `--output' (`-F') option given more than once\n", PROGRAM);
          }
          args_info->output_given++;
          if (args_info->output_arg) free(args_info->output_arg);
          args_info->output_arg = gog_strdup(val);
          break;
        
        case 'd':	 /* Request dot output (default) */
          if (args_info->dot_given) {
            fprintf(stderr, "%s: `--dot' (`-d') option given more than once\n", PROGRAM);
          }
          args_info->dot_given++;
         if (args_info->dot_given <= 1)
           args_info->dot_flag = !(args_info->dot_flag);
          break;
        
        case 'W':	 /* Specify output width (default=bbox). */
          if (args_info->width_given) {
            fprintf(stderr, "%s: `--width' (`-W') option given more than once\n", PROGRAM);
          }
          args_info->width_given++;
          args_info->width_arg = (float)strtod(val, NULL);
          break;
        
        case 'H':	 /* Specify output height (default=bbox). */
          if (args_info->height_given) {
            fprintf(stderr, "%s: `--height' (`-H') option given more than once\n", PROGRAM);
          }
          args_info->height_given++;
          args_info->height_arg = (float)strtod(val, NULL);
          break;
        
        case 'f':	 /* Set output font size. */
          if (args_info->fontsize_given) {
            fprintf(stderr, "%s: `--fontsize' (`-f') option given more than once\n", PROGRAM);
          }
          args_info->fontsize_given++;
          args_info->fontsize_arg = (int)atoi(val);
          break;
        
        case 'n':	 /* Set min node separation. */
          if (args_info->nodesep_given) {
            fprintf(stderr, "%s: `--nodesep' (`-n') option given more than once\n", PROGRAM);
          }
          args_info->nodesep_given++;
          args_info->nodesep_arg = (float)strtod(val, NULL);
          break;
        
        case 'r':	 /* Set min rank separation. */
          if (args_info->ranksep_given) {
            fprintf(stderr, "%s: `--ranksep' (`-r') option given more than once\n", PROGRAM);
          }
          args_info->ranksep_given++;
          args_info->ranksep_arg = (float)strtod(val, NULL);
          break;
        
        case 'p':	 /* Generate in portrait mode. */
          if (args_info->portrait_given) {
            fprintf(stderr, "%s: `--portrait' (`-p') option given more than once\n", PROGRAM);
          }
          args_info->portrait_given++;
         if (args_info->portrait_given <= 1)
           args_info->portrait_flag = !(args_info->portrait_flag);
          break;
        
        case 'g':	 /* Request VCG output. */
          if (args_info->vcg_given) {
            fprintf(stderr, "%s: `--vcg' (`-g') option given more than once\n", PROGRAM);
          }
          args_info->vcg_given++;
         if (args_info->vcg_given <= 1)
           args_info->vcg_flag = !(args_info->vcg_flag);
          break;
        
        case 'x':	 /* Set xspace graph parameter. */
          if (args_info->xspace_given) {
            fprintf(stderr, "%s: `--xspace' (`-x') option given more than once\n", PROGRAM);
          }
          args_info->xspace_given++;
          args_info->xspace_arg = (int)atoi(val);
          break;
        
        case 'y':	 /* Set yspace graph parameter. */
          if (args_info->yspace_given) {
            fprintf(stderr, "%s: `--yspace' (`-y') option given more than once\n", PROGRAM);
          }
          args_info->yspace_given++;
          args_info->yspace_arg = (int)atoi(val);
          break;
        
        case 0:	 /* Long option(s) with no short form */
        /* Print help and exit. */
          if (strcmp(olong, "help") == 0) {
            if (args_info->help_given) {
              fprintf(stderr, "%s: `--help' (`-h') option given more than once\n", PROGRAM);
            }
            clear_args(args_info);
            cmdline_parser_print_help();
            exit(EXIT_SUCCESS);
          
          }
          
          /* Print version and exit. */
          else if (strcmp(olong, "version") == 0) {
            if (args_info->version_given) {
              fprintf(stderr, "%s: `--version' (`-V') option given more than once\n", PROGRAM);
            }
            clear_args(args_info);
            cmdline_parser_print_version();
            exit(EXIT_SUCCESS);
          
          }
          
          /* Specify input (lower) labels file. */
          else if (strcmp(olong, "ilabels") == 0) {
            if (args_info->ilabels_given) {
              fprintf(stderr, "%s: `--ilabels' (`-i') option given more than once\n", PROGRAM);
            }
            args_info->ilabels_given++;
            if (args_info->ilabels_arg) free(args_info->ilabels_arg);
            args_info->ilabels_arg = gog_strdup(val);
          }
          
          /* Specify output (upper) labels file. */
          else if (strcmp(olong, "olabels") == 0) {
            if (args_info->olabels_given) {
              fprintf(stderr, "%s: `--olabels' (`-o') option given more than once\n", PROGRAM);
            }
            args_info->olabels_given++;
            if (args_info->olabels_arg) free(args_info->olabels_arg);
            args_info->olabels_arg = gog_strdup(val);
          }
          
          /* Set -i and -o labels simultaneously. */
          else if (strcmp(olong, "labels") == 0) {
            if (args_info->labels_given) {
              fprintf(stderr, "%s: `--labels' (`-l') option given more than once\n", PROGRAM);
            }
            args_info->labels_given++;
            if (args_info->labels_arg) free(args_info->labels_arg);
            args_info->labels_arg = gog_strdup(val);
          }
          
          /* Specify state labels file. */
          else if (strcmp(olong, "slabels") == 0) {
            if (args_info->slabels_given) {
              fprintf(stderr, "%s: `--slabels' (`-s') option given more than once\n", PROGRAM);
            }
            args_info->slabels_given++;
            if (args_info->slabels_arg) free(args_info->slabels_arg);
            args_info->slabels_arg = gog_strdup(val);
          }
          
          /* Specify output title. */
          else if (strcmp(olong, "title") == 0) {
            if (args_info->title_given) {
              fprintf(stderr, "%s: `--title' (`-t') option given more than once\n", PROGRAM);
            }
            args_info->title_given++;
            if (args_info->title_arg) free(args_info->title_arg);
            args_info->title_arg = gog_strdup(val);
          }
          
          /* Draw vertically. */
          else if (strcmp(olong, "vertical") == 0) {
            if (args_info->vertical_given) {
              fprintf(stderr, "%s: `--vertical' (`-v') option given more than once\n", PROGRAM);
            }
            args_info->vertical_given++;
           if (args_info->vertical_given <= 1)
             args_info->vertical_flag = !(args_info->vertical_flag);
          }
          
          /* Output graph specification. */
          else if (strcmp(olong, "output") == 0) {
            if (args_info->output_given) {
              fprintf(stderr, "%s: `--output' (`-F') option given more than once\n", PROGRAM);
            }
            args_info->output_given++;
            if (args_info->output_arg) free(args_info->output_arg);
            args_info->output_arg = gog_strdup(val);
          }
          
          /* Request dot output (default) */
          else if (strcmp(olong, "dot") == 0) {
            if (args_info->dot_given) {
              fprintf(stderr, "%s: `--dot' (`-d') option given more than once\n", PROGRAM);
            }
            args_info->dot_given++;
           if (args_info->dot_given <= 1)
             args_info->dot_flag = !(args_info->dot_flag);
          }
          
          /* Specify output width (default=bbox). */
          else if (strcmp(olong, "width") == 0) {
            if (args_info->width_given) {
              fprintf(stderr, "%s: `--width' (`-W') option given more than once\n", PROGRAM);
            }
            args_info->width_given++;
            args_info->width_arg = (float)strtod(val, NULL);
          }
          
          /* Specify output height (default=bbox). */
          else if (strcmp(olong, "height") == 0) {
            if (args_info->height_given) {
              fprintf(stderr, "%s: `--height' (`-H') option given more than once\n", PROGRAM);
            }
            args_info->height_given++;
            args_info->height_arg = (float)strtod(val, NULL);
          }
          
          /* Set output font size. */
          else if (strcmp(olong, "fontsize") == 0) {
            if (args_info->fontsize_given) {
              fprintf(stderr, "%s: `--fontsize' (`-f') option given more than once\n", PROGRAM);
            }
            args_info->fontsize_given++;
            args_info->fontsize_arg = (int)atoi(val);
          }
          
          /* Set output font name. */
          else if (strcmp(olong, "font") == 0) {
            if (args_info->font_given) {
              fprintf(stderr, "%s: `--font' option given more than once\n", PROGRAM);
            }
            args_info->font_given++;
            if (args_info->font_arg) free(args_info->font_arg);
            args_info->font_arg = gog_strdup(val);
          }
          
          /* Set min node separation. */
          else if (strcmp(olong, "nodesep") == 0) {
            if (args_info->nodesep_given) {
              fprintf(stderr, "%s: `--nodesep' (`-n') option given more than once\n", PROGRAM);
            }
            args_info->nodesep_given++;
            args_info->nodesep_arg = (float)strtod(val, NULL);
          }
          
          /* Set min rank separation. */
          else if (strcmp(olong, "ranksep") == 0) {
            if (args_info->ranksep_given) {
              fprintf(stderr, "%s: `--ranksep' (`-r') option given more than once\n", PROGRAM);
            }
            args_info->ranksep_given++;
            args_info->ranksep_arg = (float)strtod(val, NULL);
          }
          
          /* Generate in portrait mode. */
          else if (strcmp(olong, "portrait") == 0) {
            if (args_info->portrait_given) {
              fprintf(stderr, "%s: `--portrait' (`-p') option given more than once\n", PROGRAM);
            }
            args_info->portrait_given++;
           if (args_info->portrait_given <= 1)
             args_info->portrait_flag = !(args_info->portrait_flag);
          }
          
          /* Request VCG output. */
          else if (strcmp(olong, "vcg") == 0) {
            if (args_info->vcg_given) {
              fprintf(stderr, "%s: `--vcg' (`-g') option given more than once\n", PROGRAM);
            }
            args_info->vcg_given++;
           if (args_info->vcg_given <= 1)
             args_info->vcg_flag = !(args_info->vcg_flag);
          }
          
          /* Set xspace graph parameter. */
          else if (strcmp(olong, "xspace") == 0) {
            if (args_info->xspace_given) {
              fprintf(stderr, "%s: `--xspace' (`-x') option given more than once\n", PROGRAM);
            }
            args_info->xspace_given++;
            args_info->xspace_arg = (int)atoi(val);
          }
          
          /* Set yspace graph parameter. */
          else if (strcmp(olong, "yspace") == 0) {
            if (args_info->yspace_given) {
              fprintf(stderr, "%s: `--yspace' (`-y') option given more than once\n", PROGRAM);
            }
            args_info->yspace_given++;
            args_info->yspace_arg = (int)atoi(val);
          }
          
          /* Set shape for state nodes. */
          else if (strcmp(olong, "state-shape") == 0) {
            if (args_info->state_shape_given) {
              fprintf(stderr, "%s: `--state-shape' option given more than once\n", PROGRAM);
            }
            args_info->state_shape_given++;
            if (args_info->state_shape_arg) free(args_info->state_shape_arg);
            args_info->state_shape_arg = gog_strdup(val);
          }
          
          /* Set default state color. */
          else if (strcmp(olong, "state-color") == 0) {
            if (args_info->state_color_given) {
              fprintf(stderr, "%s: `--state-color' option given more than once\n", PROGRAM);
            }
            args_info->state_color_given++;
            if (args_info->state_color_arg) free(args_info->state_color_arg);
            args_info->state_color_arg = gog_strdup(val);
          }
          
          /* Set color for final states. */
          else if (strcmp(olong, "final-color") == 0) {
            if (args_info->final_color_given) {
              fprintf(stderr, "%s: `--final-color' option given more than once\n", PROGRAM);
            }
            args_info->final_color_given++;
            if (args_info->final_color_arg) free(args_info->final_color_arg);
            args_info->final_color_arg = gog_strdup(val);
          }
          
          else {
            fprintf(stderr, "%s: unknown long option '%s'.\n", PROGRAM, olong);
            return (EXIT_FAILURE);
          }
          break;

        case '?':	 /* Invalid Option */
          fprintf(stderr, "%s: unknown option '%s'.\n", PROGRAM, olong);
          return (EXIT_FAILURE);


        default:	/* bug: options not considered.  */
          fprintf (stderr, "%s: option unknown: %c\n", PROGRAM, oshort);
          abort ();
        } /* switch */
  return 0;
}


/* Initialize options not yet given from environmental defaults */
void
cmdline_parser_envdefaults(struct gengetopt_args_info *args_info)
{
  

  return;
}


/* Load option values from an .rc file */
void
cmdline_parser_read_rcfile(const char *filename,
			      struct gengetopt_args_info *args_info,
			      int user_specified)
{
  char *fullname;
  FILE *rcfile;

  if (!filename) return; /* ignore NULL filenames */

#if defined(HAVE_GETUID) && defined(HAVE_GETPWUID)
  if (*filename == '~') {
    /* tilde-expansion hack */
    struct passwd *pwent = getpwuid(getuid());
    if (!pwent) {
      fprintf(stderr, "%s: user-id %d not found!\n", PROGRAM, getuid());
      return;
    }
    if (!pwent->pw_dir) {
      fprintf(stderr, "%s: home directory for user-id %d not found!\n", PROGRAM, getuid());
      return;
    }
    fullname = (char *)malloc(strlen(pwent->pw_dir)+strlen(filename));
    strcpy(fullname, pwent->pw_dir);
    strcat(fullname, filename+1);
  } else {
    fullname = gog_strdup(filename);
  }
#else /* !(defined(HAVE_GETUID) && defined(HAVE_GETPWUID)) */
  fullname = gog_strdup(filename);
#endif /* defined(HAVE_GETUID) && defined(HAVE_GETPWUID) */

  /* try to open */
  rcfile = fopen(fullname,"r");
  if (!rcfile) {
    if (user_specified) {
      fprintf(stderr, "%s: warning: open failed for rc-file '%s': %s\n",
	      PROGRAM, fullname, strerror(errno));
    }
  }
  else {
   cmdline_parser_read_rc_stream(rcfile, fullname, args_info);
  }

  /* cleanup */
  if (fullname != filename) free(fullname);
  if (rcfile) fclose(rcfile);

  return;
}


/* Parse option values from an .rc file : guts */
#define OPTPARSE_GET 32
void
cmdline_parser_read_rc_stream(FILE *rcfile,
				 const char *filename,
				 struct gengetopt_args_info *args_info)
{
  char *optname  = (char *)malloc(OPTPARSE_GET);
  char *optval   = (char *)malloc(OPTPARSE_GET);
  size_t onsize  = OPTPARSE_GET;
  size_t ovsize  = OPTPARSE_GET;
  size_t onlen   = 0;
  size_t ovlen   = 0;
  int    lineno  = 0;
  char c;

#ifdef cmdline_parser_DEBUG
  fprintf(stderr, "cmdline_parser_read_rc_stream('%s'):\n", filename);
#endif

  while ((c = fgetc(rcfile)) != EOF) {
    onlen = 0;
    ovlen = 0;
    lineno++;

    /* -- get next option-name */
    /* skip leading space and comments */
    if (isspace(c)) continue;
    if (c == '#') {
      while ((c = fgetc(rcfile)) != EOF) {
	if (c == '\n') break;
      }
      continue;
    }

    /* parse option-name */
    while (c != EOF && c != '=' && !isspace(c)) {
      /* re-allocate if necessary */
      if (onlen >= onsize-1) {
	char *tmp = (char *)malloc(onsize+OPTPARSE_GET);
	strcpy(tmp,optname);
	free(optname);

	onsize += OPTPARSE_GET;
	optname = tmp;
      }
      optname[onlen++] = c;
      c = fgetc(rcfile);
    }
    optname[onlen++] = '\0';

#ifdef cmdline_parser_DEBUG
    fprintf(stderr, "cmdline_parser_read_rc_stream('%s'): line %d: optname='%s'\n",
	    filename, lineno, optname);
#endif

    /* -- get next option-value */
    /* skip leading space */
    while ((c = fgetc(rcfile)) != EOF && isspace(c)) {
      ;
    }

    /* parse option-value */
    while (c != EOF && c != '\n') {
      /* re-allocate if necessary */
      if (ovlen >= ovsize-1) {
	char *tmp = (char *)malloc(ovsize+OPTPARSE_GET);
	strcpy(tmp,optval);
	free(optval);
	ovsize += OPTPARSE_GET;
	optval = tmp;
      }
      optval[ovlen++] = c;
      c = fgetc(rcfile);
    }
    optval[ovlen++] = '\0';

    /* now do the action for the option */
    if (cmdline_parser_parse_option('\0',optname,optval,args_info) != 0) {
      fprintf(stderr, "%s: error in file '%s' at line %d.\n", PROGRAM, filename, lineno);
      
    }
  }

  /* cleanup */
  free(optname);
  free(optval);

  return;
}
