 #include <stdio.h>
#include <stdlib.h>

/* =========================================================
   TOKEN DEFINITIONS
   These MUST match 1.l exactly
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


/* =======================================================>
   LEXICAL ANALYZER
   =======================================================>

extern int yylex(void);


/* =======================================================>
   GLOBAL VARIABLES
   =======================================================>

int lookahead;
FILE *derivation;


/* =======================================================>
   FUNCTION PROTOTYPES
   =======================================================>

void program(void);
void classDecl(void);
void visibility(void);
void memberDecl(void);
void funcDecl(void);
void funcHead(void);
void funcDef(void);
void funcBody(void);
void varDecl(void);

void statement(void);
void assignStat(void);
void statBlock(void);

void expr(void);
void exprTail(void);
void relExpr(void);

void arithExpr(void);
void arithExprTail(void);
void term(void);
void termTail(void);
void factor(void);

void variable(void);
void functionCall(void);
void idnest(void);
void indice(void);

void arraySize(void);
void type(void);

void fParams(void);
void fParamsTail(void);

void aParams(void);
void aParamsTail(void);

void assignOp(void);
void relOp(void);
void addOp(void);
void multOp(void);
void sign(void);
/* =======================================================>
   GET NEXT TOKEN
   =======================================================>

void nextToken(void)
{
    lookahead = yylex();
}


/* =======================================================>
   MATCH
   =======================================================>

void match(int expected)
{
    if (lookahead == expected)
    {
        nextToken();
    }
    else
    {
        printf("\nSyntax Error!\n");
        printf("Expected token: %d\n", expected);
        printf("Found token: %d\n", lookahead);

        fprintf(derivation,
                "Syntax Error: expected token %d, found to>
                expected, lookahead);

        exit(1);
    }
}


/* =======================================================>
   PROGRAM

   prog → {{ classDecl }} {{ funcDef }} main funcBody
   =======================================================>

void program(void)
{
    fprintf(derivation,
            "prog -> classDecl* funcDef* MAIN funcBody\n");

    /* Zero or more class declarations */
    while (lookahead == CLASS)
    {
        fprintf(derivation,
                "prog -> classDecl prog\n");

        classDecl();
    }

    /* Zero or more function definitions */
    while (lookahead == ID)
    {
        fprintf(derivation,
                "prog -> funcDef prog\n");

        funcDef();
    }

    /* main */
    fprintf(derivation,
            "prog -> main funcBody\n");

    match(MAIN);

    funcBody();
}


/* =======================================================>
   CLASS DECLARATION

   classDecl →
       class id
       [[ inherits id {{ , id }} ]]
       { {{ visibility memberDecl }} } ;
   =======================================================>

void classDecl(void)
{
    fprintf(derivation,
            "classDecl -> class id inherits { visibility m>

    match(CLASS);

    match(ID);

    /* Optional inherits */
    if (lookahead == INHERITS)
    {
        match(INHERITS);

        match(ID);

        while (lookahead == COMMA)
        {
            match(COMMA);
            match(ID);
        }
    }

    match(LBRACE);

    while (lookahead == PUBLIC ||
           lookahead == PRIVATE)
    {
        visibility();

        while (lookahead == ID ||
               lookahead == INTEGER ||
               lookahead == FLOAT)
        {
            memberDecl();
        }
    }

    match(RBRACE);

    match(SEMICOLON);
}

/* =======================================================>
   VISIBILITY

   visibility → public | private
   =======================================================>

void visibility(void)
{
    if (lookahead == PUBLIC)
    {
        fprintf(derivation,
                "visibility -> public\n");

        match(PUBLIC);
    }
    else if (lookahead == PRIVATE)
    {
        fprintf(derivation,
                "visibility -> private\n");

        match(PRIVATE);
    }
    else
    {
        printf("Syntax Error: expected public/private\n");
        exit(1);
    }
}


/* =======================================================>
   MEMBER DECLARATION
   memberDecl → funcDecl | varDecl
   =======================================================>

void memberDecl(void)
{
    if (lookahead == INTEGER ||
        lookahead == FLOAT)
    {
        fprintf(derivation,
                "memberDecl -> varDecl\n");

        varDecl();
    }
    else if (lookahead == ID)
    {
        fprintf(derivation,
                "memberDecl -> funcDecl\n");

        funcDecl();
    }
    else
    {
        printf("Syntax Error in member declaration\n");
        exit(1);
    }
}


/* =======================================================>
   FUNCTION DECLARATION

   funcDecl →
       id ( fParams ) : type ;
       |
       id ( fParams ) : void ;
   =======================================================>

void funcDecl(void)
{
    fprintf(derivation,
            "funcDecl -> id ( fParams ) : type ;\n");

    match(ID);

    match(LPAREN);

    fParams();

    match(RPAREN);

    match(COLON);

    if (lookahead == VOID)
    {
        match(VOID);
    }
    else
    {
        type();
    }

    match(SEMICOLON);

  GNU nano 7.2              parse.c
}


/* =======================================================>
   FUNCTION HEADER

   funcHead →
       id ( fParams ) : type ;
       |
       id ( fParams ) : void ;
   =======================================================>

void funcHead(void)
{
    fprintf(derivation,
            "funcHead -> id ( fParams ) : type ;\n");

    match(ID);

    match(LPAREN);

    fParams();

    match(RPAREN);

    match(COLON);

    if (lookahead == VOID)
    {
        match(VOID);
    }
    else
    {
        type();
    }

    match(SEMICOLON);
}


/* =======================================================>
   FUNCTION DEFINITION

   funcDef → funcHead funcBody
   =======================================================>

void funcDef(void)
{
    fprintf(derivation,
            "funcDef -> funcHead funcBody\n");

    funcHead();

    funcBody();
}


/* =======================================================>
   FUNCTION BODY

   funcBody →
       [[ local {{ varDecl }} ]]
       do {{ statement }}
       end
   =======================================================>

void funcBody(void)
{
    fprintf(derivation,
            "funcBody -> local varDecl* do statement* end\>

    /* Optional local */
    if (lookahead == LOCAL)
    {
        match(LOCAL);

        while (lookahead == INTEGER ||
               lookahead == FLOAT)
        {
            varDecl();
        }
    }

    match(DO);

    while (lookahead == ID ||
           lookahead == IF ||
           lookahead == WHILE ||
           lookahead == READ ||
           lookahead == WRITE ||
           lookahead == RETURN)
    {
        statement();
    }

    match(END);


}


/* =======================================================>
   VARIABLE DECLARATION

   varDecl → type id : {{ arraySize }} ;
   =======================================================>

void varDecl(void)
{
    fprintf(derivation,
            "varDecl -> type id : arraySize* ;\n");

    type();

    match(ID);

    match(COLON);

    while (lookahead == LBRACKET)
    {
        arraySize();
    }

    match(SEMICOLON);
}


/* =======================================================>
   STATEMENT
   =======================================================>


void statement(void)
{
    if (lookahead == IF)
    {
        fprintf(derivation,
                "statement -> if ( relExpr ) then statBloc>

        match(IF);

        match(LPAREN);

        relExpr();

        match(RPAREN);

        match(THEN);

        statBlock();

        match(ELSE);

        statBlock();

        match(SEMICOLON);
    }

    else if (lookahead == WHILE)
    {
        fprintf(derivation,
                "statement -> while ( relExpr ) statBlock >


        match(WHILE);

        match(LPAREN);

        relExpr();

        match(RPAREN);

        statBlock();

        match(SEMICOLON);
    }

    else if (lookahead == READ)
    {
        fprintf(derivation,
                "statement -> read ( variable ) ;\n");

        match(READ);

        match(LPAREN);

        variable();

        match(RPAREN);

        match(SEMICOLON);
    }

    else if (lookahead == WRITE)
    {

        fprintf(derivation,
                "statement -> write ( expr ) ;\n");

        match(WRITE);

        match(LPAREN);

        expr();

        match(RPAREN);

        match(SEMICOLON);
    }

    else if (lookahead == RETURN)
    {
        fprintf(derivation,
                "statement -> return ( expr ) ;\n");

        match(RETURN);

        match(LPAREN);

        expr();

        match(RPAREN);

        match(SEMICOLON);
    }

    else if (lookahead == ID)
    {

        /*
        ID can begin either assignment or function call.
        */

        fprintf(derivation,
                "statement -> assignStat ;\n");

        assignStat();

        match(SEMICOLON);
    }

    else
    {
        printf("Syntax Error in statement\n");
        exit(1);
    }
}


/* =======================================================>
   ASSIGNMENT

   assignStat → variable assignOp expr
   =======================================================>

void assignStat(void)
{
    fprintf(derivation,
            "assignStat -> variable assignOp expr\n");


    variable();

    assignOp();

    expr();
}


/* =======================================================>
   STATEMENT BLOCK

   statBlock →
       do {{ statement }} end
       |
       statement
       |
       ε
   =======================================================>

void statBlock(void)
{
    if (lookahead == DO)
    {
        fprintf(derivation,
                "statBlock -> do statement* end\n");

        match(DO);

        while (lookahead == ID ||
               lookahead == IF ||
               lookahead == WHILE ||

               lookahead == READ ||
               lookahead == WRITE ||
               lookahead == RETURN)
        {
            statement();
        }

        match(END);
    }

    else if (lookahead == ID ||
             lookahead == IF ||
             lookahead == WHILE ||
             lookahead == READ ||
             lookahead == WRITE ||
             lookahead == RETURN)
    {
        fprintf(derivation,
                "statBlock -> statement\n");

        statement();
    }

    else
    {
        fprintf(derivation,
                "statBlock -> epsilon\n");
    }
}

/* =======================================================>
   EXPRESSION

   LL(1) transformed:

   expr → arithExpr exprTail
   exprTail → relOp arithExpr | ε
   =======================================================>

void expr(void)
{
    fprintf(derivation,
            "expr -> arithExpr exprTail\n");

    arithExpr();

    exprTail();
}


/* =======================================================>
   EXPRESSION TAIL
   =======================================================>

void exprTail(void)
{
    if (lookahead == EQ ||
        lookahead == NEQ ||
        lookahead == LT ||
        lookahead == GT ||
        lookahead == LE ||
        lookahead == GE)

    {
        fprintf(derivation,
                "exprTail -> relOp arithExpr\n");

        relOp();

        arithExpr();
    }
    else
    {
        fprintf(derivation,
                "exprTail -> epsilon\n");
    }
}


/* =======================================================>
   RELATIONAL EXPRESSION

   relExpr → arithExpr relOp arithExpr
   =======================================================>

void relExpr(void)
{
    fprintf(derivation,
            "relExpr -> arithExpr relOp arithExpr\n");

    arithExpr();

    relOp();

    arithExpr();


}


/* =======================================================>
   ARITHMETIC EXPRESSION

   arithExpr → term arithExprTail
   =======================================================>

void arithExpr(void)
{
    fprintf(derivation,
            "arithExpr -> term arithExprTail\n");

    term();

    arithExprTail();
}


/* =======================================================>
   ARITHMETIC EXPRESSION TAIL

   arithExprTail →
       addOp term arithExprTail
       |
       ε
   =======================================================>

void arithExprTail(void)
{
    if (lookahead == PLUS ||
        lookahead == MINUS ||
        lookahead == OR)
    {
        fprintf(derivation,
                "arithExprTail -> addOp term arithExprTail>

        addOp();

        term();

        arithExprTail();
    }
    else
    {
        fprintf(derivation,
                "arithExprTail -> epsilon\n");
    }
}


/* =======================================================>
   TERM

   term → factor termTail
   =======================================================>

void term(void)
{
    fprintf(derivation,
            "term -> factor termTail\n");

    factor();

    termTail();
}


/* =======================================================>
   TERM TAIL

   termTail →
       multOp factor termTail
       |
       ε
   =======================================================>

void termTail(void)
{
    if (lookahead == MULT ||
        lookahead == DIV ||
        lookahead == AND)
    {
        fprintf(derivation,
                "termTail -> multOp factor termTail\n");

        multOp();

        factor();

        termTail();
    }
    else
    {
        fprintf(derivation,
                "termTail -> epsilon\n");



    }
}


/* =======================================================>
   FACTOR
   =======================================================>

void factor(void)
{
    if (lookahead == ID)
    {
        fprintf(derivation,
                "factor -> variable\n");

        variable();
    }

    else if (lookahead == INTNUM)
    {
        fprintf(derivation,
                "factor -> intNum\n");

        match(INTNUM);
    }

    else if (lookahead == FLOATNUM)
    {
        fprintf(derivation,
                "factor -> floatNum\n");

        match(FLOATNUM);


    }

    else if (lookahead == LPAREN)
    {
        fprintf(derivation,
                "factor -> ( arithExpr )\n");

        match(LPAREN);

        arithExpr();

        match(RPAREN);
    }

    else if (lookahead == NOT)
    {
        fprintf(derivation,
                "factor -> not factor\n");

        match(NOT);

        factor();
    }

    else if (lookahead == PLUS ||
             lookahead == MINUS)
    {
        fprintf(derivation,
                "factor -> sign factor\n");

        sign();


        factor();
    }

    else
    {
        printf("Syntax Error in factor\n");
        exit(1);
    }
}


/* =======================================================>
   VARIABLE

   variable → {{ idnest }} id {{ indice }}
   =======================================================>

void variable(void)
{
    fprintf(derivation,
            "variable -> idnest* id indice*\n");

    match(ID);

    while (lookahead == LBRACKET)
    {
        indice();
    }
}

/* =======================================================>
   FUNCTION CALL

   functionCall → {{ idnest }} id ( aParams )
   =======================================================>

void functionCall(void)
{
    fprintf(derivation,
            "functionCall -> id ( aParams )\n");

    match(ID);

    while (lookahead == DOT)
    {
        match(DOT);

        match(ID);
    }

    match(LPAREN);

    aParams();

    match(RPAREN);
}


/* =======================================================>
   ID NEST

   idnest → id {{ indice }} .
   =======================================================>

void idnest(void)
{
    match(ID);

    while (lookahead == LBRACKET)
    {
        indice();
    }

    match(DOT);
}


/* =======================================================>
   INDEX

   indice → [ arithExpr ]
   =======================================================>

void indice(void)
{
    fprintf(derivation,
            "indice -> [ arithExpr ]\n");

    match(LBRACKET);

    arithExpr();

    match(RBRACKET);

}


/* =======================================================>
   ARRAY SIZE

   arraySize → [ intNum ] | [ ]
   =======================================================>

void arraySize(void)
{
    fprintf(derivation,
            "arraySize -> [ intNum ] | [ ]\n");

    match(LBRACKET);

    if (lookahead == INTNUM)
    {
        match(INTNUM);
    }

    match(RBRACKET);
}


/* =======================================================>
   TYPE

   type → integer | float | id
   =======================================================>

void type(void)
{
    if (lookahead == INTEGER)
    {
        fprintf(derivation,
                "type -> integer\n");

        match(INTEGER);
    }

    else if (lookahead == FLOAT)
    {
        fprintf(derivation,
                "type -> float\n");

        match(FLOAT);
    }

    else if (lookahead == ID)
    {
        fprintf(derivation,
                "type -> id\n");

        match(ID);
    }

    else
    {
        printf("Syntax Error: expected type\n");
        exit(1);
    }
}

/* =======================================================>
   FORMAL PARAMETERS

   fParams →
       type id arraySize* fParamsTail
       |
       ε
   =======================================================>

void fParams(void)
{
    if (lookahead == INTEGER ||
        lookahead == FLOAT ||
        lookahead == ID)
    {
        fprintf(derivation,
                "fParams -> type id arraySize* fParamsTail>

        type();

        match(ID);

        while (lookahead == LBRACKET)
        {
            arraySize();
        }

        fParamsTail();
    }
    else
    {

        fprintf(derivation,
                "fParams -> epsilon\n");
    }
}


/* =======================================================>
   FORMAL PARAMETER TAIL

   fParamsTail → , type id arraySize
   =======================================================>

void fParamsTail(void)
{
    while (lookahead == COMMA)
    {
        match(COMMA);

        type();

        match(ID);

        while (lookahead == LBRACKET)
        {
            arraySize();
        }
    }
}


/* =======================================================>
   ACTUAL PARAMETERS
   aParams → expr aParamsTail | ε
   =======================================================>

void aParams(void)
{
    if (lookahead == ID ||
        lookahead == INTNUM ||
        lookahead == FLOATNUM ||
        lookahead == LPAREN ||
        lookahead == NOT ||
        lookahead == PLUS ||
        lookahead == MINUS)
    {
        fprintf(derivation,
                "aParams -> expr aParamsTail\n");

        expr();

        aParamsTail();
    }
    else
    {
        fprintf(derivation,
                "aParams -> epsilon\n");
    }
}


/* =======================================================>
   ACTUAL PARAMETER TAIL

   aParamsTail → , expr
   =======================================================>

void aParamsTail(void)
{
    while (lookahead == COMMA)
    {
        match(COMMA);

        expr();
    }
}


/* =======================================================>
   ASSIGNMENT OPERATOR

   assignOp → =
   =======================================================>

void assignOp(void)
{
    match(ASSIGN);
}


/* =======================================================>
   RELATIONAL OPERATOR

   relOp → == | <> | < | > | <= | >=
   =======================================================>

void relOp(void)
{
    if (lookahead == EQ)
    {
        match(EQ);
    }

    else if (lookahead == NEQ)
    {
        match(NEQ);
    }

    else if (lookahead == LT)
    {
        match(LT);
    }

    else if (lookahead == GT)
    {
        match(GT);
    }

    else if (lookahead == LE)
    {
        match(LE);
    }

    else if (lookahead == GE)
    {
        match(GE);
    }

 else
    {
        printf("Syntax Error: relational operator expected>
        exit(1);
    }
}


/* =======================================================>
   ADD OPERATOR

   addOp → + | - | or
   =======================================================>

void addOp(void)
{
    if (lookahead == PLUS)
    {
        match(PLUS);
    }

    else if (lookahead == MINUS)
    {
        match(MINUS);
    }

    else if (lookahead == OR)
    {
        match(OR);
    }

    else
    {
        printf("Syntax Error: add operator expected\n");
        exit(1);
    }
}


/* =======================================================>
   MULT OPERATOR

   multOp → * | / | and
   =======================================================>

void multOp(void)
{
    if (lookahead == MULT)
    {
        match(MULT);
    }

    else if (lookahead == DIV)
    {
        match(DIV);
    }

    else if (lookahead == AND)
    {
        match(AND);
    }

    else
    {
        printf("Syntax Error: multiplication operator expe>
        exit(1);
    }
}


/* =======================================================>
   SIGN

   sign → + | -
   =======================================================>

void sign(void)
{
    if (lookahead == PLUS)
    {
        match(PLUS);
    }

    else if (lookahead == MINUS)
    {
        match(MINUS);
    }

    else
    {
        printf("Syntax Error: sign expected\n");
        exit(1);
    }

}


/* =======================================================>
   MAIN
   =======================================================>

int main(void)
{
    printf("=====================================\n");
    printf("       SYNTAX ANALYZER\n");
    printf("=====================================\n");

    derivation = fopen("derivation.txt", "w");

    if (derivation == NULL)
    {
        printf("Error: cannot create derivation.txt\n");
        return 1;
    }

    /*
    Get first token from Lexical Analyzer
    */
    nextToken();

    /*
    Start parsing from start symbol
    */
    program();

    /*



 /*
    0 means end of input
    */
    if (lookahead == 0)
    {
        printf("\nSyntax analysis successful!\n");

        fprintf(derivation,
                "\n=====================================\n>
        fprintf(derivation,
                "Syntax analysis successful!\n");
        fprintf(derivation,
                "=====================================\n");
    }
    else
    {
        printf("\nSyntax Error: extra input found.\n");

        fprintf(derivation,
                "\nSyntax Error: extra input found.\n");
    }

    fclose(derivation);

    return 0;
}







