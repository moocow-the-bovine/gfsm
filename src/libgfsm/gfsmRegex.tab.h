/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_GFSMREGEX_YY_GFSMREGEX_TAB_H_INCLUDED
# define YY_GFSMREGEX_YY_GFSMREGEX_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int gfsmRegex_yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    TOK_UNKNOWN = 258,
    TOK_CHAR = 259,
    TOK_UINT = 260,
    TOK_STRING = 261,
    TOK_WEIGHT = 262,
    CONCAT = 263,
    LABCONCAT = 264,
    WEIGHT = 265
  };
#endif
/* Tokens.  */
#define TOK_UNKNOWN 258
#define TOK_CHAR 259
#define TOK_UINT 260
#define TOK_STRING 261
#define TOK_WEIGHT 262
#define CONCAT 263
#define LABCONCAT 264
#define WEIGHT 265

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 37 "gfsmRegex.tab.y" /* yacc.c:1909  */

   gfsmAutomaton *fsm; //-- automaton
   GString       *gs;  //-- needs to be freed by hand
   gchar          c;
   guint32        u;
   gfsmWeight     w;

#line 82 "gfsmRegex.tab.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int gfsmRegex_yyparse (gfsmRegexCompiler* reparser);

#endif /* !YY_GFSMREGEX_YY_GFSMREGEX_TAB_H_INCLUDED  */
