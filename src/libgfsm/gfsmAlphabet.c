/*=============================================================================*\
 * File: gfsmAlphabet.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: finite state machine library: alphabet
 *
 * Copyright (c) 2004 Bryan Jurish.
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

#include <gfsmAlphabet.h>
#include <gfsmSet.h>
#include <gfsmUtils.h>
#include <gfsmError.h>

#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>

/*======================================================================
 * Constants
 */
gfsmUserAlphabetMethods gfsmUserAlphabetDefaultMethods =
  {
    NULL, //-- key_lookup
    NULL, //-- lab_lookup
    NULL, //-- insert
    //NULL, //-- key_remove
    NULL, //-- lab_remove
    NULL, //-- key_read
    NULL  //-- key_write
  };

/*======================================================================
 * Methods: Constructors
 */

/*--------------------------------------------------------------
 * new()
 */
gfsmAlphabet *gfsm_alphabet_new(gfsmAType type)
{
  gfsmAlphabet *a=NULL;
  switch (type) {
  case gfsmATIdentity:
    a = (gfsmAlphabet*)g_new0(gfsmIdentityAlphabet,1);
    break;
  case gfsmATPointer:
    a = (gfsmAlphabet*)g_new0(gfsmPointerAlphabet,1);
    break;
  case gfsmATUser:
    a = (gfsmAlphabet*)g_new0(gfsmUserAlphabet,1);
    break;
  case gfsmATString:
    a = (gfsmAlphabet*)g_new0(gfsmStringAlphabet,1);
    break;
  case gfsmATUnknown:
  case gfsmATRange:
  default:
    a = (gfsmAlphabet*)g_new0(gfsmRangeAlphabet,1);
    break;
  }
  a->type = type;
  a->lab_min = gfsmNoLabel;
  a->lab_max = gfsmNoLabel;
  return a;
}

/*--------------------------------------------------------------
 * string_new()
 */
/*gfsmAlphabet *gfsm_string_alphabet_new(void)
{
  return gfsm_string_alphabet_init(g_new(gfsmStringAlphabet,1));
  }
*/

/*--------------------------------------------------------------
 * init()
 */
gfsmAlphabet *gfsm_alphabet_init(gfsmAlphabet *a)
{
  if (!a) return NULL;

  a->lab_min = gfsmNoLabel;
  a->lab_max = gfsmNoLabel;

  switch (a->type) {
  case gfsmATIdentity:
    return gfsm_identity_alphabet_init((gfsmIdentityAlphabet*)a);
  case gfsmATPointer:
    return gfsm_pointer_alphabet_init((gfsmPointerAlphabet*)a,NULL,NULL,NULL,NULL);
  case gfsmATUser:
    return gfsm_user_alphabet_init((gfsmUserAlphabet*)a,NULL,NULL,NULL,NULL,
				   NULL,&gfsmUserAlphabetDefaultMethods);
  case gfsmATString:
    return gfsm_string_alphabet_init((gfsmStringAlphabet*)a,FALSE);
  case gfsmATUnknown:
  case gfsmATRange:
  default:
    break;
  }
  return a;
}

/*--------------------------------------------------------------
 * range_init()
 */
gfsmAlphabet *gfsm_range_alphabet_init (gfsmRangeAlphabet *a, gfsmLabelVal min, gfsmLabelVal max)
{
  a->lab_min = min;
  a->lab_max = max;
  return a;
}

/*--------------------------------------------------------------
 * identity_init()
 */
gfsmAlphabet *gfsm_identity_alphabet_init (gfsmIdentityAlphabet *a)
{
  gfsm_range_alphabet_init((gfsmRangeAlphabet*)a, gfsmNoLabel, gfsmNoLabel);
  if (!a->labels) a->labels = gfsm_set_new(gfsm_uint_compare);
  gfsm_set_clear(a->labels);
  return (gfsmAlphabet*)a;
}

/*--------------------------------------------------------------
 * pointer_init()
 */
gfsmAlphabet *gfsm_pointer_alphabet_init(gfsmPointerAlphabet *a,
					 gfsmAlphabetKeyDupFunc   key_dup_func,
					 GHashFunc      key_hash_func,
					 GEqualFunc     key_equal_func,
					 GDestroyNotify key_free_func)
{
  gfsm_range_alphabet_init((gfsmRangeAlphabet*)a,gfsmNoLabel,gfsmNoLabel);

  if (a->keys2labels) g_hash_table_destroy(a->keys2labels);
  if (a->labels2keys) g_ptr_array_free(a->labels2keys,TRUE);

  a->keys2labels  = g_hash_table_new_full(key_hash_func, key_equal_func, key_free_func, NULL);
  a->labels2keys  = g_ptr_array_new();
  a->key_dup_func = key_dup_func;

  return (gfsmAlphabet*)a;
}

/*--------------------------------------------------------------
 * string_init()
 */
gfsmAlphabet *gfsm_string_alphabet_init(gfsmStringAlphabet *a, gboolean do_copy)
{
  if (do_copy)
    return gfsm_pointer_alphabet_init(a,
				      (gfsmAlphabetKeyDupFunc)gfsm_alphabet_strdup,
				      g_str_hash, g_str_equal, g_free);
  return gfsm_pointer_alphabet_init(a, NULL, g_str_hash, g_str_equal, NULL);
}

/*--------------------------------------------------------------
 * user_init()
 */
gfsmAlphabet *gfsm_user_alphabet_init(gfsmUserAlphabet        *a,
				      gfsmAlphabetKeyDupFunc   key_dup_func,
				      GHashFunc                key_hash_func,
				      GEqualFunc               key_equal_func,
				      GDestroyNotify           key_destroy_func,
				      gpointer                 user_data,
				      gfsmUserAlphabetMethods *methods)
{
  gfsm_pointer_alphabet_init((gfsmPointerAlphabet*)a,
			     key_dup_func,
			     key_hash_func,
			     key_equal_func,
			     key_destroy_func);
  a->data    = user_data;
  a->methods = methods ? (*methods) : gfsmUserAlphabetDefaultMethods;
  return (gfsmAlphabet*)a;
}

/*--------------------------------------------------------------
 * clear()
 */
void gfsm_alphabet_clear(gfsmAlphabet *a)
{
  switch (a->type) {
  case gfsmATUnknown:
  case gfsmATRange:
    break;
  case gfsmATIdentity:
    gfsm_set_clear(((gfsmIdentityAlphabet*)a)->labels);
    break;
  case gfsmATPointer:
  case gfsmATString:
    g_ptr_array_set_size(((gfsmPointerAlphabet*)a)->labels2keys,0);
    g_hash_table_foreach_remove(((gfsmPointerAlphabet*)a)->keys2labels, gfsm_hash_clear_func, NULL);
    break;
  case gfsmATUser:
  default:
    gfsm_alphabet_foreach(a, gfsm_alphabet_foreach_remove_func, NULL);
    break;
  }

  a->lab_min = gfsmNoLabel;
  a->lab_max = gfsmNoLabel;
}

/*--------------------------------------------------------------
 * gfsm_alphabet_foreach_remove_func()
 */
gboolean gfsm_alphabet_foreach_remove_func(gfsmAlphabet *a,
					   gpointer      key,
					   gfsmLabelVal   lab,
					   gpointer      data)
{
  gfsm_alphabet_remove_label(a,lab);
  return FALSE;
}

/*--------------------------------------------------------------
 * free()
 */
void gfsm_alphabet_free(gfsmAlphabet *a)
{
  switch (a->type) {
  case gfsmATIdentity:
    gfsm_set_free(((gfsmIdentityAlphabet*)a)->labels);
    g_free((gfsmIdentityAlphabet*)a);
    return;
  case gfsmATUser:
  case gfsmATPointer:
  case gfsmATString:
    g_ptr_array_free(((gfsmPointerAlphabet*)a)->labels2keys,TRUE);
    g_hash_table_destroy(((gfsmPointerAlphabet*)a)->keys2labels);
    g_free((gfsmPointerAlphabet*)a);
    return;
  case gfsmATUnknown:
  case gfsmATRange:
  default:
    break;
  }
  g_free(a);
};

/*======================================================================
 * Methods: Utilties
 */

/*--------------------------------------------------------------
 * gfsm_alphabet_foreach()
 */
gboolean gfsm_alphabet_foreach (gfsmAlphabet            *a,
				gfsmAlphabetForeachFunc  func,
				gpointer                 data)
{
  gfsmLabelVal lab;
  gpointer     key;
  gboolean     rc=FALSE;
  for (lab = a->lab_min; !rc && lab <= a->lab_max && lab < gfsmNoLabel; lab++) {
    if ((key=gfsm_alphabet_find_key(a,lab))==gfsmNoKey) continue;
    rc  = (*func)(a,key,lab,data);
  }
  return rc;
}

/*--------------------------------------------------------------
 * strdup()
 */
gpointer gfsm_alphabet_strdup(gfsmAlphabet *a, const gchar *str)
{ return g_strdup(str); }

/*======================================================================
 * Methods: Accessors
 */

/*--------------------------------------------------------------
 * size()
 */
gfsmLabelVal gfsm_alphabet_size(gfsmAlphabet *a)
{
  guint n=0;
  switch (a->type) {
  case gfsmATIdentity:
    return gfsm_set_size(((gfsmIdentityAlphabet*)a)->labels);
  case gfsmATUser:
    gfsm_alphabet_foreach(a, (gfsmAlphabetForeachFunc)gfsm_alphabet_foreach_size_func, &n);
    return (gfsmLabelVal)n;
  case gfsmATPointer:
  case gfsmATString:
    return ((gfsmPointerAlphabet*)a)->labels2keys->len;
  case gfsmATUnknown:
  case gfsmATRange:
  default:
    return (a->lab_min != gfsmNoLabel && a->lab_max != gfsmNoLabel 
	    ? (a->lab_max - a->lab_min)
	    : 0);
  }
  return n;
}

/*--------------------------------------------------------------
 * foreach_size_func()
 */
gboolean gfsm_alphabet_foreach_size_func(gfsmAlphabet *a,
					 gpointer      key,
					 gfsmLabelVal  lab,
					 guint        *np)
{
  if (key != gfsmNoKey && lab != gfsmNoLabel) ++(*np);
  return FALSE;
}

/*--------------------------------------------------------------
 * insert()
 */
gfsmLabelVal gfsm_alphabet_insert(gfsmAlphabet *a, gpointer key, gfsmLabelVal label)
{
  switch (a->type) {

  case gfsmATIdentity:
    gfsm_set_insert(((gfsmIdentityAlphabet*)a)->labels, key);
    label = (gfsmLabelVal)key;
    break;

  case gfsmATUser:
    if (((gfsmUserAlphabet*)a)->methods.insert) {
      if (((gfsmPointerAlphabet*)a)->key_dup_func) {
	key = (*(((gfsmPointerAlphabet*)a)->key_dup_func))(((gfsmPointerAlphabet*)a),key);
      }
      (*(((gfsmUserAlphabet*)a)->methods.insert))((gfsmUserAlphabet*)a, key, label);
      break;
    }

  case gfsmATPointer:
  case gfsmATString:
    if (label==gfsmNoLabel)
      label = ((gfsmPointerAlphabet*)a)->labels2keys->len;

    if (label >= ((gfsmPointerAlphabet*)a)->labels2keys->len)
      g_ptr_array_set_size(((gfsmPointerAlphabet*)a)->labels2keys, label+1);

    if (((gfsmPointerAlphabet*)a)->key_dup_func)
      key = (*(((gfsmPointerAlphabet*)a)->key_dup_func))(((gfsmPointerAlphabet*)a),key);

    g_ptr_array_index(((gfsmPointerAlphabet*)a)->labels2keys, label) = key;
    g_hash_table_insert(((gfsmPointerAlphabet*)a)->keys2labels, key, (gpointer)(label));
    break;

  case gfsmATUnknown:
  case gfsmATRange:
  default:
    break;
  }

  //-- range
  if (label != gfsmNoLabel) {
    if (a->lab_min==gfsmNoLabel || label < a->lab_min) a->lab_min = label;
    if (a->lab_max==gfsmNoLabel || label > a->lab_max) a->lab_max = label;
  }

  return label;
}

/*--------------------------------------------------------------
 * get_full()
 */
gfsmLabelVal gfsm_alphabet_get_full(gfsmAlphabet *a, gpointer key, gfsmLabelVal label)
{
  gfsmLabelVal l = gfsm_alphabet_find_label(a,key);
  if (l != gfsmNoLabel) {
    //-- old mapping exists
    if (label == gfsmNoLabel) return l;  //-- ... but no new mapping was requested
    gfsm_alphabet_remove_label(a,l);
  }
  return gfsm_alphabet_insert(a,key,label);
}

/*--------------------------------------------------------------
 * find_label()
 */
gfsmLabelVal gfsm_alphabet_find_label (gfsmAlphabet *a, gconstpointer key)
{
  gpointer k, l;

  switch (a->type) {

  case gfsmATIdentity:
    if (gfsm_set_contains(((gfsmIdentityAlphabet*)a)->labels, key))
      return (gfsmLabelVal)((guint)key);
    break;

  case gfsmATUser:
    if (((gfsmUserAlphabet*)a)->methods.key_lookup)
      return (*(((gfsmUserAlphabet*)a)->methods.key_lookup))((gfsmUserAlphabet*)a, key);

  case gfsmATPointer:
  case gfsmATString:
    if ( g_hash_table_lookup_extended(((gfsmPointerAlphabet*)a)->keys2labels, key, &k, &l) )
      return (gfsmLabelVal)((guint)l);
    break;

  case gfsmATUnknown:
  case gfsmATRange:
  default:
    return ( ((gfsmLabelVal)((guint)key)) >= a->lab_min
	       &&
	     ((gfsmLabelVal)((guint)key)) <= a->lab_max 
	     ? ((gfsmLabelVal)((guint)key))
	     : gfsmNoLabel );
  }

  return gfsmNoLabel;
}

/*--------------------------------------------------------------
 * find_key
 */
gpointer gfsm_alphabet_find_key(gfsmAlphabet *a, gfsmLabelVal label)
{
  switch (a->type) {

  case gfsmATIdentity:
    if ( gfsm_set_contains(((gfsmIdentityAlphabet*)a)->labels, (gpointer)(label)) )
      return (gpointer)(label);
    break;

  case gfsmATUser:
    if (((gfsmUserAlphabet*)a)->methods.lab_lookup)
      return (*(((gfsmUserAlphabet*)a)->methods.lab_lookup))((gfsmUserAlphabet*)a, label);

  case gfsmATPointer:
  case gfsmATString:
    if (label < ((gfsmPointerAlphabet*)a)->labels2keys->len)
      return g_ptr_array_index(((gfsmPointerAlphabet*)a)->labels2keys,label);
    break;

  case gfsmATUnknown:
  case gfsmATRange:
  default:
    if (label >= a->lab_min && label <= a->lab_max)
      return (gpointer)(label);
  }

  return gfsmNoKey;
}

/*--------------------------------------------------------------
 * get_key()
 */
gpointer gfsm_alphabet_get_key(gfsmAlphabet *a, gfsmLabelVal label)
{
  gpointer key;
  if (label == gfsmNoLabel) return gfsmNoKey;

  key = gfsm_alphabet_find_key(a,label);
  if (key != gfsmNoKey) return key;
  gfsm_alphabet_get_full(a, gfsmNoKey, label);

  return gfsmNoKey;
}

/*--------------------------------------------------------------
 * remove_key()
 */
void gfsm_alphabet_remove_key(gfsmAlphabet *a, gconstpointer key)
{
  gfsmLabelVal label;

  switch (a->type) {

  case gfsmATIdentity:
    gfsm_set_remove(((gfsmIdentityAlphabet*)a)->labels, key);
    break;
  
  case gfsmATUser:
    if (((gfsmUserAlphabet*)a)->methods.lab_remove) {
      label = gfsm_alphabet_find_label(a,key);
      (*(((gfsmUserAlphabet*)a)->methods.lab_remove))((gfsmUserAlphabet*)a, label);
      break;
    }

  case gfsmATPointer:
  case gfsmATString:
    label = gfsm_alphabet_find_label(a,key);
    g_hash_table_remove(((gfsmPointerAlphabet*)a)->keys2labels,key);
    if (label != gfsmNoLabel && label < ((gfsmPointerAlphabet*)a)->labels2keys->len) {
      g_ptr_array_index(((gfsmPointerAlphabet*)a)->labels2keys, label) = NULL;
    }
    break;
  
  case gfsmATUnknown:
  case gfsmATRange:
  default:
    break;
  }

  //-- ranges
  //(missing)
}

/*--------------------------------------------------------------
 * remove_label()
 */
void gfsm_alphabet_remove_label(gfsmAlphabet *a, gfsmLabelVal label)
{
  gpointer key;

  switch (a->type) {
  case gfsmATIdentity:
    gfsm_set_remove(((gfsmIdentityAlphabet*)a)->labels, (gpointer)(label));
    break;
  
  case gfsmATUser:
    if (((gfsmUserAlphabet*)a)->methods.lab_remove) {
      (*(((gfsmUserAlphabet*)a)->methods.lab_remove))((gfsmUserAlphabet*)a, label);
      break;
    }

  case gfsmATPointer:
  case gfsmATString:
    if (label < ((gfsmPointerAlphabet*)a)->labels2keys->len) {
      key = g_ptr_array_index(((gfsmPointerAlphabet*)a)->labels2keys,label);
      g_ptr_array_index(((gfsmPointerAlphabet*)a)->labels2keys,label) = NULL;
      g_hash_table_remove(((gfsmPointerAlphabet*)a)->keys2labels,key);
    }
    break;
  
  case gfsmATUnknown:
  case gfsmATRange:
  default:
    break;
  }
}

/*--------------------------------------------------------------
 * union()
 */
gfsmAlphabet *gfsm_alphabet_union(gfsmAlphabet *a1, gfsmAlphabet *a2)
{
  gfsm_alphabet_foreach(a2, (gfsmAlphabetForeachFunc)gfsm_alphabet_foreach_union_func, a1);
  return a1;
}

/*--------------------------------------------------------------
 * union_func()
 */
gboolean gfsm_alphabet_foreach_union_func(gfsmAlphabet *src,
					  gpointer      src_key,
					  gfsmLabelVal   src_id,
					  gfsmAlphabet *dst)
{
  gfsm_alphabet_get_label(dst,src_key);      
  return FALSE;
}

/*--------------------------------------------------------------
 * gfsm_alphabet_labels_to_array_func()
 */
gboolean gfsm_alphabet_labels_to_array_func(gfsmAlphabet *alph,
					    gpointer      key,
					    gfsmLabelVal  lab,
					    GPtrArray    *ary)
{
  //g_array_append_val(ary, lab);
  g_ptr_array_add(ary, (gpointer)lab);
  return FALSE;
}

/*--------------------------------------------------------------
 * gfsm_alphabet_labels_to_array()
 */
void gfsm_alphabet_labels_to_array(gfsmAlphabet *alph, GPtrArray *ary)
{
  gfsm_alphabet_foreach(alph,
			(gfsmAlphabetForeachFunc)gfsm_alphabet_labels_to_array_func,
			ary);
  //return ary;
}

/*======================================================================
 * Methods: I/O
 */

/*--------------------------------------------------------------
 * string2key()
 */
gpointer gfsm_alphabet_string2key(gfsmAlphabet *a, GString *gstr)
{
  gpointer key=NULL;

  switch (a->type) {
  
  case gfsmATUser:
    if (((gfsmUserAlphabet*)a)->methods.key_read) {
      key = (*(((gfsmUserAlphabet*)a)->methods.key_read))((gfsmUserAlphabet*)a, gstr);
      break;
    }

  case gfsmATPointer:
  case gfsmATString:
    key = gstr->str;
    break;
  
  case gfsmATUnknown:
  case gfsmATRange:
  case gfsmATIdentity:
  default:
    key = (gpointer)strtol(gstr->str,NULL,10);
    break;
  }
  return key;
}

/*--------------------------------------------------------------
 * key2string()
 */
void gfsm_alphabet_key2string(gfsmAlphabet *a, gpointer key, GString *gstr)
{
  switch (a->type) {
  
  case gfsmATUser:
    if (((gfsmUserAlphabet*)a)->methods.key_write) {
      (*(((gfsmUserAlphabet*)a)->methods.key_write))((gfsmUserAlphabet*)a, key, gstr);
      break;
    }

  case gfsmATPointer:
    //-- ?
  case gfsmATString:
    g_string_assign(gstr,key);
    break;

  case gfsmATUnknown:
  case gfsmATRange:
  case gfsmATIdentity:
  default:
    g_string_printf(gstr,"%u",(guint)key);
    break;
  }
}

/*--------------------------------------------------------------
 * load_file()
 */
gboolean gfsm_alphabet_load_file (gfsmAlphabet *a, FILE *f, gfsmError **errp)
{
  char c;
  gpointer    key;
  gfsmLabelVal label;
  GString *s_key = g_string_new("");
  GString *s_lab = g_string_new("");

  //if (!myname) myname = "gfsm_string_alphabet_load_file()";

  while (!feof(f)) {
    g_string_truncate(s_key,0);
    g_string_truncate(s_lab,0);

    //-- read data fields into temp strings
    while ((c = getc(f)) != EOF && isspace(c)) ; if (c==EOF) break;
    for(g_string_append_c(s_key,c); (c = getc(f)) != EOF && !isspace(c); ) {
      g_string_append_c(s_key,c);
    }
    while ((c = getc(f)) != EOF && isspace(c)) ; if (c==EOF) break;
    for(g_string_append_c(s_lab,c); (c = getc(f)) != EOF && !isspace(c); ) {
      g_string_append_c(s_lab,c);
    }
    while (c != '\n' && (c = getc(f)) != EOF) ;

    //-- get actual key and label
    key   = gfsm_alphabet_string2key(a,s_key);
    label = strtol(s_lab->str, NULL, 10);
    if (gfsm_alphabet_find_label(a,key) != label) {
      gfsm_alphabet_remove_key(a, key);
      gfsm_alphabet_insert(a, key, label);
    }
  }
  //-- cleanup
  g_string_free(s_key,TRUE);
  g_string_free(s_lab,TRUE);
  return TRUE;
}

/*--------------------------------------------------------------
 * load_filename()
 */
gboolean gfsm_alphabet_load_filename (gfsmAlphabet *a, const gchar *filename, gfsmError **errp)
{
  FILE *f;
  gboolean rc;
  if (!(f = gfsm_open_filename(filename, "r", errp))) return FALSE;
  rc = gfsm_alphabet_load_file(a, f, errp);
  if (f != stdin) fclose(f);
  return rc;
}

/*--------------------------------------------------------------
 * save_filename()
 */
gboolean gfsm_alphabet_save_filename (gfsmAlphabet *a, const gchar *filename, gfsmError **errp)
{
  FILE *f;
  gboolean rc;
  if (!(f=gfsm_open_filename(filename,"w",errp))) return FALSE;
  rc = gfsm_alphabet_save_file(a, f, errp);
  if (f != stdout) fclose(f);
  return rc;
}

/*--------------------------------------------------------------
 * save_file()
 */
gboolean gfsm_alphabet_save_file(gfsmAlphabet *a, FILE *f, gfsmError **errp)
{
  gfsmSaveFileData sfdata;
  gboolean rc;
  sfdata.file = f;
  sfdata.errp = errp;
  sfdata.gstr = g_string_new("");
  sfdata.field_sep = "\t";
  sfdata.record_sep = "\n";

  //-- guts
  rc = gfsm_alphabet_foreach(a, (gfsmAlphabetForeachFunc)gfsm_alphabet_save_file_func, &sfdata);

  //-- cleanup
  g_string_free(sfdata.gstr,TRUE);

  return !rc;
}

/*--------------------------------------------------------------
 * save_file_func()
 */
gboolean gfsm_alphabet_save_file_func(gfsmAlphabet     *a,
				      gpointer          key,
				      gfsmLabelVal      lab,
				      gfsmSaveFileData *sfdata)
{
  gfsm_alphabet_key2string(a,key,sfdata->gstr);
  fprintf(sfdata->file,
	  "%s%s%u%s",
	  sfdata->gstr->str, sfdata->field_sep, lab, sfdata->record_sep);
  return (sfdata->errp && *(sfdata->errp));
}
