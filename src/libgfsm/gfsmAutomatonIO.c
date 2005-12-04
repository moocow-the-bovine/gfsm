
/*=============================================================================*\
 * File: gfsmAutomatonIO.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: automata: I/O
 *
 * Copyright (c) 2004 Bryan Jurish.
 *
 * For information on usage and redistribution, and for a DISCLAIMER
 * OF ALL WARRANTIES, see the file "COPYING" in this distribution.
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

#include <gfsmAutomatonIO.h>
#include <gfsmArcIter.h>
#include <gfsmUtils.h>
#include <gfsmCompat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>



/*======================================================================
 * Constants: Binary I/O
 */
const gfsmVersionInfo gfsm_version_bincompat_min_store =
  {
    0, // major
    0, // minor
    8  // micro
  };

const gfsmVersionInfo gfsm_version_bincompat_min_check =
  {
    0, // major
    0, // minor
    2  // micro
  };

const gchar gfsm_header_magic[16] = "gfsm_automaton\0";

/*======================================================================
 * Methods: Binary I/O: load()
 */
/*--------------------------------------------------------------
 * load_bin_header()
 */
gboolean gfsm_automaton_load_bin_header(gfsmAutomatonHeader *hdr, FILE *f, gfsmError **errp)
{
  if (fread(hdr, sizeof(gfsmAutomatonHeader), 1, f) != 1) {
    g_set_error(errp,
		g_quark_from_static_string("gfsm"),                      //-- domain
		g_quark_from_static_string("automaton_load_bin_header:size"), //-- code
		"could not read header");
    return FALSE;
  }
  else if (strcmp(hdr->magic, gfsm_header_magic) != 0) {
    g_set_error(errp,
		g_quark_from_static_string("gfsm"),                            //-- domain
		g_quark_from_static_string("automaton_load_bin_header:magic"), //-- code
		"bad magic");
    return FALSE;
  }
  else if (gfsm_version_compare(hdr->version, gfsm_version_bincompat_min_check) < 0) {
    g_set_error(errp,
		g_quark_from_static_string("gfsm"),                              //-- domain
		g_quark_from_static_string("automaton_load_bin_header:version"), //-- code
		"stored format v%u.%u.%u is obsolete - need at least v%u.%u.%u",
		hdr->version.major,
		hdr->version.minor,
		hdr->version.micro,
		gfsm_version_bincompat_min_check.major,
		gfsm_version_bincompat_min_check.minor,
		gfsm_version_bincompat_min_check.micro);
    return FALSE;
  }
  else if (gfsm_version_compare(gfsm_version, hdr->version_min) < 0) {
    g_set_error(errp,
		g_quark_from_static_string("gfsm"),                              //-- domain
		g_quark_from_static_string("automaton_load_bin_header:version"), //-- code
		"libgfsm v%u.%u.%u is obsolete - stored automaton needs at least v%u.%u.%u",
		gfsm_version.major,
		gfsm_version.minor,
		gfsm_version.micro,
		hdr->version_min.major,
		hdr->version_min.minor,
		hdr->version_min.micro);
    return FALSE;
  }
  if (hdr->srtype == gfsmSRTUnknown || hdr->srtype >= gfsmSRTUser) {
    //-- compatibility hack
    hdr->srtype = gfsmAutomatonDefaultSRType;
  }
  return TRUE;
}

/*--------------------------------------------------------------
 * load_bin_file_0_0_8()
 *   + supports stored file versions v0.0.8 -- CURRENT
 */
gboolean gfsm_automaton_load_bin_file_0_0_8(gfsmAutomatonHeader *hdr, gfsmAutomaton *fsm, FILE *f, gfsmError **errp)
{
  gfsmStateId     id;
  guint           arci;
  gfsmStoredArc   s_arc;
  gfsmStoredState s_state;
  gfsmState       *st;
  gboolean         rc = TRUE;
  gfsmWeight       w;

  //-- allocate states
  gfsm_automaton_reserve(fsm, hdr->n_states);

  //-- set automaton-global properties
  fsm->flags   = hdr->flags;
  gfsm_semiring_init(fsm->sr, hdr->srtype);
  fsm->root_id = hdr->root_id;

  //------ load states (one-by-one)
  for (id=0; rc && id < hdr->n_states; id++) {
    if (fread(&s_state, sizeof(gfsmStoredState), 1, f) != 1) {
      g_set_error(errp,
		  g_quark_from_static_string("gfsm"),                     //-- domain
		  g_quark_from_static_string("automaton_load_bin:state"), //-- code
		  "could not read stored state %d", id);
      rc = FALSE;
      break;
    }

    if (!s_state.is_valid) continue;

    st           = gfsm_automaton_find_state(fsm,id);
    st->is_valid = TRUE;

    if (s_state.is_final) {
      //-- read final weight
      if (fread(&w, sizeof(gfsmWeight), 1, f) != 1) {
	g_set_error(errp,
		    g_quark_from_static_string("gfsm"),                                  //-- domain
		    g_quark_from_static_string("automaton_load_bin:state:final_weight"), //-- code
		    "could not read final weight for stored state %d", id);
	rc = FALSE;
	break;
      }

      //-- set final weight
      st->is_final = TRUE;
      gfsm_weightmap_insert(fsm->finals,(gpointer)id,w);
    } else {
      st->is_final = FALSE;
    }


    //-- read arcs (one-by-one)
    st->arcs = NULL;
    for (arci=0; arci < s_state.n_arcs; arci++) {
      if (fread(&s_arc, sizeof(gfsmStoredArc), 1, f) != 1) {
	g_set_error(errp, g_quark_from_static_string("gfsm"),                   //-- domain
		    g_quark_from_static_string("automaton_load_bin:state:arc"), //-- code
		    "could not read stored arcs for state %d", id);
	rc=FALSE;
	break;
      }
      if (!rc) break;

      st->arcs = gfsm_arclist_new_full(s_arc.target,
				       s_arc.lower,
				       s_arc.upper,
				       s_arc.weight,
				       st->arcs);
    }

    //-- reverse arc-list for sorted automata
    if (fsm->flags.sort_mode != gfsmASMNone) st->arcs = g_slist_reverse(st->arcs);
  }

  return rc;
}

/*--------------------------------------------------------------
 * load_bin_file_0_0_7()
 *   + supports stored file versions 0.0.2 -- 0.0.7
 */
/// Type for a stored state (v0.0.2 .. v0.0.7)
typedef struct {
  gboolean is_valid : 1; /**< valid flag */
  gboolean is_final : 1; /**< final flag */
  guint    n_arcs;       /**< number of stored arcs */
  guint    min_arc;      /**< index of stored minimum arc (not really necessary) */
} gfsmStoredState_007;

gboolean gfsm_automaton_load_bin_file_0_0_7(gfsmAutomatonHeader *hdr, gfsmAutomaton *fsm, FILE *f, gfsmError **errp)
{
  gfsmStateId     id;
  guint           arci, n_arcs;
  gfsmStoredArc   s_arc;
  gfsmStoredState_007 s_state;
  gfsmState       *st;
  gboolean         rc = TRUE;

  //-- allocate states
  gfsm_automaton_reserve(fsm, hdr->n_states);

  //-- set automaton-global properties
  fsm->flags   = hdr->flags;
  gfsm_semiring_init(fsm->sr, hdr->srtype);
  fsm->root_id = hdr->root_id;

  //------ load states (one-by-one)
  for (id=0; rc && id < hdr->n_states; id++) {
    if (fread(&s_state, sizeof(gfsmStoredState_007), 1, f) != 1) {
      g_set_error(errp,
		  g_quark_from_static_string("gfsm"),                     //-- domain
		  g_quark_from_static_string("automaton_load_bin:state"), //-- code
		  "could not read stored state %d", id);
      rc = FALSE;
      break;
    }

    if (!s_state.is_valid) continue;

    st           = gfsm_automaton_find_state(fsm,id);
    st->is_valid = TRUE;

    if (s_state.is_final) {
      st->is_final = TRUE;
      gfsm_weightmap_insert(fsm->finals,(gpointer)(id),fsm->sr->one);
    } else {
      st->is_final = FALSE;
    }

    //-- HACK: remember number of arcs!
    st->arcs = (gfsmArcList*)(s_state.n_arcs);
  }

  //------ load arcs (state-by-state)
  for (id=0; rc && id < hdr->n_states; id++) {
    //-- get state
    st = gfsm_automaton_find_state(fsm,id);
    if (!st || !st->is_valid) continue;

    //-- read in arcs (one-by-one)
    n_arcs   = (guint)(st->arcs);
    st->arcs = NULL;
    for (arci=0; arci < n_arcs; arci++) {
      if (fread(&s_arc, sizeof(gfsmStoredArc), 1, f) != 1) {
	g_set_error(errp, g_quark_from_static_string("gfsm"),             //-- domain
		    g_quark_from_static_string("automaton_load_bin:arc"), //-- code
		    "could not read stored arcs for state %d", id);
	rc=FALSE;
	break;
      }

      st->arcs = gfsm_arclist_new_full(s_arc.target,
				       s_arc.lower,
				       s_arc.upper,
				       s_arc.weight,
				       st->arcs);
    }

    //-- reverse arc-list for sorted automata
    if (fsm->flags.sort_mode != gfsmASMNone) st->arcs = g_slist_reverse(st->arcs);
  }

  return rc;
}

/*--------------------------------------------------------------
 * load_bin_file()
 *   + dispatch
 */
gboolean gfsm_automaton_load_bin_file(gfsmAutomaton *fsm, FILE *f, gfsmError **errp)
{
  gfsmAutomatonHeader hdr;

  gfsm_automaton_clear(fsm);

  //-- load header
  if (!gfsm_automaton_load_bin_header(&hdr,f,errp)) return FALSE;

  if (gfsm_version_ge(hdr.version,((gfsmVersionInfo){0,0,8}))) {
    //-- v0.0.8 .. CURRENT
    return gfsm_automaton_load_bin_file_0_0_8(&hdr,fsm,f,errp);
  }
  else {
    //-- v0.0.2 .. v0.0.7
    return gfsm_automaton_load_bin_file_0_0_7(&hdr,fsm,f,errp);
  }
}


/*--------------------------------------------------------------
 * load_bin_filename()
 */
gboolean gfsm_automaton_load_bin_filename(gfsmAutomaton *fsm, const gchar *filename, gfsmError **errp)
{
  FILE *f;
  gboolean rc;
  if (!(f=gfsm_open_filename(filename,"rb",errp))) return FALSE;
  rc = gfsm_automaton_load_bin_file(fsm, f, errp);
  if (f != stdin) fclose(f);
  return rc;
}


/*======================================================================
 * Methods: Binary I/O: save()
 */
/*--------------------------------------------------------------
 * save_bin_file()
 */
gboolean gfsm_automaton_save_bin_file(gfsmAutomaton *fsm, FILE *f, gfsmError **errp)
{
  gfsmAutomatonHeader hdr;
  gfsmStateId         id;
  gfsmState           *st;
  gfsmStoredState     sst;
  gfsmStoredArc       sa;
  gfsmWeight           w;
  gfsmArcIter         ai;
  gboolean            rc = TRUE;

  //-- create header
  memset(&hdr, 0, sizeof(gfsmAutomatonHeader));
  strcpy(hdr.magic, gfsm_header_magic);
  hdr.version     = gfsm_version;
  hdr.version_min = gfsm_version_bincompat_min_store;
  hdr.flags       = fsm->flags;
  hdr.root_id     = fsm->root_id;
  hdr.n_states    = gfsm_automaton_n_states(fsm);
  //hdr.n_arcs_007= gfsm_automaton_n_arcs(fsm);
  hdr.srtype      = gfsm_automaton_get_semiring(fsm)->type;

  //-- write header
  if (fwrite(&hdr, sizeof(gfsmAutomatonHeader), 1, f) != 1) {
    g_set_error(errp, g_quark_from_static_string("gfsm"),                      //-- domain
		      g_quark_from_static_string("automaton_save_bin:header"), //-- code
		      "could not store header");
    return FALSE;
  }

  //-- write states
  for (id=0; rc && id < hdr.n_states; id++) {
    //-- store basic state information
    st           = &g_array_index(fsm->states, gfsmState, id);
    sst.is_valid = st->is_valid;
    sst.is_final = st->is_final;
    sst.n_arcs   = gfsm_state_out_degree(st);
    if (fwrite(&sst, sizeof(sst), 1, f) != 1) {
      g_set_error(errp, g_quark_from_static_string("gfsm"),                      //-- domain
			g_quark_from_static_string("automaton_save_bin:state"), //-- code
			"could not store state %d", id);
      rc = FALSE;
    }

    //-- store final weight (maybe)
    if (rc && sst.is_final) {
      w = gfsm_automaton_get_final_weight(fsm,id);
      if (fwrite(&w, sizeof(gfsmWeight), 1, f) != 1) {
	g_set_error(errp, g_quark_from_static_string("gfsm"),                            //-- domain
		    g_quark_from_static_string("automaton_save_bin:state:final_weight"), //-- code
		    "could not store final weight for state %d", id);
	rc = FALSE;
      }
    }

    //-- store arcs
    for (gfsm_arciter_open_ptr(&ai,fsm,st); rc && gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      sa.target = a->target;
      sa.lower  = a->lower;
      sa.upper  = a->upper;
      sa.weight = a->weight;
      if (fwrite(&sa, sizeof(sa), 1, f) != 1) {
	g_set_error(errp, g_quark_from_static_string("gfsm"),                         //-- domain
			  g_quark_from_static_string("automaton_save_bin:state:arc"), //-- code
			  "could not store arcs for state %d", id);
	rc = FALSE;
      }
    }
  }

  return rc;
}

/*--------------------------------------------------------------
 * save_bin_filename()
 */
gboolean gfsm_automaton_save_bin_filename(gfsmAutomaton *fsm, const gchar *filename, gfsmError **errp)
{
  FILE *f;
  gboolean rc;
  if (!(f=gfsm_open_filename(filename,"wb",errp))) return FALSE;
  rc = gfsm_automaton_save_bin_file(fsm, f, errp);
  if (f != stdout) fclose(f);
  return rc;
}



/*======================================================================
 * Methods: Text I/O: compile()
 */

/*--------------------------------------------------------------
 * compile_file()
 */
gboolean gfsm_automaton_compile_file_full (gfsmAutomaton *fsm,
					   FILE          *f,
					   gfsmAlphabet  *lo_alphabet,
					   gfsmAlphabet  *hi_alphabet,
					   gfsmAlphabet  *state_alphabet,
					   gfsmError     **errp)
{
  gfsmStateId q1, q2;
  gfsmLabelId lo, hi;
  gfsmWeight  w;
  char       *buf = NULL;
  size_t      buflen = 0;
  guint       lineno = 1;
  char       *b1, *b2, *b3, *b4, *b5, *end;
  GString    *gs = g_string_new("");
  gpointer    key;
  gboolean    rc = TRUE;

  extern int getline(char **, size_t *, FILE *);
  for (lineno=1; rc && getline(&buf,&buflen,f) != EOF; ++lineno) {
    b1 = b2 = b3 = b4 = b5 = end = NULL;
    short int nfields = 0;

    //-- get pointers to initial elements of the line
    for (b1 = buf; b1 && *b1 &&  isspace(*b1); b1++) { *b1 = '\0'; }

    for (b2 = b1 ; b2 && *b2 && !isspace(*b2); b2++) ;
    for (        ; b2 && *b2 &&  isspace(*b2); b2++) { *b2 = '\0'; }
    if (b2 != b1) nfields = 1;
    else {
      //-- empty line
      continue;
    }

    for (b3 = b2 ; b3 && *b3 && !isspace(*b3); b3++) ;
    for (        ; b3 && *b3 &&  isspace(*b3); b3++) { *b3 = '\0'; }
    if (b3 != b2) nfields = 2;

    for (b4 = b3 ; b4 && *b4 && !isspace(*b4); b4++) ;
    for (        ; b4 && *b4 &&  isspace(*b4); b4++) { *b4 = '\0'; }
    if (b4 != b3) nfields = 3;

    for (b5 = b4 ; b5 && *b5 && !isspace(*b5); b5++) ;
    for (        ; b5 && *b5 &&  isspace(*b5); b5++) { *b5 = '\0'; }
    if (b5 != b4) nfields = 4;

    for (end = b5; end && *end && !isspace(*end); end++) ;
    if (end) *end = '\0';
    if (end != b5) nfields = 5;

    //---- q1: source state
    if (state_alphabet) {
      g_string_assign(gs,b1);
      key = gfsm_alphabet_string2key(state_alphabet, gs);
      if ((q1 = gfsm_alphabet_find_label(state_alphabet,key)) == gfsmNoLabel)
	q1 = gfsm_alphabet_get_label(state_alphabet, key);
    } else q1 = strtol(b1,NULL,10);
    if (fsm->root_id == gfsmNoState) fsm->root_id = q1;

    //-- weightless final state?
    if (nfields == 1) {
      gfsm_automaton_set_final_state(fsm,q1,TRUE);
      continue;
    }
    //-- weighted final state?
    else if (nfields == 2) {
      w = strtod(b2,NULL);
      gfsm_automaton_set_final_state_full(fsm,q1,TRUE,w);
      continue;
    }

    //---- q2: sink state
    if (state_alphabet) {
      g_string_assign(gs,b2);
      key = gfsm_alphabet_string2key(state_alphabet, gs);
      if ((q2 = gfsm_alphabet_find_label(state_alphabet,key)) == gfsmNoLabel)
	q2 = gfsm_alphabet_get_label(state_alphabet, key);
    } else q2 = strtol(b2,NULL,10);

    //---- lo: lower label
    if (lo_alphabet) {
      g_string_assign(gs,b3);
      key = gfsm_alphabet_string2key(lo_alphabet, gs);
      if ((lo = gfsm_alphabet_find_label(lo_alphabet,key)) == gfsmNoLabel)
	lo = gfsm_alphabet_get_label(lo_alphabet, key);
    } else lo = strtol(b3,NULL,10);

    //---- hi: upper label
    if (fsm->flags.is_transducer) {
      if (nfields > 3) {
	if (hi_alphabet) {
	  g_string_assign(gs,b4);
	  key = gfsm_alphabet_string2key(hi_alphabet, gs);
	  if ((hi = gfsm_alphabet_find_label(hi_alphabet,key)) == gfsmNoLabel)
	    hi = gfsm_alphabet_get_label(hi_alphabet, key);
	}
	else hi = strtol(b4,NULL,10);
      }
      else {
	g_printerr("gfsm: Warning: no upper label given for transducer at line %u - using lower label\n",
		   lineno);
	hi = lo;
      }
    }
    else {
      //-- not a transducer
      hi = lo;
      if (nfields > 4) {
	g_printerr("gfsm: Warning: ignoring extra fields in acceptor file at line %u\n", lineno);
      }
    }

    //-- w: arc weight
    if      ( fsm->flags.is_transducer && nfields >= 5) { w = strtod(b5,NULL); }
    else if (!fsm->flags.is_transducer && nfields >= 4) { w = strtod(b4,NULL); }
    else                                                { w = fsm->sr->one; }

    gfsm_automaton_add_arc(fsm,q1,q2,lo,hi,w);
  }

  free(buf);
  g_string_free(gs,TRUE);

  return rc;
}

/*--------------------------------------------------------------
 * compile_filename()
 */
gboolean gfsm_automaton_compile_filename_full (gfsmAutomaton *fsm,
					       const gchar   *filename,
					       gfsmAlphabet  *lo_alphabet,
					       gfsmAlphabet  *hi_alphabet,
					       gfsmAlphabet  *state_alphabet,
					       gfsmError     **errp)
{
  FILE *f;
  gboolean rc;
  if (!(f=gfsm_open_filename(filename,"r",errp))) return FALSE;
  rc = gfsm_automaton_compile_file_full(fsm, f, lo_alphabet, hi_alphabet, state_alphabet, errp);
  if (f != stdin) fclose(f);
  return rc;
}


/*======================================================================
 * Methods: Text I/O: print()
 */

/*--------------------------------------------------------------
 * print_file()
 */
gboolean gfsm_automaton_print_file_full (gfsmAutomaton *fsm,
					 FILE          *f,
					 gfsmAlphabet  *lo_alphabet,
					 gfsmAlphabet  *hi_alphabet,
					 gfsmAlphabet  *state_alphabet,
					 gfsmError     **errp)
{
  guint i;
  GString *gs = g_string_new("");
  gboolean rc = TRUE;

  if (fsm->root_id == gfsmNoState) rc = FALSE; //-- sanity check

  for (i=0; rc && i < fsm->states->len; i++) {
    guint        id = (fsm->root_id + i) % fsm->states->len;
    gfsmState   *st = gfsm_automaton_find_state(fsm,id);
    gfsmArcIter  ai;
    gpointer     key;
    if (!st || !st->is_valid) continue;

    for (gfsm_arciter_open_ptr(&ai,fsm,st); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);

      //-- source state
      if (state_alphabet && (key=gfsm_alphabet_find_key(state_alphabet,id)) != gfsmNoKey) {
	gfsm_alphabet_key2string(state_alphabet,key,gs);
	fputs(gs->str, f);
      } else {
	if (state_alphabet) g_printerr("Warning: no label defined for state '%u'!\n", id);
	fprintf(f, "%u", id);
      }
      fputc('\t',f);

      //-- sink state
      if (state_alphabet && (key=gfsm_alphabet_find_key(state_alphabet,a->target)) != gfsmNoKey) {
	gfsm_alphabet_key2string(state_alphabet,key,gs);
	fputs(gs->str,f);
      } else {
	if (state_alphabet) g_printerr("Warning: no label defined for state '%u'!\n", a->target);
	fprintf(f, "%u", a->target);
      }
      fputc('\t',f);

      //-- lower label
      if (lo_alphabet && (key=gfsm_alphabet_find_key(lo_alphabet,a->lower)) != gfsmNoKey) {
	gfsm_alphabet_key2string(lo_alphabet,key,gs);
	fputs(gs->str,f);
      } else {
	if (lo_alphabet) g_printerr("Warning: no lower label defined for Id '%u'!\n", a->lower);
	fprintf(f, "%u", a->lower);
      }

      //-- upper label
      if (fsm->flags.is_transducer) {
	fputc('\t',f);
	if (hi_alphabet && (key=gfsm_alphabet_find_key(hi_alphabet,a->upper)) != gfsmNoKey) {
	  gfsm_alphabet_key2string(hi_alphabet,key,gs);
	  fputs(gs->str,f);
	} else {
	  if (hi_alphabet) g_printerr("Warning: no upper label defined for Id '%u'!\n", a->upper);
	  fprintf(f, "%u", a->upper);
	}
      }

      //-- weight
      if (fsm->flags.is_weighted) { // && a->weight != fsm->sr->one
	fprintf(f, "\t%g", a->weight);
      }

      fputc('\n', f);
    }

    //-- final?
    if (gfsm_state_is_final(st)) {
      if (state_alphabet && (key=gfsm_alphabet_find_key(state_alphabet,id)) != NULL) {
	gfsm_alphabet_key2string(state_alphabet,key,gs);
	fputs(gs->str,f);
      } else {
	fprintf(f, "%u", id);
      }
      if (fsm->flags.is_weighted) {
	fprintf(f, "\t%g", gfsm_automaton_get_final_weight(fsm,id));
      }
      fputc('\n', f);
    }
  }

  //-- cleanup
  g_string_free(gs,TRUE);
  
  return rc;
}

/*--------------------------------------------------------------
 * print_filename()
 */
gboolean gfsm_automaton_print_filename_full (gfsmAutomaton *fsm,
					     const gchar   *filename,
					     gfsmAlphabet  *lo_alphabet,
					     gfsmAlphabet  *hi_alphabet,
					     gfsmAlphabet  *state_alphabet,
					     gfsmError     **errp)
{
  FILE *f;
  gboolean rc;
  if (!(f=gfsm_open_filename(filename, "w", errp))) return FALSE;
  rc = gfsm_automaton_print_file_full(fsm, f, lo_alphabet, hi_alphabet, state_alphabet, errp);
  if (f != stdout) fclose(f);
  return rc;
}

