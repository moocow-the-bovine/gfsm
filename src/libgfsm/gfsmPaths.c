
/*=============================================================================*\
 * File: gfsmPaths.c
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

#include <gfsmPaths.h>
#include <gfsmArc.h>
#include <gfsmArcIter.h>

/*======================================================================
 * Methods: Path Utilities
 */

//--------------------------------------------------------------
gfsmLabelVector *gfsm_label_vector_copy(gfsmLabelVector *dst, gfsmLabelVector *src)
{
  int i;
  g_ptr_array_set_size(dst, src->len);
  for (i=0; i < src->len; i++) {
    g_ptr_array_index(dst,i) = g_ptr_array_index(src,i);
  }
  return dst;
}

//--------------------------------------------------------------
gfsmPath *gfsm_path_new_full(gfsmLabelVector *lo, gfsmLabelVector *hi, gfsmWeight w)
{
  gfsmPath *p = g_new(gfsmPath,1);
  p->lo = lo ? lo : g_ptr_array_new();
  p->hi = hi ? hi : g_ptr_array_new();
  p->w  = w;
  return p;
}

//--------------------------------------------------------------
gfsmPath *gfsm_path_new_copy(gfsmPath *p1)
{
  gfsmPath *p = g_new(gfsmPath,1);

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
  gfsmPath *p = g_new(gfsmPath,1);

  if (lo != gfsmEpsilon) {
    p->lo = g_ptr_array_sized_new(p1->lo->len+1);
    gfsm_label_vector_copy(p->lo, p1->lo);
    g_ptr_array_add(p->lo, (gpointer)lo);
  } else {
    p->lo = g_ptr_array_sized_new(p1->lo->len);
    gfsm_label_vector_copy(p->lo, p1->lo);
  }

  if (hi != gfsmEpsilon) {
    p->hi = g_ptr_array_sized_new(p1->hi->len+1);
    gfsm_label_vector_copy(p->hi, p1->hi);
    g_ptr_array_add(p->hi, (gpointer)hi);
  } else {
    p->hi = g_ptr_array_sized_new(p1->hi->len);
    gfsm_label_vector_copy(p->hi, p1->hi);
  }

  p->w  = gfsm_sr_times(sr, p1->w, w);

  return p;
}

//--------------------------------------------------------------
void gfsm_path_push(gfsmPath *p, gfsmLabelVal lo, gfsmLabelVal hi, gfsmWeight w, gfsmSemiring *sr)
{
  if (lo != gfsmEpsilon) g_ptr_array_add(p->lo, (gpointer)lo);
  if (hi != gfsmEpsilon) g_ptr_array_add(p->hi, (gpointer)hi);
  p->w = gfsm_sr_times(sr, p->w, w);
}

//--------------------------------------------------------------
void gfsm_path_pop(gfsmPath *p, gfsmLabelVal lo, gfsmLabelVal hi)
{
  if (lo != gfsmEpsilon) g_ptr_array_remove_index_fast(p->lo, p->lo->len-1);
  if (hi != gfsmEpsilon) g_ptr_array_remove_index_fast(p->hi, p->hi->len-1);
}

//--------------------------------------------------------------
int gfsm_label_vector_compare(const gfsmLabelVector *v1, const gfsmLabelVector *v2)
{
  int i;
  gfsmLabelVal lab1, lab2;
  if (v1==v2) return 0;

  for (i=0; i < v1->len && i < v2->len; i++) {
    lab1 = (gfsmLabelVal)g_ptr_array_index(v1,i);
    lab2 = (gfsmLabelVal)g_ptr_array_index(v2,i);
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

//--------------------------------------------------------------
void gfsm_path_free(gfsmPath *p)
{
  if (!p) return;
  if (p->lo) g_ptr_array_free(p->lo,TRUE);
  if (p->hi) g_ptr_array_free(p->hi,TRUE);
  g_free(p);
}

/*======================================================================
 * Methods: Automaton Serialization
 */

//--------------------------------------------------------------
gfsmSet *gfsm_automaton_paths(gfsmAutomaton *fsm, gfsmSet *paths)
{
  gfsmPath *tmp = gfsm_path_new(fsm->sr);
  if (paths==NULL) {
    paths = gfsm_set_new_full((GCompareDataFunc)gfsm_path_compare_data,
			      (gpointer)fsm->sr,
			      (GDestroyNotify)gfsm_path_free);
  }
  _gfsm_automaton_paths_r(fsm, paths, fsm->root_id, tmp);
  gfsm_path_free(tmp);
  return paths;
}

//--------------------------------------------------------------
gfsmSet *_gfsm_automaton_paths_r(gfsmAutomaton *fsm, gfsmSet *paths, gfsmStateId q, gfsmPath *path)
{
  gfsmArcIter ai;

  //-- if final state, add to set of full paths
  if (gfsm_automaton_is_final_state(fsm,q) && !gfsm_set_contains(paths,path)) {
    gfsm_set_insert(paths, gfsm_path_new_copy(path));
  }

  //-- investigate all outgoing arcs
  for (gfsm_arciter_open(&ai, fsm, q); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
    gfsmArc    *arc = gfsm_arciter_arc(&ai);
    gfsmWeight    w = path->w;
    gfsmLabelVal hi = fsm->flags.is_transducer ? arc->upper : gfsmEpsilon;

    gfsm_path_push(path, arc->lower, hi, arc->weight, fsm->sr);
    _gfsm_automaton_paths_r(fsm, paths, arc->target, path);

    gfsm_path_pop(path, arc->lower, hi);
    path->w = w;
  }
  gfsm_arciter_close(&ai);

  return paths;
}

//--------------------------------------------------------------
GSList *gfsm_paths_to_strings(gfsmSet *paths,
			      gfsmStringAlphabet *abet_lo,
			      gfsmStringAlphabet *abet_hi,
			      gfsmSemiring *sr,
			      gboolean warn_on_undefined,
			      gboolean att_style,
			      GSList *strings)
{
  struct _gfsm_paths_to_strings_options opts;
  opts.abet_lo = abet_lo;
  opts.abet_hi = abet_hi;
  opts.sr   = sr;
  opts.warn_on_undefined = warn_on_undefined;
  opts.att_style = att_style;
  opts.strings = NULL;

  gfsm_set_foreach(paths, (GTraverseFunc)gfsm_paths_to_strings_foreach_func, &opts);

  return g_slist_reverse(opts.strings);
}

//--------------------------------------------------------------
gboolean gfsm_paths_to_strings_foreach_func(gfsmPath *path,
					    gpointer value_dummy,
					    struct _gfsm_paths_to_strings_options *opts)
{
  GString *gs = g_string_new("");
  if (opts->abet_lo && path->lo->len > 0) {
    gfsm_alphabet_labels_to_gstring(opts->abet_lo, path->lo, gs, opts->warn_on_undefined, opts->att_style);
  }
  if (opts->abet_hi && path->hi->len > 0) {
    g_string_append(gs," : ");
    gfsm_alphabet_labels_to_gstring(opts->abet_hi, path->hi, gs, opts->warn_on_undefined, opts->att_style);
  }
  if (gfsm_sr_compare(opts->sr, path->w, opts->sr->one) != 0) {
    g_string_append_printf(gs," <%g>",path->w);
  }
  opts->strings = g_slist_prepend(opts->strings, gs->str);
  g_string_free(gs,FALSE);

  return FALSE;
}
