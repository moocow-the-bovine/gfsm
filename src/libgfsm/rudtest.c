#include <gfsm.h>
#include <stdlib.h>

char *fsm1file = "rudtest.gfst";
char *fsm2file = "rudtest.gfst";

gfsmError *errp = NULL;

void hackme(gfsmAutomaton *fsm1, gfsmAutomaton *fsm2, char *label)
{
  printf("<%s>:b reverse(fsm1)\n", label);
  gfsm_automaton_reverse(fsm1);

  printf("<%s>:a union(fsm1,fsm2)\n", label);
  gfsm_automaton_union(fsm1, fsm2);

  printf("<%s>:c determinize(fsm1)\n", label);
  gfsm_automaton_determinize(fsm1);
}


int main (void) {
  gfsmAutomaton *fsm1, *fsm2;
  //g_thread_init(NULL);

  fsm1 = gfsm_automaton_new();
  fsm2 = gfsm_automaton_new();

  gfsm_automaton_load_bin_filename(fsm1,fsm1file,&errp);
  gfsm_automaton_load_bin_filename(fsm2,fsm1file,&errp);
  if (errp) {
    g_printerr("error: %s\n", errp->message);
    exit(1);
  }

  hackme(fsm1,fsm2,"1");
  hackme(fsm1,fsm2,"2");
  hackme(fsm1,fsm2,"3");
  hackme(fsm1,fsm2,"4");

  return 0;
}
