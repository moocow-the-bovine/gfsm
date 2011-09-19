#include <gfsm.h>
#include <stdio.h>
#include <stdlib.h>


//#define NITERS 0
//#define NITERS 1
//#define NITERS 10
//#define NITERS 1024
#define NITERS 65536
//#define NITERS 131072
//#define NITERS 262144
//#define NITERS 524288
//#define NITERS 1048576

//#define PRINT_CHUNK_INFO 1
//#define DO_PROFILE 1

const char *prog = "arcpaths";
//const char *labfile = "arcpaths.lab";
const char *fsmfile = "arcpaths.gfst";

//-- vars
gfsmAutomaton *fsm;
GSList *arcpaths = NULL;
gfsmError *err = NULL;
char line[256];

//#define GFSM_AUTOMATON_ARCPATHS gfsm_automaton_arcpaths0
#define GFSM_AUTOMATON_ARCPATHS gfsm_automaton_arcpaths1

//======================================================================
// MAIN
int main(int argc, char **argv) {
  int i;

  //-- command-line: AUTOMATON
  if (argc > 1) { fsmfile = argv[1]; }

  //-- load automaton
  fsm = gfsm_automaton_new();
  if (!gfsm_automaton_load_bin_filename(fsm,fsmfile,&err)) {
    g_printerr("%s: load failed for '%s': %s\n", prog, fsmfile, (err ? err->message : "?"));
    exit(3);
  }

  //-- guts
  for (i=0; i < NITERS; i++) {
    arcpaths = GFSM_AUTOMATON_ARCPATHS(fsm);
    gfsm_arcpath_list_free(arcpaths);
  }

  return 0;
}
