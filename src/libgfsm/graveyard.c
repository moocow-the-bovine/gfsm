#define GFSM_ALPHABET_CSET \
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!\"#$%&'()*+,-./0123456789:;<=>?" \
  "\241\242\243\244\245\246\247\250\251\252\253\254\255\256\257\260\261\262\263\264" \
  "\265\266\267\270\271\272\273\274\275\276\277\300\301\302\303\304\305\306\307\310" \
  "\311\312\313\314\315\316\317\320\321\322\323\324\325\326\327\330\331\332\333\334" \
  "\335\336\337\340\341\342\343\344\345\346\347\350\351\352\353\354\355\356\357\360"

const GScannerConfig gfsm_alphabet_scanner_config = {
  /* Character sets */
  ( " \t\r\n" )             /* skip chars */,
  ( GFSM_ALPHABET_CSET )    /* identifier_first */,
  ( GFSM_ALPHABET_CSET )    /* identifier_nth */,
  "" 		            /* comment_single */,

  FALSE                     /* case_sensitive */, 
  FALSE                     /* skip_comment_multi */,
  FALSE                     /* skip_comment_single */,
  FALSE                     /* scan_comment_multi */,
  FALSE                     /* scan_comment_single */,
  TRUE                      /* scan_identifier */,
  TRUE                      /* scan_identifier_1char */,
  FALSE                     /* scan_identifier_NULL */,
  FALSE                     /* scan_symbols */,

  FALSE                     /* scan_binary */,
  TRUE                      /* scan_octal */,
  FALSE                     /* scan_float */,
  TRUE                      /* scan_hex */,
  FALSE                     /* scan_hex_dollar */,

  FALSE                     /* scan_string_sq : string: 'anything' */,
  FALSE 		    /* scan_string_dq : string: "\\-escapes!\n" */,
  TRUE                      /* numbers_2_int : bin, octal, hex => int */,
  FALSE		            /* int_2_float : int => G_TOKEN_FLOAT? */,
  FALSE 		    /* identifier_2_string : 1 */,
  FALSE                     /* 2_token : 1 : return G_TOKEN_CHAR? */,
  FALSE		            /* symbol_2_token : 1 */,
  FALSE                     /* scope_0_fallback : try scope 0 on lookups? */,
  //FALSE                     /* store_int64 : use value.v_int64 rather than v_int */,
  //0                         /* padding_dummy; */
};

gboolean gfsm_automaton_compile_file (gfsmAutomaton *fsm, FILE *f, const gchar *filename)
{
  GScanner *scanner = g_scanner_new(&gfsm_automaton_scanner_config);
  GTokenType typ;

  g_scanner_input_file(scanner, fileno(f));
  while ((typ = g_scanner_get_next_token(scanner)) != G_TOKEN_EOF) {
    g_printerr("<DEBUG:SCANNER:%u.%u> typ=%d : ",
	       g_scanner_cur_line(scanner), g_scanner_cur_position(scanner), typ);

    switch (typ) {
    case G_TOKEN_INT:
      g_printerr("(INT) value=%ld\n", g_scanner_cur_value(scanner).v_int);
      break;

    case G_TOKEN_FLOAT:
      g_printerr("(FLOAT) value=%g\n", g_scanner_cur_value(scanner).v_float);
      break;

    case G_TOKEN_CHAR:
      g_printerr("(CHAR) value=%d\n", g_scanner_cur_value(scanner).v_char);
      break;

    default:
      g_printerr("(?) text='%s'\n", scanner->text); //'text' is private: debugging use only!
      break;
    }
  }
  return TRUE;
}


/*--------------------------------------------------------------
 * scanner template for compile_file()
 */
const GScannerConfig gfsm_automaton_scanner_config = {
  /* Character sets */
  ( " \t\r" )             /* skip chars */,
  ( "" )                  /* identifier_first */,
  ( "" )                  /* identifier_nth */,
  "#\n" 	          /* comment_single */,

  FALSE                     /* case_sensitive */, 

  FALSE                     /* skip_comment_multi */,
  FALSE                     /* skip_comment_single */,
  FALSE                     /* scan_comment_multi */,

  FALSE                     /* scan_identifier */,
  FALSE                     /* scan_identifier_1char */,
  FALSE                     /* scan_identifier_NULL */,
  FALSE                     /* scan_symbols */,

  FALSE                     /* scan_binary */,
  TRUE                      /* scan_octal */, 
  TRUE                      /* scan_float */,
  TRUE                      /* scan_hex */,
  FALSE                     /* scan_hex_dollar */,

  FALSE                     /* scan_string_sq : string: 'anything' */,
  FALSE 		    /* scan_string_dq : string: "\\-escapes!\n" */,
  TRUE                      /* numbers_2_int : bin, octal, hex => int */,
  FALSE		            /* int_2_float : int => G_TOKEN_FLOAT? */,
  FALSE 		    /* identifier_2_string : 1 */,
  FALSE                     /* 2_token : 1 : return G_TOKEN_CHAR? */,
  FALSE		            /* symbol_2_token : 1 */,
  FALSE                     /* scope_0_fallback : try scope 0 on lookups? */,
  FALSE                     /* store_int64 : use value.v_int64 rather than v_int */,
  //0                         /* padding_dummy; */
};

/*--------------------------------------------------------------
 * gfsm_automaton_scan_field()
 */
gboolean gfsm_automaton_scan_field(GScanner  *scanner,
				   GTokenType expect1,
				   GTokenType expect2,
				   gboolean   complain_on_newline)
{
  GTokenType typ;
  while (1) {
    typ = g_scanner_get_next_token(scanner);
    if (expect1 != typ
	&& expect2 != typ
	&& (typ == G_TOKEN_CHAR || typ == G_TOKEN_EOF)
	&& g_scanner_cur_value(scanner).v_char == '\n')
      {
	//-- return but don't complain on unexpected newlines
	if (complain_on_newline)
	  g_scanner_unexp_token(scanner, expect1, NULL, NULL, NULL, NULL, TRUE);
	return FALSE;
      }

    if (expect1 != typ && expect2 != typ) {
      g_scanner_unexp_token(scanner, expect1, NULL, NULL, NULL, NULL, TRUE);
      return FALSE;
    }
    return TRUE;
  }
  return FALSE;
}
