/** Get a pointer to the ::gfsmState object with the ID \a qid in \a fsm, if any.
 *  When you are done with the returned pointer, you must release it
 *  with gfsm_automaton_state_close().
 *  \param fsm fsm from which to extract a state
 *  \param qid ID of the state to extract
 *  \returns a ::gfsmState* for \a qid, or NULL if \a qid not a state of \a fsm
 */
gfsmState *gfsm_automaton_open_state(gfsmAutomaton *fsm, gfsmStateId qid);

/** Open a pointer to a (possibly new) state with ID \a qid.
 *  Really just a wrapper for gfsm_automaton_ensure_state() and gfsm_automaton_open_state().
 *  \returns as for gfsm_automaton_open_state()
 */
//gfsmState *gfsm_automaton_open_state_force(gfsmAutomaton *fsm, gfsmStateId qid);
#define gfsm_automaton_open_state_force(fsm,qid) \
  gfsm_automaton_open_state((fsm), gfsm_automaton_ensure_state((fsm),(id)))

/** Close a pointer to a ::gfsmState object which was previously opened by
 *  gfsm_automaton_open_state().
 *  \param fsm fsm for which the state was opened
 *  \param sp  state pointer returned by gfsm_automaton_open_state()
 *  \warning
 *     Bad things may happen if you change the implementation class of \a fsm
 *     while you have open state pointers for it hanging around.
 */
void gfsm_automaton_close_state(gfsmAutomaton *fsm, gfsmState *sp);
