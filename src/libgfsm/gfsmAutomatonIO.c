
/*=============================================================================*\
 * File: gfsmAutomatonIO.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: automata: I/O
 *
 * Copyright (c) 2004 Bryan Jurish.
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See file LICENSE for further informations on licensing terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *=============================================================================*/

#include <gfsmAutomatonIO.h>
#include <gfsmArcIter.h>
#include <gfsmUtils.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>


/*======================================================================
 * Constants: Binary I/O
 */
const gfsmVersionInfo gfsm_version_bincompat_min =
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
  else if (hdr->version.major < gfsm_version_bincompat_min.major
	   || hdr->version.minor < gfsm_version_bincompat_min.minor
	   || hdr->version.micro < gfsm_version_bincompat_min.micro) {
    g_set_error(errp,
		g_quark_from_static_string("gfsm"),                              //-- domain
		g_quark_from_static_string("automaton_load_bin_header:version"), //-- code
		"stored format v%u.%u.%u is obsolete - need at least v%u.%u.%u",
		hdr->version.major,
		hdr->version.minor,
		hdr->version.micro,
		gfsm_version_bincompat_min.major,
		gfsm_version_bincompat_min.minor,
		gfsm_version_bincompat_min.micro);
    return FALSE;
  }
  else if (hdr->version_min.major > gfsm_version.major
	   || hdr->version_min.minor > gfsm_version.minor
	   || hdr->version_min.micro > gfsm_version.micro) {
    g_set_error(errp,
		g_quark_from_static_string("gfsm"),                              //-- domain
		g_quark_from_static_string("automaton_load_bin_header:version"), //-- code
		"libgfsm v%u.%u.%u is obsolete - stored file needs least v%u.%u.%u",
		gfsm_version.major,
		gfsm_version.minor,
		gfsm_version.micro,
		hdr->version_min.major,
		hdr->version_min.minor,
		hdr->version_min.micro);
  }
  if (hdr->srtype == gfsmSRTUnknown || hdr->srtype >= gfsmSRTUser) {
    //-- compatibility hack
    hdr->srtype = gfsmAutomatonDefaultSRType;
  }
  return TRUE;
}

/*--------------------------------------------------------------
 * load_bin_file()
 */
gboolean gfsm_automaton_load_bin_file(gfsmAutomaton *fsm, FILE *f, gfsmError **errp)
{
  gfsmAutomatonHeader hdr;
  gfsmStateId     id;
  guint           arci;
  gfsmStoredArc   *s_arcs;
  gfsmStoredState *s_states;
  gfsmState       *st;
  gfsmStoredState *sst;
  gboolean         rc = TRUE;

  gfsm_automaton_clear(fsm);

  //-- load header
  if (!gfsm_automaton_load_bin_header(&hdr,f,errp)) return FALSE;

  //-- allocate states
  gfsm_automaton_reserve(fsm, hdr.n_states);

  //-- set automaton-global properties
  fsm->flags   = hdr.flags;
  gfsm_semiring_init(fsm->sr, hdr.srtype);
  fsm->root_id = hdr.root_id;

  //-- allocate space for stored arcs and states
  s_arcs   = g_new(gfsmStoredArc,   hdr.n_arcs);
  s_states = g_new(gfsmStoredState, hdr.n_states);

  //-- load state and arc vectors
  if (fread(s_states, sizeof(gfsmStoredState), hdr.n_states, f) != hdr.n_states)
    {
      g_set_error(errp,
		  g_quark_from_static_string("gfsm"),                    //-- domain
		  g_quark_from_static_string("automaton_load_bin:states"), //-- code
		  "could not read stored states");
      rc = FALSE;
    }
  else if (fread(s_arcs, sizeof(gfsmStoredArc), hdr.n_arcs, f) != hdr.n_arcs)
    {
      g_set_error(errp, g_quark_from_static_string("gfsm"),                    //-- domain
			g_quark_from_static_string("automaton_load_bin:arcs"), //-- code
			"could not read stored arcs");
      rc = FALSE;
    }

  //-- load state-by-state
  for (id=0; rc && id < hdr.n_states; id++) {
    sst          = s_states + id;
    if (!sst->is_valid) continue;

    st           = gfsm_automaton_find_state(fsm,id);
    st->is_valid = TRUE;

    if (sst->is_final) {
      //gfsm_automaton_set_final_state(fsm,id,TRUE);
      st->is_final = TRUE;
      gfsm_set_insert(fsm->finals,(gpointer)(id));
    } else {
      st->is_final = FALSE;
    }

    st->arcs = NULL;
    for (arci = sst->n_arcs; arci > 0; arci--) {
      gfsmStoredArc *sa = s_arcs + arci + sst->min_arc - 1;
      gfsmArcList   *al;
      al = gfsm_arclist_new_full(sa->target,
				 sa->lower,
				 sa->upper,
				 sa->weight,
				 st->arcs);
      st->arcs = al;
      //gfsm_automaton_add_arc_ptr(fsm, st, al);
    }
  }

  //-- cleanup
  g_free(s_arcs);
  g_free(s_states);

  return rc;
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
  guint               arcid = 0;
  gboolean            rc = TRUE;

  //-- create header
  memset(&hdr, 0, sizeof(gfsmAutomatonHeader));
  strcpy(hdr.magic, gfsm_header_magic);
  hdr.version     = gfsm_version;
  hdr.version_min = gfsm_version_bincompat_min;
  hdr.flags       = fsm->flags;
  hdr.root_id     = fsm->root_id;
  hdr.n_states    = gfsm_automaton_n_states(fsm);
  hdr.n_arcs      = gfsm_automaton_n_arcs(fsm);
  hdr.srtype      = gfsm_automaton_get_semiring(fsm)->type;

  //-- write header
  if (fwrite(&hdr, sizeof(gfsmAutomatonHeader), 1, f) != 1) {
    g_set_error(errp, g_quark_from_static_string("gfsm"),                      //-- domain
		      g_quark_from_static_string("automaton_save_bin:header"), //-- code
		      "could not store header");
    return FALSE;
  }

  //-- write states
  for (id=0; rc &&id < hdr.n_states; id++) {
    st           = &g_array_index(fsm->states, gfsmState, id);
    sst.is_valid = st->is_valid;
    sst.is_final = st->is_final;
    sst.n_arcs   = gfsm_state_out_degree(st);
    sst.min_arc  = arcid;
    arcid       += sst.n_arcs;
    if (fwrite(&sst, sizeof(sst), 1, f) != 1) {
      g_set_error(errp, g_quark_from_static_string("gfsm"),                      //-- domain
			g_quark_from_static_string("automaton_save_bin:states"), //-- code
			"could not store states");
      rc = FALSE;
    }
  }
  //-- write arcs
  for (id=0; rc && id < hdr.n_states; id++) {
    gfsmArcIter ai;
    for (gfsm_arciter_open(&ai,fsm,id); gfsm_arciter_ok(&ai); gfsm_arciter_next(&ai)) {
      gfsmArc *a = gfsm_arciter_arc(&ai);
      sa.target = a->target;
      sa.lower  = a->lower;
      sa.upper  = a->upper;
      sa.weight = a->weight;
      if (fwrite(&sa, sizeof(sa), 1, f) != 1) {
	g_set_error(errp, g_quark_from_static_string("gfsm"),                      //-- domain
			  g_quark_from_static_string("automaton_save_bin:states"), //-- code
			  "could not store arcs");
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
      if (w != 0)
	g_printerr("gfsm: Warning: final weights are unsupported at line %u\n", lineno);

      gfsm_automaton_set_final_state(fsm,q1,TRUE);
      continue;
    }
    else if (nfields == 3) {
      g_printerr("gfsm: Warning: no upper label given (using <epsilon>) at line %u\n", lineno);
      hi = 0;
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
    if (hi_alphabet) {
      g_string_assign(gs,b4);
      key = gfsm_alphabet_string2key(hi_alphabet, gs);
      if ((hi = gfsm_alphabet_find_label(hi_alphabet,key)) == gfsmNoLabel)
	hi = gfsm_alphabet_get_label(hi_alphabet, key);
    } else hi = strtol(b4,NULL,10);

    if (nfields == 5)
      w = strtod(b5,NULL);     //-- w : arc weight
    else w = 0;

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
    gfsmState   *st = gfsm_automaton_get_state(fsm,id);
    gfsmArcIter  ai;
    gpointer     key;
    if (!gfsm_state_is_ok(st)) continue;

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
      fputc('\t',f);

      //-- upper label
      if (hi_alphabet && (key=gfsm_alphabet_find_key(hi_alphabet,a->upper)) != gfsmNoKey) {
	gfsm_alphabet_key2string(hi_alphabet,key,gs);
	fputs(gs->str,f);
      } else {
	if (hi_alphabet) g_printerr("Warning: no upper label defined for Id '%u'!\n", a->upper);
	fprintf(f, "%u", a->upper);
      }
      fputc('\t',f);

      //-- weight
      fprintf(f, "%g\n", a->weight);
    }

    //-- final?
    if (gfsm_state_is_final(st)) {
      if (state_alphabet && (key=gfsm_alphabet_find_key(state_alphabet,id)) != NULL) {
	gfsm_alphabet_key2string(state_alphabet,key,gs);
	fputs(gs->str,f);
      } else {
	fprintf(f, "%u", id);
      }
      fputs("\t0\n",f);
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

