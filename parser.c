#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokens.h"

static FILE *derivFile;
static int   errorCount = 0;

typedef struct { int type; char lexeme[128]; int line; } TokRec;
static TokRec *toks = NULL;
static int nToks = 0, tokCap = 0, tp = 0;

static void loadTokens(void) {
    tokCap = 256; nToks = 0; tp = 0;
    toks = malloc(sizeof(TokRec) * tokCap);
    for (;;) {
        int t = yylex();
        if (nToks >= tokCap) { tokCap *= 2; toks = realloc(toks, sizeof(TokRec) * tokCap); }
        toks[nToks].type = t;
        strncpy(toks[nToks].lexeme, yytext, sizeof(toks[nToks].lexeme) - 1);
        toks[nToks].lexeme[sizeof(toks[nToks].lexeme) - 1] = '\0';
        toks[nToks].line = current_line;
        nToks++;
        if (t == TK_EOF) break;
    }
}

#define lookahead (toks[tp].type)
static int peek(int k) { int i = tp + k; return (i < nToks) ? toks[i].type : TK_EOF; }
static int mark(void) { return tp; }
static void reset(int m) { tp = m; }

static const char *tokenName(int t) {
    switch (t) {
        case TK_EOF: return "EOF";
        case TK_CLASS: return "class"; case TK_INHERITS: return "inherits";
        case TK_PUBLIC: return "public"; case TK_PRIVATE: return "private";
        case TK_INTEGER_KW: return "integer"; case TK_FLOAT_KW: return "float";
        case TK_VOID_KW: return "void"; case TK_LOCAL: return "local";
        case TK_DO: return "do"; case TK_END: return "end"; case TK_IF: return "if";
        case TK_THEN: return "then"; case TK_ELSE: return "else"; case TK_WHILE: return "while";
        case TK_READ: return "read"; case TK_WRITE: return "write"; case TK_RETURN: return "return";
        case TK_MAIN: return "main"; case TK_NOT: return "not"; case TK_AND: return "and";
        case TK_OR: return "or"; case TK_ID: return "id"; case TK_INTNUM: return "intNum";
        case TK_FLOATNUM: return "floatNum"; case TK_ASSIGN: return "=";
        case TK_EQ: return "=="; case TK_NE: return "<>"; case TK_LT: return "<";
        case TK_GT: return ">"; case TK_LE: return "<="; case TK_GE: return ">=";
        case TK_PLUS: return "+"; case TK_MINUS: return "-"; case TK_STAR: return "*";
        case TK_SLASH: return "/"; case TK_LPAREN: return "("; case TK_RPAREN: return ")";
        case TK_LBRACE: return "{"; case TK_RBRACE: return "}"; case TK_LBRACKET: return "[";
        case TK_RBRACKET: return "]"; case TK_SEMI: return ";"; case TK_COMMA: return ",";
        case TK_COLON: return ":"; case TK_DOT: return "."; case TK_SR: return "::";
        default: return "?";
    }
}

static void advance(void) { if (tp < nToks - 1) tp++; }

static void skipBalanced(int openTok, int closeTok) {
    int depth = 1;
    while (depth > 0 && lookahead != TK_EOF) {
        if (lookahead == openTok) { depth++; advance(); }
        else if (lookahead == closeTok) { depth--; if (depth > 0) advance(); }
        else advance();
    }
}

static void trace(const char *nt, const char *prod) {
    fprintf(derivFile, "%s -> %s\n", nt, prod);
}

static int inSet(int tok, const int *set) {
    for (int i = 0; set[i] != -1; i++) if (set[i] == tok) return 1;
    return 0;
}

static void syntaxError(const char *nt, const char *expected) {
    int line = toks[tp].line;
    fprintf(stderr, "[SYNTAX ERROR] line %d: in %s, unexpected token '%s' (expected %s)\n",
            line, nt, tokenName(lookahead), expected);
    fprintf(derivFile, "** SYNTAX ERROR line %d in %s: unexpected '%s', expected %s **\n",
            line, nt, tokenName(lookahead), expected);
    errorCount++;
}

static void recover(const char *nt, const int *firstSet, const int *followSet) {
    while (lookahead != TK_EOF && !inSet(lookahead, firstSet) && !inSet(lookahead, followSet)) {
        advance();
    }
}

static void match(int expected) {
    if (lookahead == expected) {
        advance();
    } else {
        syntaxError("match", tokenName(expected));
        advance();
    }
}

static void parseClassDeclList(void); static void parseFuncDefList(void);
static void parseClassDecl(void); static void parseInheritOpt(void);
static void parseIdTailList(void); static void parseMemberList(void);
static void parseVisibility(void); static void parseMemberDecl(void);
static void parseFuncDecl(void); static void parseFuncHead(void); static void parseFuncHeadTail(void);
static void parseFuncDef(void); static void parseFuncBody(void);
static void parseLocalOpt(void); static void parseVarDeclList(void);
static void parseStmtList(void); static void parseVarDecl(void);
static void parseArraySizeList(void); static void parseArraySize(void);
static void parseStatement(void); static void parseAssignOrCallStat(void);
static void parseAssignOrCallTail(void); static void parseStatBlock(void);
static void parseExpr(void); static void parseExprTail(void); static void parseRelExpr(void);
static void parseArithExpr(void); static void parseArithExprPrime(void);
static void parseSign(void); static void parseTerm(void); static void parseTermPrime(void);
static void parseFactor(void); static void parseVariableOrCall(void); static void parseCallOrIndex(void);
static void parseVariable(void); static void parseIdnestList(void); static void parseIdnest(void);
static void parseIdnestTail(void); static void parseIndice(void); static void parseIndiceList(void);
static void parseType(void); static void parseFParams(void); static void parseFParamsTailList(void);
static void parseFParamsTail(void); static void parseAParams(void); static void parseAParamsTailList(void);
static void parseAParamsTail(void); static void parseAssignOp(void); static void parseRelOp(void);
static void parseAddOp(void); static void parseMultOp(void);

static const int FIRST_statement[]  = {TK_ID, TK_IF, TK_WHILE, TK_READ, TK_WRITE, TK_RETURN, -1};
static const int FOLLOW_statement[] = {TK_ID, TK_IF, TK_WHILE, TK_READ, TK_WRITE, TK_RETURN,
                                        TK_END, TK_ELSE, TK_SEMI, -1};
static const int FIRST_memberDecl[] = {TK_ID, TK_INTEGER_KW, TK_FLOAT_KW, -1};
static const int FOLLOW_memberDecl[]= {TK_PUBLIC, TK_PRIVATE, TK_RBRACE, -1};
static const int FIRST_varDecl[]    = {TK_INTEGER_KW, TK_FLOAT_KW, TK_ID, -1};
static const int FOLLOW_varDecl[]   = {TK_INTEGER_KW, TK_FLOAT_KW, TK_ID, TK_DO,
                                        TK_PUBLIC, TK_PRIVATE, TK_RBRACE, -1};

void parseProg(void) {
    trace("prog", "classDeclList funcDefList main funcBody");
    parseClassDeclList();
    parseFuncDefList();
    match(TK_MAIN);
    parseFuncBody();
    if (lookahead != TK_EOF) {
        syntaxError("prog", "end of file");
    }
}

static void parseClassDeclList(void) {
    if (lookahead == TK_CLASS) {
        trace("classDeclList", "classDecl classDeclList");
        parseClassDecl();
        parseClassDeclList();
    } else {
        trace("classDeclList", "epsilon");
    }
}

static void parseFuncDefList(void) {
    if (lookahead == TK_ID) {
        trace("funcDefList", "funcDef funcDefList");
        parseFuncDef();
        parseFuncDefList();
    } else {
        trace("funcDefList", "epsilon");
    }
}

static void parseClassDecl(void) {
    trace("classDecl", "class id inheritOpt { memberList } ;");
    match(TK_CLASS); match(TK_ID);
    parseInheritOpt();
    match(TK_LBRACE);
    parseMemberList();
    match(TK_RBRACE);
    match(TK_SEMI);
}

static void parseInheritOpt(void) {
    if (lookahead == TK_INHERITS) {
        trace("inheritOpt", "inherits id idTailList");
        match(TK_INHERITS); match(TK_ID);
        parseIdTailList();
    } else {
        trace("inheritOpt", "epsilon");
    }
}

static void parseIdTailList(void) {
    if (lookahead == TK_COMMA) {
        trace("idTailList", ", id idTailList");
        match(TK_COMMA); match(TK_ID);
        parseIdTailList();
    } else {
        trace("idTailList", "epsilon");
    }
}

static void parseMemberList(void) {
    if (lookahead == TK_PUBLIC || lookahead == TK_PRIVATE) {
        trace("memberList", "visibility memberDecl memberList");
        parseVisibility();
        parseMemberDecl();
        parseMemberList();
    } else {
        trace("memberList", "epsilon");
    }
}

static void parseVisibility(void) {
    if (lookahead == TK_PUBLIC) { trace("visibility", "public"); match(TK_PUBLIC); }
    else if (lookahead == TK_PRIVATE) { trace("visibility", "private"); match(TK_PRIVATE); }
    else { syntaxError("visibility", "public or private"); recover("visibility", FIRST_memberDecl, FOLLOW_memberDecl); }
}

static void parseMemberDecl(void) {
    if (lookahead == TK_INTEGER_KW || lookahead == TK_FLOAT_KW) {
        trace("memberDecl", "varDecl");
        parseVarDecl();
    } else if (lookahead == TK_ID) {
        if (peek(1) == TK_LPAREN) {
            trace("memberDecl", "funcDecl");
            parseFuncDecl();
        } else if (peek(1) == TK_ID) {
            trace("memberDecl", "varDecl");
            parseVarDecl();
        } else {
            syntaxError("memberDecl", "'(' after id (funcDecl), or a class-typed field name (varDecl)");
            recover("memberDecl", FIRST_memberDecl, FOLLOW_memberDecl);
        }
    } else {
        syntaxError("memberDecl", "id, integer, or float");
        recover("memberDecl", FIRST_memberDecl, FOLLOW_memberDecl);
    }
}

static void parseFuncDecl(void) {
    trace("funcDecl", "id ( fParams ) : type ; | id ( fParams ) : void ;");
    match(TK_ID); match(TK_LPAREN);
    parseFParams();
    match(TK_RPAREN); match(TK_COLON);
    if (lookahead == TK_VOID_KW) match(TK_VOID_KW); else parseType();
    match(TK_SEMI);
}

static void parseFuncHead(void) {
    trace("funcHead", "id funcHeadTail");
    match(TK_ID);
    parseFuncHeadTail();
}

static void parseFuncHeadTail(void) {
    if (lookahead == TK_SR) {
        trace("funcHeadTail", ":: id ( fParams ) : type|void ;");
        match(TK_SR); match(TK_ID); match(TK_LPAREN);
        parseFParams();
        match(TK_RPAREN); match(TK_COLON);
        if (lookahead == TK_VOID_KW) match(TK_VOID_KW); else parseType();
        match(TK_SEMI);
    } else if (lookahead == TK_LPAREN) {
        trace("funcHeadTail", "( fParams ) : type|void ;");
        match(TK_LPAREN);
        parseFParams();
        match(TK_RPAREN); match(TK_COLON);
        if (lookahead == TK_VOID_KW) match(TK_VOID_KW); else parseType();
        match(TK_SEMI);
    } else {
        syntaxError("funcHeadTail", "'::' or '('");
    }
}

static void parseFuncDef(void) {
    trace("funcDef", "funcHead funcBody");
    parseFuncHead();
    parseFuncBody();
}

static void parseFuncBody(void) {
    trace("funcBody", "localOpt do stmtList end");
    parseLocalOpt();
    match(TK_DO);
    parseStmtList();
    match(TK_END);
}

static void parseLocalOpt(void) {
    if (lookahead == TK_LOCAL) {
        trace("localOpt", "local varDeclList");
        match(TK_LOCAL);
        parseVarDeclList();
    } else {
        trace("localOpt", "epsilon");
    }
}

static void parseVarDeclList(void) {
    if (lookahead == TK_INTEGER_KW || lookahead == TK_FLOAT_KW || lookahead == TK_ID) {
        trace("varDeclList", "varDecl varDeclList");
        parseVarDecl();
        parseVarDeclList();
    } else {
        trace("varDeclList", "epsilon");
    }
}

static void parseStmtList(void) {
    if (lookahead == TK_ID || lookahead == TK_IF || lookahead == TK_WHILE ||
        lookahead == TK_READ || lookahead == TK_WRITE || lookahead == TK_RETURN) {
        trace("stmtList", "statement stmtList");
        parseStatement();
        parseStmtList();
    } else {
        trace("stmtList", "epsilon");
    }
}

static void parseVarDecl(void) {
    trace("varDecl", "type id : arraySizeList ;");
    parseType();
    match(TK_ID); match(TK_COLON);
    parseArraySizeList();
    match(TK_SEMI);
}

static void parseArraySizeList(void) {
    if (lookahead == TK_LBRACKET) {
        trace("arraySizeList", "arraySize arraySizeList");
        parseArraySize();
        parseArraySizeList();
    } else {
        trace("arraySizeList", "epsilon");
    }
}

static void parseArraySize(void) {
    trace("arraySize", "[ intNum ] | [ ]");
    match(TK_LBRACKET);
    if (lookahead == TK_INTNUM) match(TK_INTNUM);
    match(TK_RBRACKET);
}

static void parseStatement(void) {
    if (lookahead == TK_ID) {
        trace("statement", "assignOrCallStat ;");
        parseAssignOrCallStat();
        match(TK_SEMI);
    } else if (lookahead == TK_IF) {
        trace("statement", "if ( relExpr ) then statBlock else statBlock ;");
        match(TK_IF); match(TK_LPAREN);
        parseRelExpr();
        match(TK_RPAREN); match(TK_THEN);
        parseStatBlock();
        match(TK_ELSE);
        parseStatBlock();
        match(TK_SEMI);
    } else if (lookahead == TK_WHILE) {
        trace("statement", "while ( relExpr ) statBlock ;");
        match(TK_WHILE); match(TK_LPAREN);
        parseRelExpr();
        match(TK_RPAREN);
        parseStatBlock();
        match(TK_SEMI);
    } else if (lookahead == TK_READ) {
        trace("statement", "read ( variable ) ;");
        match(TK_READ); match(TK_LPAREN);
        parseVariable();
        match(TK_RPAREN); match(TK_SEMI);
    } else if (lookahead == TK_WRITE) {
        trace("statement", "write ( expr ) ;");
        match(TK_WRITE); match(TK_LPAREN);
        parseExpr();
        match(TK_RPAREN); match(TK_SEMI);
    } else if (lookahead == TK_RETURN) {
        trace("statement", "return ( expr ) ;");
        match(TK_RETURN); match(TK_LPAREN);
        parseExpr();
        match(TK_RPAREN); match(TK_SEMI);
    } else {
        syntaxError("statement", "id, if, while, read, write, or return");
        recover("statement", FIRST_statement, FOLLOW_statement);
    }
}

static void parseAssignOrCallStat(void) {
    trace("assignOrCallStat", "idnestList id assignOrCallTail");
    parseIdnestList();
    match(TK_ID);
    parseAssignOrCallTail();
}

static void parseAssignOrCallTail(void) {
    if (lookahead == TK_LPAREN) {
        trace("assignOrCallTail", "( aParams )");
        match(TK_LPAREN);
        parseAParams();
        match(TK_RPAREN);
    } else if (lookahead == TK_LBRACKET || lookahead == TK_ASSIGN) {
        trace("assignOrCallTail", "indiceList assignOp expr");
        parseIndiceList();
        parseAssignOp();
        parseExpr();
    } else {
        syntaxError("assignOrCallTail", "'(', '[', or '='");
    }
}

static void parseStatBlock(void) {
    if (lookahead == TK_DO) {
        trace("statBlock", "do stmtList end");
        match(TK_DO);
        parseStmtList();
        match(TK_END);
    } else if (lookahead == TK_ID || lookahead == TK_IF || lookahead == TK_WHILE ||
               lookahead == TK_READ || lookahead == TK_WRITE || lookahead == TK_RETURN) {
        trace("statBlock", "statement");
        parseStatement();
    } else {
        trace("statBlock", "epsilon");
    }
}

static void parseExpr(void) {
    trace("expr", "arithExpr exprTail");
    parseArithExpr();
    parseExprTail();
}

static void parseExprTail(void) {
    if (lookahead == TK_EQ || lookahead == TK_NE || lookahead == TK_LT ||
        lookahead == TK_GT || lookahead == TK_LE || lookahead == TK_GE) {
        trace("exprTail", "relOp arithExpr");
        parseRelOp();
        parseArithExpr();
    } else {
        trace("exprTail", "epsilon");
    }
}

static void parseRelExpr(void) {
    trace("relExpr", "arithExpr relOp arithExpr");
    parseArithExpr();
    parseRelOp();
    parseArithExpr();
}

static void parseArithExpr(void) {
    trace("arithExpr", "term arithExpr'");
    parseTerm();
    parseArithExprPrime();
}

static void parseArithExprPrime(void) {
    if (lookahead == TK_PLUS || lookahead == TK_MINUS || lookahead == TK_OR) {
        trace("arithExpr'", "addOp term arithExpr'");
        parseAddOp();
        parseTerm();
        parseArithExprPrime();
    } else {
        trace("arithExpr'", "epsilon");
    }
}

static void parseSign(void) {
    if (lookahead == TK_PLUS) { trace("sign", "+"); match(TK_PLUS); }
    else { trace("sign", "-"); match(TK_MINUS); }
}

static void parseTerm(void) {
    trace("term", "factor term'");
    parseFactor();
    parseTermPrime();
}

static void parseTermPrime(void) {
    if (lookahead == TK_STAR || lookahead == TK_SLASH || lookahead == TK_AND) {
        trace("term'", "multOp factor term'");
        parseMultOp();
        parseFactor();
        parseTermPrime();
    } else {
        trace("term'", "epsilon");
    }
}

static void parseFactor(void) {
    if (lookahead == TK_ID) {
        trace("factor", "variableOrCall");
        parseVariableOrCall();
    } else if (lookahead == TK_INTNUM) {
        trace("factor", "intNum"); match(TK_INTNUM);
    } else if (lookahead == TK_FLOATNUM) {
        trace("factor", "floatNum"); match(TK_FLOATNUM);
    } else if (lookahead == TK_LPAREN) {
        trace("factor", "( arithExpr )");
        match(TK_LPAREN); parseArithExpr(); match(TK_RPAREN);
    } else if (lookahead == TK_NOT) {
        trace("factor", "not factor");
        match(TK_NOT); parseFactor();
    } else if (lookahead == TK_PLUS || lookahead == TK_MINUS) {
        trace("factor", "sign factor");
        parseSign(); parseFactor();
    } else {
        syntaxError("factor", "id, intNum, floatNum, '(', not, '+', or '-'");
    }
}

static void parseVariableOrCall(void) {
    trace("variableOrCall", "idnestList id callOrIndex");
    parseIdnestList();
    match(TK_ID);
    parseCallOrIndex();
}

static void parseCallOrIndex(void) {
    if (lookahead == TK_LPAREN) {
        trace("callOrIndex", "( aParams )");
        match(TK_LPAREN); parseAParams(); match(TK_RPAREN);
    } else {
        trace("callOrIndex", "indiceList");
        parseIndiceList();
    }
}

static void parseVariable(void) {
    trace("variable", "idnestList id indiceList");
    parseIdnestList();
    match(TK_ID);
    parseIndiceList();
}

static void parseIdnestList(void) {
    if (lookahead != TK_ID) { trace("idnestList", "epsilon"); return; }

    int save = mark();
    advance();
    if (lookahead == TK_LPAREN) {
        advance();
        skipBalanced(TK_LPAREN, TK_RPAREN);
        if (lookahead == TK_RPAREN) advance();
    } else {
        while (lookahead == TK_LBRACKET) {
            advance();
            skipBalanced(TK_LBRACKET, TK_RBRACKET);
            if (lookahead == TK_RBRACKET) advance();
        }
    }
    int isNest = (lookahead == TK_DOT);
    reset(save);

    if (isNest) {
        trace("idnestList", "idnest idnestList");
        parseIdnest();
        parseIdnestList();
    } else {
        trace("idnestList", "epsilon");
    }
}

static void parseIdnest(void) {
    trace("idnest", "id idnestTail");
    match(TK_ID);
    parseIdnestTail();
}

static void parseIdnestTail(void) {
    if (lookahead == TK_LPAREN) {
        trace("idnestTail", "( aParams ) .");
        match(TK_LPAREN); parseAParams(); match(TK_RPAREN); match(TK_DOT);
    } else {
        trace("idnestTail", "indiceList .");
        parseIndiceList();
        match(TK_DOT);
    }
}

static void parseIndice(void) {
    trace("indice", "[ arithExpr ]");
    match(TK_LBRACKET); parseArithExpr(); match(TK_RBRACKET);
}

static void parseIndiceList(void) {
    if (lookahead == TK_LBRACKET) {
        trace("indiceList", "indice indiceList");
        parseIndice();
        parseIndiceList();
    } else {
        trace("indiceList", "epsilon");
    }
}

static void parseType(void) {
    if (lookahead == TK_INTEGER_KW) { trace("type", "integer"); match(TK_INTEGER_KW); }
    else if (lookahead == TK_FLOAT_KW) { trace("type", "float"); match(TK_FLOAT_KW); }
    else if (lookahead == TK_ID) { trace("type", "id"); match(TK_ID); }
    else { syntaxError("type", "integer, float, or id"); }
}

static void parseFParams(void) {
    if (lookahead == TK_INTEGER_KW || lookahead == TK_FLOAT_KW || lookahead == TK_ID) {
        trace("fParams", "type id arraySizeList fParamsTailList");
        parseType(); match(TK_ID);
        parseArraySizeList();
        parseFParamsTailList();
    } else {
        trace("fParams", "epsilon");
    }
}

static void parseFParamsTailList(void) {
    if (lookahead == TK_COMMA) {
        trace("fParamsTailList", "fParamsTail fParamsTailList");
        parseFParamsTail();
        parseFParamsTailList();
    } else {
        trace("fParamsTailList", "epsilon");
    }
}

static void parseFParamsTail(void) {
    trace("fParamsTail", ", type id arraySizeList");
    match(TK_COMMA); parseType(); match(TK_ID);
    parseArraySizeList();
}

static void parseAParams(void) {
    if (lookahead == TK_ID || lookahead == TK_INTNUM || lookahead == TK_FLOATNUM ||
        lookahead == TK_LPAREN || lookahead == TK_NOT || lookahead == TK_PLUS || lookahead == TK_MINUS) {
        trace("aParams", "expr aParamsTailList");
        parseExpr();
        parseAParamsTailList();
    } else {
        trace("aParams", "epsilon");
    }
}

static void parseAParamsTailList(void) {
    if (lookahead == TK_COMMA) {
        trace("aParamsTailList", "aParamsTail aParamsTailList");
        parseAParamsTail();
        parseAParamsTailList();
    } else {
        trace("aParamsTailList", "epsilon");
    }
}

static void parseAParamsTail(void) {
    trace("aParamsTail", ", expr");
    match(TK_COMMA); parseExpr();
}

static void parseAssignOp(void) { trace("assignOp", "="); match(TK_ASSIGN); }

static void parseRelOp(void) {
    trace("relOp", tokenName(lookahead));
    switch (lookahead) {
        case TK_EQ: case TK_NE: case TK_LT: case TK_GT: case TK_LE: case TK_GE:
            advance(); break;
        default:
            syntaxError("relOp", "==, <>, <, >, <=, or >=");
    }
}

static void parseAddOp(void) {
    trace("addOp", tokenName(lookahead));
    switch (lookahead) {
        case TK_PLUS: case TK_MINUS: case TK_OR: advance(); break;
        default: syntaxError("addOp", "+, -, or or");
    }
}

static void parseMultOp(void) {
    trace("multOp", tokenName(lookahead));
    switch (lookahead) {
        case TK_STAR: case TK_SLASH: case TK_AND: advance(); break;
        default: syntaxError("multOp", "*, /, or and");
    }
}

int runParser(const char *derivationPath) {
    loadTokens();
    derivFile = fopen(derivationPath, "w");
    if (!derivFile) { fprintf(stderr, "cannot open %s for writing\n", derivationPath); return -1; }
    parseProg();
    fclose(derivFile);
    free(toks);
    return errorCount;
}
