/* -*- Mode: C -*-
 *
 * File: gfsmarcsort_cmdparser.h
 * Description: Headers for command-line parser struct gengetopt_args_info.
 *
 * File autogenerated by optgen.perl version 0.07.
 *
 */

#ifndef gfsmarcsort_cmdparser_h
#define gfsmarcsort_cmdparser_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * moocow: Never set PACKAGE and VERSION here.
 */

struct gengetopt_args_info {
  int lower_flag;	 /* Sort by (lower,upper,target) [default]. (default=0). */
  int upper_flag;	 /* Sort by (upper,lower,target). (default=0). */
  int weight_flag;	 /* Sort by weight (ascending). (default=0). */
  int i_flag;	 /* Alias for '--lower' (default=0). */
  int o_flag;	 /* Alias for '--upper' (default=0). */
  int cost_flag;	 /* Alias for '--weight' (default=0). */
  char * mode_arg;	 /* Sort by explicit mode string (overrides -l, -u, etc.) (default=). */
  int unique_flag;	 /* After sorting, collect weights on otherwise identical arcs (default=0). */
  int compress_arg;	 /* Specify compression level of output file. (default=-1). */
  char * output_arg;	 /* Specifiy output file (default=stdout). (default=-). */

  int help_given;	 /* Whether help was given */
  int version_given;	 /* Whether version was given */
  int lower_given;	 /* Whether lower was given */
  int upper_given;	 /* Whether upper was given */
  int weight_given;	 /* Whether weight was given */
  int i_given;	 /* Whether - was given */
  int o_given;	 /* Whether - was given */
  int cost_given;	 /* Whether cost was given */
  int mode_given;	 /* Whether mode was given */
  int unique_given;	 /* Whether unique was given */
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
#endif /* gfsmarcsort_cmdparser_h */
