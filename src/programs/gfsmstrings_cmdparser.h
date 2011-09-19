/* -*- Mode: C -*-
 *
 * File: gfsmstrings_cmdparser.h
 * Description: Headers for command-line parser struct gengetopt_args_info.
 *
 * File autogenerated by optgen.perl version 0.06.
 *
 */

#ifndef gfsmstrings_cmdparser_h
#define gfsmstrings_cmdparser_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * moocow: Never set PACKAGE and VERSION here.
 */

struct gengetopt_args_info {
  char * ilabels_arg;	 /* Specify input (lower) labels file. (default=NULL). */
  char * olabels_arg;	 /* Specify output (upper) labels file. (default=NULL). */
  char * labels_arg;	 /* Set -i and -o labels simultaneously. (default=NULL). */
  char * qlabels_arg;	 /* Specify state labels file (for -align mode). (default=NULL). */
  int show_states_flag;	 /* Show numeric states in -align mode output (default=0). */
  int att_flag;	 /* Output in AT&T regex format. (default=0). */
  int align_flag;	 /* Output aligned arc paths. (default=0). */
  int viterbi_flag;	 /* Treat input automaton as a Viterbi trellis. (default=0). */
  char * output_arg;	 /* Output file. (default=NULL). */

  int help_given;	 /* Whether help was given */
  int version_given;	 /* Whether version was given */
  int ilabels_given;	 /* Whether ilabels was given */
  int olabels_given;	 /* Whether olabels was given */
  int labels_given;	 /* Whether labels was given */
  int qlabels_given;	 /* Whether qlabels was given */
  int show_states_given;	 /* Whether show-states was given */
  int att_given;	 /* Whether att was given */
  int align_given;	 /* Whether align was given */
  int viterbi_given;	 /* Whether viterbi was given */
  int output_given;	 /* Whether output was given */
  
  char **inputs;         /* unnamed arguments */
  unsigned inputs_num;   /* number of unnamed arguments */
};

/* read rc files (if any) and parse all command-line options in one swell foop */
int  cmdline_parser (int argc, char *const *argv, struct gengetopt_args_info *args_info);

/* instantiate defaults from environment variables: you must call this yourself! */
void cmdline_parser_envdefaults (struct gengetopt_args_info *args_info);

/* read a single rc-file */
void cmdline_parser_read_rcfile (const char *filename,
				    struct gengetopt_args_info *args_info,
				    int user_specified);

/* read a single rc-file (stream) */
void cmdline_parser_read_rc_stream (FILE *rcfile,
				       const char *filename,
				       struct gengetopt_args_info *args_info);

/* parse a single option */
int cmdline_parser_parse_option (char oshort, const char *olong, const char *val,
				    struct gengetopt_args_info *args_info);

/* print help message */
void cmdline_parser_print_help(void);

/* print version */
void cmdline_parser_print_version(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* gfsmstrings_cmdparser_h */
