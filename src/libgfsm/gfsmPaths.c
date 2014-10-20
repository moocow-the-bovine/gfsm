
/*=============================================================================*\
 * File: gfsmPaths.c
 * Author: Bryan Jurish <moocow.bovine@gmail.com>
 * Description: finite state machine library
 *
 * Copyright (c) 2005-2011 Bryan Jurish.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
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

#include <gfsmPaths.h>
#include <gfsmArc.h>
#include <gfsmArcIter.h>
#include <stdlib.h>

//-- inline definitions
#ifndef GFSM_INLINE_ENABLED
# include <gfsmPaths.hi>
#endif

/*======================================================================
 * Methods: Path Utilities: gfsmLabelVector
 */

//--------------------------------------------------------------
gfsmLabelVector *gfsm_label_vector_copy(gfsmLabelVector *dst, gfsmLabelVector *src)
{
  guint i;
  g_ptr_array_set_size(dst, src->len);
  for (i=0; i < src->len; i++) {
    g_ptr_array_index(dst,i) = g_ptr_array_index(src,i);
  }
  return dst;
}

//--------------------------------------------------------------
gfsmLabelVector *gfsm_label_vector_reverse(gfsmLabelVector *v)
{
  guint i, mid;
  gpointer tmp;
  mid = v->len/2;
  for (i=0; i < mid; i++) {
    tmp = g_ptr_array_index(v,i);
    g_ptr_array_index(v,i) = g_ptr_array_index(v,v->len-i-1);
    g_ptr_array_index(v,v->len-i-1) = tmp;
  }
  return v;
}

/*======================================================================
 * Methods: Path Utilities: gfsmPath
 */


//--------------------------------------------------------------
gfsmPath *gfsm_path_new_copy(gfsmPath *p1)
{
  gfsmPath *p = gfsm_slice_new(gfsmPath);

  p->lo = g_ptr_array_sized_new(p1->lo->len);
  p->hi = g_ptr_array_sized_new(p1->hi->len);

  gfsm_label_vector_copy(p->lo, p1->lo);
  gfsm_label_vector_copy(p->hi, p1->hi);

  p->w  = p1->w;

  return p;
}

//--------------------------------------------------------------
gfsmPath *gfsm_path_new_append(gfsmPath *p1, gfsmLabelVal lo, gfsmLabelVal hi, gfsmWeight w, gfsmSemiring *sr)
{
  gfsmPath *p = gfsm_slice_new(gfsmPath);

  if (lo != gfsmEpsilon) {
    p->lo = g_ptr_array_sized_new(p1->lo->len+1);
    gfsm_label_vector_copy(p->lo, p1->lo);
    g_ptr_array_add(p->lo, GUINT_TO_POINTER(lo));
  } else {
    p->lo = g_ptr_array_sized_new(p1->lo->len);
    gfsm_label_vector_copy(p->lo, p1->lo);
  }

  if (hi != gfsmEpsilon) {
    p->hi = g_ptr_array_sized_new(p1->hi->len+1);
    gfsm_label_vector_copy(p->hi, p1->hi);
    g_ptr_array_add(p->hi, GUINT_TO_POINTER(hi));
  } else {
    p->hi = g_ptr_array_sized_new(p1->hi->len);
    gfsm_label_vector_copy(p->hi, p1->hi);
  }

  p->w  = gfsm_sr_times(sr, p1->w, w);

  return p;
}

//--------------------------------------------------------------
gfsmPath *gfsm_path_new_times_w(gfsmPath *p1, gfsmWeight w, gfsmSemiring *sr)
{
  gfsmPath *p = gfsm_slice_new(gfsmPath);

  p->lo = g_ptr_array_sized_new(p1->lo->len);
  gfsm_label_vector_copy(p->lo, p1->lo);

  p->hi = g_ptr_array_sized_new(p1->hi->len);
  gfsm_label_vector_copy(p->hi, p1->hi);

  p->w = gfsm_sr_times(sr, p1->w, w);

  return p;
}


//--------------------------------------------------------------
int gfsm_label_vector_compare(const gfsmLabelVector *v1, const gfsmLabelVector *v2)
{
  guint i;
  gfsmLabelVal lab1, lab2;
  if (v1==v2) return 0;

  for (i=0; i < v1->len && i < v2->len; i++) {
    lab1 = (gfsmLabelVal)GPOINTER_TO_UINT(g_ptr_array_index(v1,i));
    lab2 = (gfsmLabelVal)GPOINTER_TO_UINT(g_ptr_array_index(v2,i));
    if (lab1 < lab2) return -1;
    if (lab1 > lab2) return  1;
  }
  if (v1->len <  v2->len) return -1;
  if (v1->len >  v2->len) return  1;
  return 0;
}

//--------------------------------------------------------------
int gfsm_path_compare_data(const gfsmPath *p1, const gfsmPath *p2, gfsmSemiring *sr)
{
  int cmp;
  if (p1==p2) return 0;
  if ((cmp=gfsm_sr_compare(sr, p1->w, p2->w))!=0) return cmp;
  if ((cmp=gfsm_label_vector_compare(p1->lo,p2->lo))!=0) return cmp;
  if ((cmp=gfsm_label_vector_compare(p1->hi,p2->hi))!=0) return cmp;
  return 0;
}


/*======================================================================
 * Methods: Automaton Serialization: paths()
 */

//--------------------------------------------------------------
gfsmSet *gfsm_automaton_paths(gfsmAutomaton *fsm, gfsmSet *paths)
{
  return gfsm_automaton_paths_full(fsm, paths, (fsm->flags.is_transducer ? gfsmLSBoth : gfsmLSLower));
}

//--------------------------------------------------------------
gfsmSet *gfsm_automaton_paths_full(gfsmAutomaton *fsm, gfsmSet *paths, gfsmLabelSide which)
{
  gfsmPath *tmp = gfsm_path_new(fsm->sr);
  if (paths==NULL) {
    paths = gfsm_set_new_full((GCompareDataFunc)gfsm_path_compare_data,
			      (gpointer)fsm->sr,
			      (GDestroyNotify)gfsm_path_free);
  }
  _gfsm_automaton_paths_r(fsm, paths, which, fsm->root_id, tmp);
  gfsm_path_free(tmp);
  return paths;
}

//--------------------------------------------------------------
gfsmSet *_gfsm_automaton_paths_r(gfsmAutomaton *fsm,
				 gfsmSet       *paths,
				 gfsmLabelSide  which, 
				 gfsmStateId    q,
				 gfsmPath      *path)
{
  gfsmArcIter ai;
  gfsmWeight  fw;

  //-- sanity check

  //-- if final state, add to set of full paths
  if (gfsm_automaton_lookup_final(fsm,q,&fw)) {
    gfsmWeight path_w = path->w;
    path->w = gfsm_sr_times(fsm->sr, fw, path_w);

    if (!gfsm_set_contains(paths,path)) {
      gfsm_set_insert(paths, gfsm_path_new_copy(path));
    }
    path->w = path_w;
  }

  //-- investigate all outgoing arcs
  for (gfsm_arciter_open(&ai, fsm, q); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
    gfsmArc    *arc = gfsm_arciter_arc(&ai);
    gfsmWeight    w = path->w;
    gfsmLabelVal lo,hi;

    if (which==gfsmLSLower) {
      lo = arc->lower;
      hi = gfsmEpsilon;
    } else if (which==gfsmLSUpper) {
      lo = gfsmEpsilon;
      hi = arc->upper;
    } else {
      lo = arc->lower;
      hi = arc->upper;
    }

    gfsm_path_push(path, lo, hi, arc->weight, fsm->sr);
    _gfsm_automaton_paths_r(fsm, paths, which, arc->target, path);

    gfsm_path_pop(path, lo, hi);
    path->w = w;
  }
  gfsm_arciter_close(&ai);

  return paths;
}

/*======================================================================
 * Methods: Automaton Serialization: paths_to_strings()
 */

//--------------------------------------------------------------
GSList *gfsm_paths_to_strings(gfsmSet *paths,
			      gfsmAlphabet *abet_lo,
			      gfsmAlphabet *abet_hi,
			      gfsmSemiring *sr,
			      gboolean warn_on_undefined,
			      gboolean att_style,
			      GFSM_UNUSED GSList *strings)
{
  gfsmPathsToStringsOptions opts =
    {
      abet_lo,
      abet_hi,
      sr,
      warn_on_undefined,
      att_style,
      NULL
    };

  gfsm_set_foreach(paths, (GTraverseFunc)_gfsm_paths_to_strings_foreach_func, &opts);

  return g_slist_reverse(opts.strings);
}

//--------------------------------------------------------------
gboolean _gfsm_paths_to_strings_foreach_func(gfsmPath *path,
					     GFSM_UNUSED gpointer value_dummy,
					     gfsmPathsToStringsOptions *opts)
{
  GString *gs = gfsm_path_to_gstring(path, NULL,
				     opts->abet_lo, opts->abet_hi, opts->sr,
				     opts->warn_on_undefined, opts->att_style);
  opts->strings = g_slist_prepend(opts->strings, gs->str);
  g_string_free(gs,FALSE);

  return FALSE;
}

//--------------------------------------------------------------
GString *gfsm_path_to_gstring(gfsmPath     *path,
			      GString      *gs,
			      gfsmAlphabet *abet_lo,
			      gfsmAlphabet *abet_hi,
			      gfsmSemiring *sr,
			      gboolean      warn_on_undefined,
			      gboolean      att_style)
{
  if (!gs) gs = g_string_new("");
  if (abet_lo && path->lo->len > 0) {
    gfsm_alphabet_labels_to_gstring(abet_lo, path->lo, gs, warn_on_undefined, att_style);
  }
  if (abet_lo && abet_hi)
    g_string_append(gs," : ");
  if (abet_hi && path->hi->len > 0) {
    gfsm_alphabet_labels_to_gstring(abet_hi, path->hi, gs, warn_on_undefined, att_style);
  }
  if (gfsm_sr_compare(sr, path->w, sr->one) != 0) {
    g_string_append_printf(gs," <%g>",path->w);
  }
  return gs;
}

//--------------------------------------------------------------
char *gfsm_path_to_string(gfsmPath     *path,
			  gfsmAlphabet *abet_lo,
			  gfsmAlphabet *abet_hi,
			  gfsmSemiring *sr,
			  gboolean      warn_on_undefined,
			  gboolean      att_style)
{
  GString *gs = gfsm_path_to_gstring(path,NULL,abet_lo,abet_hi,sr,warn_on_undefined,att_style);
  char    *s  = gs->str;
  g_string_free(gs,FALSE);
  return s;
}

/*======================================================================
 * Methods: Automaton Serialization: arcpaths()
 */

//--------------------------------------------------------------
gfsmArcPath *gfsm_arcpath_copy(gfsmArcPath *dst, gfsmArcPath *src)
{
  guint i;
  g_ptr_array_set_size(dst, src->len);
  for (i=0; i < src->len; i++) {
    g_ptr_array_index(dst,i) = g_ptr_array_index(src,i);
  }
  return dst;
}

//--------------------------------------------------------------
void gfsm_arcpath_list_free(GSList *arcpaths)
{
  GSList *pi;
  for (pi=arcpaths; pi != NULL; pi = pi->next) {
    gfsm_arcpath_free((gfsmArcPath*)pi->data);
  }
  g_slist_free(arcpaths);
}


//--------------------------------------------------------------
GSList *gfsm_automaton_arcpaths(gfsmAutomaton *fsm)
{
  GSList *r_paths = NULL, *r_nodes  = NULL, *paths = NULL, *rpi;
  _gfsm_automaton_arcpaths_r(fsm, gfsm_automaton_get_root(fsm), NULL, 0, &r_paths, &r_nodes);

  //-- convert reversed GSList* r_paths to forward gfsmArcPath* ap
  for (rpi=r_paths; rpi != NULL; rpi=rpi->next) {
    GSList       *rpl = (GSList*)rpi->data;
    guint         len = g_slist_length(rpl);
    gfsmArcPath   *ap = g_ptr_array_sized_new(len);
    ap->len           = len;
    for (; rpl != NULL; rpl=rpl->next) {
      --len;
      g_ptr_array_index(ap,len) = rpl->data;
    }
    paths = g_slist_prepend(paths, ap);
  }

  //-- free all temporary nodes
  for (rpi=r_nodes; rpi != NULL; rpi=rpi->next) {
    GSList *rpl = (GSList*)rpi->data;
    g_slist_free_1(rpl);
  }
  g_slist_free(r_nodes);
  g_slist_free(r_paths);

  return paths;
}

//--------------------------------------------------------------
void _gfsm_automaton_arcpaths_r(gfsmAutomaton  *fsm,
				gfsmStateId     qid,
				GSList         *path,     //-- current path to qid
				guint           path_len, //-- current path length (for cheap cyclicity check)
				GSList        **paths,    //-- GSList of *(GSList of gfsmArc*): all complete paths, reversed
				GSList        **nodes     //-- GSList of *(GSList of gfsmArc*): all allocated nodes
				)
				    
{
  gfsmArcIter ai;
  gfsmWeight  fw;

  //-- sanity check
  if (path_len > gfsm_automaton_n_states(fsm)) {
    g_printerr("_gfsm_automaton_arcpaths_r(): cyclic automaton detected!\n");
    exit(1);
  }

  //-- check for final state
  if (gfsm_automaton_lookup_final(fsm,qid,&fw)) {
    GSList *fp = g_slist_prepend(path, gfsm_weight2ptr(fw)); //-- add pseudo-arc for final weight
    *nodes     = g_slist_prepend(*nodes, fp);
    *paths     = g_slist_prepend(*paths, fp);
  }

  //-- investigate all outgoing arcs
  for (gfsm_arciter_open(&ai, fsm, qid); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
    gfsmArc       *arc = gfsm_arciter_arc(&ai);
    GSList         *qp = g_slist_prepend(  path, arc);
    *nodes             = g_slist_prepend(*nodes,  qp);
    _gfsm_automaton_arcpaths_r(fsm, arc->target, qp, path_len+1, paths, nodes);
  }
}

/*======================================================================
 * Methods: Stringification: arcpaths()
 */

//--------------------------------------------------------------
GString *gfsm_arcpath_to_gstring(gfsmArcPath *path, GString *gs, gfsmArcPathToStringOptions *opts)
{
  guint i;
  GString *gsym = g_string_new(""); //-- temporary for gfsm_alphabet_label_to_gstring()
  if (!gs) gs = g_string_new("");
  
  //-- source state (root)
  if (opts->show_states) {
    gfsmStateId q = path->len<=1 ? opts->q0 : ((gfsmArc*)g_ptr_array_index(path,0))->source;
    g_string_append_c(gs,'@');
    gfsm_alphabet_label_to_gstring(opts->abet_q, q, gs, opts->warn_on_undefined, opts->att_style, gsym);
    g_string_append(gs,"@ ");
  }

  //-- guts
  for (i=0; i+1 < path->len; i++) {
    gfsmArc *a = (gfsmArc*)g_ptr_array_index(path,i);

    //-- arc labels
    if (opts->compress_id && a->lower==a->upper) {
      gfsm_alphabet_label_to_gstring(opts->abet_lo,a->lower,gs,opts->warn_on_undefined,opts->att_style,gsym);
    } else {
      g_string_append_c(gs,'(');
      gfsm_alphabet_label_to_gstring(opts->abet_lo,a->lower,gs,opts->warn_on_undefined,opts->att_style,gsym);
      g_string_append_c(gs,':');
      gfsm_alphabet_label_to_gstring(opts->abet_hi,a->upper,gs,opts->warn_on_undefined,opts->att_style,gsym);
      g_string_append_c(gs,')');
    }

    //-- arc weight
    if (!opts->sr || (a->weight != gfsm_sr_one(opts->sr))) {
      g_string_append_printf(gs, "<%g>", a->weight);
    }
    
    //-- target state
    if (opts->show_states) {
      g_string_append(gs," @");
      gfsm_alphabet_label_to_gstring(opts->abet_q,a->target,gs,opts->warn_on_undefined,opts->att_style,gsym);
      g_string_append(gs,"@ ");
    }
  }

  //-- final weight
  if (path->len>0) {
    gfsmWeight  fw = gfsm_ptr2weight(g_ptr_array_index(path,path->len-1));
    if (!opts->sr || (fw != gfsm_sr_one(opts->sr))) {
      g_string_append_printf(gs,"<%g>",fw);
    }
  }

  //-- return
  return gs;
}


//--------------------------------------------------------------
char *gfsm_arcpath_to_string(gfsmArcPath *path, gfsmArcPathToStringOptions *opts)
{
  GString *gs = gfsm_arcpath_to_gstring(path,NULL,opts);
  char    *s  = gs->str;
  g_string_free(gs,FALSE);
  return s;
}

//--------------------------------------------------------------
GSList *gfsm_arcpaths_to_strings(GSList *paths, gfsmArcPathToStringOptions *opts)
{
  GSList *pi, *strings=NULL;
  for (pi=paths; pi != NULL; pi=pi->next) {
    char *s = gfsm_arcpath_to_string((gfsmArcPath*)pi->data, opts);
    strings = g_slist_prepend(strings, s);
  }
  return strings;
}



/*======================================================================
 * Methods: Viterbi trellis: paths
 */

//--------------------------------------------------------------
gfsmSet *gfsm_viterbi_trellis_paths_full(gfsmAutomaton *trellis, gfsmSet *paths, gfsmLabelSide which)
{
  gfsmArcIter ai;

  //-- sanity check: create path-set if given as NULL
  if (!paths) {
    paths = gfsm_set_new_full((GCompareDataFunc)gfsm_path_compare_data,
			      (gpointer)trellis->sr,
			      (GDestroyNotify)gfsm_path_free);
  }

  //-- get & follow pseudo-root of all paths
  for (gfsm_arciter_open(&ai, trellis, trellis->root_id); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
    gfsmArc  *arc  = gfsm_arciter_arc(&ai);
    gfsmPath *path = gfsm_path_new(trellis->sr);

    _gfsm_viterbi_trellis_bestpath_r(trellis, path, which, arc->target);
    path->w = arc->weight;

    //-- reverse the path we've created
    gfsm_path_reverse(path);

    //-- ... and maybe insert it
    if (gfsm_set_contains(paths,path)) {
      //-- oops: we've already got this one: free it
      gfsm_path_free(path);
    } else {
      //-- it's a bona-fide new path: insert it
      gfsm_set_insert(paths,path);
    }
  }

  return paths;
}

//--------------------------------------------------------------
gfsmPath *gfsm_viterbi_trellis_bestpath_full(gfsmAutomaton *trellis, gfsmPath *path, gfsmLabelSide which)
{
  gfsmArcIter ai;

  //-- sanity check: create path if NULL
  if (!path) { path = gfsm_path_new(trellis->sr); }

  //-- get & follow pseudo-root of best path
  gfsm_arciter_open(&ai, trellis, trellis->root_id);
  if (gfsm_arciter_ok(&ai)) {
    gfsmArc *arc = gfsm_arciter_arc(&ai);
    _gfsm_viterbi_trellis_bestpath_r(trellis, path, which, arc->target);
    path->w = arc->weight;
  } else {
    path->w = trellis->sr->zero;
  }

  //-- reverse the path we've created
  gfsm_path_reverse(path);

  return path;
}

//--------------------------------------------------------------
void _gfsm_viterbi_trellis_bestpath_r(gfsmAutomaton *trellis,
				      gfsmPath      *path,
				      gfsmLabelSide  which,
				      gfsmStateId    qid)
{
  while (TRUE) {
    gfsmArcIter ai;
    gfsm_arciter_open(&ai, trellis, qid);

    if (gfsm_arciter_ok(&ai)) {
      gfsmArc *arc = gfsm_arciter_arc(&ai);
      gfsm_path_push(path,
		     (which!=gfsmLSUpper ? arc->lower : gfsmEpsilon),
		     (which!=gfsmLSLower ? arc->upper : gfsmEpsilon),
		     trellis->sr->one, trellis->sr);
      qid = arc->target;
    }
    else break;
  }
}
