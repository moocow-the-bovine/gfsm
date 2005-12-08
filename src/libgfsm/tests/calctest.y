/*======================================================================
 * Bison Options
 */
%pure_parser

%{
/*======================================================================
 * Bison C Header
 */
#include <stdio.h>
#include "calctest.tab.h"
#include "calctest.lex.h"

extern void calctest_yyerror(const char *msg);

#define YYLEX_PARAM   scanner
#define YYPARSE_PARAM scanner

yyscan_t scanner;
%}

/*======================================================================
 * Bison Definitions
 */
%union {
    double dbl;
}

%token <dbl> NUMBER PLUS MINUS TIMES DIV LPAREN RPAREN NEWLINE

%type  <dbl> expr exprs

%left PLUS MINUS
%left TIMES DIV
%nonassoc UMINUS

/*======================================================================
 * Bison Rules
 */
%%

exprs:	        /* empty */ { $$=0; }
	|	exprs expr NEWLINE
		{ printf("%g\n", $2); $$=0; }
	;

expr:		NUMBER              { $$=$1; }
	|	LPAREN expr RPAREN  { $$=$2; }
	|	MINUS expr          { $$=-$2; }
	|	expr TIMES expr     { $$=$1*$3; }
	|	expr DIV expr       { $$=$1/$3; }
	|	expr PLUS expr      { $$=$1+$3; }
	|	expr MINUS expr  %prec UMINUS { $$=$1-$3; } 
	;

%%

/*======================================================================
 * User C Code
 */

void calctest_yyerror(const char *msg)
{
    fprintf(stderr, "yyerror: %s\n", msg);
}

int main (void) {

  calctest_yylex_init(&scanner); //-- initialize reentrant flex scanner

  calctest_yyparse(scanner);

  calctest_yylex_destroy(scanner);   //-- cleanup reentrant flex scanner
  
  return 0;
}
