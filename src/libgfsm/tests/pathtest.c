#include <gfsm.h>

//#define NITERS 1024
//#define NITERS 65536
//#define NITERS 131072
//#define NITERS 262144
#define NITERS 524288
//#define NITERS 1048576

#define DO_LOOKUP  1
#define DO_PATHS   1
#define DO_STRINGS 1

const char *progname = "pathtest";
const char *fstfile  = "lkptest.gfst";
const char *labfile  = "test.lab";
gfsmLabelVector *input = NULL;
gfsmStringAlphabet *abet = NULL;
gfsmAutomaton *fst = NULL;
gfsmAutomaton *result = NULL;
gfsmSet       *paths = NULL;
GSList        *strings = NULL;
gfsmError     *err = NULL;


int main(int argc, char **argv) {
  int i;
  GSList *sltmp;

  //-- load fst
  fst = gfsm_automaton_new();
  if (!gfsm_automaton_load_bin_filename(fst,fstfile,&err)) {
    g_printerr("%s: load failed for '%s': %s\n", progname, fstfile, (err ? err->message : "?"));
    exit(3);
  }

  //-- load labels
  abet = (gfsmStringAlphabet*)gfsm_string_alphabet_new();
  if (!gfsm_alphabet_load_filename((gfsmAlphabet*)abet,labfile,&err)) {
    g_printerr("%s: load failed for labels file '%s': %s\n",
	       progname, labfile, (err ? err->message : "?"));
    exit(2);
  }

  //-- setup input vector
  input = g_ptr_array_new();
  g_ptr_array_add(input,(gpointer)2);
  g_ptr_array_add(input,(gpointer)2);
  g_ptr_array_add(input,(gpointer)3);

  //-- guts
  for (i=0; i < NITERS; i++) {
#ifdef DO_LOOKUP
    result  = gfsm_lookup(input, fst, result);
#endif
#ifdef DO_PATHS
    paths   = gfsm_automaton_paths(result,paths);
#endif
#ifdef DO_STRINGS
    strings = gfsm_paths_to_strings(paths, abet, NULL, fst->sr, TRUE, TRUE, strings);
#endif

    //-- cleanup
    for (sltmp=strings; sltmp; sltmp=sltmp->next) { g_free(sltmp->data);  }
    if (strings) g_slist_free(strings);
    if (paths)   gfsm_set_clear(paths);
  }

  //-- cleanup
  if (result) gfsm_automaton_free(result);
  if (paths)  gfsm_set_free(paths);

  return 0;
}
