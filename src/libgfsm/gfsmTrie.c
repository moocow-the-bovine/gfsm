
/*=============================================================================*\
 * File: gfsmTrie.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library
 *
 * Copyright (c) 2005 Bryan Jurish.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *=============================================================================*/

#include <gfsmTrie.h>
#include <gfsmArcIter.h>

/*======================================================================
 * Constants
 */

const gfsmAutomatonFlags gfsmTrieDefaultFlags =
  {
    TRUE,        //-- is_transducer:1
    TRUE,        //-- is_weighted:1
    gfsmASMLower, //-- sort_mode:4
    TRUE,        //-- is_deterministic:1
    0            //-- unused:25
  };

const gfsmSRType gfsmTrieDefaultSRType = gfsmSRTReal;


/*======================================================================
 * Methods: ensure paths (adding weight)
 */

//--------------------------------------------------------------
gfsmStateId gfsm_trie_add_paths(gfsmTrie        *trie,
				gfsmLabelVector *lo,
				gfsmLabelVector *hi,
				gfsmWeight       w)
{
  gfsmStateId  qid;
  guint i;

  //-- ensure trie has a root state
  if (trie->root_id==gfsmNoState) {
    trie->root_id = gfsm_automaton_add_state(trie);
  }
  qid = trie->root_id;

  //-- add lower path
  for (i=0; lo && i < lo->len; i++) {
    qid = gfsm_trie_get_arc_lower(trie, qid, ((gfsmLabelVal)g_ptr_array_index(lo,i)), w);
  }

  //-- add separator (do something like this by hand for efficient lookup)
  //qid = gfsm_trie_get_arc_lower(trie, qid, gfsmNoLabel, w);

  //-- add upper path
  for (i=0; hi && i < hi->len; i++) {
    qid = gfsm_trie_get_arc_upper(trie, qid, ((gfsmLabelVal)g_ptr_array_index(hi,i)), w);
  }

  //-- add final epsilon-arc
  qid = gfsm_trie_get_arc_both(trie, qid, gfsmEpsilon, gfsmEpsilon, w);
  gfsm_automaton_set_final_state(trie,qid,TRUE);

  return qid;
}

/*======================================================================
 * Methods: find arcs
 */

//--------------------------------------------------------------
gfsmArc* gfsm_trie_find_arc_lower(gfsmTrie *trie, gfsmStateId qid, gfsmLabelVal lab)
{
  gfsmArcIter ai;
  gfsmArc     *a=NULL;
  for (gfsm_arciter_open(&ai, trie, qid), gfsm_arciter_seek_lower(&ai, lab);
       gfsm_arciter_ok(&ai); 
       gfsm_arciter_next(&ai), gfsm_arciter_seek_lower(&ai,lab))
    {
      a = gfsm_arciter_arc(&ai);
      break;
    }
  gfsm_arciter_close(&ai);
  return a;
}

//--------------------------------------------------------------
gfsmArc* gfsm_trie_find_arc_upper(gfsmTrie *trie, gfsmStateId qid, gfsmLabelVal lab)
{
  gfsmArcIter ai;
  gfsmArc     *a=NULL;
  for (gfsm_arciter_open(&ai, trie, qid), gfsm_arciter_seek_upper(&ai, lab);
       gfsm_arciter_ok(&ai); 
       gfsm_arciter_next(&ai), gfsm_arciter_seek_upper(&ai,lab))
    {
      a = gfsm_arciter_arc(&ai);
      break;
    }
  gfsm_arciter_close(&ai);
  return a;
}

//--------------------------------------------------------------
gfsmArc* gfsm_trie_find_arc_both(gfsmTrie *trie, gfsmStateId qid, gfsmLabelVal lo, gfsmLabelVal hi)
{
  gfsmArcIter ai;
  gfsmArc     *a=NULL;
  for (gfsm_arciter_open(&ai,trie,qid), gfsm_arciter_seek_both(&ai,lo,hi);
       gfsm_arciter_ok(&ai); 
       gfsm_arciter_next(&ai), gfsm_arciter_seek_both(&ai,lo,hi))
    {
      a = gfsm_arciter_arc(&ai);
      break;
    }
  gfsm_arciter_close(&ai);
  return a;
}


/*======================================================================
 * Methods: find or insert arcs
 */

//--------------------------------------------------------------
gfsmStateId gfsm_trie_get_arc_lower(gfsmTrie *trie, gfsmStateId qid, gfsmLabelVal lab, gfsmWeight w)
{
  gfsmArc *a=gfsm_trie_find_arc_lower(trie,qid,lab);

  if (a==NULL) {
    gfsmStateId qid2 = gfsm_automaton_add_state(trie);
    gfsm_automaton_add_arc(trie,qid,qid2,lab,gfsmEpsilon,w);
    return qid2;
  }

  //-- found an existing arc
  a->weight = gfsm_sr_plus(trie->sr, a->weight, w);
  return a->target;
}

//--------------------------------------------------------------
gfsmStateId gfsm_trie_get_arc_upper(gfsmTrie *trie, gfsmStateId qid, gfsmLabelVal lab, gfsmWeight w)
{
  gfsmArc *a=gfsm_trie_find_arc_upper(trie,qid,lab);

  if (a==NULL) {
    gfsmStateId qid2 = gfsm_automaton_add_state(trie);
    gfsm_automaton_add_arc(trie,qid,qid2,gfsmEpsilon,lab,w);
    //trie->flags.is_deterministic = TRUE; //-- HACK
    return qid2;
  }

  //-- found an existing arc
  a->weight = gfsm_sr_plus(trie->sr, a->weight, w);
  return a->target;
}

//--------------------------------------------------------------
gfsmStateId gfsm_trie_get_arc_both(gfsmTrie *trie, gfsmStateId qid, gfsmLabelVal lo, gfsmLabelVal hi, gfsmWeight w)
{
  gfsmArc *a=gfsm_trie_find_arc_both(trie,qid,lo,hi);

  if (a==NULL) {
    gfsmStateId qid2 = gfsm_automaton_add_state(trie);
    gfsm_automaton_add_arc(trie,qid,qid2,lo,hi,w);
    //trie->flags.is_deterministic = TRUE; //-- HACK
    return qid2;
  }

  //-- found an existing arc
  a->weight = gfsm_sr_plus(trie->sr, a->weight, w);
  return a->target;
}
