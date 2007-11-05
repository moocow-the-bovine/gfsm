#include <gfsm.h>
#include <stdio.h>
#include <stdlib.h>
#include "labprobs.h"

/*======================================================================
 * Globals
 */
const char *prog = "seektest";

const char *fsmfile   = "tagh-lo.gfst";
//const char *xfsmfile  = "tagh-lo.gfstx";

gfsmStateId qid_test = 0;
gulong count_test = 
//16777216
//1048576
4194304
;


/*======================================================================
 * bench_seek_vanilla()
 */
double bench_seek_vanilla(gfsmAutomaton *fsm) {
  guint i;
  double elapsed;
  GPtrArray *ary = g_ptr_array_sized_new(gfsm_automaton_out_degree(fsm,qid_test));
  GTimer *timer  = g_timer_new();

  g_timer_start(timer);
  for (i=0; i < count_test; i++) {
    //-- BEGIN TEST CODE
    gfsmStateId qid = qid_test;
    gfsmLabelId lab = g_array_index(seekus,gfsmLabelId,i);
    gfsmArcIter ai;
    ary->len=0;
    for (gfsm_arciter_open(&ai,fsm,qid), gfsm_arciter_seek_lower(&ai,lab);
	 gfsm_arciter_ok(&ai);
	 gfsm_arciter_next(&ai), gfsm_arciter_seek_lower(&ai,lab))
      {
	gfsmArc *a = gfsm_arciter_arc(&ai);
	if (fsm->flags.sort_mode==gfsmASMLower && a->lower!=lab) break;
	g_ptr_array_add(ary, a);
      }
    //-- END TEST CODE
  }
  elapsed = g_timer_elapsed(timer,NULL);

  //-- cleanup
  g_ptr_array_free(ary,TRUE);
  g_timer_destroy(timer);

  return elapsed;
}

/*======================================================================
 * indexed_lower_lb()
 */
gfsmArcId indexed_lower_lb(gfsmIndexedAutomaton *fsm,
			   gfsmLabelId lab,
			   gfsmArcId aid_lo,
			   gfsmArcId aid_hi)

{
  gfsmArcId aid_mid;
  gfsmArc   *a;

  //if (!gfsm_indexed_automaton_has_state(fsm,qid)) return gfsmNoArc;
  /*
  aid_lo = g_array_index(fsm->state_first_arc, gfsmArcId, qid);
  aid_hi = g_array_index(fsm->state_first_arc, gfsmArcId, qid+1);
  */

  while (aid_lo < aid_hi) {
    aid_mid = (aid_lo+aid_hi)/2;
    a = &g_array_index(fsm->arcs, gfsmArc, g_array_index(fsm->arcix_lower, gfsmArcId, aid_mid));
    if (a->lower < lab) { aid_lo = aid_mid+1; }
    else                { aid_hi = aid_mid; }
  }
  return aid_lo <= aid_hi ? aid_lo : gfsmNoArc;
}


/*======================================================================
 * bench_seek_indexed()
 */
#define SEEK_INDEXED_BINSEARCH_CUTOFF 32
double bench_seek_indexed(gfsmIndexedAutomaton *fsm) {
#if 1
  guint i;
  double elapsed;
  GPtrArray *ary = g_ptr_array_sized_new(gfsm_indexed_automaton_out_degree(fsm,qid_test));
  GTimer *timer  = g_timer_new();

  g_timer_start(timer);
  for (i=0; i < count_test; i++) {
    //-- BEGIN TEST CODE
    gfsmStateId qid = qid_test;
    gfsmLabelId lab = g_array_index(seekus,gfsmLabelId,i);
    gfsmArcId aid_lo = g_array_index(fsm->state_first_arc, gfsmArcId, qid);
    gfsmArcId aid_hi = g_array_index(fsm->state_first_arc, gfsmArcId, qid+1);
    gfsmArcId aid;
    gfsmArc *a;
    ary->len=0;
    if (aid_hi-aid_lo >= SEEK_INDEXED_BINSEARCH_CUTOFF) {
      for (aid=indexed_lower_lb(fsm,lab,aid_lo,aid_hi); aid<aid_hi; aid++) {
	a = &g_array_index(fsm->arcs, gfsmArc, g_array_index(fsm->arcix_lower, gfsmArcId, aid));
	if (a->lower!=lab) break;
	g_ptr_array_add(ary, a);
      }
    } else {
      for (a=((gfsmArc*)fsm->arcs->data)+aid_lo; a < ((gfsmArc*)fsm->arcs->data)+aid_hi; a++) {
	if (a->lower==lab) g_ptr_array_add(ary,a);
      }
    }
    //-- END TEST CODE
  }
  elapsed = g_timer_elapsed(timer,NULL);

  //-- cleanup
  g_ptr_array_free(ary,TRUE);
  g_timer_destroy(timer);

  return elapsed;
#else
  return 1e38; //-- dummy
#endif
}


/*======================================================================
 * Report
 */
void report_elapsed(char *label, double elapsed) {
  fprintf(stderr, "BENCH[%16s]: %ld iters in %.2g sec: %.2g iters/sec\n",
	  label, count_test, elapsed, ((double)count_test)/elapsed);
  fflush(stderr);
}

/*======================================================================
 * Main
 */
int main(int argc, char **argv)
{
  char *qid_str="0";
  gfsmError *err=NULL;
  gfsmAutomaton *fsm;
  gfsmIndexedAutomaton *xfsm;
  double elapsed_vanilla, elapsed_sorted, elapsed_indexed;

  //-- sanity check(s)
  if (argc < 2) {
    fprintf(stderr, "Usage: %s STATE_ID\n", prog);
    exit(1);
  }
  qid_str  = argv[1];

  //-- parse args
  qid_test = strtol(qid_str,NULL,0);

  //-- load probabilities & initialize
  load_label_probs();
  populate_seek_labels();

  //-- load automata
  fsm = gfsm_automaton_new();
  if (!gfsm_automaton_load_bin_filename(fsm,fsmfile,&err)) {
    fprintf(stderr,"%s: load failed for '%s': %s\n", prog, fsmfile, (err ? err->message : "?"));
    exit(3);
  }
  fsm->flags.sort_mode = gfsmASMNone;

  //-- report
  fprintf(stderr, "%s: qid=%d, out_degree=%u, count=%lu\n",
	  prog, qid_test, gfsm_automaton_out_degree(fsm,qid_test), count_test);
  fflush(stderr);

  //-- benchmark: vanilla
  elapsed_vanilla = bench_seek_vanilla(fsm);
  report_elapsed("vanilla", elapsed_vanilla);

  //-- benchmark: vanilla+sorted
  gfsm_automaton_arcsort(fsm,gfsmASMLower);
  elapsed_sorted  = bench_seek_vanilla(fsm);
  report_elapsed("sorted", elapsed_sorted);

  //-- benchmark: indexed
  xfsm = gfsm_automaton_to_indexed(fsm,NULL);
  elapsed_indexed = bench_seek_indexed(xfsm);
  report_elapsed("indexed", elapsed_indexed);

  //-- cleanup
  gfsm_automaton_free(fsm);
  gfsm_indexed_automaton_free(xfsm);

  return 0;
}
