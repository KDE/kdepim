/* A Bison parser, made by GNU Bison 1.875.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     EQ = 258,
     COLON = 259,
     DOT = 260,
     SEMICOLON = 261,
     SPACE = 262,
     HTAB = 263,
     LINESEP = 264,
     NEWLINE = 265,
     BEGIN_VCARD = 266,
     END_VCARD = 267,
     BEGIN_VCAL = 268,
     END_VCAL = 269,
     BEGIN_VEVENT = 270,
     END_VEVENT = 271,
     BEGIN_VTODO = 272,
     END_VTODO = 273,
     ID = 274,
     STRING = 275
   };
#endif
#define EQ 258
#define COLON 259
#define DOT 260
#define SEMICOLON 261
#define SPACE 262
#define HTAB 263
#define LINESEP 264
#define NEWLINE 265
#define BEGIN_VCARD 266
#define END_VCARD 267
#define BEGIN_VCAL 268
#define END_VCAL 269
#define BEGIN_VEVENT 270
#define END_VEVENT 271
#define BEGIN_VTODO 272
#define END_VTODO 273
#define ID 274
#define STRING 275




/* Copy the first part of user declarations.  */
#line 1 "vcc.y"


/***************************************************************************
(C) Copyright 1996 Apple Computer, Inc., AT&T Corp., International             
Business Machines Corporation and Siemens Rolm Communications Inc.             
                                                                               
For purposes of this license notice, the term Licensors shall mean,            
collectively, Apple Computer, Inc., AT&T Corp., International                  
Business Machines Corporation and Siemens Rolm Communications Inc.             
The term Licensor shall mean any of the Licensors.                             
                                                                               
Subject to acceptance of the following conditions, permission is hereby        
granted by Licensors without the need for written agreement and without        
license or royalty fees, to use, copy, modify and distribute this              
software for any purpose.                                                      
                                                                               
The above copyright notice and the following four paragraphs must be           
reproduced in all copies of this software and any software including           
this software.                                                                 
                                                                               
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS AND NO LICENSOR SHALL HAVE       
ANY OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS OR       
MODIFICATIONS.                                                                 
                                                                               
IN NO EVENT SHALL ANY LICENSOR BE LIABLE TO ANY PARTY FOR DIRECT,              
INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES OR LOST PROFITS ARISING OUT         
OF THE USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH         
DAMAGE.                                                                        
                                                                               
EACH LICENSOR SPECIFICALLY DISCLAIMS ANY WARRANTIES, EXPRESS OR IMPLIED,       
INCLUDING BUT NOT LIMITED TO ANY WARRANTY OF NONINFRINGEMENT OR THE            
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR             
PURPOSE.                                                                       

The software is provided with RESTRICTED RIGHTS.  Use, duplication, or         
disclosure by the government are subject to restrictions set forth in          
DFARS 252.227-7013 or 48 CFR 52.227-19, as applicable.                         

***************************************************************************/

/*
 * src: vcc.c
 * doc: Parser for vCard and vCalendar. Note that this code is
 * generated by a yacc parser generator. Generally it should not
 * be edited by hand. The real source is vcc.y. The #line directives
 * can be commented out here to make it easier to trace through
 * in a debugger. However, if a bug is found it should 
 * be fixed in vcc.y and this file regenerated.
 */


/* debugging utilities */
#if __DEBUG
#define DBG_(x) printf x
#else
#define DBG_(x)
#endif

/****  External Functions  ****/

/* assign local name to parser variables and functions so that
   we can use more than one yacc based parser.
*/

#define yyparse mime_parse
#define yylex mime_lex
#define yyerror mime_error
#define yychar mime_char
/* #define p_yyval p_mime_val */
#undef yyval
#define yyval mime_yyval
/* #define p_yylval p_mime_lval */
#undef yylval
#define yylval mime_yylval
#define yydebug mime_debug
#define yynerrs mime_nerrs
#define yyerrflag mime_errflag
#define yyss mime_ss
#define yyssp mime_ssp
#define yyvs mime_vs
#define yyvsp mime_vsp
#define yylhs mime_lhs
#define yylen mime_len
#define yydefred mime_defred
#define yydgoto mime_dgoto
#define yysindex mime_sindex
#define yyrindex mime_rindex
#define yygindex mime_gindex
#define yytable mime_table
#define yycheck mime_check
#define yyname mime_name
#define yyrule mime_rule
#undef YYPREFIX
#define YYPREFIX "mime_"


#ifndef _NO_LINE_FOLDING
#define _SUPPORT_LINE_FOLDING 1
#endif

#include <string.h>
#if !defined(__FreeBSD__) && !defined(__APPLE__)
#include <malloc.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "vcc.h"

/* The following is a hack that I hope will get things compiling
 * on SunOS 4.1.x systems 
 */
#ifndef SEEK_SET
#define SEEK_SET        0       /* Seek from beginning of file.  */
#define SEEK_CUR        1       /* Seek from current position.  */
#define SEEK_END        2       /* Seek from end of file.  */
#endif

/****  Types, Constants  ****/

#define YYDEBUG		0	/* 1 to compile in some debugging code */
#define MAXTOKEN	256	/* maximum token (line) length */
#define YYSTACKSIZE 	1000	/* ~unref ? */
#define MAXLEVEL	10	/* max # of nested objects parseable */
				/* (includes outermost) */


/****  Global Variables  ****/
int mime_lineNum, mime_numErrors; /* yyerror() can use these */
static VObject* vObjList;
static VObject *curProp;
static VObject *curObj;
static VObject* ObjStack[MAXLEVEL];
static int ObjStackTop;


/* A helpful utility for the rest of the app. */
#if __CPLUSPLUS__
extern "C" {
#endif

  /*    static void Parse_Debug(const char *s);*/
    static void yyerror(char *s);

#if __CPLUSPLUS__
    };
#endif

int yyparse();
static int yylex();

static VObject* popVObject();


enum LexMode {
	L_NORMAL,
	L_VCARD,
	L_VCAL,
	L_VEVENT,
	L_VTODO,
	L_VALUES,
	L_BASE64,
	L_QUOTED_PRINTABLE
	};

/****  Private Forward Declarations  ****/
static int pushVObject(const char *prop);
static VObject* popVObject();
char* lexDataFromBase64();
static void lexPopMode(int top);
static int lexWithinMode(enum LexMode mode);
static void lexPushMode(enum LexMode mode);
static void enterProps(const char *s);
static void enterAttr(const char *s1, const char *s2);
/* static void enterValues(const char *value); */
static void appendValue(const char *value);
static void mime_error_(char *s);



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 185 "vcc.y"
typedef union YYSTYPE {
    char *str;
    VObject *vobj;
    } YYSTYPE;
/* Line 191 of yacc.c.  */
#line 300 "vcc.tab.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 312 "vcc.tab.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  12
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   56

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  21
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  31
/* YYNRULES -- Number of rules. */
#define YYNRULES  47
/* YYNRULES -- Number of states. */
#define YYNSTATES  62

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   275

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned char yyprhs[] =
{
       0,     0,     3,     5,     6,    10,    12,    14,    16,    17,
      22,    23,    27,    30,    32,    33,    39,    41,    42,    46,
      48,    51,    53,    56,    58,    62,    64,    65,    70,    72,
      74,    75,    76,    81,    82,    86,    89,    91,    93,    95,
      97,    98,   103,   104,   108,   109,   114,   115
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      22,     0,    -1,    23,    -1,    -1,    25,    24,    23,    -1,
      25,    -1,    26,    -1,    41,    -1,    -1,    11,    27,    29,
      12,    -1,    -1,    11,    28,    12,    -1,    30,    29,    -1,
      30,    -1,    -1,    32,     4,    31,    38,     9,    -1,     1,
      -1,    -1,    37,    33,    34,    -1,    37,    -1,    35,    34,
      -1,    35,    -1,     6,    36,    -1,    37,    -1,    37,     3,
      37,    -1,    19,    -1,    -1,    40,     6,    39,    38,    -1,
      40,    -1,    20,    -1,    -1,    -1,    13,    42,    44,    14,
      -1,    -1,    13,    43,    14,    -1,    45,    44,    -1,    45,
      -1,    46,    -1,    49,    -1,    29,    -1,    -1,    15,    47,
      29,    16,    -1,    -1,    15,    48,    16,    -1,    -1,    17,
      50,    29,    18,    -1,    -1,    17,    51,    18,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   213,   213,   217,   216,   219,   223,   224,   229,   228,
     239,   238,   250,   251,   255,   254,   264,   268,   267,   272,
     278,   279,   282,   285,   289,   296,   299,   299,   300,   304,
     306,   311,   310,   316,   315,   321,   322,   326,   327,   328,
     333,   332,   344,   343,   357,   356,   368,   367
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "EQ", "COLON", "DOT", "SEMICOLON", "SPACE", 
  "HTAB", "LINESEP", "NEWLINE", "BEGIN_VCARD", "END_VCARD", "BEGIN_VCAL", 
  "END_VCAL", "BEGIN_VEVENT", "END_VEVENT", "BEGIN_VTODO", "END_VTODO", 
  "ID", "STRING", "$accept", "mime", "vobjects", "@1", "vobject", "vcard", 
  "@2", "@3", "items", "item", "@4", "prop", "@5", "attr_params", 
  "attr_param", "attr", "name", "values", "@6", "value", "vcal", "@7", 
  "@8", "calitems", "calitem", "eventitem", "@9", "@10", "todoitem", 
  "@11", "@12", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    21,    22,    24,    23,    23,    25,    25,    27,    26,
      28,    26,    29,    29,    31,    30,    30,    33,    32,    32,
      34,    34,    35,    36,    36,    37,    39,    38,    38,    40,
      40,    42,    41,    43,    41,    44,    44,    45,    45,    45,
      47,    46,    48,    46,    50,    49,    51,    49
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     0,     3,     1,     1,     1,     0,     4,
       0,     3,     2,     1,     0,     5,     1,     0,     3,     1,
       2,     1,     2,     1,     3,     1,     0,     4,     1,     1,
       0,     0,     4,     0,     3,     2,     1,     1,     1,     1,
       0,     4,     0,     3,     0,     4,     0,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       0,     8,    31,     0,     2,     3,     6,     7,     0,     0,
       0,     0,     1,     0,    16,    25,     0,     0,     0,    17,
      11,    40,    44,    39,     0,     0,    37,    38,    34,     4,
       9,    12,    14,     0,     0,     0,     0,     0,    32,    35,
      30,     0,    18,    21,     0,    43,     0,    47,    29,     0,
      28,    22,    23,    20,    41,    45,    15,    26,     0,    30,
      24,    27
};

/* YYDEFGOTO[NTERM-NUM]. */
static const yysigned_char yydefgoto[] =
{
      -1,     3,     4,    13,     5,     6,     8,     9,    23,    17,
      40,    18,    33,    42,    43,    51,    19,    49,    59,    50,
       7,    10,    11,    24,    25,    26,    34,    35,    27,    36,
      37
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -18
static const yysigned_char yypact[] =
{
      -5,   -10,   -11,     5,   -18,    10,   -18,   -18,     3,    -1,
      12,    16,   -18,    -5,   -18,   -18,    20,     0,    29,    30,
     -18,    19,    18,   -18,    23,     6,   -18,   -18,   -18,   -18,
     -18,   -18,   -18,    32,     3,    24,     3,    21,   -18,   -18,
      22,    25,   -18,    32,    27,   -18,    28,   -18,   -18,    36,
      41,   -18,    45,   -18,   -18,   -18,   -18,   -18,    25,    22,
     -18,   -18
};

/* YYPGOTO[NTERM-NUM].  */
static const yysigned_char yypgoto[] =
{
     -18,   -18,    37,   -18,   -18,   -18,   -18,   -18,    -8,   -18,
     -18,   -18,   -18,     8,   -18,   -18,   -17,    -7,   -18,   -18,
     -18,   -18,   -18,    31,   -18,   -18,   -18,   -18,   -18,   -18,
     -18
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -47
static const yysigned_char yytable[] =
{
      16,    14,   -10,   -33,    14,    12,     1,    14,     2,    31,
      -5,    20,   -13,    14,   -13,   -13,   -13,   -13,   -13,    15,
     -36,    21,    15,    22,    52,    15,    44,    21,    46,    22,
      28,    15,    30,    32,   -19,   -42,   -46,    38,    41,    47,
      45,    60,    48,    54,    15,    56,    55,    57,    58,     0,
      29,    53,    61,     0,     0,     0,    39
};

static const yysigned_char yycheck[] =
{
       8,     1,    12,    14,     1,     0,    11,     1,    13,    17,
       0,    12,    12,     1,    14,    15,    16,    17,    18,    19,
      14,    15,    19,    17,    41,    19,    34,    15,    36,    17,
      14,    19,    12,     4,     4,    16,    18,    14,     6,    18,
      16,    58,    20,    16,    19,     9,    18,     6,     3,    -1,
      13,    43,    59,    -1,    -1,    -1,    25
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    11,    13,    22,    23,    25,    26,    41,    27,    28,
      42,    43,     0,    24,     1,    19,    29,    30,    32,    37,
      12,    15,    17,    29,    44,    45,    46,    49,    14,    23,
      12,    29,     4,    33,    47,    48,    50,    51,    14,    44,
      31,     6,    34,    35,    29,    16,    29,    18,    20,    38,
      40,    36,    37,    34,    16,    18,     9,     6,     3,    39,
      37,    38
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrlab1

/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)         \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (cinluded).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylineno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylineno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 3:
#line 217 "vcc.y"
    { addList(&vObjList, yyvsp[0].vobj); curObj = 0; ;}
    break;

  case 5:
#line 220 "vcc.y"
    { addList(&vObjList, yyvsp[0].vobj); curObj = 0; ;}
    break;

  case 8:
#line 229 "vcc.y"
    {
	lexPushMode(L_VCARD);
	if (!pushVObject(VCCardProp)) YYERROR;
	;}
    break;

  case 9:
#line 234 "vcc.y"
    {
	lexPopMode(0);
	yyval.vobj = popVObject();
	;}
    break;

  case 10:
#line 239 "vcc.y"
    {
	lexPushMode(L_VCARD);
	if (!pushVObject(VCCardProp)) YYERROR;
	;}
    break;

  case 11:
#line 244 "vcc.y"
    {
	lexPopMode(0);
	yyval.vobj = popVObject();
	;}
    break;

  case 14:
#line 255 "vcc.y"
    {
	lexPushMode(L_VALUES);
	;}
    break;

  case 15:
#line 259 "vcc.y"
    {
	if (lexWithinMode(L_BASE64) || lexWithinMode(L_QUOTED_PRINTABLE))
	   lexPopMode(0);
	lexPopMode(0);
	;}
    break;

  case 17:
#line 268 "vcc.y"
    {
	enterProps(yyvsp[0].str);
	;}
    break;

  case 19:
#line 273 "vcc.y"
    {
	enterProps(yyvsp[0].str);
	;}
    break;

  case 23:
#line 286 "vcc.y"
    {
	enterAttr(yyvsp[0].str,0);
	;}
    break;

  case 24:
#line 290 "vcc.y"
    {
	enterAttr(yyvsp[-2].str,yyvsp[0].str);

	;}
    break;

  case 26:
#line 299 "vcc.y"
    { appendValue(yyvsp[-1].str); ;}
    break;

  case 28:
#line 301 "vcc.y"
    { appendValue(yyvsp[0].str); ;}
    break;

  case 30:
#line 306 "vcc.y"
    { yyval.str = 0; ;}
    break;

  case 31:
#line 311 "vcc.y"
    { if (!pushVObject(VCCalProp)) YYERROR; ;}
    break;

  case 32:
#line 314 "vcc.y"
    { yyval.vobj = popVObject(); ;}
    break;

  case 33:
#line 316 "vcc.y"
    { if (!pushVObject(VCCalProp)) YYERROR; ;}
    break;

  case 34:
#line 318 "vcc.y"
    { yyval.vobj = popVObject(); ;}
    break;

  case 40:
#line 333 "vcc.y"
    {
	lexPushMode(L_VEVENT);
	if (!pushVObject(VCEventProp)) YYERROR;
	;}
    break;

  case 41:
#line 339 "vcc.y"
    {
	lexPopMode(0);
	popVObject();
	;}
    break;

  case 42:
#line 344 "vcc.y"
    {
	lexPushMode(L_VEVENT);
	if (!pushVObject(VCEventProp)) YYERROR;
	;}
    break;

  case 43:
#line 349 "vcc.y"
    {
	lexPopMode(0);
	popVObject();
	;}
    break;

  case 44:
#line 357 "vcc.y"
    {
	lexPushMode(L_VTODO);
	if (!pushVObject(VCTodoProp)) YYERROR;
	;}
    break;

  case 45:
#line 363 "vcc.y"
    {
	lexPopMode(0);
	popVObject();
	;}
    break;

  case 46:
#line 368 "vcc.y"
    {
	lexPushMode(L_VTODO);
	if (!pushVObject(VCTodoProp)) YYERROR;
	;}
    break;

  case 47:
#line 373 "vcc.y"
    {
	lexPopMode(0);
	popVObject();
	;}
    break;


    }

/* Line 991 of yacc.c.  */
#line 1429 "vcc.tab.c"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("syntax error, unexpected ") + 1;
	  yysize += yystrlen (yytname[yytype]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        {
	  /* Pop the error token.  */
          YYPOPSTACK;
	  /* Pop the rest of the stack.  */
	  while (yyss < yyssp)
	    {
	      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
	      yydestruct (yystos[*yyssp], yyvsp);
	      YYPOPSTACK;
	    }
	  YYABORT;
        }

      YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
      yydestruct (yytoken, &yylval);
      yychar = YYEMPTY;

    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab2;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:

  /* Suppress GCC warning that yyerrlab1 is unused when no action
     invokes YYERROR.  */
#if defined (__GNUC_MINOR__) && 2093 <= (__GNUC__ * 1000 + __GNUC_MINOR__) \
    && !defined __cplusplus
  __attribute__ ((__unused__))
#endif


  goto yyerrlab2;


/*---------------------------------------------------------------.
| yyerrlab2 -- pop states until the error token can be shifted.  |
`---------------------------------------------------------------*/
yyerrlab2:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      yyvsp--;
      yystate = *--yyssp;

      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 379 "vcc.y"

/****************************************************************************/
static int pushVObject(const char *prop)
    {
    VObject *newObj;
    if (ObjStackTop == MAXLEVEL)
	return FALSE;

    ObjStack[++ObjStackTop] = curObj;

    if (curObj) {
        newObj = addProp(curObj,prop);
        curObj = newObj;
	}
    else
	curObj = newVObject(prop);

    return TRUE;
    }


/****************************************************************************/
/* This pops the recently built vCard off the stack and returns it. */
static VObject* popVObject()
    {
    VObject *oldObj;
    if (ObjStackTop < 0) {
	yyerror("pop on empty Object Stack\n");
	return 0;
	}
    oldObj = curObj;
    curObj = ObjStack[ObjStackTop--];

    return oldObj;
    }


/* static void enterValues(const char *value) */
/*     { */
/*     if (fieldedProp && *fieldedProp) { */
/* 	if (value) { */
/* 	    addPropValue(curProp,*fieldedProp,value); */
/* 	    } */
 	/* else this field is empty, advance to next field */ 
/* 	fieldedProp++; */
/* 	} */
/*     else { */
/* 	if (value) { */
/* 	    setVObjectUStringZValue_(curProp,fakeUnicode(value,0)); */
/* 	    } */
/* 	} */
/*     deleteStr(value); */
/*     } */

static void appendValue(const char *value)
{
  char *p1, *p2;
  wchar_t *p3;
  int i;

  if (fieldedProp && *fieldedProp) {
    if (value) {
      addPropValue(curProp, *fieldedProp, value);
    }
    /* else this field is empty, advance to next field */
    fieldedProp++;
  } else {
    if (value) {
      if (vObjectUStringZValue(curProp)) {
	p1 = fakeCString(vObjectUStringZValue(curProp));
	p2 = malloc(sizeof(char *) * (strlen(p1)+strlen(value)+1));
	strcpy(p2, p1);
	deleteStr(p1);

	i = strlen(p2);
	p2[i] = ',';
	p2[i+1] = '\0';
	p2 = strcat(p2, value);
	p3 = (wchar_t *) vObjectUStringZValue(curProp);
	free(p3);
	setVObjectUStringZValue_(curProp,fakeUnicode(p2,0));
	deleteStr(p2);
      } else {
	setVObjectUStringZValue_(curProp,fakeUnicode(value,0));
      }
    }
  }
  deleteStr(value);
}
      

static void enterProps(const char *s)
    {
    curProp = addGroup(curObj,s);
    deleteStr(s);
    }

static void enterAttr(const char *s1, const char *s2)
    {
    const char *p1=0L, *p2=0L;
    p1 = lookupProp_(s1);
    if (s2) {
	VObject *a;
	p2 = lookupProp_(s2);
	a = addProp(curProp,p1);
	setVObjectStringZValue(a,p2);
	}
    else
	addProp(curProp,p1);
    if (strcasecmp(p1,VCBase64Prop) == 0 || (s2 && strcasecmp(p2,VCBase64Prop)==0))
	lexPushMode(L_BASE64);
    else if (strcasecmp(p1,VCQuotedPrintableProp) == 0
	    || (s2 && strcasecmp(p2,VCQuotedPrintableProp)==0))
	lexPushMode(L_QUOTED_PRINTABLE);
    deleteStr(s1); deleteStr(s2);
    }


#define MAX_LEX_LOOKAHEAD_0 32
#define MAX_LEX_LOOKAHEAD 64
#define MAX_LEX_MODE_STACK_SIZE 10
#define LEXMODE() (lexBuf.lexModeStack[lexBuf.lexModeStackTop])

struct LexBuf {
	/* input */
    FILE *inputFile;
    char *inputString;
    unsigned long curPos;
    unsigned long inputLen;
	/* lookahead buffer */
	/*   -- lookahead buffer is short instead of char so that EOF
	 /      can be represented correctly.
	*/
    unsigned long len;
    short buf[MAX_LEX_LOOKAHEAD];
    unsigned long getPtr;
	/* context stack */
    unsigned long lexModeStackTop;
    enum LexMode lexModeStack[MAX_LEX_MODE_STACK_SIZE];
	/* token buffer */
    unsigned long maxToken;
    char *strs;
    unsigned long strsLen;
    } lexBuf;

static void lexPushMode(enum LexMode mode)
    {
    if (lexBuf.lexModeStackTop == (MAX_LEX_MODE_STACK_SIZE-1))
	yyerror("lexical context stack overflow");
    else {
	lexBuf.lexModeStack[++lexBuf.lexModeStackTop] = mode;
	}
    }

static void lexPopMode(int top)
    {
    /* special case of pop for ease of error recovery -- this
	version will never underflow */
    if (top)
	lexBuf.lexModeStackTop = 0;
    else
	if (lexBuf.lexModeStackTop > 0) lexBuf.lexModeStackTop--;
    }

static int lexWithinMode(enum LexMode mode) {
    unsigned long i;
    for (i=0;i<lexBuf.lexModeStackTop;i++)
	if (mode == lexBuf.lexModeStack[i]) return 1;
    return 0;
    }

static int lexGetc_()
    {
    /* get next char from input, no buffering. */
    if (lexBuf.curPos == lexBuf.inputLen)
	return EOF;
    else if (lexBuf.inputString)
	return *(lexBuf.inputString + lexBuf.curPos++);
    else {
	if (!feof(lexBuf.inputFile))
	  return fgetc(lexBuf.inputFile);
	else
	  return EOF;
	}
    }

static int lexGeta()
    {
    ++lexBuf.len;
    return (lexBuf.buf[lexBuf.getPtr] = lexGetc_());
    }

static int lexGeta_(int i)
    {
    ++lexBuf.len;
    return (lexBuf.buf[(lexBuf.getPtr+i)%MAX_LEX_LOOKAHEAD] = lexGetc_());
    }

static void lexSkipLookahead() {
    if (lexBuf.len > 0 && lexBuf.buf[lexBuf.getPtr]!=EOF) {
	/* don't skip EOF. */
        lexBuf.getPtr = (lexBuf.getPtr + 1) % MAX_LEX_LOOKAHEAD;
	lexBuf.len--;
        }
    }

static int lexLookahead() {
    int c = (lexBuf.len)?
	lexBuf.buf[lexBuf.getPtr]:
	lexGeta();
    /* do the \r\n -> \n or \r -> \n translation here */
    if (c == '\r') {
	int a = (lexBuf.len>1)?
	    lexBuf.buf[(lexBuf.getPtr+1)%MAX_LEX_LOOKAHEAD]:
	    lexGeta_(1);
	if (a == '\n') {
	    lexSkipLookahead();
	    }
	lexBuf.buf[lexBuf.getPtr] = c = '\n';
	}
    else if (c == '\n') {
	int a;
	if (lexBuf.len > 1)
	  a = lexBuf.buf[lexBuf.getPtr];
	else
	  a = lexGeta_(1);
	if (a == '\r') {
	    lexSkipLookahead();
	    }
	lexBuf.buf[lexBuf.getPtr] = '\n';
	}
    return c;
    }

static int lexGetc() {
    int c = lexLookahead();
    if (lexBuf.len > 0 && lexBuf.buf[lexBuf.getPtr]!=EOF) {
	/* EOF will remain in lookahead buffer */
        lexBuf.getPtr = (lexBuf.getPtr + 1) % MAX_LEX_LOOKAHEAD;
	lexBuf.len--;
        }
    return c;
    }

static void lexSkipLookaheadWord() {
    if (lexBuf.strsLen <= lexBuf.len) {
	lexBuf.len -= lexBuf.strsLen;
	lexBuf.getPtr = (lexBuf.getPtr + lexBuf.strsLen) % MAX_LEX_LOOKAHEAD;
	}
    }

static void lexClearToken()
    {
    lexBuf.strsLen = 0;
    }

static void lexAppendc(int c)
    {
    /* not sure if I am doing this right to fix purify report  -- PGB */
    lexBuf.strs = (char *) realloc(lexBuf.strs, (size_t) lexBuf.strsLen + 1);  
    lexBuf.strs[lexBuf.strsLen] = c;
    /* append up to zero termination */
    if (c == 0) return;
    lexBuf.strsLen++;
    if (lexBuf.strsLen > lexBuf.maxToken) {
	/* double the token string size */
	lexBuf.maxToken <<= 1;
	lexBuf.strs = (char*) realloc(lexBuf.strs,(size_t)lexBuf.maxToken);
	}
    }

static char* lexStr() {
    return dupStr(lexBuf.strs,(size_t)lexBuf.strsLen+1);
    }

static void lexSkipWhite() {
    int c = lexLookahead();
    while (c == ' ' || c == '\t') {
	lexSkipLookahead();
	c = lexLookahead();
	}
    }

static char* lexGetWord() {
    int c;
    lexSkipWhite();
    lexClearToken();
    c = lexLookahead();
    /* some "words" have a space in them, like "NEEDS ACTION".
       this may be an oversight of the spec, but it is true nevertheless.
       while (c != EOF && !strchr("\t\n ;:=",c)) { */
    while (c != EOF && !strchr("\n;:=",c)) {
	lexAppendc(c);
	lexSkipLookahead();
	c = lexLookahead();
	}
    lexAppendc(0);
    return lexStr();
    }

void lexPushLookahead(char *s, int len) {
    int putptr;
    if (len == 0) len = strlen(s);
    putptr = (int)lexBuf.getPtr - len;
    /* this function assumes that length of word to push back
     /  is not greater than MAX_LEX_LOOKAHEAD.
     */
    if (putptr < 0) putptr += MAX_LEX_LOOKAHEAD;
    lexBuf.getPtr = putptr;
    while (*s) {
	lexBuf.buf[putptr] = *s++;
	putptr = (putptr + 1) % MAX_LEX_LOOKAHEAD;
	}
    lexBuf.len += len;
    }

static void lexPushLookaheadc(int c) {
    int putptr;
    /* can't putback EOF, because it never leaves lookahead buffer */
    if (c == EOF) return;
    putptr = (int)lexBuf.getPtr - 1;
    if (putptr < 0) putptr += MAX_LEX_LOOKAHEAD;
    lexBuf.getPtr = putptr;
    lexBuf.buf[putptr] = c;
    lexBuf.len += 1;
    }

static char* lexLookaheadWord() {
    /* this function can lookahead word with max size of MAX_LEX_LOOKAHEAD_0
     /  and thing bigger than that will stop the lookahead and return 0;
     / leading white spaces are not recoverable.
     */
    int c;
    int len = 0;
    int curgetptr = 0;
    lexSkipWhite();
    lexClearToken();
    curgetptr = (int)lexBuf.getPtr;	/* remember! */
    while (len < (MAX_LEX_LOOKAHEAD_0)) {
	c = lexGetc();
	len++;
	if (c == EOF || strchr("\t\n ;:=", c)) {
	    lexAppendc(0);
	    /* restore lookahead buf. */
	    lexBuf.len += len;
	    lexBuf.getPtr = curgetptr;
	    return lexStr();
	    }
        else
	    lexAppendc(c);
	}
    lexBuf.len += len;	/* char that has been moved to lookahead buffer */
    lexBuf.getPtr = curgetptr;
    return 0;
    }

#ifdef _SUPPORT_LINE_FOLDING
static void handleMoreRFC822LineBreak(int c) {
    /* suport RFC 822 line break in cases like
     *	ADR: foo;
     *    morefoo;
     *    more foo;
     */
    if (c == ';') {
	int a;
	lexSkipLookahead();
	/* skip white spaces */
	a = lexLookahead();
	while (a == ' ' || a == '\t') {
	    lexSkipLookahead();
	    a = lexLookahead();
	    }
	if (a == '\n') {
	    lexSkipLookahead();
	    a = lexLookahead();
	    if (a == ' ' || a == '\t') {
		/* continuation, throw away all the \n and spaces read so
		 * far
		 */
		lexSkipWhite();
		lexPushLookaheadc(';');
		}
	    else {
		lexPushLookaheadc('\n');
		lexPushLookaheadc(';');
		}
	    }
	else {
	    lexPushLookaheadc(';');
	    }
	}
    }

static char* lexGet1Value() {
    int c;
    lexSkipWhite();
    c = lexLookahead();
    lexClearToken();
    while (c != EOF && c != ';') {
	if (c == '\n') {
	    int a;
	    lexSkipLookahead();
	    a  = lexLookahead();
	    if (a == ' ' || a == '\t') {
		lexAppendc(' ');
		lexSkipLookahead();
		}
	    else {
		lexPushLookaheadc('\n');
		break;
		}
	    }
	else {
	    lexAppendc(c);
	    lexSkipLookahead();
	    }
	c = lexLookahead();
	}
    lexAppendc(0);
    handleMoreRFC822LineBreak(c);
    return c==EOF?0:lexStr();
    }
#endif

char* lexGetStrUntil(char *termset) {
    int c = lexLookahead();
    lexClearToken();
    while (c != EOF && !strchr(termset,c)) {
	lexAppendc(c);
	lexSkipLookahead();
	c = lexLookahead();
	}
    lexAppendc(0);
    return c==EOF?0:lexStr();
    }

static int match_begin_name(int end) {
    char *n = lexLookaheadWord();
    int token = ID;
    if (n) {
	if (!strcasecmp(n,"vcard")) token = end?END_VCARD:BEGIN_VCARD;
	else if (!strcasecmp(n,"vcalendar")) token = end?END_VCAL:BEGIN_VCAL;
	else if (!strcasecmp(n,"vevent")) token = end?END_VEVENT:BEGIN_VEVENT;
	else if (!strcasecmp(n,"vtodo")) token = end?END_VTODO:BEGIN_VTODO;
	deleteStr(n);
	return token;
	}
    return 0;
    }


void initLex(const char *inputstring, unsigned long inputlen, FILE *inputfile)
    {
    /* initialize lex mode stack */
    lexBuf.lexModeStack[lexBuf.lexModeStackTop=0] = L_NORMAL;

    /* iniatialize lex buffer. */
    lexBuf.inputString = (char*) inputstring;
    lexBuf.inputLen = inputlen;
    lexBuf.curPos = 0;
    lexBuf.inputFile = inputfile;

    lexBuf.len = 0;
    lexBuf.getPtr = 0;

    lexBuf.maxToken = MAXTOKEN;
    lexBuf.strs = (char*)malloc(MAXTOKEN);
    lexBuf.strsLen = 0;

    }

static void finiLex() {
    free(lexBuf.strs);
    }


/****************************************************************************/
/* This parses and converts the base64 format for binary encoding into
 * a decoded buffer (allocated with new).  See RFC 1521.
 */
static char * lexGetDataFromBase64()
    {
    unsigned long bytesLen = 0, bytesMax = 0;
    int quadIx = 0, pad = 0;
    unsigned long trip = 0;
    unsigned char b;
    int c;
    unsigned char *bytes = NULL;
    unsigned char *oldBytes = NULL;

    DBG_(("db: lexGetDataFromBase64\n"));
    while (1) {
	c = lexGetc();
	if (c == '\n') {
	    ++mime_lineNum;
	    if (lexLookahead() == '\n') {
		/* a '\n' character by itself means end of data */
		break;
		}
	    else continue; /* ignore '\n' */
	    }
	else {
	    if ((c >= 'A') && (c <= 'Z'))
		b = (unsigned char)(c - 'A');
	    else if ((c >= 'a') && (c <= 'z'))
		b = (unsigned char)(c - 'a') + 26;
	    else if ((c >= '0') && (c <= '9'))
		b = (unsigned char)(c - '0') + 52;
	    else if (c == '+')
		b = 62;
	    else if (c == '/')
		b = 63;
	    else if (c == '=') {
		b = 0;
		pad++;
	    } else if ((c == ' ') || (c == '\t')) {
		continue;
	    } else { /* error condition */
		if (bytes) free(bytes);
		else if (oldBytes) free(oldBytes);
		/* error recovery: skip until 2 adjacent newlines. */
		DBG_(("db: invalid character 0x%x '%c'\n", c,c));
		if (c != EOF)  {
		    c = lexGetc();
		    while (c != EOF) {
			if (c == '\n' && lexLookahead() == '\n') {
			    ++mime_lineNum;
			    break;
			    }
			c = lexGetc();
			}
		    }
		return NULL;
		}
	    trip = (trip << 6) | b;
	    if (++quadIx == 4) {
		unsigned char outBytes[3];
		int numOut;
		int i;
		for (i = 0; i < 3; i++) {
		    outBytes[2-i] = (unsigned char)(trip & 0xFF);
		    trip >>= 8;
		    }
		numOut = 3 - pad;
		if (bytesLen + numOut > bytesMax) {
		    if (!bytes) {
			bytesMax = 1024;
			bytes = (unsigned char*)malloc((size_t)bytesMax);
			}
		    else {
			bytesMax <<= 2;
			oldBytes = bytes;
			bytes = (unsigned char*)realloc(bytes,(size_t)bytesMax);
			}
		    if (bytes == 0) {
			mime_error("out of memory while processing BASE64 data\n");
			}
		    }
		if (bytes) {
		    memcpy(bytes + bytesLen, outBytes, numOut);
		    bytesLen += numOut;
		    }
		trip = 0;
		quadIx = 0;
		}
	    }
	} /* while */
    DBG_(("db: bytesLen = %d\n",  bytesLen));
    /* kludge: all this won't be necessary if we have tree form
	representation */
    if (bytes) {
	setValueWithSize(curProp,bytes,(unsigned int)bytesLen);
	free(bytes);
	}
    else if (oldBytes) {
	setValueWithSize(curProp,oldBytes,(unsigned int)bytesLen);
	free(oldBytes);
	}
    return 0;
    }

static int match_begin_end_name(int end) {
    int token;
    lexSkipWhite();
    if (lexLookahead() != ':') return ID;
    lexSkipLookahead();
    lexSkipWhite();
    token = match_begin_name(end);
    if (token == ID) {
	lexPushLookaheadc(':');
	DBG_(("db: ID '%s'\n", yylval.str));
	return ID;
	}
    else if (token != 0) {
	lexSkipLookaheadWord();
	deleteStr(yylval.str);
	DBG_(("db: begin/end %d\n", token));
	return token;
	}
    return 0;
    }

static char* lexGetQuotedPrintable()
    {
    char cur;

    lexClearToken();
    do {
	cur = lexGetc();
	switch (cur) {
	    case '=': {
		int c = 0;
		int next[2];
		int i;
		for (i = 0; i < 2; i++) {
		    next[i] = lexGetc();
		    if (next[i] >= '0' && next[i] <= '9')
			c = c * 16 + next[i] - '0';
		    else if (next[i] >= 'A' && next[i] <= 'F')
			c = c * 16 + next[i] - 'A' + 10;
		    else
			break;
		    }
		if (i == 0) {
		    /* single '=' follow by LINESEP is continuation sign? */
		    if (next[0] == '\n') {
			++mime_lineNum;
			}
		    else {
			lexPushLookaheadc('=');
			goto EndString;
			}
		    }
		else if (i == 1) {
		    lexPushLookaheadc(next[1]);
		    lexPushLookaheadc(next[0]);
		    lexAppendc('=');
		} else {
		    lexAppendc(c);
		    }
		break;
		} /* '=' */
	    case '\n': {
		lexPushLookaheadc('\n');
		goto EndString;
		}
	    case (char)EOF:
		break;
	    default:
		lexAppendc(cur);
		break;
	    } /* switch */
	} while (cur != (char)EOF);

EndString:
    lexAppendc(0);
    return lexStr();
    } /* LexQuotedPrintable */

static int yylex() {

    int lexmode = LEXMODE();
    if (lexmode == L_VALUES) {
	int c = lexGetc();
	if (c == ';') {
	    DBG_(("db: SEMICOLON\n"));
	    lexPushLookaheadc(c);
	    handleMoreRFC822LineBreak(c);
	    lexSkipLookahead();
	    return SEMICOLON;
	    }
	else if (strchr("\n",c)) {
	    ++mime_lineNum;
	    /* consume all line separator(s) adjacent to each other */
	    c = lexLookahead();
	    while (strchr("\n",c)) {
		lexSkipLookahead();
		c = lexLookahead();
		++mime_lineNum;
		}
	    DBG_(("db: LINESEP\n"));
	    return LINESEP;
	    }
	else {
	    char *p = 0;
	    lexPushLookaheadc(c);
	    if (lexWithinMode(L_BASE64)) {
		/* get each char and convert to bin on the fly... */
		p = lexGetDataFromBase64();
		yylval.str = p;
		return STRING;
		}
	    else if (lexWithinMode(L_QUOTED_PRINTABLE)) {
		p = lexGetQuotedPrintable();
		}
	    else {
#ifdef _SUPPORT_LINE_FOLDING
		p = lexGet1Value();
#else
		p = lexGetStrUntil(";\n");
#endif
		}
	    if (p) {
		DBG_(("db: STRING: '%s'\n", p));
		yylval.str = p;
		return STRING;
		}
	    else return 0;
	    }
	}
    else {
	/* normal mode */
	while (1) {
	    int c = lexGetc();
	    switch(c) {
		case ':': {
		    /* consume all line separator(s) adjacent to each other */
		    /* ignoring linesep immediately after colon. */
		    c = lexLookahead();
		    while (strchr("\n",c)) {
			lexSkipLookahead();
			c = lexLookahead();
			++mime_lineNum;
			}
		    DBG_(("db: COLON\n"));
		    return COLON;
		    }
		case ';':
		    DBG_(("db: SEMICOLON\n"));
		    return SEMICOLON;
		case '=':
		    DBG_(("db: EQ\n"));
		    return EQ;
		/* ignore tabs/newlines in this mode.  We can't ignore
		 * spaces, because values like NEEDS ACTION have a space. */
	        case '\t': continue;
		case '\n': {
		    ++mime_lineNum;
		    continue;
		    }
		case EOF: return 0;
		    break;
		default: {
		    lexPushLookaheadc(c);
		    if (isalpha(c) || c == ' ') {
			char *t = lexGetWord();
			yylval.str = t;
			if (!strcasecmp(t, "begin")) {
			    return match_begin_end_name(0);
			    }
			else if (!strcasecmp(t,"end")) {
			    return match_begin_end_name(1);
			    }
		        else {
			    DBG_(("db: ID '%s'\n", t));
			    return ID;
			    }
			}
		    else {
			/* unknown token */
			return 0;
			}
		    break;
		    }
		}
	    }
	}
    return 0;
    }


/***************************************************************************/
/***	Public Functions						****/
/***************************************************************************/

static VObject* Parse_MIMEHelper()
    {
    ObjStackTop = -1;
    mime_numErrors = 0;
    mime_lineNum = 1;
    vObjList = 0;
    curObj = 0;

    if (yyparse() != 0)
	return 0;

    finiLex();
    return vObjList;
    }

/****************************************************************************/
VObject* Parse_MIME(const char *input, unsigned long len)
    {
    initLex(input, len, 0);
    return Parse_MIMEHelper();
    }


VObject* Parse_MIME_FromFile(FILE *file)
    {
    VObject *result;	
    long startPos;

    initLex(0,(unsigned long)-1,file);
    startPos = ftell(file);
    if (!(result = Parse_MIMEHelper())) {
	fseek(file,startPos,SEEK_SET);
	}
    return result;
    }

VObject* Parse_MIME_FromFileName(const char *fname)
    {
    FILE *fp = fopen(fname,"r");
    if (fp) {
	VObject* o = Parse_MIME_FromFile(fp);
	fclose(fp);
	return o;
	}
    else {
	char msg[255];
	snprintf(msg, sizeof(msg), "can't open file '%s' for reading\n", fname);
	mime_error_(msg);
	return 0;
	}
    }

/****************************************************************************/
void YYDebug(const char *s)
{
	Parse_Debug(s);
}


static MimeErrorHandler mimeErrorHandler;

void registerMimeErrorHandler(MimeErrorHandler me)
    {
    mimeErrorHandler = me;
    }

static void mime_error(char *s)
    {
    char msg[256];
    if (mimeErrorHandler) {
	snprintf(msg, sizeof(msg), "%s at line %d", s, mime_lineNum);
	mimeErrorHandler(msg);
	}
    }

static void mime_error_(char *s)
    {
    if (mimeErrorHandler) {
	mimeErrorHandler(s);
	}
    }



