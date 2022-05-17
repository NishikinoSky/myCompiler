%{
    #include <stdio.h>
    #include "ast.h"
    int yylex(void);
    void yyerror(char *);
    extern int mistakeNum;
    extern astNode* astRoot;
%}

%union
{
    class astNode* asTree;
}
/*terminals*/
%token <asTree> ELSE
%token <asTree> IF
%token <asTree> RETURN
%token <asTree> WHILE
%token <asTree> BREAK
%token <asTree> TYPE
%token <asTree> BOOL

%token <asTree> INT
%token <asTree> FLOAT
%token <asTree> CHAR
%token <asTree> STR
%token <asTree> ID
%token <asTree> COMMA
%token <asTree> SEMICOLON

%token <asTree> PLUS
%token <asTree> MINUS
%token <asTree> MULTI
%token <asTree> DIV
%token <asTree> RELOP
%token <asTree> ASSIGN
%token <asTree> LOGIC
%token <asTree> NOT
%token <asTree> LPT
%token <asTree> RPT
%token <asTree> LSB
%token <asTree> RSB
%token <asTree> LCB
%token <asTree> RCB

%type <asTree> program
%type <asTree> decList
%type <asTree> dec
%type <asTree> varDeclaration
%type <asTree> funcDeclaration
%type <asTree> typeSpecifier
%type <asTree> varDecList
%type <asTree> varDef
%type <asTree> funcDec
%type <asTree> compoundStmt
%type <asTree> paramList
%type <asTree> paramDec
%type <asTree> paramDef
%type <asTree> content
%type <asTree> localDec
%type <asTree> stmtList
%type <asTree> stmt
%type <asTree> expStmt
%type <asTree> selecStmt
%type <asTree> iterStmt
%type <asTree> retStmt
%type <asTree> exp
%type <asTree> dbOper
%type <asTree> sgOper
%type <asTree> Array
%type <asTree> funcCall
%type <asTree> argList
%type <asTree> sgFactor

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%left PLUS MINUS MULTI DIV
%left RELOP ASSIGN LOGIC
%left LPT RPT LSB RSB
%right NOT

%%
program:
    decList {
        $$ = new astNode("", "program", 1, $1);
        astRoot = $$;
    };

decList:
    dec decList {
        $$ = new astNode("", "decList", 2, $1, $2);
    }
    | %empty {
        $$ = NULL;
    };

dec:
    varDeclaration {
        $$ = new astNode("", "dec", 1, $1);
    }
    | funcDeclaration {
        $$ = new astNode("", "dec", 1, $1);
    };

varDeclaration:
    typeSpecifier varDecList SEMICOLON {
        $$ = new astNode("", "varDeclaration", 3, $1, $2, $3);
    };

typeSpecifier:
    TYPE {
         $$ = new astNode("", "typeSpecifier", 1, $1);
    };

varDecList:
    varDef {
        $$ = new astNode("", "varDecList", 1, $1);
    }
    | varDef COMMA varDecList {
        $$ = new astNode("", "varDef", 3, $1, $2, $3);
    };

varDef:
    ID {
        $$ = new astNode("", "varDef", 1, $1);
    }
    | ID LSB INT RSB {
        $$ = new astNode("", "varDef", 4, $1, $2, $3, $4);
    };

funcDeclaration:
    typeSpecifier funcDec compoundStmt {
        $$ = new astNode("", "funcDeclaration", 3, $1, $2, $3);
    };

funcDec:
    ID LPT paramList RPT {
        $$ = new astNode("", "funcDec", 4, $1, $2, $3, $4);
    }
    | ID LPT RPT {
        $$ = new astNode("", "funcDec", 3, $1, $2, $3);
    };

paramList:
    paramDec COMMA paramList {
        $$ = new astNode("", "paramList", 3, $1, $2, $3);
    }
    | paramDec {
        $$ = new astNode("", "paramList", 1, $1);
    };

paramDec:
    typeSpecifier paramDec {
        $$ = new astNode("", "paramDec", 2, $1, $2);
    };

paramDef:
    ID {
        $$ = new astNode("", "paramDef", 1, $1);
    }
    | ID LSB RSB {
        $$ = new astNode("", "paramDef", 3, $1, $2, $3);
    };

compoundStmt:
    LCB content RCB {
        $$ = new astNode("", "compoundStmt", 3, $1, $2, $3);
    };

content:
    localDec stmtList {
        $$ = new astNode("", "content", 2, $1, $2);
    }
    | %empty {
        $$ = NULL;
    };

localDec:
    varDeclaration localDec {
        $$ = new astNode("", "localDec", 2, $1, $2);
    }
    | %empty {
        $$ = NULL;
    };

stmtList:
    stmt stmtList {
        $$ = new astNode("", "stmtList", 2, $1, $2);
    }
    | %empty {
        $$ = NULL;
    };

stmt:
    expStmt {
        $$ = new astNode("", "stmt", 1, $1);
    }
    | compoundStmt {
        $$ = new astNode("", "stmt", 1, $1);
    }
    | selecStmt {
        $$ = new astNode("", "stmt", 1, $1);
    }
    | iterStmt {
        $$ = new astNode("", "stmt", 1, $1);
    }
    | retStmt {
        $$ = new astNode("", "stmt", 1, $1);
    };

selecStmt:
    IF LPT exp RPT stmt %prec LOWER_THAN_ELSE{
        $$ = new astNode("", "selecStmt", 5, $1, $2, $3, $4, $5);
    }
    | IF LPT exp RPT stmt ELSE stmt {
        $$ = new astNode("", "selecStmt", 7, $1, $2, $3, $4, $5, $6, $7);
    };

iterStmt:
    WHILE LPT exp RPT stmt {
        $$ = new astNode("", "iterStmt", 5, $1, $2, $3, $4, $5);
    };

retStmt:
    RETURN exp SEMICOLON {
        $$ = new astNode("", "retStmt", 3, $1, $2, $3);
    }
    | RETURN SEMICOLON {
        $$ = new astNode("", "retStmt", 2, $1, $2);
    }
    | BREAK SEMICOLON {
        $$ = new astNode("", "retStmt", 2, $1, $2);
    };

expStmt:
    exp SEMICOLON {
        $$ = new astNode("", "expStmt", 2, $1, $2);
    }
    | SEMICOLON {
        $$ = new astNode("", "expStmt", 1, $1);
    }

exp:
    exp dbOper exp {
        $$ = new astNode("", "exp", 3, $1, $2, $3);
    }
    | sgOper exp {
        $$ = new astNode("", "exp", 2, $1, $2);
    }
    | LPT exp RPT {
        $$ = new astNode("", "exp", 3, $1, $2, $3);
    }
    | ID {
        $$ = new astNode("", "exp", 1, $1);
    }
    | ID Array {
        $$ = new astNode("", "exp", 2, $1, $2);
    }
    | ID funcCall {
        $$ = new astNode("", "exp", 2, $1, $2);
    }
    | sgFactor {
        $$ = new astNode("", "exp", 1, $1);
    };

dbOper:
    PLUS {
        $$ = new astNode("", "dbOper", 1, $1);
    }
    | MINUS {
        $$ = new astNode("", "dbOper", 1, $1);
    }
    | MULTI {
        $$ = new astNode("", "dbOper", 1, $1);
    }
    | DIV {
        $$ = new astNode("", "dbOper", 1, $1);
    }
    | RELOP {
        $$ = new astNode("", "dbOper", 1, $1);
    }
    | ASSIGN {
        $$ = new astNode("", "dbOper", 1, $1);
    }
    | LOGIC {
        $$ = new astNode("", "dbOper", 1, $1);
    }

sgOper:
    MINUS {
        $$ = new astNode("", "sgOper", 1, $1);
    }
    | NOT {
        $$ = new astNode("", "sgOper", 1, $1);
    }
    | PLUS {
        $$ = new astNode("", "sgOper", 1, $1);
    };

Array:
    LSB exp RSB {
        $$ = new astNode("", "Array", 3, $1, $2, $3);
    }
    | LSB RSB {
        $$ = new astNode("", "Array", 2, $1, $2);
    };

funcCall:
    LPT argList RPT {
        $$ = new astNode("", "funcCall", 3, $1, $2, $3);
    }
    | LPT RPT {
        $$ = new astNode("", "funcCall", 2, $1, $2);
    };

argList:
    exp COMMA argList {
        $$ = new astNode("", "argList", 3, $1, $2, $3);
    }
    | exp {
        $$ = new astNode("", "argList", 1, $1);
    };

sgFactor:
    INT {
        $$ = new astNode("", "sgFactor", 1, $1);
    }
    | FLOAT {
        $$ = new astNode("", "sgFactor", 1, $1);
    }
    | CHAR {
        $$ = new astNode("", "sgFactor", 1, $1);
    }
    | BOOL {
        $$ = new astNode("", "sgFactor", 1, $1);
    }
    | STR {
        $$ = new astNode("", "sgFactor", 1, $1);
    };

%%
void yyerror(char *str)
{
    fprintf(stderr,"error: %s\n",str);
}

int yywrap()
{
    return 1;
}