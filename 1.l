%{
#include <stdio.h>
#include <stdlib.h>

/* =========================================================
   TOKEN DEFINITIONS
   ========================================================= */

#define ID          256
#define INTNUM      257
#define FLOATNUM    258

/* Reserved words */
#define IF          259
#define THEN        260
#define ELSE        261
#define WHILE       262
#define CLASS       263
#define INHERITS    264
#define PUBLIC      265
#define PRIVATE     266
#define LOCAL       267
#define INTEGER     268
#define FLOAT       269
#define VOID        270
#define NOT         271
#define OR          272
#define AND         273
#define WRITE       274
#define READ        275
#define MAIN        276
#define RETURN      277
#define DO          278
#define END         279

/* Operators */
#define PLUS        280
#define MINUS       281
#define LT          282
#define GT          283
#define LE          284
#define GE          285
#define ASSIGN      286
#define EQ          287
#define NEQ         288
#define MULT        289
#define DIV         290

/* Punctuation */
#define LPAREN      291
#define RPAREN      292
#define LBRACE      293
#define RBRACE      294
#define LBRACKET    295
#define RBRACKET    296
#define SEMICOLON   297
#define COMMA       298
#define COLON       299
#define DOT         300

int line = 1;

%}

/* =========================================================
   LEXICAL ELEMENT DEFINITIONS
   ========================================================= */

LETTER          [a-zA-Z]
DIGIT           [0-9]
NONZERO         [1-9]
ALPHANUM        [a-zA-Z0-9_]

IDPATTERN       {LETTER}{ALPHANUM}*
INTPATTERN      (0|{NONZERO}{DIGIT}*)

FRACTION        (\.{DIGIT}*{NONZERO}|\.[0])
FLOATPATTERN    {INTPATTERN}{FRACTION}([eE][+-]?{INTPATTERN})?

%x COMMENT

%%

    /* =========================================================
       RESERVED WORDS
       ========================================================= */

"if"            { return IF; }
"then"          { return THEN; }
"else"          { return ELSE; }
"while"         { return WHILE; }

"class"         { return CLASS; }
"inherits"      { return INHERITS; }

"public"        { return PUBLIC; }
"private"       { return PRIVATE; }
"local"         { return LOCAL; }

"integer"       { return INTEGER; }
"float"         { return FLOAT; }
"void"          { return VOID; }

"not"           { return NOT; }

"or"            { return OR; }
"and"           { return AND; }

"write"         { return WRITE; }
"read"          { return READ; }

"main"          { return MAIN; }
"return"        { return RETURN; }

"do"            { return DO; }
"end"           { return END; }


    /* =========================================================
       COMMENTS
       ========================================================= */

    /* Inline comment */
"//".*          { }

    /* Start block comment */
"/*"            { BEGIN(COMMENT); }

    /* Inside block comment */
<COMMENT>"*/"   { BEGIN(INITIAL); }

<COMMENT>\n     { line++; }
<COMMENT>.      { }


    /* =========================================================
       FLOATING POINT
       ========================================================= */

{FLOATPATTERN}  {
                    printf("FLOATNUM: %s\n", yytext);
                    return FLOATNUM;
                }


    /* =========================================================
       INTEGER
       ========================================================= */

{INTPATTERN}    {
                    printf("INTNUM: %s\n", yytext);
                    return INTNUM;
                }


    /* =========================================================

       IDENTIFIER
       ========================================================= */

{IDPATTERN}     {
                    printf("ID: %s\n", yytext);
                    return ID;
                }


    /* =========================================================
       OPERATORS
       ========================================================= */

    /* Two-character operators first */

"=="            { return EQ; }
"<>"            { return NEQ; }
"<="            { return LE; }
">="            { return GE; }

    /* One-character operators */

"<"             { return LT; }
">"             { return GT; }
"="             { return ASSIGN; }

"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return MULT; }
"/"             { return DIV; }


    /* =========================================================
       PUNCTUATION
       ========================================================= */

"("             { return LPAREN; }
")"             { return RPAREN; }

"{"             { return LBRACE; }
"}"             { return RBRACE; }

"["             { return LBRACKET; }
"]"             { return RBRACKET; }

";"             { return SEMICOLON; }
","             { return COMMA; }
":"             { return COLON; }
"."             { return DOT; }


 /* =========================================================
       WHITESPACE
       ========================================================= */

[ \t\r]+        { }

\n              { line++; }


    /* =========================================================
       LEXICAL ERROR
       ========================================================= */

. {
      printf("Lexical Error at line %d: invalid character '%s'\n",
             line, yytext);
  }

%%

/* =========================================================
   C CODE
   ========================================================= */

int yywrap(void)
{

    return 1;
}
