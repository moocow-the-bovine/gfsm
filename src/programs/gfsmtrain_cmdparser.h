/* -*- Mode: C -*-
 *
 * File: gfsmtrain_cmdparser.h
 * Description: Headers for command-line parser struct gengetopt_args_info.
 *
 * File autogenerated by optgen.perl version 0.07.
 *
 */

#ifndef gfsmtrain_cmdparser_h
#define gfsmtrain_cmdparser_h

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
  int att_mode_flag;	 /* Parse string(s) in AT&T-compatible mode. (default=0). */
  int quiet_flag;	 /* Suppress warnings about undefined symbols. (default=0). */
  int utf8_flag;	 /* Assume UTF-8 encoded alphabet and input. (default=0). */
  int best_flag;	 /* Only consider cost-minimal path(s) for each training pair. (default=0). */
  int ordered_flag;	 /* Count permutations in arc-order as multiple paths. (default=0). */
  int distribute_by_path_flag;	 /* Distribute pair-mass over multiple paths. (default=0). */
  int distribute_by_arc_flag;	 /* Distribute path-mass over arcs. (default=0). */
  char * fst_arg;	 /* Transducer to apply (required). (default=NULL). */
  int compress_arg;	 /* Specify compression level of output file. (default=-1). */
  char * output_arg;	 /* Specifiy output file (default=stdout). (default=-). */

  int help_given;	 /* Whether help was given */
  int version_given;	 /* Whether version was given */
  int ilabels_given;	 /* Whether ilabels was given */
  int olabels_given;	 /* Whether olabels was given */
  int labels_given;	 /* Whether labels was given */
  int att_mode_given;	 /* Whether att-mode was given */
  int quiet_given;	 /* Whether quiet was given */
  int utf8_given;	 /* Whether utf8 was given */
  int best_given;	 /* Whether best was given */
  int ordered_given;	 /* Whether ordered was given */
  int distribute_by_path_given;	 /* Whether distribute-by-path was given */
  int distribute_by_arc_given;	 /* Whether distribute-by-arc was given */
  int fst_given;	 /* Whether fst was given */
  int compress_given;	 /* Whether compress was given */
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
#endif /* gfsmtrain_cmdparser_h */
