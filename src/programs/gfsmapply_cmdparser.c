/* -*- Mode: C -*-
 *
 * File: gfsmapply_cmdparser.c
 * Description: Code for command-line parser struct gengetopt_args_info.
 *
 * File autogenerated by optgen.perl version 0.07
 * generated with the following command:
 * /usr/local/bin/optgen.perl -u -l --no-handle-rcfile --nopod -F gfsmapply_cmdparser gfsmapply.gog
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
# define PROGRAM "gfsmapply"
#endif

/* #define cmdline_parser_DEBUG */

/* Check for "configure's" getopt check result.  */
#ifndef HAVE_GETOPT_LONG
# include "getopt.h"
#else
# include <getopt.h>
#endif

#include "gfsmapply_cmdparser.h"


/* user code section */

/* end user  code section */


void
cmdline_parser_print_version (void)
{
  printf("gfsmapply (%s %s) by Bryan Jurish <moocow.bovine@gmail.com>\n", PACKAGE, VERSION);
}

void
cmdline_parser_print_help (void)
{
  cmdline_parser_print_version ();
  printf("\n");
  printf("Purpose:\n");
  printf("  Apply a transducer to a list of input strings\n");
  printf("\n");
  
  printf("Usage: %s [OPTIONS]... INPUT...\n", "gfsmapply");
  
  printf("\n");
  printf(" Arguments:\n");
  printf("   INPUT...  Input files or strings (with -w)\n");
  
  printf("\n");
  printf(" Options:\n");
  printf("   -h         --help            Print help and exit.\n");
  printf("   -V         --version         Print version and exit.\n");
  printf("   -w         --words           INPUT(s) are words to lookup, not filenames.\n");
  printf("   -iLABELS   --ilabels=LABELS  Specify input (lower) labels file.\n");
  printf("   -oLABELS   --olabels=LABELS  Specify output (upper) labels file.\n");
  printf("   -lLABELS   --labels=LABELS   Set -i and -o labels simultaneously.\n");
  printf("   -a         --att-mode        Parse/format string(s) in AT&T-compatible mode.\n");
  printf("   -u         --utf8            Assume UTF-8 encoded alphabet(s) and input(s).\n");
  printf("   -q         --quiet           Suppress warnings about undefined symbols.\n");
  printf("   -fFSTFILE  --fst=FSTFILE     Transducer to apply (default=stdin).\n");
  printf("   -QN        --maxq=N          Maximum number of result states to generate (default=0:system limit)\n");
  printf("   -A         --align           Output aligned arc paths.\n");
  printf("   -zLEVEL    --compress=LEVEL  Specify compression level of output file.\n");
  printf("   -FFILE     --output=FILE     Specifiy output file (default=stdout).\n");
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
  args_info->words_flag = 0; 
  args_info->ilabels_arg = NULL; 
  args_info->olabels_arg = NULL; 
  args_info->labels_arg = NULL; 
  args_info->att_mode_flag = 0; 
  args_info->utf8_flag = 0; 
  args_info->quiet_flag = 0; 
  args_info->fst_arg = gog_strdup("-"); 
  args_info->maxq_arg = 0; 
  args_info->align_flag = 0; 
  args_info->compress_arg = -1; 
  args_info->output_arg = gog_strdup("-"); 
}


int
cmdline_parser (int argc, char * const *argv, struct gengetopt_args_info *args_info)
{
  int c;	/* Character of the parsed option.  */
  int missing_required_options = 0;	

  args_info->help_given = 0;
  args_info->version_given = 0;
  args_info->words_given = 0;
  args_info->ilabels_given = 0;
  args_info->olabels_given = 0;
  args_info->labels_given = 0;
  args_info->att_mode_given = 0;
  args_info->utf8_given = 0;
  args_info->quiet_given = 0;
  args_info->fst_given = 0;
  args_info->maxq_given = 0;
  args_info->align_given = 0;
  args_info->compress_given = 0;
  args_info->output_given = 0;

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
	{ "words", 0, NULL, 'w' },
	{ "ilabels", 1, NULL, 'i' },
	{ "olabels", 1, NULL, 'o' },
	{ "labels", 1, NULL, 'l' },
	{ "att-mode", 0, NULL, 'a' },
	{ "utf8", 0, NULL, 'u' },
	{ "quiet", 0, NULL, 'q' },
	{ "fst", 1, NULL, 'f' },
	{ "maxq", 1, NULL, 'Q' },
	{ "align", 0, NULL, 'A' },
	{ "compress", 1, NULL, 'z' },
	{ "output", 1, NULL, 'F' },
        { NULL,	0, NULL, 0 }
      };
      static char short_options[] = {
	'h',
	'V',
	'w',
	'i', ':',
	'o', ':',
	'l', ':',
	'a',
	'u',
	'q',
	'f', ':',
	'Q', ':',
	'A',
	'z', ':',
	'F', ':',
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
        
        case 'w':	 /* INPUT(s) are words to lookup, not filenames. */
          if (args_info->words_given) {
            fprintf(stderr, "%s: `--words' (`-w') option given more than once\n", PROGRAM);
          }
          args_info->words_given++;
         if (args_info->words_given <= 1)
           args_info->words_flag = !(args_info->words_flag);
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
        
        case 'a':	 /* Parse/format string(s) in AT&T-compatible mode. */
          if (args_info->att_mode_given) {
            fprintf(stderr, "%s: `--att-mode' (`-a') option given more than once\n", PROGRAM);
          }
          args_info->att_mode_given++;
         if (args_info->att_mode_given <= 1)
           args_info->att_mode_flag = !(args_info->att_mode_flag);
          break;
        
        case 'u':	 /* Assume UTF-8 encoded alphabet(s) and input(s). */
          if (args_info->utf8_given) {
            fprintf(stderr, "%s: `--utf8' (`-u') option given more than once\n", PROGRAM);
          }
          args_info->utf8_given++;
         if (args_info->utf8_given <= 1)
           args_info->utf8_flag = !(args_info->utf8_flag);
          break;
        
        case 'q':	 /* Suppress warnings about undefined symbols. */
          if (args_info->quiet_given) {
            fprintf(stderr, "%s: `--quiet' (`-q') option given more than once\n", PROGRAM);
          }
          args_info->quiet_given++;
         if (args_info->quiet_given <= 1)
           args_info->quiet_flag = !(args_info->quiet_flag);
          break;
        
        case 'f':	 /* Transducer to apply (default=stdin). */
          if (args_info->fst_given) {
            fprintf(stderr, "%s: `--fst' (`-f') option given more than once\n", PROGRAM);
          }
          args_info->fst_given++;
          if (args_info->fst_arg) free(args_info->fst_arg);
          args_info->fst_arg = gog_strdup(val);
          break;
        
        case 'Q':	 /* Maximum number of result states to generate (default=0:system limit) */
          if (args_info->maxq_given) {
            fprintf(stderr, "%s: `--maxq' (`-Q') option given more than once\n", PROGRAM);
          }
          args_info->maxq_given++;
          args_info->maxq_arg = (int)atoi(val);
          break;
        
        case 'A':	 /* Output aligned arc paths. */
          if (args_info->align_given) {
            fprintf(stderr, "%s: `--align' (`-A') option given more than once\n", PROGRAM);
          }
          args_info->align_given++;
         if (args_info->align_given <= 1)
           args_info->align_flag = !(args_info->align_flag);
          break;
        
        case 'z':	 /* Specify compression level of output file. */
          if (args_info->compress_given) {
            fprintf(stderr, "%s: `--compress' (`-z') option given more than once\n", PROGRAM);
          }
          args_info->compress_given++;
          args_info->compress_arg = (int)atoi(val);
          break;
        
        case 'F':	 /* Specifiy output file (default=stdout). */
          if (args_info->output_given) {
            fprintf(stderr, "%s: `--output' (`-F') option given more than once\n", PROGRAM);
          }
          args_info->output_given++;
          if (args_info->output_arg) free(args_info->output_arg);
          args_info->output_arg = gog_strdup(val);
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
          
          /* INPUT(s) are words to lookup, not filenames. */
          else if (strcmp(olong, "words") == 0) {
            if (args_info->words_given) {
              fprintf(stderr, "%s: `--words' (`-w') option given more than once\n", PROGRAM);
            }
            args_info->words_given++;
           if (args_info->words_given <= 1)
             args_info->words_flag = !(args_info->words_flag);
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
          
          /* Parse/format string(s) in AT&T-compatible mode. */
          else if (strcmp(olong, "att-mode") == 0) {
            if (args_info->att_mode_given) {
              fprintf(stderr, "%s: `--att-mode' (`-a') option given more than once\n", PROGRAM);
            }
            args_info->att_mode_given++;
           if (args_info->att_mode_given <= 1)
             args_info->att_mode_flag = !(args_info->att_mode_flag);
          }
          
          /* Assume UTF-8 encoded alphabet(s) and input(s). */
          else if (strcmp(olong, "utf8") == 0) {
            if (args_info->utf8_given) {
              fprintf(stderr, "%s: `--utf8' (`-u') option given more than once\n", PROGRAM);
            }
            args_info->utf8_given++;
           if (args_info->utf8_given <= 1)
             args_info->utf8_flag = !(args_info->utf8_flag);
          }
          
          /* Suppress warnings about undefined symbols. */
          else if (strcmp(olong, "quiet") == 0) {
            if (args_info->quiet_given) {
              fprintf(stderr, "%s: `--quiet' (`-q') option given more than once\n", PROGRAM);
            }
            args_info->quiet_given++;
           if (args_info->quiet_given <= 1)
             args_info->quiet_flag = !(args_info->quiet_flag);
          }
          
          /* Transducer to apply (default=stdin). */
          else if (strcmp(olong, "fst") == 0) {
            if (args_info->fst_given) {
              fprintf(stderr, "%s: `--fst' (`-f') option given more than once\n", PROGRAM);
            }
            args_info->fst_given++;
            if (args_info->fst_arg) free(args_info->fst_arg);
            args_info->fst_arg = gog_strdup(val);
          }
          
          /* Maximum number of result states to generate (default=0:system limit) */
          else if (strcmp(olong, "maxq") == 0) {
            if (args_info->maxq_given) {
              fprintf(stderr, "%s: `--maxq' (`-Q') option given more than once\n", PROGRAM);
            }
            args_info->maxq_given++;
            args_info->maxq_arg = (int)atoi(val);
          }
          
          /* Output aligned arc paths. */
          else if (strcmp(olong, "align") == 0) {
            if (args_info->align_given) {
              fprintf(stderr, "%s: `--align' (`-A') option given more than once\n", PROGRAM);
            }
            args_info->align_given++;
           if (args_info->align_given <= 1)
             args_info->align_flag = !(args_info->align_flag);
          }
          
          /* Specify compression level of output file. */
          else if (strcmp(olong, "compress") == 0) {
            if (args_info->compress_given) {
              fprintf(stderr, "%s: `--compress' (`-z') option given more than once\n", PROGRAM);
            }
            args_info->compress_given++;
            args_info->compress_arg = (int)atoi(val);
          }
          
          /* Specifiy output file (default=stdout). */
          else if (strcmp(olong, "output") == 0) {
            if (args_info->output_given) {
              fprintf(stderr, "%s: `--output' (`-F') option given more than once\n", PROGRAM);
            }
            args_info->output_given++;
            if (args_info->output_arg) free(args_info->output_arg);
            args_info->output_arg = gog_strdup(val);
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
